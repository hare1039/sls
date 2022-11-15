#pragma once
#ifndef JSON_REPLACEMENT_HPP__
#define JSON_REPLACEMENT_HPP__

#include "serializer.hpp"

namespace slsfs::jsre
{

enum class type_t : std::int8_t
{
    file,
    metadata,
    wakeup,
    storagetest,
};

enum class operation_t : std::int8_t
{
    write,
    read,
};

using key_t = pack::key_t;

struct request
{
    type_t        type;
    operation_t   operation;
    key_t         uuid;
    std::uint32_t position;
    std::uint32_t size;

    void to_network_format()
    {
        position = pack::hton(position);
        size     = pack::hton(size);
    }
};

template<typename CharType> requires (sizeof (CharType) == 8/8)
struct request_parser
{
    CharType const *refdata;
    request_parser(CharType * ref): refdata {ref} {}

    auto type() const -> type_t
    {
        std::underlying_type_t<type_t> t;
        std::memcpy(&t, refdata + offsetof(request, type), sizeof(t));
        return static_cast<type_t>(pack::ntoh(t));
    }

    auto operation() const -> operation_t
    {
        std::underlying_type_t<operation_t> op;
        std::memcpy(&op, refdata + offsetof(request, operation), sizeof(op));
        return static_cast<operation_t>(pack::ntoh(op));
    }

    auto uuid() const -> key_t
    {
        key_t k;
        std::memcpy(k.data(), refdata + offsetof(request, uuid), k.size());
        return k;
    }

    auto position() const -> std::uint32_t
    {
        std::uint32_t pos;
        std::memcpy(&pos, refdata + offsetof(request, position), sizeof(pos));
        return pack::ntoh(pos);
    }

    auto size() const -> std::uint32_t
    {
        std::uint32_t s;
        std::memcpy(&s, refdata + offsetof(request, size), sizeof(s));
        return pack::ntoh(s);
    }

    auto data() const -> const CharType*
    {
        return refdata + sizeof(request);
    }
};



} // namespace jsre

#endif // JSON_REPLACEMENT_HPP__
