#pragma once

#ifndef WORKER_HPP__
#define WORKER_HPP__

#include <slsfs.hpp>

namespace slsfsdf
{

using boost::asio::ip::tcp;

class worker : public std::enable_shared_from_this<worker>
{
    boost::asio::io_context& io_;
    boost::asio::io_context::strand write_io_strand_;
    tcp::socket &socket_;
    std::function<void(slsfs::pack::packet_pointer)> job_;
    boost::asio::steady_timer recv_deadline_;

public:
    template<typename Func>
    worker(boost::asio::io_context& io, tcp::socket& s, Func job):
        io_{io}, write_io_strand_{io}, socket_{s}, job_{job}, recv_deadline_{io} {}

    void timer_reset()
    {
        using namespace std::chrono_literals;
        recv_deadline_.async_wait(
            [self=this->shared_from_this()] (boost::system::error_code ec) {
                if (not ec)
                {
                    slsfs::log::logstring("read header timeout");

                    slsfs::pack::packet_pointer pack = std::make_shared<slsfs::pack::packet>();
                    pack->header.type = slsfs::pack::msg_t::worker_dereg;
                    self->start_write(
                        pack,
                        [self=self->shared_from_this()] (boost::system::error_code ec, std::size_t /* lenght */) {
                            self->socket_.shutdown(tcp::socket::shutdown_receive, ec);
                            slsfs::log::logstring("send shutdown");
                        });
                }
        });
        recv_deadline_.expires_from_now(1s);
    }

    void start_listen_commands()
    {
        slsfs::log::logstring("start_listen_commands");
        timer_reset();
        auto readbuf = std::make_shared<std::array<slsfs::pack::unit_t, slsfs::pack::packet_header::bytesize>>();
        boost::asio::async_read(
            socket_, boost::asio::buffer(readbuf->data(), readbuf->size()),
            [self=this->shared_from_this(), readbuf] (boost::system::error_code ec, std::size_t /*length*/) {
                self->recv_deadline_.cancel();
                slsfs::log::logstring<slsfs::log::level::info>("start_listen_commands get cmd");

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
                    self->start_job(pack);

                    slsfs::pack::packet_pointer ok = std::make_shared<slsfs::pack::packet>();
                    ok->header = pack->header;
                    ok->header.type = slsfs::pack::msg_t::ack;

                    self->start_write(ok);
                    self->start_listen_commands();
                }
                else
                    slsfs::log::logstring<slsfs::log::level::error>("start_listen_commands_body: "); // + ec.message()
            });
    }

    void start_write(slsfs::pack::packet_pointer pack)
    {
        auto buf_pointer = pack->serialize();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(buf_pointer->data(), buf_pointer->size()),
            boost::asio::bind_executor(
                write_io_strand_,
                [self=this->shared_from_this(), buf_pointer] (boost::system::error_code ec, std::size_t /* lenght */) {
                    if (not ec)
                        slsfs::log::logstring("worker sent msg");
                }));
    }

    template<typename Function>
    void start_write(slsfs::pack::packet_pointer pack, Function next)
    {
        auto buf_pointer = pack->serialize();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(buf_pointer->data(), buf_pointer->size()),
            boost::asio::bind_executor(write_io_strand_, next));
    }

    void start_job(slsfs::pack::packet_pointer pack)
    {
        boost::asio::post(
            io_,
            [self=this->shared_from_this(), pack] {
                slsfs::log::logstring<slsfs::log::level::debug>("Start Job");
                std::invoke(self->job_, pack);
                self->start_write(pack);
            });
    }
};

} // namespace slsfsdf
#endif // WORKER_HPP__
