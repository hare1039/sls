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
    //std::string const uuid = slsfs::uuid::get_uuid(parentpath);
    slsfs::pack::key_t uuid = slsfs::uuid::get_uuid(parentpath);

    //std::string const rvc_chan = uuid + "-query";
    slsfs::pack::key_t rvc_chan = uuid;
    std::vector<slsfs::pack::unit_t> sp = slsfs::uuid::gen_rand(4);
    std::copy(sp.begin(), sp.end(), rvc_chan.rbegin());
//    std::string v = "_data_ ";
//    for (int i : rvc_chan)
//        v += std::to_string(i) + ", ";
//    slsfs::log::logstring(v);

    slsfs::base::json jsondata;
    jsondata["filename"] = parentpath;
    jsondata["uuid"] = uuid;
    jsondata["data"] = purefilename;
    jsondata["type"] = "metadata";
    jsondata["operation"] = "addnewfile";
    jsondata["returnchannel"] = slsfs::base::encode(rvc_chan);

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

        std::string const uuid = "/"s + slsfs::uuid::get_uuid_str(filename);
        datastorage.write_key(uuid, write_buf);

        slsfs::log::logstring("_data_ perform_single_request send_metadata");
        send_metadata(filename);

        single_response["response"] = "ok";
        break;
    }

    case "read"_:
    {
        slsfs::base::buf const data = datastorage.read_key("/"s + slsfs::uuid::get_uuid_str(filename));
        std::string const datastr = slsfs::base::to_string(data);

        single_response["data"] = datastr;
        single_response["response"] = "ok";
        break;
    }
    }

    if (input.contains("returnchannel"))
    {
        std::cerr << "df send " << input["returnchannel"] << " with value " << single_response << "\n";
        slsfs::log::logstring("_data_ perform_single_request send_kafka");

        auto cont = slsfs::base::decode(input["returnchannel"]);
        slsfs::pack::key_t key;
        std::copy(cont.begin(), cont.end(), key.begin());
        slsfs::send_kafka(key, single_response);
    }

    slsfs::log::logstring("_data_ perform_single_request end");
    return single_response;
}


} // namespace datafunction
#endif // DATAFUNCTION_HPP__
