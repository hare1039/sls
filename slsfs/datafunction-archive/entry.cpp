
#include "datafunction.hpp"
//#include "metadatafunction.hpp"
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
#include <map>
#include <ctime>

template<typename Function, typename ... Args>
auto record(Function &&f, Args &&... args) -> long int
{
    auto const start = std::chrono::high_resolution_clock::now();
    std::invoke(f, std::forward<Args>(args)...);
    auto const now = std::chrono::high_resolution_clock::now();
    auto relativetime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
    return relativetime;
}

template<typename Iterator>
void stats(Iterator start, Iterator end, std::string const memo = "")
{
    int const size = std::distance(start, end);

    double sum = std::accumulate(start, end, 0.0);
    double mean = sum / size, var = 0;

    std::map<int, int> dist;
    for (; start != end; start++)
    {
        dist[(*start)/1000000]++;
        var += std::pow((*start) - mean, 2);
    }

    var /= size;
    slsfs::log::logstring<slsfs::log::level::info>(fmt::format("{0} avg={1:.3f} sd={2:.3f}", memo, mean, std::sqrt(var)));
    for (auto && [time, count] : dist)
        slsfs::log::logstring<slsfs::log::level::info>(fmt::format("{0} {1}: {2}", memo, time, count));
}

namespace slsfsdf
{

using boost::asio::ip::tcp;

auto perform(slsfsdf::storage_conf &datastorage,
             slsfs::jsre::request_parser<slsfs::base::byte> const& single_input)
    -> slsfs::base::buf
{
    slsfs::log::logstring<slsfs::log::level::debug>("perform start");

    //auto const datatype = single_input["type"].get<std::string>();
    SCOPE_DEFER([] { slsfs::log::logstring<slsfs::log::level::debug>("perform end"); });

    switch (single_input.type())
    {
    case slsfs::jsre::type_t::file:
    {
        return slsfsdf::perform_single_request(datastorage, single_input);
        break;
    }

    case slsfs::jsre::type_t::metadata:
    {
//        return metadata::perform_single_request(datastorage, single_input);
        break;
    }

    case slsfs::jsre::type_t::wakeup:
    {
        break;
    }

    case slsfs::jsre::type_t::storagetest:
    {
        //slsfs::base::buf const write_buf = slsfs::base::to_buf(slsfs::uuid::gen_rand_str(1024));
        //std::string const uuid = slsfs::uuid::get_uuid_str("/storagetest.please.delete");

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

            slsfs::log::logstring<slsfs::log::level::info>(fmt::format("start request"));
            auto const start = std::chrono::high_resolution_clock::now();

            slsfs::log::logstring<slsfs::log::level::info>(fmt::format("start request: parsed"));
            //slsfs::base::json input = slsfs::base::json::parse(pack->data.buf.begin(), pack->data.buf.end());

            slsfs::jsre::request_parser<slsfs::base::byte> input {pack->data.buf.data()};

            slsfs::log::logstring<slsfs::log::level::info>(fmt::format("start request: parsed done"));

            slsfsdf::storage_conf* datastorage = slsfsdf::get_thread_local_datastorage().get();

            slsfs::log::logstring(fmt::format("perform start"));

            slsfs::base::buf v = perform(*datastorage, input);

            slsfs::log::logstring(fmt::format("perform finish"));

            //std::string v = "{}";
            pack->header.type = slsfs::pack::msg_t::worker_response;
            pack->data.buf.resize(v.size());// = std::vector<slsfs::pack::unit_t>(v.size(), '\0');
            std::memcpy(pack->data.buf.data(), v.data(), v.size());
            auto const end = std::chrono::high_resolution_clock::now();

            auto relativetime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            slsfs::log::logstring<slsfs::log::level::info>(fmt::format("req finish in: {}", relativetime));
            //pack->data.buf = std::vector<slsfs::pack::unit_t>{65, 66, 67};
        });

    boost::asio::async_connect(
        socket,
        //resolver.resolve("ow-ctrl", "12000"),
        resolver.resolve("10.0.0.240", "12000"),
        [&socket, worker_ptr] (boost::system::error_code const & ec, tcp::endpoint const& endpoint) {
            socket.set_option(tcp::no_delay(true));
            if (ec)
            {
                std::stringstream ss;
                ss << " connect to proxy error: " << ec.message();
                slsfs::log::logstring(ss.str());
            }
            else
            {
                slsfs::pack::packet_pointer ptr = std::make_shared<slsfs::pack::packet>();
                ptr->header.type = slsfs::pack::msg_t::worker_reg;
                ptr->header.gen();

                worker_ptr->start_write(
                    ptr,
                    [worker_ptr, ptr] (boost::system::error_code ec, std::size_t length) {
                        worker_ptr->start_listen_commands();
                    });
            }

        });

    using json = slsfs::base::json;
    using namespace std::literals;
    json input;

    boost::asio::deadline_timer t(ioc, boost::posix_time::seconds(10));
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
                    //slsfsdf::set_thread_local_datastorage(new slsfsdf::storage_conf_cass);
                    datastorage = slsfsdf::get_thread_local_datastorage().get();
                    // tech dept; need fix in the storage-conf.hpp and the future;
                }

                datastorage->init();
                datastorage->connect();
                slsfs::log::logstring("starting ioc");
                ioc.run();
            });

    std::cin >> input;
    std::cerr << "get request from stdin: " << input << "\n";
    slsfs::log::logstring("parsed json");

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
        std::cerr << "starting as action loop" << std::endl;
        //records.push_back(record([&](){ slsfsdf::do_datafunction(ow_out); }));
        int error = slsfsdf::do_datafunction(ow_out);
        if (error != 0)
            return error;
    }
    return 0;
#else
    std::cerr << "starting as normal" << std::endl;
    return slsfsdf::do_datafunction(std::cout);
    //slsfs::log::push_logs();
#endif
}
