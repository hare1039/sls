

#include "datafunction.hpp"
//#include "metadatafunction.hpp"
#include "worker.hpp"
#include "storage-conf.hpp"
#include "storage-conf-cass.hpp"
#include "storage-conf-ssbd.hpp"
#include "proxy-command.hpp"

#include <slsfs.hpp>

#include <oneapi/tbb/concurrent_hash_map.h>
#include <oneapi/tbb/concurrent_queue.h>

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

auto perform(slsfsdf::storage_conf *datastorage,
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
        return slsfsdf::perform_single_request(*datastorage, single_input);
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
    slsfs::server::queue_map queue_map;
    boost::asio::io_context ioc;
    tcp::resolver resolver(ioc);

    slsfs::server::proxy_set proxys;

    using json = slsfs::base::json;
    json input;

    std::cin >> input;
    input = input["value"];
//    slsfs::log::logstring(fmt::format("get request from stdin: {}", input));
    auto proxy_command_ptr = std::make_shared<slsfs::server::proxy_command>(ioc, queue_map, perform, proxys);

    proxy_command_ptr->start_connect(resolver.resolve(input["host"].get<std::string>(),
                                                      input["port"].get<std::string>()));

//    boost::asio::deadline_timer t(ioc, boost::posix_time::seconds(10));
//    t.async_wait([](boost::system::error_code const&) {});

    std::vector<std::thread> v;
    unsigned int const worker = std::min<unsigned int>(4, std::thread::hardware_concurrency());
    v.reserve(worker);
    for(unsigned int i = 0; i < worker; i++)
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
        slsfs::log::logstring("starting as action loop");
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
