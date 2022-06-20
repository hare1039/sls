#pragma once

#ifndef SLSFS_STORAGE_HPP__
#define SLSFS_STORAGE_HPP__

#include "basetypes.hpp"

namespace slsfs::base
{

struct state
{
    int optimal_size;
    int priority;
    int max_size;
};

class storage_interface
{
protected:
    state state_;
public:
    virtual ~storage_interface() {}
    virtual void connect() = 0;

    // block interface [int] -> buf
    virtual buf  read_block(std::uint32_t const offset) = 0;
    virtual void write_block(std::uint32_t const offset, buf const& buffer) = 0;

    // key interface   [str] -> buf
    virtual buf  read_key(std::string const name) = 0;
    virtual void write_key(std::string const name, buf const& buffer) = 0;

    // list interface  [str] -> buf
    virtual void append_list_key(std::string const name, buf const& buffer) = 0;
    virtual void merge_list_key(std::string const name, std::function<void(std::vector<buf> const&)> reduce) = 0;
    virtual buf  get_list_key(std::string const name) = 0;

    virtual void create_volume(std::uint32_t const size, std::string const& name) = 0;
    auto current_state() -> state& { return state_; }
};

}


#endif // SLSFS_STORAGE_HPP__
