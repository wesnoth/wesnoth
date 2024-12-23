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

#include "network_asio.hpp"

#include "log.hpp"
#include "serialization/parser.hpp"
#include "tls_root_store.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

namespace
{
std::deque<boost::asio::const_buffer> split_buffer(const boost::asio::streambuf::const_buffers_type& source_buffer)
{
	const unsigned int chunk_size = 4096;

	std::deque<boost::asio::const_buffer> buffers;
	unsigned int remaining_size = boost::asio::buffer_size(source_buffer);

	const uint8_t* data = static_cast<const uint8_t*>(source_buffer.data());

	while(remaining_size > 0u) {
		unsigned int size = std::min(remaining_size, chunk_size);
		buffers.emplace_back(data, size);
		data += size;
		remaining_size -= size;
	}

	return buffers;
}
}

namespace network_asio
{
using boost::system::system_error;

connection::connection(const std::string& host, const std::string& service)
	: io_context_()
	, host_(host)
	, service_(service)
	, resolver_(io_context_)
	, use_tls_(true)
	, socket_(raw_socket(new raw_socket::element_type{io_context_}))
	, done_(false)
	, write_buf_()
	, read_buf_()
	, handshake_response_()
	, payload_size_(0)
	, bytes_to_write_(0)
	, bytes_written_(0)
	, bytes_to_read_(0)
	, bytes_read_(0)
{
	boost::system::error_code ec;
	auto result = resolver_.resolve(host, service, boost::asio::ip::resolver_query_base::numeric_host, ec);
	if(!ec) { // if numeric resolve succeeds then we got raw ip address so TLS host name validation would never pass
		use_tls_ = false;
		boost::asio::post(io_context_, [this, ec, result](){ handle_resolve(ec, { result } ); } );
	} else {
		resolver_.async_resolve(host, service,
			std::bind(&connection::handle_resolve, this, std::placeholders::_1, std::placeholders::_2));
	}

	LOG_NW << "Resolving hostname: " << host;
}

connection::~connection()
{
	if(auto socket = utils::get_if<tls_socket>(&socket_)) {
		boost::system::error_code ec;
		// this sends close_notify for secure connection shutdown
		(*socket)->async_shutdown([](const boost::system::error_code&) {} );
		const char buffer[] = "";
		// this write is needed to trigger immediate close instead of waiting for other side's close_notify
		boost::asio::write(**socket, boost::asio::buffer(buffer, 0), ec);
	}
}

void connection::handle_resolve(const boost::system::error_code& ec, const results_type& results)
{
	if(ec) {
		throw system_error(ec);
	}

	boost::asio::async_connect(*utils::get<raw_socket>(socket_), results,
		std::bind(&connection::handle_connect, this, std::placeholders::_1, std::placeholders::_2));
}

void connection::handle_connect(const boost::system::error_code& ec, endpoint endpoint)
{
	if(ec) {
		ERR_NW << "Tried all IPs. Giving up";
		throw system_error(ec);
	} else {
		LOG_NW << "Connected to " << endpoint.address();

		handshake();
	}
}

void connection::handshake()
{
	static const uint32_t handshake = 0;
	static const uint32_t tls_handshake = htonl(uint32_t(1));

	boost::asio::async_write(
		*utils::get<raw_socket>(socket_),
		boost::asio::buffer(use_tls_ ? reinterpret_cast<const char*>(&tls_handshake) : reinterpret_cast<const char*>(&handshake), 4),
		std::bind(&connection::handle_write, this, std::placeholders::_1, std::placeholders::_2)
	);

	boost::asio::async_read(*utils::get<raw_socket>(socket_), boost::asio::buffer(reinterpret_cast<std::byte*>(&handshake_response_), 4),
		std::bind(&connection::handle_handshake, this, std::placeholders::_1));
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

void connection::handle_handshake(const boost::system::error_code& ec)
{
	if(ec) {
		if(ec == boost::asio::error::eof && use_tls_) {
			// immediate disconnect likely means old server not supporting TLS handshake code
			fallback_to_unencrypted();
			return;
		}

		throw system_error(ec);
	}

	if(use_tls_) {
		if(handshake_response_ == 0xFFFFFFFFU) {
			use_tls_ = false;
			handle_handshake(ec);
			return;
		}

		if(handshake_response_ == 0x00000000) {
			load_tls_root_certs(tls_context_);
			raw_socket s { std::move(utils::get<raw_socket>(socket_)) };
			tls_socket ts { new tls_socket::element_type { std::move(*s), tls_context_ } };
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

			socket.async_handshake(boost::asio::ssl::stream_base::client, [this](const boost::system::error_code& ec) {
				if(ec) {
					throw system_error(ec);
				}

				done_ = true;
			});
			return;
		}

		fallback_to_unencrypted();
	} else {
		done_ = true;
	}
}

void connection::fallback_to_unencrypted()
{
	assert(use_tls_ == true);
	use_tls_ = false;

	boost::asio::ip::tcp::endpoint endpoint { utils::get<raw_socket>(socket_)->remote_endpoint() };
	utils::get<raw_socket>(socket_)->close();

	utils::get<raw_socket>(socket_)->async_connect(endpoint,
		std::bind(&connection::handle_connect, this, std::placeholders::_1, endpoint));
}

void connection::transfer(const config& request, config& response)
{
	io_context_.restart();
	done_ = false;

	write_buf_.reset(new boost::asio::streambuf);
	read_buf_.reset(new boost::asio::streambuf);
	std::ostream os(write_buf_.get());
	write_gz(os, request);

	bytes_to_write_ = write_buf_->size() + 4;
	bytes_written_ = 0;
	payload_size_ = htonl(bytes_to_write_ - 4);

	auto bufs = split_buffer(write_buf_->data());
	bufs.push_front(boost::asio::buffer(reinterpret_cast<const char*>(&payload_size_), 4));

	utils::visit([this, &bufs, &response](auto&& socket) {
		boost::asio::async_write(*socket, bufs,
			std::bind(&connection::is_write_complete, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&connection::handle_write, this, std::placeholders::_1, std::placeholders::_2));

		boost::asio::async_read(*socket, *read_buf_,
			std::bind(&connection::is_read_complete, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&connection::handle_read, this, std::placeholders::_1, std::placeholders::_2, std::ref(response)));
	}, socket_);
}

void connection::cancel()
{
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
	bytes_to_write_ = 0;
	bytes_written_ = 0;
	bytes_to_read_ = 0;
	bytes_read_ = 0;
}

std::size_t connection::is_write_complete(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if(ec) {
		throw system_error(ec);
	}

	bytes_written_ = bytes_transferred;
	return bytes_to_write_ - bytes_transferred;
}

void connection::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	DBG_NW << "Written " << bytes_transferred << " bytes.";
	if(write_buf_)
		write_buf_->consume(bytes_transferred);

	if(ec) {
		throw system_error(ec);
	}
}

std::size_t connection::is_read_complete(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if(ec) {
		throw system_error(ec);
	}

	bytes_read_ = bytes_transferred;
	if(bytes_transferred < 4) {
		return 4;
	}

	if(!bytes_to_read_) {
		std::istream is(read_buf_.get());
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

void connection::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, config& response)
{
	DBG_NW << "Read " << bytes_transferred << " bytes.";

	bytes_to_read_ = 0;
	bytes_to_write_ = 0;
	done_ = true;

	if(ec && ec != boost::asio::error::eof) {
		throw system_error(ec);
	}

	std::istream is(read_buf_.get());
	read_gz(response, is);
}
}
