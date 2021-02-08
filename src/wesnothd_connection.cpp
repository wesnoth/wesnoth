/*
   Copyright (C) 2011 - 2018 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define BOOST_ASIO_NO_DEPRECATED

#include "wesnothd_connection.hpp"

#include "gettext.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>

#include <cstdint>
#include <deque>
#include <functional>

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

#if 0
// code for the travis test
#include <sys/types.h>
#include <unistd.h>
namespace {
struct mptest_log
{
	mptest_log(const char* functionname)
	{
		WRN_NW << "Process:" << getpid() << " Thread:" << std::this_thread::get_id() << " Function: " << functionname << " Start\n";
	}
};
}
#define MPTEST_LOG mptest_log mptest_log__(__func__)
#else
#define MPTEST_LOG ((void)0)
#endif

using boost::system::error_code;
using boost::system::system_error;

// main thread
wesnothd_connection::wesnothd_connection(const std::string& host, const std::string& service)
	: worker_thread_()
	, io_context_()
	, resolver_(io_context_)
	, tls_context_(boost::asio::ssl::context::sslv23)
	, host_(host)
	, service_(service)
	, socket_(raw_socket{io_context_})
	, last_error_()
	, last_error_mutex_()
	, handshake_finished_()
	, read_buf_()
	, handshake_response_()
	, recv_queue_()
	, recv_queue_mutex_()
	, recv_queue_lock_()
	, payload_size_(0)
	, bytes_to_write_(0)
	, bytes_written_(0)
	, bytes_to_read_(0)
	, bytes_read_(0)
{
	MPTEST_LOG;
#if BOOST_VERSION >= 106600
	resolver_.async_resolve(host, service,
#else
	resolver_.async_resolve(boost::asio::ip::tcp::resolver::query(host, service),
#endif
		std::bind(&wesnothd_connection::handle_resolve, this, std::placeholders::_1, std::placeholders::_2));

	// Starts the worker thread. Do this *after* the above async_resolve call or it will just exit immediately!
	worker_thread_ = std::thread([this]() {
		try {
			io_context_.run();
		} catch(const boost::system::system_error&) {
			try {
				// Attempt to pass the exception on to the handshake promise.
				handshake_finished_.set_exception(std::current_exception());
			} catch(const std::future_error&) {
				// Handshake already complete. Do nothing.
			}
		} catch(...) {
		}

		LOG_NW << "wesnothd_connection::io_service::run() returned\n";
	});

	LOG_NW << "Resolving hostname: " << host << '\n';
}

wesnothd_connection::~wesnothd_connection()
{
	MPTEST_LOG;

	// Stop the io_service and wait for the worker thread to terminate.
	stop();
	worker_thread_.join();
}

// worker thread
void wesnothd_connection::handle_resolve(const error_code& ec, results_type results)
{
	MPTEST_LOG;
	if(ec) {
		LOG_NW << __func__ << " Throwing: " << ec << "\n";
		throw system_error(ec);
	}

	boost::asio::async_connect(utils::get<raw_socket>(socket_), results,
		std::bind(&wesnothd_connection::handle_connect, this, std::placeholders::_1, std::placeholders::_2));
}

// worker thread
void wesnothd_connection::handle_connect(const boost::system::error_code& ec, endpoint endpoint)
{
	MPTEST_LOG;
	if(ec) {
		ERR_NW << "Tried all IPs. Giving up" << std::endl;
		throw system_error(ec);
	} else {
#if BOOST_VERSION >= 106600
		LOG_NW << "Connected to " << endpoint.address() << '\n';
#else
		LOG_NW << "Connected to " << endpoint->endpoint().address() << '\n';
#endif
		handshake();
	}
}

// worker thread
void wesnothd_connection::handshake()
{
	MPTEST_LOG;
	static const uint32_t handshake = 0;
	static const uint32_t tls_handshake = htonl(uint32_t(1));

	boost::asio::async_write(utils::get<raw_socket>(socket_), boost::asio::buffer(use_tls_ ? reinterpret_cast<const char*>(&tls_handshake) : reinterpret_cast<const char*>(&handshake), 4),
		[](const error_code& ec, std::size_t) { if(ec) { throw system_error(ec); } });
	boost::asio::async_read(utils::get<raw_socket>(socket_), boost::asio::buffer(&handshake_response_.binary, 4),
		std::bind(&wesnothd_connection::handle_handshake, this, std::placeholders::_1));
}

// worker thread
void wesnothd_connection::handle_handshake(const error_code& ec)
{
	MPTEST_LOG;
	if(ec) {
		if(ec == boost::asio::error::eof) {
			throw std::runtime_error("Failed to complete handshake with server");
		}
		LOG_NW << __func__ << " Throwing: " << ec << "\n";
		throw system_error(ec);
	}
	
	if(use_tls_) {
		if(handshake_response_.num == 0xFFFFFFFFU) {
			throw std::runtime_error("The server doesn't support TLS");
		}

		if(handshake_response_.num == 0x00000000) {
			tls_context_.set_default_verify_paths();
			raw_socket s { std::move(utils::get<raw_socket>(socket_)) };
			socket_.emplace<1>(std::move(s), tls_context_);
			
			auto& socket { utils::get<tls_socket>(socket_) };

			socket.set_verify_mode(
				boost::asio::ssl::verify_peer |
				boost::asio::ssl::verify_fail_if_no_peer_cert
			);

			socket.set_verify_callback(boost::asio::ssl::host_name_verification(host_));

			socket.async_handshake(boost::asio::ssl::stream_base::client, [this](const error_code& ec) {
				if(ec) {
					LOG_NW << __func__ << " Throwing: " << ec << "\n";
					throw system_error(ec);
				}

				handshake_finished_.set_value();
				recv();
			});
			return;
		}

		throw std::runtime_error("Invalid handshake");
	} else {
		handshake_finished_.set_value();
		recv();
	}
}

// main thread
void wesnothd_connection::wait_for_handshake()
{
	MPTEST_LOG;
	LOG_NW << "Waiting for handshake" << std::endl;

	try {
		handshake_finished_.get_future().get();
	} catch(const boost::system::system_error& err) {
		if(err.code() == boost::asio::error::operation_aborted || err.code() == boost::asio::error::eof) {
			return;
		}

		WRN_NW << __func__ << " Rethrowing: " << err.code() << "\n";
		throw error(err.code());
	} catch(const std::future_error& e) {
		if(e.code() == std::future_errc::future_already_retrieved) {
			return;
		}
	}
}

// main thread
void wesnothd_connection::send_data(const configr_of& request)
{
	MPTEST_LOG;

#if BOOST_VERSION >= 106600
	auto buf_ptr = std::make_unique<boost::asio::streambuf>();
#else
	auto buf_ptr = std::make_shared<boost::asio::streambuf>();
#endif

	std::ostream os(buf_ptr.get());
	write_gz(os, request);

	// No idea why io_context::post doesn't like this lambda while asio::post does.
#if BOOST_VERSION >= 106600
	boost::asio::post(io_context_, [this, buf_ptr = std::move(buf_ptr)]() mutable {
#else
	io_context_.post([this, buf_ptr]() {
#endif
		DBG_NW << "In wesnothd_connection::send_data::lambda\n";
		send_queue_.push(std::move(buf_ptr));

		if(send_queue_.size() == 1) {
			send();
		}
	});
}

// main thread
void wesnothd_connection::cancel()
{
	MPTEST_LOG;
	utils::visit([](auto&& socket) {
		if(socket.lowest_layer().is_open()) {
			boost::system::error_code ec;

#ifdef _MSC_VER
// Silence warning about boost::asio::basic_socket<Protocol>::cancel always
// returning an error on XP, which we don't support anymore.
#pragma warning(push)
#pragma warning(disable:4996)
#endif
		socket.lowest_layer().cancel(ec);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

			if(ec) {
				WRN_NW << "Failed to cancel network operations: " << ec.message() << std::endl;
			}
		}
	}, socket_);
}

// main thread
void wesnothd_connection::stop()
{
	// TODO: wouldn't cancel() have the same effect?
	MPTEST_LOG;
	io_context_.stop();
}

// worker thread
std::size_t wesnothd_connection::is_write_complete(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	MPTEST_LOG;
	if(ec) {
		{
			std::lock_guard lock(last_error_mutex_);
			last_error_ = ec;
		}

		LOG_NW << __func__ << " Error: " << ec << "\n";

		io_context_.stop();
		return bytes_to_write_ - bytes_transferred;
	}

	bytes_written_ = bytes_transferred;
	return bytes_to_write_ - bytes_transferred;
}

// worker thread
void wesnothd_connection::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	MPTEST_LOG;
	DBG_NW << "Written " << bytes_transferred << " bytes.\n";

	send_queue_.pop();

	if(ec) {
		{
			std::lock_guard lock(last_error_mutex_);
			last_error_ = ec;
		}

		LOG_NW << __func__ << " Error: " << ec << "\n";

		io_context_.stop();
		return;
	}

	if(!send_queue_.empty()) {
		send();
	}
}

// worker thread
std::size_t wesnothd_connection::is_read_complete(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	// We use custom is_write/read_complete function to be able to see the current progress of the upload/download
	MPTEST_LOG;
	if(ec) {
		{
			std::lock_guard lock(last_error_mutex_);
			last_error_ = ec;
		}

		LOG_NW << __func__ << " Error: " << ec << "\n";

		io_context_.stop();
		return bytes_to_read_ - bytes_transferred;
	}

	bytes_read_ = bytes_transferred;

	if(bytes_transferred < 4) {
		return 4;
	}

	if(!bytes_to_read_) {
		std::istream is(&read_buf_);
		data_union data_size;

		is.read(data_size.binary, 4);
		bytes_to_read_ = ntohl(data_size.num) + 4;

		// Close immediately if we receive an invalid length
		if(bytes_to_read_ < 4) {
			bytes_to_read_ = bytes_transferred;
		}
	}

	return bytes_to_read_ - bytes_transferred;
}

// worker thread
void wesnothd_connection::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	MPTEST_LOG;
	DBG_NW << "Read " << bytes_transferred << " bytes.\n";

	bytes_to_read_ = 0;
	if(last_error_ && ec != boost::asio::error::eof) {
		{
			std::lock_guard lock(last_error_mutex_);
			last_error_ = ec;
		}

		LOG_NW << __func__ << " Error: " << ec << "\n";

		io_context_.stop();
		return;
	}

	std::istream is(&read_buf_);
	config data;
	read_gz(data, is);
	if(!data.empty()) { DBG_NW << "Received:\n" << data; }

	{
		std::lock_guard lock(recv_queue_mutex_);
		recv_queue_.emplace(std::move(data));
		recv_queue_lock_.notify_all();
	}

	recv();
}

// worker thread
void wesnothd_connection::send()
{
	MPTEST_LOG;
	auto& buf = *send_queue_.front();

	std::size_t buf_size = buf.size();
	bytes_to_write_ = buf_size + 4;
	bytes_written_ = 0;
	payload_size_ = htonl(buf_size);

	std::deque<boost::asio::const_buffer> bufs {
		boost::asio::buffer(reinterpret_cast<const char*>(&payload_size_), 4),
		buf.data()
	};

	utils::visit([this, &bufs](auto&& socket) {
		boost::asio::async_write(socket, bufs,
			std::bind(&wesnothd_connection::is_write_complete, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&wesnothd_connection::handle_write, this, std::placeholders::_1, std::placeholders::_2));
	}, socket_);
}

// worker thread
void wesnothd_connection::recv()
{
	MPTEST_LOG;

	utils::visit([this](auto&& socket) {
		boost::asio::async_read(socket, read_buf_,
			std::bind(&wesnothd_connection::is_read_complete, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&wesnothd_connection::handle_read, this, std::placeholders::_1, std::placeholders::_2));
	}, socket_);
}

// main thread
bool wesnothd_connection::receive_data(config& result)
{
	MPTEST_LOG;

	{
		std::lock_guard lock(recv_queue_mutex_);
		if(!recv_queue_.empty()) {
			result.swap(recv_queue_.front());
			recv_queue_.pop();
			return true;
		}
	}

	{
		std::lock_guard lock(last_error_mutex_);
		if(last_error_) {
			std::string user_msg;

			if(last_error_ == boost::asio::error::eof) {
				user_msg = _("Disconnected from server.");
			}

			throw error(last_error_, user_msg);
		}
	}

	return false;
}

bool wesnothd_connection::wait_and_receive_data(config& data)
{
	{
		std::unique_lock<std::mutex> lock(recv_queue_mutex_);
		recv_queue_lock_.wait(lock, [this]() { return has_data_received(); });
	}

	return receive_data(data);
};
