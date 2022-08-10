#pragma once

#ifndef SLSFS_STORAGE_SSBD_HPP__
#define SLSFS_STORAGE_SSBD_HPP__

#include "storage.hpp"
#include "basetypes.hpp"
#include "scope-exit.hpp"

namespace slsfs::storage
{

class ssbd : public storage_interface
{
public:
    virtual ~ssbd() {}
    virtual void connect() {};

    // block interface [int] -> buf
    virtual buf  read_block(std::uint32_t const offset) { return {}; };
    virtual void write_block(std::uint32_t const offset, buf const& buffer) {};

    // key interface   [str] -> buf
    virtual buf  read_key(std::string const name) { return {}; };
    virtual void write_key(std::string const name, buf const& buffer) {};

    // list interface  [str] -> buf
    virtual void append_list_key(std::string const name, buf const& buffer) {};
    virtual void merge_list_key(std::string const name, std::function<void(std::vector<buf> const&)> reduce) {};
    virtual buf  get_list_key(std::string const name) { return {}; };

    virtual void create_volume(std::uint32_t const size, std::string const& name) {};
    auto current_state() -> state& { return state_; }
};

}


#endif // SLSFS_STORAGE_SSBD_HPP__
