#pragma once
#ifndef INODE_HPP__
#define INODE_HPP__

#include "inode-def.hpp"

#include <cstdint>
#include <array>
#include <bitset>
#include <variant>
#include <algorithm>
#include <iostream>

namespace inode
{

using empty_block = std::monostate;

struct data_block
{
    std::uint32_t vol_id;
    std::uint32_t offset;
    std::uint32_t size;
};

auto operator<< (std::ostream& os, data_block const& db) -> std::ostream&
{
    os << "volid:" << db.vol_id << "|block #" << db.offset << "|size: " << db.size;
    return os;
}

struct directory_block
{
    block_ptr block_id;
    std::array<char, sizeof(data_block) - sizeof(block_id)> filename;
};

auto operator<< (std::ostream& os, directory_block const& db) -> std::ostream&
{
    os << "filename=";
    for (char c : db.filename)
        os << c;
    os << "; block #" << db.block_id;
    return os;
}

static_assert(sizeof(data_block) == sizeof(directory_block));

struct inode
{
    /* attr index [0..15]
        0, 1, 2 = user  rwx
        3, 4, 5 = group rwx
        6, 7, 8 = other rwx
        9 = is directory
        other bits are reserved now
    */
    using attr_type = std::bitset<16>;
    attr_type attr = 0;

    using block_type = std::variant<empty_block, data_block, directory_block>;
    std::array<block_type, block_list_size> blocks{};

    inline bool is_directory() { return attr.test(9); }
    inline void set_attr(std::size_t pos, bool value) { attr.set(pos, value); }

    auto directory_stream(std::ostream &os) -> std::ostream&
    {
        for (block_type& vb : blocks)
            if (directory_block *db = std::get_if<directory_block>(&vb))
                os << (*db) << " || ";

        return os;
    }

    auto directory_find(std::string const& childname) -> inode_ptr
    {
        std::cerr << "directory_find target: '" << childname << "'\n";
        for (block_type& vb : blocks)
        {
            directory_block * db = std::get_if<directory_block>(&vb);
            if (db)
            {
                std::cerr << "block_type: " << *db << "\n";
                if (std::equal(childname.begin(), childname.end(), db->filename.begin()))
                {
                    std::cerr << "found filename\n";
                    return db->block_id;
                }
            }
        }
        return null_inode_ptr;
    }

    void set(std::size_t index, block_type v) {
        blocks.at(index) = v;
    }

    auto blocksize() const -> std::size_t
    {
        std::size_t counter = 0;
        for (block_type const & v: blocks)
        {
            if (std::get_if<empty_block>(&v))
                return counter;
            counter++;
        }
        return counter;
    }
};

auto operator<< (std::ostream& os, inode const& i) -> std::ostream&
{
    os << "attr=" << i.attr << " | size=" << i.blocksize();
    return os;
}

} // namespace inode

#endif // INODE_HPP__
