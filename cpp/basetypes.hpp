#pragma once

#ifndef BASETYPES_HPP__
#define BASETYPES_HPP__

#include <vector>
#include <cstdint>
#include <sstream>
#include "cppcodec/base64_rfc4648.hpp"

namespace base
{

using byte = std::uint8_t;
using buf = std::vector<byte>;

auto decode(std::string const& v) -> buf
{
    using base64 = cppcodec::base64_rfc4648;
    return base64::decode(v);
}

auto encode(buf const& v) -> std::string
{
    using base64 = cppcodec::base64_rfc4648;
    return base64::encode(v);
}

auto to_string(buf const& v) -> std::string
{
    std::stringstream ss;
    ss << "[ ";
    for (byte b : v)
        ss << b << " ";
    ss << "]";
    return ss.str();
}

buf to_buf(std::string const& s)
{
    buf b;
    b.reserve(s.size());
    std::copy(s.begin(), s.end(), std::back_inserter(b));
    return b;
}

template<typename T>
buf to_buf(T const& t)
{
    buf b(sizeof(T), 0);
    std::memcpy(b.data(), &t, sizeof(T));
    return b;
}

template<typename T>
T   from_buf(buf const& b)
{
    T t;
    std::memcpy(&t, b.data(), b.size());
    return t;
}

} // namespace base

#endif // BASETYPES_HPP__
