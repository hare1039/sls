#pragma once

#ifndef SLSFS_STORAGE_HPP__
#define SLSFS_STORAGE_HPP__

#include "basetypes.hpp"

namespace slsfs::storage
{

class interface
{
public:
    virtual ~interface() {}
    virtual void connect() {};

    // block interface [int] -> buf
//    virtual auto read_block(std::uint32_t const offset) -> base::buf { return {}; };
//    virtual void write_block(std::uint32_t const offset, base::buf const& buffer) {};

    // key interface   [str] -> buf
    virtual auto read_key(std::string const name, std::size_t partition, std::size_t location, std::size_t size) -> base::buf { return {}; };
    virtual void write_key(std::string const name, std::size_t partition, base::buf const& buffer, std::size_t location, std::uint32_t version) {};
    virtual bool check_version_ok(std::string const name, std::size_t partition, std::uint32_t& version) { return true; };

    // list interface  [str] -> buf
    virtual void append_list_key(std::string const name, base::buf const& buffer) {};
    virtual void merge_list_key(std::string const name, std::function<void(std::vector<base::buf> const&)> reduce) {};
    virtual auto get_list_key(std::string const name) -> base::buf { return {}; };
};

}


#endif // SLSFS_STORAGE_HPP__
