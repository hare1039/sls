#pragma once
#ifndef METADATA_FUNCTION_HPP__
#define METADATA_FUNCTION_HPP__

#include <slsfs.hpp>

#include <set>

namespace metadata
{

auto perform_single_request(
    df::storage_conf &datastorage,
    slsfs::base::json const& input,
    std::uint32_t& version) -> slsfs::base::json
{
    slsfs::log::logstring("_meta_ perform_single_request start");

    using namespace std::literals;
    std::cerr << "meta processing request: " << input << "\n";
    auto const operation = input["operation"].get<std::string>();
    auto const filename = input["filename"].get<std::string>();
    slsfs::base::json single_response;

    switch (slsfs::sswitch::hash(operation))
    {
        using namespace slsfs::sswitch;

    case "addnewfile"_:
    {
        auto const data = input["data"].get<std::string>();
        slsfs::base::buf const write_buf = slsfs::base::to_buf(data);
        single_response["response"] = "ok";

        std::string const uuid = slsfs::uuid::get_uuid_str(filename);
        slsfs::log::logstring("_meta_ perform_single_request datastorage.append");
        datastorage.append_list_key(uuid, write_buf);
        break;
    }

    case "ls"_:
    {
        std::string outbuf;
        slsfs::log::logstring("_meta_ perform_single_request datastorage.merge");

        datastorage.merge_list_key(
            slsfs::uuid::get_uuid_str(filename),
            [&outbuf, &single_response] (std::vector<slsfs::base::buf> const & buf) {
                std::set<std::string> files;

                for (slsfs::base::buf const &b : buf)
                {
                    std::string const n = slsfs::base::to_string(b);

                    if (files.find(n) == files.end())
                        outbuf += (n + "; ");

                    files.insert(n);
                }
                std::cerr << outbuf << "\n";
                single_response["data"] = outbuf;
            });

        single_response["response"] = "ok";
        single_response["data"] = outbuf;

        break;
    }

    case "read"_:
    {
        slsfs::log::logstring("_meta_ perform_single_request datastorage.read");
        slsfs::base::buf const data = datastorage.read_key(slsfs::uuid::get_uuid_str(filename));
        std::string const datastr = slsfs::base::to_string(data);

        single_response["response"] = "ok";
        single_response["data"] = datastr;

        break;
    }

    case "create"_:
    {
        slsfs::log::logstring("_meta_ perform_single_request create");
        slsfs::create(filename.c_str());

        single_response["response"] = "ok";
        break;
    }

    }

    if (input.contains("returnchannel"))
    {
        slsfs::log::logstring("_meta_ perform_single_request send_kafka");
        std::cerr << "mdf send " << input["returnchannel"] << " with value " << single_response << "\n";

        slsfs::pack::packet_pointer request = std::make_shared<slsfs::pack::packet>();
        request->header.type = slsfs::pack::msg_t::put;
        request->header.key = slsfs::uuid::get_uuid(filename);
        request->header.gen_sequence();

        auto const vec = slsfs::base::decode(input["returnchannel"]);
        assert(vec.size() == request->header.random_salt.size());
        std::copy(vec.cbegin(), vec.cend(), request->header.random_salt.begin());

        std::string v = single_response.dump();
        request->data.buf = std::vector<slsfs::pack::unit_t>(v.begin(), v.end());

        slsfs::send_kafka(request);
    }

    slsfs::log::logstring("_meta_ perform_single_request end");
    return single_response;
}

} // namespace metadata

#endif // METADATA_FUNCTION_HPP__
