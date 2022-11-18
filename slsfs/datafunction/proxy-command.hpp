#pragma once

#ifndef PROXY_COMMAND_HPP__
#define PROXY_COMMAND_HPP__

#include <oneapi/tbb/concurrent_hash_map.h>
#include <boost/signals2.hpp>

#include <slsfs.hpp>

namespace slsfs::server
{

namespace
{
    using boost::asio::ip::tcp;
}

using queue_map = oneapi::tbb::concurrent_hash_map<uuid::uuid,
                                                   std::shared_ptr<boost::asio::io_context::strand>,
                                                   uuid::hash_compare>;
using queue_map_accessor = queue_map::accessor;

//std::invocable<slsfs::base::buf(slsfsdf::storage_conf*, jsre::request_parser<base::byte> const&)>;
template<typename Func>
concept StorageOperationConcept = requires(Func func)
{
    { std::invoke(func,
                  std::declval<slsfsdf::storage_conf*>(),
                  std::declval<jsre::request_parser<base::byte> const&>()) }
    -> std::convertible_to<slsfs::base::buf>;
};

class proxy_command;

using proxy_set = oneapi::tbb::concurrent_hash_map<std::shared_ptr<proxy_command>, int /* unused */>;

class proxy_command : public std::enable_shared_from_this<proxy_command>
{
    boost::asio::io_context&        io_context_;
    boost::asio::ip::tcp::socket    socket_;
    boost::asio::io_context::strand write_io_strand_;
    boost::asio::steady_timer       recv_deadline_;
    queue_map queue_map_;
    boost::signals2::signal<slsfs::base::buf(slsfsdf::storage_conf*, jsre::request_parser<base::byte> const&)> storage_perform_;
    proxy_set proxy_set_;

    void timer_reset()
    {
        using namespace std::chrono_literals;
        recv_deadline_.async_wait(
            [self=this->shared_from_this()] (boost::system::error_code ec) {
                if (not ec)
                {
                    log::logstring<log::level::info>("read header timeout");

                    pack::packet_pointer pack = std::make_shared<pack::packet>();
                    pack->header.type = pack::msg_t::worker_dereg;
                    self->start_write(
                        pack,
                        [self=self->shared_from_this()] (boost::system::error_code ec, std::size_t length) {
                            self->socket_.shutdown(tcp::socket::shutdown_receive, ec);
                            log::logstring("send shutdown");
                        });
                }
        });
        recv_deadline_.expires_from_now(1s);
    }

public:
    template<StorageOperationConcept StorageOperation>
    proxy_command(boost::asio::io_context& io_context,
                  queue_map& qm,
                  StorageOperation op,
                  proxy_set& ps)
        : io_context_{io_context},
          socket_{io_context_},
          write_io_strand_{io_context_},
          recv_deadline_{io_context_},
          queue_map_{qm},
          proxy_set_{ps} {
        storage_perform_.connect(op);
    }

    void start_connect(boost::asio::ip::tcp::resolver::results_type endpoint)
    {
        boost::asio::async_connect(
            socket_,
            endpoint,
            [self=shared_from_this()] (boost::system::error_code const & ec, tcp::endpoint const& endpoint) {
                self->socket_.set_option(tcp::no_delay(true));
                if (ec)
                    slsfs::log::logstring(fmt::format("connect to proxy error: {}", ec.message()));
                else
                {
                    slsfs::pack::packet_pointer ptr = std::make_shared<slsfs::pack::packet>();
                    ptr->header.type = slsfs::pack::msg_t::worker_reg;
                    ptr->header.gen();

                    self->start_write(ptr);
                    self->start_listen_commands();
                }
            });
    }

    void start_listen_commands()
    {
        slsfs::log::logstring("start_listen_commands called");
        timer_reset();
        auto readbuf = std::make_shared<std::array<slsfs::pack::unit_t, slsfs::pack::packet_header::bytesize>>();

        boost::asio::async_read(
            socket_, boost::asio::buffer(readbuf->data(), readbuf->size()),
            [self=this->shared_from_this(), readbuf] (boost::system::error_code ec, std::size_t /*length*/) {
                self->recv_deadline_.cancel();
                slsfs::log::logstring("start_listen_commands get cmd");

                if (not ec)
                {
                    slsfs::log::logstring<slsfs::log::level::debug>("get cmd");
                    slsfs::pack::packet_pointer pack = std::make_shared<slsfs::pack::packet>();
                    pack->header.parse(readbuf->data());
                    self->start_listen_commands_body(pack);
                }
                else
                {
                    std::stringstream ss;
                    ss << ec.message();
                    slsfs::log::logstring(std::string("error listen command ") + ss.str());
                }
            });
    }

    void start_listen_commands_body(slsfs::pack::packet_pointer pack)
    {
        auto read_buf = std::make_shared<std::vector<slsfs::pack::unit_t>>(pack->header.datasize);
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(read_buf->data(), read_buf->size()),
            [self=this->shared_from_this(), read_buf, pack] (boost::system::error_code ec, std::size_t length) {
                if (not ec)
                {
                    pack->data.parse(length, read_buf->data());

                    if (pack->header.type == slsfs::pack::msg_t::proxyjoin)
                    {
                        slsfs::log::logstring("add connection");
                    }
                    else
                        self->start_job(pack);

                    slsfs::pack::packet_pointer ok = std::make_shared<slsfs::pack::packet>();
                    ok->header = pack->header;
                    ok->header.type = slsfs::pack::msg_t::ack;

                    self->start_write(ok);
                    self->start_listen_commands();
                }
                else
                    slsfs::log::logstring<slsfs::log::level::error>("start_listen_commands_body: "); // + ec.messag$
            });
    }

    void start_write(slsfs::pack::packet_pointer pack)
    {
        start_write(pack, [](boost::system::error_code, std::size_t) {});
    }

    template<typename Func>
    void start_write(slsfs::pack::packet_pointer pack, Func next)
    {
        auto buf_pointer = pack->serialize();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(buf_pointer->data(), buf_pointer->size()),
            boost::asio::bind_executor(
                write_io_strand_,
                [self=this->shared_from_this(), buf_pointer, next] (boost::system::error_code ec, std::size_t length) {
                    if (not ec)
                        slsfs::log::logstring("worker sent msg");
                    std::invoke(next, ec, length);
                }));
    }

    void start_job(slsfs::pack::packet_pointer pack)
    {
        queue_map_accessor it;
        boost::asio::io_context::strand * ptr = nullptr;
        if (not queue_map_.find(it, uuid::to_uuid(pack->header.key)))
        {
            auto new_strand = std::make_unique<boost::asio::io_context::strand>(io_context_);
            ptr = new_strand.get();
            queue_map_.emplace(uuid::to_uuid(pack->header.key), std::move(new_strand));
        }
        else
            ptr = it->second.get();

        boost::asio::post(
            boost::asio::bind_executor(
                *ptr,
                [self=this->shared_from_this(), pack] {
                    //self->start_write(pack);
                    pack->header.type = slsfs::pack::msg_t::worker_response;

                    auto const start = std::chrono::high_resolution_clock::now();

                    slsfs::jsre::request_parser<slsfs::base::byte> input {pack->data.buf.data()};
                    slsfsdf::storage_conf* datastorage = slsfsdf::get_thread_local_datastorage().get();
                    slsfs::base::buf v = self->storage_perform_(datastorage, input).get_value_or({});

                    pack->header.type = slsfs::pack::msg_t::worker_response;
                    pack->data.buf.resize(v.size());// = std::vector<slsfs::pack::unit_t>(v.size(), '\0');
                    std::memcpy(pack->data.buf.data(), v.data(), v.size());
                    auto const end = std::chrono::high_resolution_clock::now();

                    auto relativetime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                    slsfs::log::logstring<slsfs::log::level::info>(fmt::format("req finish in: {}", relativetime));
                }
            )
        );
    }
};

} // namespace slsfs::server

#endif // PROXY_COMMAND_HPP__
