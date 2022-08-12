#pragma once

#ifndef SLSFS_STORAGE_SSBD_HPP__
#define SLSFS_STORAGE_SSBD_HPP__

#include "storage.hpp"
#include "basetypes.hpp"
#include "scope-exit.hpp"
#include "rocksdb-serializer.hpp"
#include "debuglog.hpp"

#include <boost/asio.hpp>

namespace slsfs::storage
{

namespace
{
    using boost::asio::ip::tcp;
} // namespace

class ssbd : public interface
{
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    char const * host_;
    char const * port_;

public:
    ssbd(boost::asio::io_context& io, char const * host, char const * port):
        io_context_{io}, socket_(io), host_{host}, port_{port} {}

    void connect() override
    {
        tcp::resolver resolver(io_context_);
        boost::asio::connect(socket_, resolver.resolve(host_, port_));
    }

    auto read_key(std::string const name, std::size_t partition,
                  std::size_t location, std::size_t size) -> base::buf override
    {
        { // send get
            rocksdb_pack::packet_pointer ptr = std::make_shared<rocksdb_pack::packet>();
            ptr->header.type = rocksdb_pack::msg_t::get;
            std::copy(name.begin(), name.end(), ptr->header.uuid.begin());

            ptr->header.blockid = partition;
            ptr->header.position = location;
            ptr->header.datasize = size;
            auto buf = ptr->serialize_header();
            boost::asio::write(socket_, boost::asio::buffer(buf->data(), buf->size()));
        }

        { // read resp
            rocksdb_pack::packet_pointer resp = std::make_shared<rocksdb_pack::packet>();
            std::vector<rocksdb_pack::unit_t> headerbuf(rocksdb_pack::packet_header::bytesize);
            boost::asio::read(socket_, boost::asio::buffer(headerbuf.data(), headerbuf.size()));

            resp->header.parse(headerbuf.data());

            std::string bodybuf(resp->header.datasize, 0);
            boost::asio::read(socket_, boost::asio::buffer(bodybuf.data(), bodybuf.size()));
            log::logstring(bodybuf);
        } // read resp
        return {};
    }

    bool check_version_ok(std::string const name, std::size_t partition,
                          std::uint32_t &version) override
    {
        // request commit
        rocksdb_pack::packet_pointer ptr = std::make_shared<rocksdb_pack::packet>();
        ptr->header.type = rocksdb_pack::msg_t::merge_request_commit;
        std::copy(name.begin(), name.end(), ptr->header.uuid.begin());
        ptr->header.blockid = partition;

        version = rocksdb_pack::hton(version);
        ptr->data.buf = std::vector<rocksdb_pack::unit_t> (sizeof(version));
        std::memcpy(ptr->data.buf.data(), &version, sizeof(version));

        auto buf = ptr->serialize();
        boost::asio::write(socket_, boost::asio::buffer(buf->data(), buf->size()));

        // read resp
        rocksdb_pack::packet_pointer resp = std::make_shared<rocksdb_pack::packet>();
        std::vector<rocksdb_pack::unit_t> headerbuf(rocksdb_pack::packet_header::bytesize);

        boost::asio::read(socket_, boost::asio::buffer(headerbuf.data(), headerbuf.size()));
        resp->header.parse(headerbuf.data());

        log::logstring("send_kafka start");

        std::remove_reference_t<decltype(version)> updated_version;
        assert(resp->header.datasize == sizeof(updated_version));
        boost::asio::read(socket_,
                          boost::asio::buffer(std::addressof(updated_version), sizeof(updated_version)));
        updated_version = rocksdb_pack::hton(updated_version);

        switch(resp->header.type)
        {
        case rocksdb_pack::msg_t::merge_vote_agree:
            return true;

        case rocksdb_pack::msg_t::merge_vote_abort:
            version = updated_version;
            return false;

        default:
            log::logstring("unwanted header type ");
            return false;
        }
    };

    void write_key(std::string const name, std::size_t partition,
                   base::buf const& buffer, std::size_t location,
                   std::uint32_t version) override
    {
        rocksdb_pack::packet_pointer ptr = std::make_shared<rocksdb_pack::packet>();
        ptr->header.type = rocksdb_pack::msg_t::merge_execute_commit;
        std::copy(name.begin(), name.end(), ptr->header.uuid.begin());
        ptr->header.blockid = partition;
        ptr->header.position = location;

        version = rocksdb_pack::hton(version);
        ptr->data.buf = std::vector<rocksdb_pack::unit_t> (sizeof(version) + buffer.size());
        std::memcpy(ptr->data.buf.data(), &version, sizeof(version));
        std::copy(buffer.begin(), buffer.end(),
                  std::next(ptr->data.buf.begin(), sizeof(version)));

        auto buf = ptr->serialize();
        boost::asio::write(socket_, boost::asio::buffer(buf->data(), buf->size()));
    }

    void append_list_key(std::string const name, base::buf const& buffer) override
    {
    }

    void merge_list_key(std::string const name, std::function<void(std::vector<base::buf> const&)> reduce) override
    {
    }

    auto  get_list_key(std::string const name) -> base::buf override
    {
        return {};
    }
};

} // namespace storage

#endif // SLSFS_STORAGE_SSBD_HPP__
