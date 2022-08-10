#pragma once

#ifndef STORAGE_HPP__
#define STORAGE_HPP__

#include "basetypes.hpp"
#include "inode-def.hpp"

namespace base
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
    virtual buf  read_block(inode::block_ptr const offset) = 0;
    virtual void write_block(inode::block_ptr const offset, buf const& buffer) = 0;
    virtual void create_volume(inode::block_ptr const size, std::string const& name) = 0;
    auto current_state() -> state& { return state_; }
};

}


#endif // STORAGE_HPP__
