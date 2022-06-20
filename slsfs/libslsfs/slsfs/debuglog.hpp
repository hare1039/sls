#pragma once
#ifndef SLSFS_DEBUGLOG_HPP__
#define SLSFS_DEBUGLOG_HPP__

#include "http-verb.hpp"
#include "fmt/core.h"

#include <curl/curl.h>

#include <thread>
#include <memory>
#include <chrono>

namespace slsfs::log
{

namespace
{


auto async_send()
{
//    multi_handle = curl_multi_init();
//
//    curl_multi_add_handle(multi_handle, http_handle);
//    curl_multi_add_handle(multi_handle, http_handle2);
}


class log_sender : std::enable_shared_from_this<log_sender>
{
    bool running_ = false;
    std::unique_ptr<std::thread> th_;

public:
    void start_bg_logger()
    {
        running_ = true;
        th_ = std::make_unique<std::thread>(
            [self = shared_from_this()] {
                self->sendlog_background_thread();
            });
    }


    void sendlog_background_thread()
    {
        while (running_)
        {
        }
    }
};

struct global_info
{
    char const ** signature;
    std::chrono::high_resolution_clock::time_point start;
};

auto global_info_instance() -> global_info&
{
    static global_info info;
    return info;
}

} // namespace

auto init(char const * &signature)
{
    global_info& info = global_info_instance();
    info.start = std::chrono::high_resolution_clock::now();
    info.signature = std::addressof(signature);
    return info;
}

auto logstring(std::string const & msg) -> std::string
{
    auto const now = std::chrono::high_resolution_clock::now();
    global_info& info = global_info_instance();

    auto relativetime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - info.start).count();

    std::string const finalmsg = fmt::format("[{0:12d} {1}] {2}", relativetime, (*info.signature), msg);
    httpdo::logget("http://zion01:2015", finalmsg);
    return finalmsg;
}

auto log(base::json msg) -> slsfs::base::json
{
    auto const now = std::chrono::high_resolution_clock::now();
    global_info& info = global_info_instance();

    auto relativetime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - info.start).count();

    std::stringstream ss;
    ss << "[" << relativetime << " " << (*info.signature) << "] ";

    msg["now"] = relativetime;
    msg["signature"] = (*info.signature);

    httpdo::logget("http://zion01:2015", msg.dump());
    return msg;
}

} // namespace slsfs::log

#endif // SLSFS_DEBUGLOG_HPP__
