//#include <boost/asio.hpp>

#include "datafunction.hpp"
#include "metadatafunction.hpp"

#include <slsfs.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <iostream>
#include <memory>
#include <thread>
#include <sstream>

namespace slsfsdf
{

auto perform(slsfs::storage::interface &datastorage, slsfs::base::json const& single_input)
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
        return datafunction::perform_single_request(datastorage, single_input);
        break;
    }

    case "metadata"_:
    {
        return metadata::perform_single_request(datastorage, single_input);
        break;
    }

    case "storagetest"_:
    {
        slsfs::base::buf const write_buf = slsfs::base::to_buf(slsfs::uuid::gen_rand_str(1024));
        std::string const uuid = slsfs::uuid::get_uuid_str("/storagetest.please.delete");
        slsfs::log::logstring("start do write to storage");
        for (int i = 0; i < 1000; i++)
        {
            datastorage.write_key(uuid, write_buf);
        }
        slsfs::log::logstring("end write to storage (1000)");
        slsfs::log::logstring("start do read from storage");
        for (int i = 0; i < 1000; i++)
        {
            datastorage.read_key(uuid);
        }
        slsfs::log::logstring("end read from storage (1000)");
        slsfs::log::push_logs();
    }

    }
    return {};
}

int do_datafunction(std::ostream &ow_out, slsfs::storage::interface &datastorage)
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
            ow_out << perform(datastorage, input["value"]).dump() << std::endl;
        }
        else if (input.contains("messages"))
            for (json & single_input : input["messages"])
            {
                json const converted = slsfs::base::decode_kafkajson(single_input["value"].get<std::string>());
                output["response"].push_back(perform(datastorage, converted));
            }
        else
            output["response"].push_back(perform(datastorage, input));
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
    slsfs::storage::cassandra datastorage{"192.168.2.27"};
    datastorage.connect();
    slsfs::log::logstring("connected to datastorage");

    SCOPE_DEFER([] { slsfs::log::push_logs(); });

#ifdef AS_ACTIONLOOP
    namespace io = boost::iostreams;
    //io::stream_buffer<io::file_descriptor_sink> fpstream(3, io::close_handle);
    io::stream_buffer<io::file_descriptor_sink> fpstream(3, io::close_handle);
    std::ostream ow_out {&fpstream};
    while (true)
    {
        int error = slsfsdf::do_datafunction(ow_out, datastorage);
        if (error != 0)
            return error;
    }
    return 0;
#else
    return slsfsdf::do_datafunction(std::cout, datastorage);
    //slsfs::log::push_logs();
#endif
}
