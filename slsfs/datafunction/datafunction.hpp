#pragma once
#ifndef DATAFUNCTION_HPP__
#define DATAFUNCTION_HPP__

#include "storage-conf-cass.hpp"
#include "storage-conf-ssbd.hpp"

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
    //slsfs::log::logstring(std::string("_data_ send_metadata") + v);
    request->data.buf = std::vector<slsfs::pack::unit_t>(v.begin(), v.end());

    slsfs::send_kafka(request);

    slsfs::log::logstring("_data_ send_metadata sent kafka + listen kafka");
    //slsfs::base::json done = slsfs::listen_kafka(response);

    slsfs::log::logstring("_data_ send_metadata end");
}

auto perform_single_request(
    storage_conf &datastorage,
    slsfs::base::json const& input,
    std::uint32_t& version) -> slsfs::base::json
{
    slsfs::log::logstring("_data_ perform_single_request start");

    using namespace std::literals;
    std::cerr << "processing request: " << input << "\n";

    auto const operation = input["operation"].get<std::string>();
    auto const filename = input["filename"].get<std::string>();
    slsfs::base::json single_response;


    switch (slsfs::sswitch::hash(operation))
    {
        using namespace slsfs::sswitch;

    case "write"_:
    {
        slsfs::log::logstring("_data_ perform_single_request get data");
        auto const data = input["data"].get<std::string>();
        slsfs::base::buf const write_buf = slsfs::base::to_buf(data);

        slsfs::pack::key_t const uuid = slsfs::uuid::get_uuid(filename);

        int const realpos = input["position"].get<int>();
        int const blockid = realpos / datastorage.blocksize();
        int const offset = realpos % datastorage.blocksize();

        // 2PC first phase
        slsfs::log::logstring("_data_ perform_single_request check version");
        bool version_valid = false;
//        while (not version_valid)
//        {
//            version_valid = true;
//            slsfs::log::logstring("checking hosts version=" + std::to_string(version));
//
//            datastorage.foreach(
//                [&] (std::shared_ptr<slsfs::storage::interface> host) {
//                    bool ok = host->check_version_ok(uuid, blockid, version);
//                    if (not ok)
//                    {
//                        version_valid = false;
//                    }
//                    //** add failure and recovery here **
//                });
//        }
//        version++;

        slsfs::log::logstring("_data_ perform_single_request agreed");
        // 2PC second phase
        datastorage.foreach(
            [&] (std::shared_ptr<slsfs::storage::interface> host) {
                host->write_key(uuid, blockid, write_buf, offset, version);
            });

//        slsfs::log::logstring("_data_ perform_single_request send_metadata");
        send_metadata(filename);

        single_response["response"] = "ok";
        break;
    }

    case "read"_:
    {
        int const realpos = input["position"].get<int>();
        int const blockid = realpos / datastorage.blocksize();
        int const offset = realpos % datastorage.blocksize();

        std::size_t const size = input["size"].get<std::size_t>();

        slsfs::base::buf b;
        datastorage.foreach(
            [&] (std::shared_ptr<slsfs::storage::interface> host) {
                b = host->read_key(slsfs::uuid::get_uuid(filename), blockid, offset, size);
            });

        single_response["data"] = slsfs::base::to_string(b);
        single_response["response"] = "ok";
        break;
    }
    }

    if (input.contains("returnchannel"))
    {
        std::cerr << "df send " << input["returnchannel"] << " with value " << single_response << "\n";
        slsfs::log::logstring("_data_ perform_single_request send_kafka");

        slsfs::pack::packet_pointer request = std::make_shared<slsfs::pack::packet>();
        request->header.type = slsfs::pack::msg_t::put;
        request->header.key = slsfs::uuid::get_uuid(filename);

        auto const vec = slsfs::base::decode(input["returnchannel"]);
        assert(vec.size() == request->header.random_salt.size());
        std::copy(vec.cbegin(), vec.cend(), request->header.random_salt.begin());

        std::string v = single_response.dump();
        request->data.buf = std::vector<slsfs::pack::unit_t>(v.begin(), v.end());

        slsfs::send_kafka(request);
    }

    slsfs::log::logstring("_data_ perform_single_request end");
    return single_response;
}

} // namespace slsfsdf
#endif // DATAFUNCTION_HPP__
