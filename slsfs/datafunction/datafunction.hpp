#pragma once
#ifndef DATAFUNCTION_HPP__
#define DATAFUNCTION_HPP__

#include "storage-conf-cass.hpp"
#include "storage-conf-ssbd.hpp"
#include "version.hpp"

#include <slsfs.hpp>

namespace slsfsdf
{

void send_metadata(std::string const & filename)
{
    slsfs::log::logstring("_data_ send_metadata start");

    auto const && [parentpath, purefilename] = slsfs::base::parsename(filename);

    slsfs::pack::packet_pointer request = std::make_shared<slsfs::pack::packet>();
    slsfs::pack::key_t const uuid = slsfs::uuid::get_uuid(parentpath);
    request->header.type = slsfs::pack::msg_t::put;
    request->header.key = uuid;
    request->header.gen_sequence();

    slsfs::pack::packet_pointer response = std::make_shared<slsfs::pack::packet>();
    response->header.key = request->header.key;
    response->header.gen();

    slsfs::base::json jsondata;
    jsondata["filename"] = parentpath;
    jsondata["data"] = purefilename;
    jsondata["type"] = "metadata";
    jsondata["operation"] = "addnewfile";
    jsondata["returnchannel"] = slsfs::base::encode(response->header.random_salt);

    std::string const v = jsondata.dump();
    request->data.buf = std::vector<slsfs::pack::unit_t>(v.begin(), v.end());

    slsfs::send_kafka(request);

    slsfs::log::logstring("_data_ send_metadata sent kafka + listen kafka");
    //slsfs::base::json done = slsfs::listen_kafka(response);

    slsfs::log::logstring("_data_ send_metadata end");
}

auto perform_single_request(storage_conf &datastorage,
                            slsfs::jsre::request_parser<slsfs::base::byte> const& input)
    -> slsfs::base::buf
{
    slsfs::log::logstring("_data_ perform_single_request start: ");

    using namespace std::literals;
//    std::cerr << "processing request: " << input << "\n";

    //auto const operation = input["operation"].get<std::string>();
    //auto const filename = input["filename"].get<std::string>();
    slsfs::base::buf response;

    switch (input.operation())
    {
    case slsfs::jsre::operation_t::write:
    {
        slsfs::log::logstring("_data_ perform_single_request get data");
        //auto const data = input["data"].get<std::string>();
        //auto const data = input.data();
        //slsfs::base::buf const write_buf = slsfs::base::to_buf(data);

        auto const write_buf = input.data();

        slsfs::pack::key_t const uuid = input.uuid();

        //int const realpos = input["position"].get<int>();
        int const realpos = input.position();
        int const blockid = realpos / datastorage.blocksize();
        int const offset  = realpos % datastorage.blocksize();

        // 2PC first phase
        slsfs::log::logstring("_data_ perform_single_request check version");
        bool version_valid = false;
        std::uint32_t v = 0;
        while (not version_valid)
        {
            version_valid = true;
            std::uint32_t setv = std::max(v, version());

            //slsfs::log::logstring(fmt::format("get new v: {}, {}, {}", v, setv, version()));
            datastorage.foreach(
                [&uuid, &blockid, &setv, &v, &version_valid] (std::shared_ptr<slsfs::storage::interface> host) {
                    bool ok = host->check_version_ok(uuid, blockid, setv);
                    if (not ok)
                        version_valid = false;
                    v = setv;
                    //** add failure and recovery here **
                });
        }

        slsfs::log::logstring("_data_ perform_single_request agreed");
        // 2PC second phase
        datastorage.foreach(
            [&] (std::shared_ptr<slsfs::storage::interface> host) {
                slsfs::base::buf b;
                std::copy_n(write_buf, input.size(), std::back_inserter(b));
                host->write_key(uuid, blockid, b, offset, v);
            });

//        slsfs::log::logstring("_data_ perform_single_request send_metadata");
        //send_metadata(filename);

        response = {'A'};
        break;
    }

    case slsfs::jsre::operation_t::read:
    {
        //auto const write_buf = input.data();

        int const realpos = input.position();
        //int const realpos = input["position"].get<int>();

        int const blockid = realpos / datastorage.blocksize();
        int const offset  = realpos % datastorage.blocksize();
        slsfs::log::logstring("_data_ perform_single_request reading");

        std::uint32_t const size = input.size(); // input["size"].get<std::size_t>();

        slsfs::log::logstring(fmt::format("_data_ perform_single_request sending: {}, {}, {}, {}", blockid, offset, size, slsfs::pack::ntoh(size)));
        datastorage.foreach(
            [&] (std::shared_ptr<slsfs::storage::interface> host) {
                response = host->read_key(input.uuid(), blockid, offset, size);
            });

        slsfs::log::logstring("_data_ perform_single_request read from ssbd");
        //single_response["data"] = slsfs::base::to_string(b);
        break;
    }
    }

//    if (input.contains("returnchannel"))
//    {
//        std::cerr << "df send " << input["returnchannel"] << " with value " << single_response << "\n";
//        slsfs::log::logstring("_data_ perform_single_request send_kafka");
//
//        slsfs::pack::packet_pointer request = std::make_shared<slsfs::pack::packet>();
//        request->header.type = slsfs::pack::msg_t::put;
//        request->header.key = slsfs::uuid::get_uuid(filename);
//
//        auto const vec = slsfs::base::decode(input["returnchannel"]);
//        assert(vec.size() == request->header.random_salt.size());
//        std::copy(vec.cbegin(), vec.cend(), request->header.random_salt.begin());
//
//        std::string v = single_response.dump();
//        request->data.buf = std::vector<slsfs::pack::unit_t>(v.begin(), v.end());
//
//        slsfs::send_kafka(request);
//    }

    slsfs::log::logstring("_data_ perform_single_request end");
    return response;
}

} // namespace slsfsdf
#endif // DATAFUNCTION_HPP__
