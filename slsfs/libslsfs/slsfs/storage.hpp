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

    //data operations, key interface   [str] -> buf


    virtual auto read_key (pack::key_t const& name, std::size_t partition, std::size_t location, std::size_t size) -> base::buf { return {}; };

    // update operation
    virtual void write_key(pack::key_t const& name, std::size_t partition, base::buf const& buffer, std::size_t location, std::uint32_t version) {};


    virtual bool check_version_ok(pack::key_t const& name, std::size_t partition, std::uint32_t& version) { return true; };

    // Metatdata operations, list interface  [str] -> buf

    // Create file operation
    virtual void append_list_key(pack::key_t const& name, base::buf const& buffer) {};

    // 
    virtual void merge_list_key (pack::key_t const& name, std::function<void(std::vector<base::buf> const&)> reduce) {};
    virtual auto get_list_key   (pack::key_t const& name) -> base::buf { return {}; };
};

}


#endif // SLSFS_STORAGE_HPP__
