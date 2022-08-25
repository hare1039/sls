#pragma once
#ifndef CPP_SERIALIZER_OBJECTPACK_HPP__
#define CPP_SERIALIZER_OBJECTPACK_HPP__

#include <arpa/inet.h>

//#include <boost/functional/hash.hpp>

#include <ios>
#include <iostream>
#include <vector>
#include <memory>
#include <cstring>
#include <array>
#include <tuple>
#include <algorithm>
#include <random>

namespace slsfs::pack
{

namespace
{

template <typename Integer>
void hash_combine(std::size_t& seed, Integer value)
{
    seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template <typename It>
void hash_range(std::size_t& seed, It first, It last)
{
    for(; first != last; ++first)
        hash_combine(seed, *first);
}

} // namespace


using unit_t = unsigned char;
static_assert(sizeof(unit_t) == 8/8);

// key = [32] byte main key // sha256 bit
using key_t = std::array<unit_t, 256 / 8 / sizeof(unit_t)>;
enum class msg_t: unit_t
{
    err = 0,
    put = 1,
    get = 2,
    ack = 4,
    worker_reg = 8,
    worker_dereg = 9,
    worker_push_request = 10,
    worker_response = 11,
    trigger = 16,
};

template<typename Integer>
auto hton(Integer i) -> Integer
{
    if constexpr (sizeof(Integer) == sizeof(decltype(htonl(i))))
        return htonl(i);
    else if constexpr (sizeof(Integer) == sizeof(decltype(htons(i))))
        return htons(i);
    else if constexpr (sizeof(Integer) == 1)
        return i;
    else
    {
        static_assert("not supported conversion");
        return -1;
    }
}

template<typename Integer>
auto ntoh(Integer i) -> Integer
{
    if constexpr (sizeof(Integer) == sizeof(decltype(ntohl(i))))
        return ntohl(i);
    else if constexpr (sizeof(Integer) == sizeof(decltype(ntohs(i))))
        return ntohs(i);
    else if constexpr (sizeof(Integer) == 1)
        return i;
    else
    {
        static_assert("not supported conversion");
        return -1;
    }
}


struct packet_header
{
    msg_t type;
    key_t key;
    std::uint32_t datasize; // not in byte form
    std::array<unit_t, 4> sequence{};
    std::array<unit_t, 4> random_salt{};

    static constexpr int bytesize =
        sizeof(datasize) + sizeof(type) +
        std::tuple_size<decltype(key)>::value +
        std::tuple_size<decltype(sequence)>::value +
        std::tuple_size<decltype(random_salt)>::value;

    auto static_rand_engine() -> std::mt19937&
    {
        static thread_local std::random_device rd;
        static thread_local std::mt19937 gen(rd());
        return gen;
    }

    void gen_random_salt()
    {
        std::uniform_int_distribution<unit_t> distrib(1, 0xFF);
        std::mt19937& gen = static_rand_engine();
        std::generate(random_salt.begin(), random_salt.end(), [&] { return distrib(gen); });
    }

    void gen_sequence()
    {
        std::uniform_int_distribution<unit_t> distrib(1, 0xFF);
        std::mt19937& gen = static_rand_engine();
        std::generate(sequence.begin(), sequence.end(), [&] { return distrib(gen); });
    }

    void gen()
    {
        gen_random_salt();
        gen_sequence();
    }

    void parse(unit_t *pos)
    {
        // |type|
        std::memcpy(std::addressof(type), pos, sizeof(type));
        pos += sizeof(type);

        // |key|
        std::memcpy(key.data(), pos, key.size());
        pos += key.size();

        // |sequence|
        std::memcpy(sequence.data(), pos, sequence.size());
        pos += sequence.size();

        // |random_salt|
        std::memcpy(random_salt.data(), pos, random_salt.size());
        pos += random_salt.size();

        // |datasize|
        std::memcpy(std::addressof(datasize), pos, sizeof(datasize));
        datasize = ntoh(datasize);
    }

    auto dump(unit_t *pos) -> unit_t*
    {
        // |type|
        std::memcpy(pos, std::addressof(type), sizeof(type));
        pos += sizeof(type);

        // |key|
        std::memcpy(pos, key.data(), key.size());
        pos += key.size();

        // |sequence|
        std::memcpy(pos, sequence.data(), sequence.size());
        pos += sequence.size();

        // |random_salt|
        std::memcpy(pos, random_salt.data(), random_salt.size());
        pos += random_salt.size();

        // |datasize|
        decltype(datasize) datasize_copy = hton(datasize);
        std::memcpy(pos, std::addressof(datasize_copy), sizeof(datasize_copy));
        return pos + sizeof(datasize_copy);
    }

    bool is_trigger() { return random_salt.back() == 0; } // if buf[-1] == 0 => is a trigger
};

struct packet_header_key_hash
{
    auto operator() (packet_header const& k) const -> std::size_t
    {
        std::size_t seed = 0x1b873593;
        hash_range(seed, k.key.begin(), k.key.end());
        hash_range(seed, k.random_salt.begin(), k.random_salt.end());
        return seed;
    }
};

struct packet_header_key_compare
{
    bool operator() (packet_header const& key1, packet_header const& key2) const
    {
        return (std::tie(key1.key, key1.random_salt) ==
                std::tie(key2.key, key2.random_salt));
    }
};

auto operator <<(std::ostream &os, packet_header const& pd) -> std::ostream&
{
    os << "[t=" << static_cast<int>(pd.type) << "|k=";
    for (key_t::value_type v: pd.key)
        os << std::hex << static_cast<int>(v);
    os << ",seq=";
    for (key_t::value_type v: pd.sequence)
        os << std::hex << static_cast<int>(v);
    os << ",salt=";
    for (key_t::value_type v: pd.random_salt)
        os << std::hex << static_cast<int>(v);
    os << "|d=" << pd.datasize << "]";
    return os;
}

struct packet_data
{
    std::vector<unit_t> buf;

    void parse(std::uint32_t const& size, unit_t *pos)
    {
        buf.resize(size);
        std::memcpy(buf.data(), pos, size);
    }

    auto dump(unit_t *pos) -> unit_t*
    {
        std::memcpy(pos, buf.data(), buf.size());
        return pos + buf.size();
    }
};


struct packet
{
    packet_header header;
    packet_data data;

    auto serialize() -> std::shared_ptr<std::vector<unit_t>>
    {
        header.datasize = data.buf.size();
        auto r = std::make_shared<std::vector<unit_t>>(packet_header::bytesize + header.datasize);

        unit_t* pos = header.dump(r->data());
        data.dump(pos);

        return r;
    }
};

using packet_pointer = std::shared_ptr<packet>;

} // namespace pack

#endif // CPP_SERIALIZER_OBJECTPACK_HPP__
