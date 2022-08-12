//#include <boost/asio.hpp>

#include "datafunction.hpp"
//#include "metadatafunction.hpp"
#include "storage-conf.hpp"
#include "storage-conf-cass.hpp"

#include <slsfs.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <iostream>
#include <memory>
#include <thread>
#include <sstream>

namespace slsfsdf
{

auto perform(df::storage_conf &datastorage, slsfs::base::json const& single_input, std::uint32_t& version)
    -> slsfs::base::json
{
    slsfs::log::logstring("perform start");
    SCOPE_DEFER([]{ slsfs::log::logstring("perform end"); });
    auto const datatype = single_input["type"].get<std::string>();

    switch (slsfs::sswitch::hash(datatype))
    {
        using namespace slsfs::sswitch;

    case "file"_:
    {
        return datafunction::perform_single_request(datastorage, single_input, version);
        break;
    }

    case "metadata"_:
    {
//        return metadata::perform_single_request(datastorage, single_input);
        break;
    }

    case "storagetest"_:
    {
        slsfs::base::buf const write_buf = slsfs::base::to_buf(slsfs::uuid::gen_rand_str(1024));
        std::string const uuid = slsfs::uuid::get_uuid_str("/storagetest.please.delete");

        slsfs::log::logstring("end read from storage (1000)");
        slsfs::log::push_logs();
    }

    }
    return {};
}

int do_datafunction(std::ostream &ow_out, df::storage_conf &datastorage, std::uint32_t& version)
{
//    auto start = std::chrono::high_resolution_clock::now();
//    auto end = std::chrono::high_resolution_clock::now();
//    auto pass = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    //std::cout << "{\"hello\": \"world\", \"time\": " << pass << "}\n";
//    ow_out << "{\"hello\": \"world\"}" << std::endl;
//    return 0;

    using json = slsfs::base::json;
    using namespace std::literals;
    json input;
    try
    {
        std::cin >> input;
    }
    catch (std::exception const & e)
    {
        return -1;
    }

    std::cerr << "get request: " << input << "\n";
    slsfs::log::logstring("parsed json");

    json output;
    output["original-request"] = input;
    output["response"] = json::array();

    try
    {
        if (input.contains("action_name"))
        {
            ow_out << perform(datastorage, input["value"], version).dump() << std::endl;
        }
        else if (input.contains("messages"))
            for (json & single_input : input["messages"])
            {
                json const converted = slsfs::base::decode_kafkajson(single_input["value"].get<std::string>());
                output["response"].push_back(perform(datastorage, converted, version));
            }
        else
            output["response"].push_back(perform(datastorage, input, version));
    }
    catch (std::exception const & e)
    {
        output["exception thrown"] = e.what();
        std::cerr << "exception thrown" << e.what() << "\n";
        return -1;
    }

    ow_out << output.dump() << std::endl;
    return 0;
}

} // namespace slsfsdf

int main(int argc, char *argv[])
{
    std::string name = fmt::format("DF:{0:4d}", slsfs::uuid::gen_rand_number());
    char const* name_cstr = name.c_str();
    slsfs::log::init(name_cstr);
    slsfs::log::logstring("data function start");

//    slsfs::base::storage_interface datastorage;
    df::storage_conf_cass datastorage;

    datastorage.init();
    datastorage.connect();
    //slsfs::storage::cassandra datastorage{"192.168.2.27"};
    //datastorage.connect();
    slsfs::log::logstring("connected to datastorage");

    std::uint32_t version = 10;
    SCOPE_DEFER([] { slsfs::log::push_logs(); });

#ifdef AS_ACTIONLOOP
    namespace io = boost::iostreams;
    //io::stream_buffer<io::file_descriptor_sink> fpstream(3, io::close_handle);
    io::stream_buffer<io::file_descriptor_sink> fpstream(3, io::close_handle);
    std::ostream ow_out {&fpstream};
    while (true)
    {
        int error = slsfsdf::do_datafunction(ow_out, datastorage, version);
        if (error != 0)
            return error;
    }
    return 0;
#else
    return slsfsdf::do_datafunction(std::cout, datastorage, version);
    //slsfs::log::push_logs();
#endif
}
