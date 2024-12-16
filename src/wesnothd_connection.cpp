/*
	Copyright (C) 2011 - 2024
	by Sergey Popov <loonycyborg@gmail.com>
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
#include "gui/dialogs/loading_screen.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "serialization/parser.hpp"
#include "tls_root_store.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>

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
		WRN_NW << "Process:" << getpid() << " Thread:" << std::this_thread::get_id() << " Function: " << functionname << " Start";
	}
};
}
#define MPTEST_LOG mptest_log mptest_log__(__func__)
#else
#define MPTEST_LOG ((void)0)
#endif

using boost::system::error_code;
using boost::system::system_error;

using namespace std::chrono_literals; // s, ms, etc

// main thread
wesnothd_connection::wesnothd_connection(const std::string& host, const std::string& service)
	: worker_thread_()
	, io_context_()
	, resolver_(io_context_)
	, tls_context_(boost::asio::ssl::context::sslv23)
	, host_(host)
	, service_(service)
	, use_tls_(true)
	, socket_(raw_socket{ new raw_socket::element_type{io_context_} })
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

	error_code ec;
	auto result = resolver_.resolve(host, service, boost::asio::ip::resolver_query_base::numeric_host, ec);
	if(!ec) { // if numeric resolve succeeds then we got raw ip address so TLS host name validation would never pass
		use_tls_ = false;
		boost::asio::post(io_context_, [this, ec, result](){ handle_resolve(ec, { result } ); } );
	} else {
		resolver_.async_resolve(host, service,
			std::bind(&wesnothd_connection::handle_resolve, this, std::placeholders::_1, std::placeholders::_2));
	}

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
			DBG_NW << "wesnothd_connection worker thread threw general exception: " << utils::get_unknown_exception_type();
		}

		LOG_NW << "wesnothd_connection::io_service::run() returned";
	});

	LOG_NW << "Resolving hostname: " << host;
}

wesnothd_connection::~wesnothd_connection()
{
	MPTEST_LOG;

	if(auto socket = utils::get_if<tls_socket>(&socket_)) {
		error_code ec;
		// this sends close_notify for secure connection shutdown
		(*socket)->async_shutdown([](const error_code&) {} );
		const char buffer[] = "";
		// this write is needed to trigger immediate close instead of waiting for other side's close_notify
		boost::asio::write(**socket, boost::asio::buffer(buffer, 0), ec);
	}
	// Stop the io_service and wait for the worker thread to terminate.
	stop();
	worker_thread_.join();
}

// worker thread
void wesnothd_connection::handle_resolve(const error_code& ec, const results_type& results)
{
	MPTEST_LOG;
	if(ec) {
		LOG_NW << __func__ << " Throwing: " << ec;
		throw system_error(ec);
	}

	boost::asio::async_connect(*utils::get<raw_socket>(socket_), results,
		std::bind(&wesnothd_connection::handle_connect, this, std::placeholders::_1, std::placeholders::_2));
}

// worker thread
void wesnothd_connection::handle_connect(const boost::system::error_code& ec, endpoint endpoint)
{
	MPTEST_LOG;
	if(ec) {
		ERR_NW << "Tried all IPs. Giving up";
		throw system_error(ec);
	} else {
		LOG_NW << "Connected to " << endpoint.address();

		if(endpoint.address().is_loopback()) {
			use_tls_ = false;
		}
		handshake();
	}
}

// worker thread
void wesnothd_connection::handshake()
{
	MPTEST_LOG;

	DBG_NW << "Connecting with keepalive of: " << prefs::get().keepalive_timeout();
	set_keepalive(prefs::get().keepalive_timeout());

	static const uint32_t handshake = 0;
	static const uint32_t tls_handshake = htonl(uint32_t(1));

	boost::asio::async_write(*utils::get<raw_socket>(socket_), boost::asio::buffer(use_tls_ ? reinterpret_cast<const char*>(&tls_handshake) : reinterpret_cast<const char*>(&handshake), 4),
		[](const error_code& ec, std::size_t) { if(ec) { throw system_error(ec); } });
	boost::asio::async_read(*utils::get<raw_socket>(socket_), boost::asio::buffer(reinterpret_cast<std::byte*>(&handshake_response_), 4),
		std::bind(&wesnothd_connection::handle_handshake, this, std::placeholders::_1));
}

template<typename Verifier> auto verbose_verify(Verifier&& verifier)
{
	return [verifier](bool preverified, boost::asio::ssl::verify_context& ctx) {
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
		bool verified = verifier(preverified, ctx);
		DBG_NW << "Verifying TLS certificate: " << subject_name << ": " <<
			(verified ? "verified" : "failed");
		BIO* bio = BIO_new(BIO_s_mem());
		char buffer[1024];
		X509_print(bio, cert);
		while(BIO_read(bio, buffer, 1024) > 0)
		{
			DBG_NW << buffer;
		}
		BIO_free(bio);
		return verified;
	};
}

// worker thread
void wesnothd_connection::handle_handshake(const error_code& ec)
{
	MPTEST_LOG;
	if(ec) {
		if(ec == boost::asio::error::eof && use_tls_) {
			// immediate disconnect likely means old server not supporting TLS handshake code
			fallback_to_unencrypted();
			return;
		}
		LOG_NW << __func__ << " Throwing: " << ec;
		throw system_error(ec);
	}

	if(use_tls_) {
		if(handshake_response_ == 0xFFFFFFFFU) {
			use_tls_ = false;
			handle_handshake(ec);
			return;
		}

		if(handshake_response_ == 0x00000000) {
			network_asio::load_tls_root_certs(tls_context_);
			raw_socket s { std::move(utils::get<raw_socket>(socket_)) };
			tls_socket ts { new tls_socket::element_type{std::move(*s), tls_context_} };
			socket_ = std::move(ts);

			auto& socket { *utils::get<tls_socket>(socket_) };

			socket.set_verify_mode(
				boost::asio::ssl::verify_peer |
				boost::asio::ssl::verify_fail_if_no_peer_cert
			);

#if BOOST_VERSION >= 107300
			socket.set_verify_callback(verbose_verify(boost::asio::ssl::host_name_verification(host_)));
#else
			socket.set_verify_callback(verbose_verify(boost::asio::ssl::rfc2818_verification(host_)));
#endif

			socket.async_handshake(boost::asio::ssl::stream_base::client, [this](const error_code& ec) {
				if(ec) {
					LOG_NW << __func__ << " Throwing: " << ec;
					throw system_error(ec);
				}

				handshake_finished_.set_value();
				recv();
			});
			return;
		}

		fallback_to_unencrypted();
	} else {
		handshake_finished_.set_value();
		recv();
	}
}

// worker thread
void wesnothd_connection::fallback_to_unencrypted()
{
	assert(use_tls_ == true);
	use_tls_ = false;

	boost::asio::ip::tcp::endpoint endpoint { utils::get<raw_socket>(socket_)->remote_endpoint() };
	utils::get<raw_socket>(socket_)->close();

	utils::get<raw_socket>(socket_)->async_connect(endpoint,
		std::bind(&wesnothd_connection::handle_connect, this, std::placeholders::_1, endpoint));
}

// main thread
void wesnothd_connection::wait_for_handshake()
{
	MPTEST_LOG;
	LOG_NW << "Waiting for handshake";

	try {
		// TODO: make this duration customizable. Should default to 1 minute.
		auto timeout = 60s;

		auto future = handshake_finished_.get_future();
		for(auto time = 0ms;
			future.wait_for(10ms) == std::future_status::timeout
				&& time < timeout;
			time += 10ms)
		{
			gui2::dialogs::loading_screen::spin();
		}

		switch(future.wait_for(0ms)) {
		case std::future_status::ready:
			// This is a void future, so this just serves to re-throw any system_error exceptions
			// stored by the worker thread. Additional handling occurs in the catch block below.
			future.get();
			break;
		case std::future_status::timeout:
			throw error(boost::asio::error::make_error_code(boost::asio::error::timed_out));
		default:
			break;
		}
	} catch(const boost::system::system_error& err) {
		if(err.code() == boost::asio::error::operation_aborted || err.code() == boost::asio::error::eof) {
			return;
		}

		WRN_NW << __func__ << " Rethrowing: " << err.code();
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

	auto buf_ptr = std::make_unique<boost::asio::streambuf>();

	std::ostream os(buf_ptr.get());
	write_gz(os, request);

	boost::asio::post(io_context_, [this, buf_ptr = std::move(buf_ptr)]() mutable {

		DBG_NW << "In wesnothd_connection::send_data::lambda";
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
		if(socket->lowest_layer().is_open()) {
			boost::system::error_code ec;

#ifdef _MSC_VER
// Silence warning about boost::asio::basic_socket<Protocol>::cancel always
// returning an error on XP, which we don't support anymore.
#pragma warning(push)
#pragma warning(disable:4996)
#endif
		socket->lowest_layer().cancel(ec);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

			if(ec) {
				WRN_NW << "Failed to cancel network operations: " << ec.message();
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
			std::scoped_lock lock(last_error_mutex_);
			last_error_ = ec;
		}

		LOG_NW << __func__ << " Error: " << ec;

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
	DBG_NW << "Written " << bytes_transferred << " bytes.";

	send_queue_.pop();

	if(ec) {
		{
			std::scoped_lock lock(last_error_mutex_);
			last_error_ = ec;
		}

		LOG_NW << __func__ << " Error: " << ec;

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
			std::scoped_lock lock(last_error_mutex_);
			last_error_ = ec;
		}

		LOG_NW << __func__ << " Error: " << ec;

		io_context_.stop();
		return bytes_to_read_ - bytes_transferred;
	}

	bytes_read_ = bytes_transferred;

	if(bytes_transferred < 4) {
		return 4;
	}

	if(!bytes_to_read_) {
		std::istream is(&read_buf_);
		uint32_t data_size;

		is.read(reinterpret_cast<char*>(&data_size), 4);
		bytes_to_read_ = ntohl(data_size) + 4;

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
	DBG_NW << "Read " << bytes_transferred << " bytes.";

	bytes_to_read_ = 0;
	if(last_error_ && ec != boost::asio::error::eof) {
		{
			std::scoped_lock lock(last_error_mutex_);
			last_error_ = ec;
		}

		LOG_NW << __func__ << " Error: " << ec;

		io_context_.stop();
		return;
	}

	std::istream is(&read_buf_);
	config data;
	read_gz(data, is);
	if(!data.empty()) { DBG_NW << "Received:\n" << data; }

	{
		std::scoped_lock lock(recv_queue_mutex_);
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
		boost::asio::async_write(*socket, bufs,
			std::bind(&wesnothd_connection::is_write_complete, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&wesnothd_connection::handle_write, this, std::placeholders::_1, std::placeholders::_2));
	}, socket_);
}

// worker thread
void wesnothd_connection::recv()
{
	MPTEST_LOG;

	utils::visit([this](auto&& socket) {
		boost::asio::async_read(*socket, read_buf_,
			std::bind(&wesnothd_connection::is_read_complete, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&wesnothd_connection::handle_read, this, std::placeholders::_1, std::placeholders::_2));
	}, socket_);
}

// main thread
bool wesnothd_connection::receive_data(config& result)
{
	MPTEST_LOG;

	{
		std::scoped_lock lock(recv_queue_mutex_);
		if(!recv_queue_.empty()) {
			result.swap(recv_queue_.front());
			recv_queue_.pop();
			return true;
		}
	}

	{
		std::scoped_lock lock(last_error_mutex_);
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
		while(!recv_queue_lock_.wait_for(
		      lock, 10ms, [this]() { return has_data_received(); }))
		{
			gui2::dialogs::loading_screen::spin();
		}
	}

	return receive_data(data);
};

void wesnothd_connection::set_keepalive(int seconds)
{
	boost::asio::socket_base::keep_alive option(true);
	utils::get<raw_socket>(socket_)->set_option(option);

#ifdef __linux__
	int timeout = 10;
	int cnt = std::max((seconds - 10) / 10, 1);
	int interval = 10;
	setsockopt(utils::get<raw_socket>(socket_)->native_handle(), SOL_TCP, TCP_KEEPIDLE, &timeout, sizeof(timeout));
	setsockopt(utils::get<raw_socket>(socket_)->native_handle(), SOL_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
	setsockopt(utils::get<raw_socket>(socket_)->native_handle(), SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
#elif defined(__APPLE__) && defined(__MACH__)
	setsockopt(utils::get<raw_socket>(socket_)->native_handle(), IPPROTO_TCP, TCP_KEEPALIVE, &seconds, sizeof(seconds));
#elif defined(_WIN32)
	// these are in milliseconds for windows
	DWORD timeout_ms = seconds * 1000;
	setsockopt(utils::get<raw_socket>(socket_)->native_handle(), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms));
	setsockopt(utils::get<raw_socket>(socket_)->native_handle(), SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms));
#endif
}
