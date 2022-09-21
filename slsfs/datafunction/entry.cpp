
#include "datafunction.hpp"
#include "metadatafunction.hpp"
#include "worker.hpp"
#include "storage-conf.hpp"
#include "storage-conf-cass.hpp"
#include "storage-conf-ssbd.hpp"

#include <slsfs.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <memory>
#include <thread>
#include <sstream>
#include <ctime>


namespace slsfsdf
{

using boost::asio::ip::tcp;

auto perform(slsfsdf::storage_conf &datastorage, slsfs::base::json const& single_input)
    -> slsfs::base::json
{
    slsfs::log::logstring<slsfs::log::level::debug>("perform start");
    auto const datatype = single_input["type"].get<std::string>();
    SCOPE_DEFER([] { slsfs::log::logstring<slsfs::log::level::debug>("perform end"); });


    switch (slsfs::sswitch::hash(datatype))
    {
        using namespace slsfs::sswitch;

    case "file"_:
    {
        return slsfsdf::perform_single_request(datastorage, single_input);
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

        slsfs::log::logstring("end read from storage (1000)");
        slsfs::log::push_logs();
    }

    }
    return {};
}

int do_datafunction(std::ostream &ow_out)
try
{
//    auto start = std::chrono::high_resolution_clock::now();
//    auto end = std::chrono::high_resolution_clock::now();
//    auto pass = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
//    ow_out << "{\"hello\": \"world\"}" << std::endl; // enter and flush
    //std::cout << "{\"hello\": \"world\", \"time\": " << pass << "}\n";
//    return 0;

    boost::asio::io_context ioc;
    tcp::socket socket(ioc);
    tcp::resolver resolver(ioc);

    //slsfsdf::storage_conf_ssbd datastorage(ioc);

    auto worker_ptr = std::make_shared<worker>(
        ioc, socket,
        [&ow_out] (slsfs::pack::packet_pointer pack) {
            pack->header.type = slsfs::pack::msg_t::worker_response;
            //std::string v2 = "{}";
            //pack->data.buf.resize(v2.size());
            //std::memcpy(pack->data.buf.data(), v2.data(), v2.size());
            //return;

            slsfs::base::json input = slsfs::base::json::parse(pack->data.buf.begin(), pack->data.buf.end());

            std::string v;
            try
            {
                slsfsdf::storage_conf* datastorage = slsfsdf::get_thread_local_datastorage().get();

                v = perform(*datastorage, input).dump();
            } catch (slsfs::base::json::exception e) {
                v = "{}";
            }

            pack->header.type = slsfs::pack::msg_t::worker_response;
            pack->data.buf.resize(v.size());// = std::vector<slsfs::pack::unit_t>(v.size(), '\0');
            std::memcpy(pack->data.buf.data(), v.data(), v.size());
            //pack->data.buf = std::vector<slsfs::pack::unit_t>{65, 66, 67};
        });

    boost::asio::async_connect(
        socket, resolver.resolve("ow-ctrl", "12000"),
        [&socket, worker_ptr] (boost::system::error_code const & ec, tcp::endpoint const& endpoint) {
            slsfs::pack::packet_pointer ptr = std::make_shared<slsfs::pack::packet>();
            ptr->header.type = slsfs::pack::msg_t::worker_reg;
            ptr->header.gen();

            worker_ptr->start_write(
                ptr,
                [worker_ptr, ptr] (boost::system::error_code ec, std::size_t length) {
                    worker_ptr->start_listen_commands();
                });
        });

    using json = slsfs::base::json;
    using namespace std::literals;
    json input;

    boost::asio::deadline_timer t(ioc, boost::posix_time::seconds(1));
    t.async_wait([](boost::system::error_code const&) {});

    std::vector<std::thread> v;
    unsigned int const worker = std::min<unsigned int>(4, std::thread::hardware_concurrency());
    v.reserve(worker);
    for(int i = 0; i < worker; i++)
        v.emplace_back(
            [&ioc] {
                slsfsdf::storage_conf * datastorage = slsfsdf::get_thread_local_datastorage().get();
                if (datastorage == nullptr)
                {
                    slsfsdf::set_thread_local_datastorage(new slsfsdf::storage_conf_ssbd(ioc));
                    // tech dept; need fix in the storage-conf.hpp and the future;
                }

                datastorage->init();
                datastorage->connect();
                ioc.run();
            });

    //std::cin >> input;
    //std::cerr << "get request from stdin: " << input << "\n";
    //slsfs::log::logstring("parsed json");

    json output;
    output["original-request"] = input;
    output["response"] = json::array();

    for (std::thread& th : v)
        th.join();

    slsfs::log::logstring("data function shutdown");
    ow_out << "{\"hello\": \"world\"}" << std::endl;

    return 0;
}
catch (std::exception const & e)
{
    slsfs::log::logstring(std::string("exception thrown ") + e.what());
    std::cerr << "do function ecxception: " << e.what() << std::endl;
    return -1;
}



//    try
//    {
//        if (input.contains("action_name"))
//        {
//            ow_out << perform(datastorage, input["value"], version).dump() << std::endl;
//        }
//        else if (input.contains("messages"))
//            for (json & single_input : input["messages"])
//            {
//                json const converted = slsfs::base::decode_kafkajson(single_input["value"].get<std::string>());
//                output["response"].push_back(perform(datastorage, converted, version));
//            }
//        else
//            output["response"].push_back(perform(datastorage, input, version));
//    }
//    catch (std::exception const & e)
//    {
//        output["exception thrown"] = e.what();
//        std::cerr << "exception thrown" << e.what() << "\n";
//        return -1;
//    }


} // namespace slsfsdf

int main(int argc, char *argv[])
{
    std::string name = fmt::format("DF:{0:4d}", slsfs::uuid::gen_rand_number());
    char const* name_cstr = name.c_str();
    slsfs::log::init(name_cstr);
    slsfs::log::logstring("data function start");

    //std::uint32_t version = std::time(nullptr);//const auto p1 = std::chrono::system_clock::now();;
    SCOPE_DEFER([] { slsfs::log::push_logs(); });

#ifdef AS_ACTIONLOOP
    namespace io = boost::iostreams;
    io::stream_buffer<io::file_descriptor_sink> fpstream(3, io::close_handle);
    std::ostream ow_out {&fpstream};
    while (true)
    {
        int error = slsfsdf::do_datafunction(ow_out);
        if (error != 0)
            return error;
    }
    return 0;
#else
    return slsfsdf::do_datafunction(std::cout);
    //slsfs::log::push_logs();
#endif
}
