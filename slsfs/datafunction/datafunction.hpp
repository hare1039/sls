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
    slsfs::base::buf response = datastorage.perform(input);

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
