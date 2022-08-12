#pragma once
#ifndef STORAGE_CONF_SSBD_HPP__
#define STORAGE_CONF_SSBD_HPP__

#include <slsfs.hpp>

#include "storage-conf.hpp"

namespace df
{

class storage_conf_ssbd : public storage_conf
{
    boost::asio::io_context &io_context_;
public:
    storage_conf_ssbd(boost::asio::io_context &io): io_context_{io} {}

    void init() override
    {
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "zion08", "12000"));
        //hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "zion08", "12000"));
        //hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "zion08", "12000"));
    }

    constexpr std::size_t fullsize()   { return 4 * 1024; } // byte
    constexpr std::size_t headersize() { return 4; } // byte
    int blocksize() override { return fullsize() - headersize(); }
};

} // namespace df

#endif // STORAGE_CONF_SSBD_HPP__
