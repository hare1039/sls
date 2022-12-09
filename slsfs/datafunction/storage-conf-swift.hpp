#pragma once
#ifndef STORAGE_CONF_SWIFT_HPP__
#define STORAGE_CONF_SWIFT_HPP__

#include "storage-conf.hpp"

#include <slsfs/storage-swiftkv-minio.hpp>


namespace slsfsdf
{

// Storage configuration for swift object storage.
class storage_conf_swift : public storage_conf
{
public:
    virtual void init() override {
        hostlist_.push_back(std::make_shared<slsfs::storage::swiftkv>("moc-kvstore"));
    }

    virtual int blocksize() override { return 4096; } // byte
    virtual auto perform(slsfs::jsre::request_parser<slsfs::base::byte> const& input) -> slsfs::base::buf override
    {
        slsfs::base::buf response;
        switch (input.operation())
        {
        case slsfs::jsre::operation_t::write:
        {
            slsfs::log::logstring("_data_ swiftkv perform_single_request get data");
            auto const write_buf = input.data();
            slsfs::pack::key_t const uuid = input.uuid();

            int const realpos = input.position();
            int const blockid = realpos / blocksize();
            int const offset  = realpos % blocksize();

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
            slsfs::log::logstring("_data_ swiftkv perform_single_request reading");

            std::uint32_t const size = input.size(); // input["size"].get<std::size_t>();

            slsfs::log::logstring(fmt::format("_data_ swiftkv perform_single_request sending: {}, {}, {}, {}", blockid, offset, size, slsfs::pack::ntoh(size)));
            for (std::shared_ptr<slsfs::storage::interface> host : hostlist_)
                response = host->read_key(input.uuid(), blockid, offset, size);

            slsfs::log::logstring("_data_ swiftkv perform_single_request read from ssbd");
            break;
        }
        }
        return response;
    }
};

} // namespace slsfsdf

#endif // STORAGE_CONF_SWIFT_HPP__
