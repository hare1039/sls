#pragma once
#ifndef INDEXSTORAGE_HPP__
#define INDEXSTORAGE_HPP__

#include "storage.hpp"
#include "inode.hpp"
#include "freelist.hpp"

namespace indexstorage
{

struct info
{
    base::freelist<1024 * 8> inode_used_;
    base::freelist<1024 * 8> block_used_;
};

class indexstorage
{
    std::unique_ptr<base::storage_interface> indexstorage_ptr_;
    inode::inode_ptr const inode_start_ptr_ = 0;
    inode::inode_ptr const inode_end_ptr_ = 1024 * 8;
    inode::inode_ptr const root_ptr_ = 1;
    info info_;

    static
    auto basedir(std::string filename) -> std::string
    {
        std::size_t const slash_start = filename.find_first_of('/');
        std::size_t const slash_end = filename.find_last_of('/') - slash_start;
        std::string directory_path = filename.substr(slash_start, slash_end);
        if (directory_path.empty())
            return directory_path;
        else
            return directory_path.substr(1);
    }

    static
    auto basename(std::string filename) -> std::string
    {
        std::size_t const slash_end = filename.find_last_of('/');
        std::string directory_path = filename.substr(slash_end);
        return directory_path.substr(1);
    }

public:
    indexstorage(std::unique_ptr<base::storage_interface> && p):
        indexstorage_ptr_{std::move(p)}
    {
        indexstorage_ptr_->connect();

        base::buf b = indexstorage_ptr_->read_block(inode_start_ptr_);
        info_ = base::from_buf<info>(b);
    }

    ~indexstorage()
    {
        save_info();
    }


public: // userapi
    void format()
    {
        info_.inode_used_.set_base(inode_start_ptr_);
        info_.inode_used_.set(inode_start_ptr_, true);
        info_.inode_used_.reset();
        info_.block_used_.set_base(inode_end_ptr_+1);
        info_.block_used_.reset();

        inode::inode newroot;
        newroot.set_attr(9, true);
        info_.inode_used_.set(root_ptr_, true);
        save_inode(root_ptr_, newroot);

        touch_from_root("/aaaa.txt");
        mkdir_from_root("/directoy");
        touch_from_root("/directoy/dddd.txt");
    }

    auto list_from_root(std::string const& filename) -> std::string
    {
        inode::inode_ptr const p = find(filename);
        std::cerr << "list_from_root inode::inode_ptr=" << p << "\n";

        std::stringstream ss;
        inode::inode idir = dereference(p);

        idir.directory_stream(ss);
        return ss.str();
    }


    // expect full path name e.g. /etc/config/test.txt
    void touch_from_root(std::string const& filename, inode::inode::attr_type a = 0)
    {
        std::cerr << "create file '" << filename << "'\n";

        inode::inode_ptr const p = resolve_path(root_ptr_, basedir(filename));
        if (p == inode::null_inode_ptr)
        {
            std::cerr << "resolve failed\n";
            return;
        }

        std::cerr << "touch_from_root inode::inode_ptr=" << p << "\n";

        create_file(p, basename(filename), a);
    }

    inline
    void mkdir_from_root(std::string const& dirname)
    {
        inode::inode::attr_type a;
        a.set(9, true);
        touch_from_root(dirname, a);
    }

    auto find(std::string const& filename) -> inode::inode_ptr
    {
        std::cerr << "resolving: " << filename << "\n";
        inode::inode_ptr p = resolve_path(root_ptr_, filename.substr(1));
        std::cerr << "found ptr=" << p << "\n";
        return p;
    }

    void write_file(std::string const& filename, base::buf const &write_buf, base::storage_interface *datastorage)
    {
        inode::inode_ptr const i_ptr = find(filename);
        inode::inode fileinode = dereference(i_ptr);
        std::cerr << "fileinode [" << i_ptr << "]=" << fileinode << "\n";

        int const blocksize = datastorage->current_state().optimal_size;

        auto begin_iter = write_buf.begin(), end_iter = write_buf.end();
        std::size_t blockindex = 0;
        for (; blockindex * blocksize < write_buf.size(); blockindex++)
        {
            std::cerr << "blockindex " << blockindex << "\n";
            std::advance(begin_iter, blockindex * blocksize);
            std::uint32_t single_block_size = std::min<std::uint32_t>(std::distance(begin_iter, end_iter), blocksize);
            base::buf single_block_buf (single_block_size);

            std::copy_n(begin_iter, single_block_size, single_block_buf.begin());

            inode::block_ptr place = getfree_block();
            info_.block_used_.set(place, true);
            std::cerr << "freeblock " << place << "\n";
            inode::data_block b {
                .vol_id = 0,
                .offset = place,
                .size = single_block_size,
            };
            fileinode.set(blockindex, b);
            datastorage->write_block(place, single_block_buf);
        }
        for (; blockindex < fileinode.blocks.size(); blockindex++)
            fileinode.set(blockindex, inode::empty_block{});

        save_inode(i_ptr, fileinode);
    }

    auto readall(std::string const& filename) -> base::buf
    {
        inode::inode_ptr ptr = resolve_path(root_ptr_, filename.substr(1));
        if (ptr == inode::null_inode_ptr)
        {
            std::cerr << "ptr not found\n";
            return {};
        }

        std::cerr << "readall ptr=" << ptr << "\n";

        inode::inode target = dereference(ptr);

        std::cerr << "deref=" << target << "\n";
        base::buf acc;
        int leftblocks = target.blocksize();

        std::cerr << "left=" << leftblocks << "\n";
        for (inode::inode::block_type& vb : target.blocks)
        {
            if (leftblocks-- <= 0)
                break;
            inode::data_block bl = std::get<inode::data_block>(vb);
            base::buf b = indexstorage_ptr_->read_block(bl.offset);
            std::cerr << "read=" << bl.offset << " size=" << bl.size << "\n";
            b.resize(bl.size);
            acc.insert(acc.end(), b.begin(), b.end());
        }

        std::cerr << "acc=" << acc.size() << "\n";
        return acc;
    }

public:
    void create_file(inode::inode_ptr const directory,
                     std::string const& filename,
                     inode::inode::attr_type attr)
    {
        inode::inode idir = dereference(directory);

        std::cerr << "directory=" << idir << "\n";

        // already exist file
        if (idir.directory_find(filename) != inode::null_inode_ptr)
            return;

        inode::directory_block db;
        db.block_id = getfree_inode();
        std::cerr << "newinode: " << db.block_id << "\n";
        std::copy_n(filename.begin(), db.filename.size(), db.filename.begin());

        idir.set(idir.blocksize(), db);

        inode::inode newinode;
        newinode.attr = attr;

        info_.inode_used_.set(db.block_id, true);
        save_inode(directory, idir);
        save_inode(db.block_id, newinode);
    }

    void unlink(inode::block_ptr const ptr) {
        info_.block_used_.set(ptr, false);
    }

    void save_info() {
        indexstorage_ptr_->write_block(inode_start_ptr_, base::to_buf(info_));
    }

    auto getfree_inode() -> inode::inode_ptr
    {
        for (inode::inode_ptr i = info_.inode_used_.nbegin() + 2; /* 0 = freelist, 1 = root inode*/
             i < info_.inode_used_.nend(); i++)
            if (not info_.inode_used_.test(i))
                return i;
        return inode::null_inode_ptr;
    }

    auto getfree_block() -> inode::block_ptr
    {
        for (inode::block_ptr i = info_.block_used_.nbegin();
             i < info_.block_used_.nend(); i++)
            if (not info_.block_used_.test(i))
                return i;
        return inode::null_block_ptr;
    }

    auto dereference(inode::inode_ptr const ptr) -> inode::inode {
        return read_inode_ptr(ptr, indexstorage_ptr_.get());
    }

    void save_inode(inode::inode_ptr const ptr, inode::inode const& inode)
    {
        write_inode_ptr(ptr, indexstorage_ptr_.get(), inode);
        save_info();
    }

    // filename never start with /
    auto resolve_path(inode::inode_ptr directory, std::string const& filename) -> inode::inode_ptr
    {
        std::cerr << "resolve directory=" << directory << " with name: '" << filename << "'\n";
        if (filename.empty())
            return directory;

        std::string::size_type pos = filename.find_first_of("/");

        std::string const token = filename.substr(0, pos);

        std::cerr << "find from dir='" << directory << "'; "
                  << "token='" << token << "'; \n";

        inode::inode dirinode = dereference(directory);
        std::cerr << "dir: " << dirinode << "\n";
        if (not dirinode.is_directory())
            return inode::null_inode_ptr;

        inode::inode_ptr ptr = dirinode.directory_find(token);
        if (ptr == inode::null_inode_ptr)
            return inode::null_inode_ptr;

        if (pos == std::string::npos)
            return ptr;

        return resolve_path(ptr, filename.substr(pos+1));


//        std::istringstream iss(filename);
//        std::string token;
//        std::getline(iss, token, '/');
//
//        if (token == "") //
//            return directory;
//
//        else if ()
//        for (std::string token; std::getline(iss, token, '/');)
//        {
//            std::cerr << "token: '" << token << "' in dir=" << directory << "\n";
//            inode::inode i = dereference(directory);
//            if (i.is_directory())
//            {
//                inode::inode_ptr ptr = i.directory_find(token);
//                if (ptr == inode::null_inode_ptr)
//                    return inode::null_inode_ptr;
//                directory = ptr;
//            }
//        }
//        return directory;
    }

public:
    static
    auto read_inode_ptr(inode::inode_ptr const ptr, base::storage_interface *interface) -> inode::inode {
        std::cerr << "read inode storage ptr=" << ptr << "\n";
        return base::from_buf<inode::inode>(interface->read_block(ptr));
    }

    static
    void write_inode_ptr(inode::inode_ptr const ptr,
                         base::storage_interface *interface,
                         inode::inode const& i) {
        std::cerr << "write to inode storage ptr=" << ptr << "\n";
        interface->write_block(ptr, base::to_buf(i));
    }
};

} // namespace indexstorage

#endif // INDEXSTORAGE_HPP__
