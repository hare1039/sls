#pragma once
#ifndef CPP_ROCKSDB_SERIALIZER_OBJECTPACK_HPP__
#define CPP_ROCKSDB_SERIALIZER_OBJECTPACK_HPP__

#include <arpa/inet.h>

#include <ios>
#include <iostream>
#include <vector>
#include <memory>
#include <cstring>
#include <array>
#include <bitset>
#include <tuple>
#include <algorithm>
#include <random>

namespace rocksdb_pack
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

using unit_t = char; // exp
static_assert(sizeof(char) == 8/8);

// key = [32] byte main key // sha256 bit
using key_t = std::array<unit_t, 256 / 8 / sizeof(unit_t)>;
enum class msg_t: unit_t
{
    err = 0b00000000,
    ack = 0b00000001,
    get = 0b00000010,
    merge_ack_commit     = 0b00001000,
    merge_request_commit = 0b00001001,
    merge_vote_agree     = 0b00001010,
    merge_vote_abort     = 0b00001011,
    merge_execute_commit = 0b00001100,
    merge_rollback_commit= 0b00001101,

};

auto operator << (std::ostream &os, msg_t const& msg) -> std::ostream&
{
    using under_t = std::underlying_type<msg_t>::type;
    std::bitset<sizeof(under_t) * 8> m (static_cast<char>(msg));
    os << m;
    return os;
}

bool is_merge_request(msg_t msg) {
    return static_cast<unit_t>(msg) & 0b00001000;
}

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
    key_t uuid;
    std::uint32_t blockid;
    std::uint16_t position;
    std::uint32_t datasize;

    static constexpr int bytesize =
        sizeof(type) +
        std::tuple_size<decltype(uuid)>::value +
        sizeof(blockid) +
        sizeof(position) +
        sizeof(datasize);

    auto as_string() -> std::string
    {
        std::string key;
        std::copy(uuid.begin(), uuid.end(), std::back_inserter(key));
        key += std::to_string(blockid);
        return key;
    }

    void parse(unit_t *pos)
    {
        // |type|
        std::memcpy(std::addressof(type), pos, sizeof(type));
        pos += sizeof(type);

        // |uuid|
        std::memcpy(uuid.data(), pos, uuid.size());
        pos += uuid.size();

        // |blockid|
        std::memcpy(std::addressof(blockid), pos, sizeof(blockid));
        pos += sizeof(blockid);
        blockid = ntoh(blockid);

        // |position|
        std::memcpy(std::addressof(position), pos, sizeof(position));
        pos += sizeof(position);
        position = ntoh(position);

        // |datasize|
        std::memcpy(std::addressof(datasize), pos, sizeof(datasize));
        pos += sizeof(datasize);
        datasize = ntoh(datasize);
    }

    auto dump(unit_t *pos) -> unit_t*
    {
        // |type|
        std::memcpy(pos, std::addressof(type), sizeof(type));
        pos += sizeof(type);

        // |uuid|
        std::memcpy(pos, uuid.data(), uuid.size());
        pos += uuid.size();

        // |blockid|
        decltype(blockid) blockid_copy = hton(blockid);
        std::memcpy(pos, std::addressof(blockid_copy), sizeof(blockid_copy));
        pos += sizeof(blockid_copy);

        // |position|
        decltype(position) position_copy = hton(position);
        std::memcpy(pos, std::addressof(position_copy), sizeof(position_copy));
        pos += sizeof(position_copy);

        // |datasize|
        decltype(datasize) datasize_copy = hton(datasize);
        std::memcpy(pos, std::addressof(datasize_copy), sizeof(datasize_copy));
        return pos + sizeof(datasize_copy);
    }
};

struct packet_header_key_hash
{
    auto operator() (packet_header const& k) const -> std::size_t
    {
        std::size_t seed = 0x1b873593 + k.blockid;
        hash_range(seed, k.uuid.begin(), k.uuid.end());
        return seed;
    }
};

struct packet_header_key_compare
{
    bool operator() (packet_header const& key1, packet_header const& key2) const
    {
        return (std::tie(key1.uuid, key1.blockid) ==
                std::tie(key2.uuid, key2.blockid));
    }
};

auto operator << (std::ostream &os, packet_header const& pd) -> std::ostream&
{
    os << "[t=" << pd.type << "|k=";
    for (key_t::value_type v: pd.uuid)
        os << std::hex << static_cast<int>(v);
    os << ",blkid=" << std::hex << pd.blockid;
    os << ",position=" << std::hex << pd.position;
    os << "|datasize=" << pd.datasize << "]";
    return os;
}

struct packet_data
{
    std::vector<unit_t> buf;

    auto as_string() -> std::string
    {
        std::string data;
        std::copy(buf.begin(), buf.end(), std::back_inserter(data));
        return data;
    }

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

    auto serialize_header() -> std::shared_ptr<std::vector<unit_t>>
    {
        auto r = std::make_shared<std::vector<unit_t>>(packet_header::bytesize);
        header.dump(r->data());
        return r;
    }
};

using packet_pointer = std::shared_ptr<packet>;

} // namespace pack

#endif // CPP_ROCKSDB_SERIALIZER_OBJECTPACK_HPP__
