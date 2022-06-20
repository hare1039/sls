#pragma once
#ifndef DATAFUNCTION_HPP__
#define DATAFUNCTION_HPP__

#include <slsfs.hpp>

namespace datafunction
{

void send_metadata(std::string const & filename)
{
    slsfs::log::logstring("_data_ send_metadata start");

    auto const && [parentpath, purefilename] = slsfs::base::parsename(filename);
    std::string const uuid = slsfs::uuid::get_uuid(parentpath);
    std::string const rvc_chan = uuid + "-query";

    slsfs::base::json jsondata;
    jsondata["filename"] = parentpath;
    jsondata["uuid"] = uuid;
    jsondata["data"] = purefilename;
    jsondata["type"] = "metadata";
    jsondata["operation"] = "addnewfile";
    jsondata["returnchannel"] = rvc_chan;

    slsfs::send_kafka(uuid, jsondata);

    slsfs::log::logstring("_data_ send_metadata sent kafka + listen kafka");
    slsfs::base::json done = slsfs::listen_kafka(rvc_chan);

    slsfs::log::logstring("_data_ send_metadata end");
}

auto perform_single_request(
    slsfs::base::storage_interface &datastorage,
    slsfs::base::json const& input) -> slsfs::base::json
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
        auto const data = input["data"].get<std::string>();
        slsfs::base::buf const write_buf = slsfs::base::to_buf(data);

        std::string const uuid = "/"s + slsfs::uuid::get_uuid(filename);
        datastorage.write_key(uuid, write_buf);

        slsfs::log::logstring("_data_ perform_single_request send_metadata");
        send_metadata(filename);

        single_response["response"] = "ok";
        break;
    }

    case "read"_:
    {
        slsfs::base::buf const data = datastorage.read_key("/"s + slsfs::uuid::get_uuid(filename));
        std::string const datastr = slsfs::base::to_string(data);

        single_response["data"] = datastr;
        single_response["response"] = "ok";
        break;
    }
    }

    std::cerr << "df send " << input["returnchannel"] << " with value " << single_response << "\n";
    slsfs::log::logstring("_data_ perform_single_request send_kafka");
    slsfs::send_kafka(input["returnchannel"], single_response);

    slsfs::log::logstring("_data_ perform_single_request end");
    return single_response;
}


} // namespace datafunction
#endif // DATAFUNCTION_HPP__
