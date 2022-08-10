#pragma once

#ifndef SLSFS_STORAGE_HPP__
#define SLSFS_STORAGE_HPP__

#include "basetypes.hpp"

namespace slsfs::storage
{

struct state
{
    int optimal_size;
    int priority;
    int max_size;
};

class interface
{
protected:
    state state_;
public:
    virtual ~interface() {}
    virtual void connect() {};

    // block interface [int] -> buf
    virtual auto read_block(std::uint32_t const offset) -> base::buf { return {}; };
    virtual void write_block(std::uint32_t const offset, base::buf const& buffer) {};

    // key interface   [str] -> buf
    virtual auto read_key(std::string const name) -> base::buf { return {}; };
    virtual void write_key(std::string const name, base::buf const& buffer) {};

    // list interface  [str] -> buf
    virtual void append_list_key(std::string const name, base::buf const& buffer) {};
    virtual void merge_list_key(std::string const name, std::function<void(std::vector<base::buf> const&)> reduce) {};
    virtual auto get_list_key(std::string const name) -> base::buf { return {}; };

    virtual void create_volume(std::uint32_t const size, std::string const& name) {};
    auto current_state() -> state& { return state_; }
};

}


#endif // SLSFS_STORAGE_HPP__
