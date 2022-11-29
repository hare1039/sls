#pragma once
#ifndef STORAGE_CONF_SSBD_HPP__
#define STORAGE_CONF_SSBD_HPP__

#include <slsfs.hpp>

#include "storage-conf.hpp"

namespace slsfsdf
{

class storage_conf_ssbd : public storage_conf
{
    boost::asio::io_context &io_context_;
public:
    storage_conf_ssbd(boost::asio::io_context &io): io_context_{io} {}

    void init() override
    {
        //hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-2", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "192.168.0.165", "12000"));
        //hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "zion08", "12000"));
        //hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "zion08", "12000"));
    }

    constexpr std::size_t fullsize()   { return 4 * 1024; } // byte
    constexpr std::size_t headersize() { return 4; } // byte
    int blocksize() override { return fullsize() - headersize(); }

    virtual auto perform(slsfs::jsre::request_parser<slsfs::base::byte> const& input) -> slsfs::base::buf override
    {
        slsfs::base::buf response;
        switch (input.operation())
        {
        case slsfs::jsre::operation_t::write:
        {
            slsfs::log::logstring("_data_ perform_single_request get data");
            auto const write_buf = input.data();
            slsfs::pack::key_t const uuid = input.uuid();

            int const realpos = input.position();
            int const blockid = realpos / blocksize();
            int const offset  = realpos % blocksize();

            slsfs::log::logstring("_data_ perform_single_request agreed");
            slsfs::base::buf b;
            std::copy_n(write_buf, input.size(), std::back_inserter(b));

            for (std::shared_ptr<slsfs::storage::interface> host : hostlist_)
                host->write_key(uuid, blockid, b, offset, 0);

            response = {'O', 'K'};
            break;
        }

        case slsfs::jsre::operation_t::create:
        {
            break;
        }

        case slsfs::jsre::operation_t::read:
        {
            int const realpos = input.position();
            int const blockid = realpos / blocksize();
            int const offset  = realpos % blocksize();
            slsfs::log::logstring("_data_ perform_single_request reading");

            std::uint32_t const size = input.size(); // input["size"].get<std::size_t>();

            slsfs::log::logstring(fmt::format("_data_ perform_single_request sending: {}, {}, {}, {}", blockid, offset, size, slsfs::pack::ntoh(size)));
            for (std::shared_ptr<slsfs::storage::interface> host : hostlist_)
                response = host->read_key(input.uuid(), blockid, offset, size);

            slsfs::log::logstring("_data_ perform_single_request read from ssbd");
            break;
        }
        }

        return response;
    }
};

} // namespace slsfsdf

#endif // STORAGE_CONF_SSBD_HPP__
