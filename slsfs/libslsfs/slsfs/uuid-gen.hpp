#pragma once

#ifndef UUID_GEN_HPP__
#define UUID_GEN_HPP__

#include "serializer.hpp"

#include <Poco/Crypto/DigestEngine.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Base64Decoder.h>

#include <sstream>
#include <random>
#include <vector>

namespace slsfs::uuid
{

struct uuid : public pack::key_t
{
    template<typename CharType = char>
    auto to_vector() const -> std::vector<CharType>
    {
        std::vector<CharType> v;
        v.reserve(pack::key_t::size());

        std::copy(pack::key_t::begin(), pack::key_t::end(), std::back_inserter(v));
        return v;
    }

    auto encode_base64() const -> std::string
    {
        std::stringstream ss;
        Poco::Base64Encoder encoder{ss};
        std::for_each(pack::key_t::begin(), pack::key_t::end(),
                      [&encoder] (auto c) {
                          static_assert(sizeof(c) == sizeof(char));
                          encoder << c;
                      });
        encoder.close();
        std::string raw_encode = ss.str();
        for (char& c : raw_encode)
            if (c == '/')
                c = '_';
        return raw_encode;
    }
};

struct hash_compare
{
    static
    auto hash (uuid const& id) -> std::size_t
    {
        std::size_t seed = 0x1b873594;
        pack::hash::range(seed, id.begin(), id.end());
        return seed;
    }

    static
    bool equal (uuid const& lhs, uuid const& rhs) {
        return lhs == rhs;
    }
};

inline
auto to_uuid(pack::key_t &key) -> uuid& {
    return *static_cast<uuid*>(&key);
}

inline
auto to_uuid(pack::key_t const &key) -> uuid const & {
    return *static_cast<uuid const*>(&key);
}

uuid get_uuid(std::string const& buffer)
{
    uuid id;
    Poco::Crypto::DigestEngine engine{"SHA256"};
    engine.update(buffer.data(), buffer.size());
    Poco::DigestEngine::Digest const& digest = engine.digest();

    std::copy(digest.begin(), digest.end(), id.begin());
    //std::cout << Poco::DigestEngine::digestToHex(digest) << "\n";
    return id;
}

uuid gen_uuid()
{
    static std::mt19937 rng;
    uuid id;
    std::random_device rd;
    Poco::Crypto::DigestEngine engine{"SHA256"};

    rng.seed(rd());
    int const r1 = rng();
    engine.update(&r1, sizeof(r1));

    rng.seed(rd());
    int const r2 = rng();
    engine.update(&r2, sizeof(r2));

    Poco::DigestEngine::Digest const& digest = engine.digest();

    std::copy(digest.begin(), digest.end(), id.begin());
//    std::cout << Poco::DigestEngine::digestToHex(digest) << "\n";
    return id;
}

auto encode_base64(pack::key_t const& key) -> std::string
{
    return to_uuid(key).encode_base64();
}

auto decode_base64(std::string& base64str) -> uuid
{
    std::stringstream ss;
    for (char& c : base64str)
        if (c == '_')
            c = '/';

    ss << base64str;
    Poco::Base64Decoder decoder {ss};

    uuid id;
    std::copy(std::istreambuf_iterator<char>(decoder),
              std::istreambuf_iterator<char>(),
              id.begin());
    return id;
}

auto operator << (std::ostream &os, uuid const& id) -> std::ostream&
{
    os << id.encode_base64();
    return os;
}

int gen_rand_number()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist(0, 9999);

    return dist(rng);
}


} // namespace uuid

#endif // UUID_GEN_HPP__



//namespace slsfs::uuid
//{
//
//auto get_uuid_str(std::string const & filename) -> std::string
//{
//    std::string digest = "slsfs-";
////    CryptoPP::SHA256 hash;
////
////    CryptoPP::StringSource ss( // no memleak here according to: https://stackoverflow.com/a/7045815/5921729
////        filename, true,
////        new CryptoPP::HashFilter(
////            hash,
////            new CryptoPP::Base64URLEncoder(
////                new CryptoPP::StringSink(digest)
////            )
////        )
////    );
//
//    return digest;
//}
//
//auto gen_rand_str(std::size_t length) -> std::string
//{
//    static constexpr char const * allowed_chars = "123456789BCDFGHJKLMNPQRSTVWXZbcdfghjklmnpqrstvwxz";
//
//    static thread_local std::mt19937 rng(std::random_device{}());
//    static thread_local std::uniform_int_distribution<int> dist(0, sizeof(allowed_chars) - 1);
//
//    std::string id(length, '\0');
//    std::generate_n(id.begin(), length, [&]() { return allowed_chars[dist(rng)]; } );
//
//    return id;
//}
//
//auto get_uuid(std::string const & filename) -> pack::key_t
//{
//    pack::key_t digest{};
//    CryptoPP::SHA256 hash;
//
//    static_assert(CryptoPP::SHA256::DIGESTSIZE == std::tuple_size<pack::key_t>::value);
//
//    // enable to use reinterpret_cast<const char*>(const byte *) or the other round
//    // please see https://stackoverflow.com/a/16261758/5921729
//    static_assert(std::is_same_v<std::uint8_t, char> ||
//                  std::is_same_v<std::uint8_t, unsigned char>,
//                  "Make sure std::uint8_t is implemented as char or unsigned char.");
//
//    hash.CalculateDigest(digest.data(), reinterpret_cast<const pack::unit_t *>(filename.data()), filename.size());
//    //hash.CalculateDigest(digest.data(), id.data(), id.size());
//    return digest;
//}
//
//auto to_string(pack::key_t const & p) -> std::string
//{
//
//    return digest;
//}
//
//auto gen_rand(std::size_t length) -> std::vector<pack::unit_t>
//{
//    std::vector<pack::unit_t> id(length);
//    static thread_local std::mt19937 rng(std::random_device{}());
//    static thread_local std::uniform_int_distribution<int> dist(0, 255);
//
//    std::generate_n(id.begin(), length, [&]() { return dist(rng); } );
//
//    return id;
//}
//
//int gen_rand_number()
//{
//    static thread_local std::mt19937 rng(std::random_device{}());
//    static thread_local std::uniform_int_distribution<int> dist(0, 9999);
//
//    return dist(rng);
//}
//
//
//} // namespace uuid
//
//#endif // UUID_GEN_HPP__
//
