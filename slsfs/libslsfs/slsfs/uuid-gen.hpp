#pragma once
#ifndef SLSFS_UUID_GEN_HPP__
#define SLSFS_UUID_GEN_HPP__

#include "serializer.hpp"

#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>

#include <random>

namespace slsfs::uuid
{

auto get_uuid_str(std::string const & filename) -> std::string
{
    std::string digest = "slsfs-";
    CryptoPP::SHA256 hash;

    CryptoPP::StringSource ss( // no memleak here according to: https://stackoverflow.com/a/7045815/5921729
        filename, true,
        new CryptoPP::HashFilter(
            hash,
            new CryptoPP::Base64URLEncoder(
                new CryptoPP::StringSink(digest)
            )
        )
    );

    return digest;
}

auto gen_rand_str(std::size_t length) -> std::string
{
    static constexpr char const * allowed_chars = "123456789BCDFGHJKLMNPQRSTVWXZbcdfghjklmnpqrstvwxz";

    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist(0, sizeof(allowed_chars) - 1);

    std::string id(length, '\0');
    std::generate_n(id.begin(), length, [&]() { return allowed_chars[dist(rng)]; } );

    return id;
}

auto get_uuid(std::string const & filename) -> pack::key_t
{
    pack::key_t digest{};
    CryptoPP::SHA256 hash;

    static_assert(CryptoPP::SHA256::DIGESTSIZE == std::tuple_size<pack::key_t>::value);

    // enable to use reinterpret_cast<const char*>(const byte *) or the other round
    // please see https://stackoverflow.com/a/16261758/5921729
    static_assert(std::is_same_v<std::uint8_t, char> ||
                  std::is_same_v<std::uint8_t, unsigned char>,
                  "Make sure std::uint8_t is implemented as char or unsigned char.");

    hash.CalculateDigest(digest.data(), reinterpret_cast<const pack::unit_t *>(filename.data()), filename.size());
    //hash.CalculateDigest(digest.data(), id.data(), id.size());
    return digest;
}

auto to_string(pack::key_t const & p) -> std::string
{
    std::string digest = "slsfs-";
    CryptoPP::SHA256 hash;

    CryptoPP::StringSource ss( // no memleak here according to: https://stackoverflow.com/a/7045815/592172$
        p.data(), p.size(),
        true,
        new CryptoPP::HashFilter(
            hash,
            new CryptoPP::Base64URLEncoder(
                new CryptoPP::StringSink(digest)
                )
            )
        );

    return digest;
}

auto gen_rand(std::size_t length) -> std::vector<pack::unit_t>
{
    std::vector<pack::unit_t> id(length);
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist(0, 255);

    std::generate_n(id.begin(), length, [&]() { return dist(rng); } );

    return id;
}

int gen_rand_number()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist(0, 9999);

    return dist(rng);
}


} // namespace uuid

#endif // UUID_GEN_HPP__
