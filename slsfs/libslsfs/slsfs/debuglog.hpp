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

enum class level
{
    trace = 0,
    debug,
    info,
    warning,
    error,
    fatal,
    none
};

namespace
{

struct global_info
{
    char const ** signature;
    std::chrono::high_resolution_clock::time_point start;
    static constexpr bool to_remote = false;
    static constexpr level current_level = level::trace;
};

auto global_info_instance() -> global_info&
{
    static global_info info;
    return info;
}

//auto global_msg_vec() -> std::vector<std::string>&
//{
//    static std::vector<std::string> reg;
//    return reg;
//}

} // namespace

template<level Level = level::trace>
void logstring(std::string const & msg);

auto init(char const * &signature)
{
    global_info& info = global_info_instance();
    info.start = std::chrono::high_resolution_clock::now();
    info.signature = std::addressof(signature);

    logstring(fmt::format("{} unixtime",
                          std::chrono::duration_cast<std::chrono::nanoseconds>(
                              info.start.time_since_epoch()).count()));
    return info;
}

void push_logs()
{
//    for (std::string const &finalmsg : global_msg_vec())
//        httpdo::logget("http://zion01:2015", finalmsg);
}

template<level Level = level::trace>
void logstring(std::string const & msg)
{
#ifdef NDEBUG
//    return;
#endif // NDEBUG

    auto const now = std::chrono::high_resolution_clock::now();
    global_info& info = global_info_instance();
    auto relativetime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - info.start).count();

    if constexpr (global_info::current_level <= Level)
    {
//        std::stringstream ss;
//        ss << std::this_thread::get_id();
        std::string const finalmsg = fmt::format("[{0:12d} {1}] {2}", relativetime, (*info.signature), msg);
        if (global_info::to_remote)
            httpdo::logget("http://zion01:2015", finalmsg);

        //global_msg_vec().push_back(finalmsg);

        std::cout << finalmsg << std::endl;
    }
    return;
}

//auto log(base::json msg) -> slsfs::base::json
//{
//    auto const now = std::chrono::high_resolution_clock::now();
//    global_info& info = global_info_instance();
//
//    auto relativetime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - info.start).count();
//
//    std::stringstream ss;
//    ss << "[" << relativetime << " " << (*info.signature) << "] ";
//
//    msg["now"] = relativetime;
//    msg["signature"] = (*info.signature);
//
//    logstring(msg.dump());
////    httpdo::logget("http://zion01:2015", msg.dump());
//    return msg;
//}

} // namespace slsfs::log

#endif // SLSFS_DEBUGLOG_HPP__
