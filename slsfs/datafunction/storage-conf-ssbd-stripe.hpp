#pragma once
#ifndef STORAGE_CONF_SSBD_STRIPE_HPP__
#define STORAGE_CONF_SSBD_STRIPE_HPP__

#include "storage-conf.hpp"

#include <slsfs.hpp>

#include <vector>

namespace slsfsdf
{

class storage_conf_ssbd_stripe : public storage_conf
{
    boost::asio::io_context &io_context_;
    static auto static_engine() -> std::mt19937&
    {
        static thread_local std::mt19937 mt;
        return mt;
    }

    auto select_replica(slsfs::pack::key_t const& uuid, int count) -> std::vector<int>
    {
        std::seed_seq seeds {uuid.begin(), uuid.end()};
        static_engine().seed(seeds);

        std::uniform_int_distribution<> dist(0, hostlist_.size());
        auto gen = [this, &dist] () { return dist(static_engine()); };

        std::vector<int> rv(count);
        std::generate(rv.begin(), rv.end(), gen);

        return rv;
    }


public:
    storage_conf_ssbd_stripe(boost::asio::io_context &io): io_context_{io} {}

    void init() override
    {
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-1", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-2", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-3", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-4", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-5", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-6", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-7", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-8", "12000"));
        hostlist_.push_back(std::make_shared<slsfs::storage::ssbd>(io_context_, "ssbd-9", "12000"));
    }

    constexpr std::size_t fullsize()   { return 4 * 1024; } // byte
    constexpr std::size_t headersize() { return 4; } // byte
    int blocksize() override { return fullsize() - headersize(); }
    constexpr int replication_size()   { return 3; }

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

            std::vector<int> const selected_host_index = select_replica(uuid, replication_size());

            // 2PC first phase
            slsfs::log::logstring("_data_ perform_single_request check version");
            bool version_valid = false;
            std::uint32_t v = 0;
            while (not version_valid)
            {
                version_valid = true;
                std::uint32_t setv = std::max(v, version());

                for (int const index : selected_host_index)
                {
                    if (bool ok = hostlist_.at(index)->check_version_ok(input.uuid(), blockid, setv); not ok)
                        version_valid = false;
                    v = setv;
                }
                //** add failure and recovery here **
            }

            slsfs::log::logstring("_data_ perform_single_request agreed");
            // 2PC second phase
            slsfs::base::buf b;
            std::copy_n(write_buf, input.size(), std::back_inserter(b));
            for (int const index : selected_host_index)
                hostlist_.at(index)->write_key(uuid, blockid, b, offset, v);

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

            std::vector<int> const selected_host_index = select_replica(input.uuid(), replication_size());
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

#endif // STORAGE_CONF_SSBD_STRIPE_HPP__
