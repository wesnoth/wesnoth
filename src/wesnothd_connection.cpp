/*
   Copyright (C) 2011 - 2017 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "wesnothd_connection.hpp"

#include "gui/dialogs/loading_screen.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "utils/functional.hpp"
#include "video.hpp"

#include <boost/thread.hpp>
#include <SDL_timer.h>

#include <cstdint>
#include <deque>

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
		WRN_NW << "Process:" << getpid() << " Thread:" << boost::this_thread::get_id() << " Function: " << functionname << " Start\n";
	}
};
}


#define MPTEST_LOG mptest_log mptest_log__( __func__)
#else
#define MPTEST_LOG ((void)0)
#endif

using boost::system::system_error;
using boost::system::error_code;

// main thread
wesnothd_connection::wesnothd_connection(const std::string& host, const std::string& service)
	: worker_thread_()
	, io_service_()
	, resolver_(io_service_)
	, socket_(io_service_)
	, last_error_()
	, last_error_mutex_()
	, handshake_finished_(false)
	, read_buf_()
	, handshake_response_()
	, recv_queue_()
	, recv_queue_mutex_()
	, payload_size_(0)
	, bytes_to_write_(0)
	, bytes_written_(0)
	, bytes_to_read_(0)
	, bytes_read_(0)
{
	MPTEST_LOG;
	resolver_.async_resolve(
		boost::asio::ip::tcp::resolver::query(host, service),
		std::bind(&wesnothd_connection::handle_resolve, this, _1, _2)
	);
	LOG_NW << "Resolving hostname: " << host << '\n';
}

// main thread
void wesnothd_connection::handle_resolve(const error_code& ec, resolver::iterator iterator)
{
	MPTEST_LOG;
	if (ec) {
		LOG_NW << __func__ << " Throwing: " << ec << "\n";
		throw system_error(ec);
	}
	connect(iterator);
}

// main thread
void wesnothd_connection::connect(resolver::iterator iterator)
{
	MPTEST_LOG;
	socket_.async_connect(*iterator, std::bind(
		&wesnothd_connection::handle_connect, this, _1, iterator)
	);
	LOG_NW << "Connecting to " << iterator->endpoint().address() << '\n';
}

// main thread
void wesnothd_connection::handle_connect(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		)
{
	MPTEST_LOG;
	if(ec) {
		WRN_NW << "Failed to connect to " <<
			iterator->endpoint().address() << ": " <<
			ec.message() << '\n';
		socket_.close();
		if(++iterator == resolver::iterator()) {
			ERR_NW << "Tried all IPs. Giving up" << std::endl;
			throw system_error(ec);
		}
		else {
			connect(iterator);
		}
	} else {
		LOG_NW << "Connected to " << iterator->endpoint().address() << '\n';
		handshake();
	}
}
// main thread
void wesnothd_connection::handshake()
{
	MPTEST_LOG;
	static const uint32_t handshake = 0;
	boost::asio::async_write(socket_,
		boost::asio::buffer(reinterpret_cast<const char*>(&handshake), 4),
		[](const error_code& ec, std::size_t) { if (ec) throw system_error(ec); }
	);
	boost::asio::async_read(socket_,
		boost::asio::buffer(&handshake_response_.binary, 4),
		std::bind(&wesnothd_connection::handle_handshake, this, _1)
	);
}

// main thread
void wesnothd_connection::handle_handshake(const error_code& ec)
{
	MPTEST_LOG;
	if (ec) {
		LOG_NW << __func__ << " Throwing: " << ec << "\n";
		throw system_error(ec);
	}
	handshake_finished_ = true;

	recv();
	worker_thread_.reset(new boost::thread( [this](){
		// worker thread
		std::shared_ptr<wesnothd_connection> this_ptr = this->shared_from_this();
		io_service_.run();
		LOG_NW << "wesnothd_connection::io_service::run() returned\n";
	} ));
}

// main thread
void wesnothd_connection::send_data(const configr_of& request)
{
	MPTEST_LOG;
	//C++11 doesnt allow lambda captuting by moving, this could maybe use std::unique_ptr in c++14;
	std::shared_ptr<boost::asio::streambuf> buf_ptr( new boost::asio::streambuf());
	std::ostream os(buf_ptr.get());
	write_gz(os, request);

	//TODO: shoudl i capturea  shared_ptr for this?
	io_service_.post([this, buf_ptr](){
		DBG_NW << "In wesnothd_connection::send_data::lambda\n";
		send_queue_.push_back(buf_ptr);
		if (send_queue_.size() == 1) {
			send();
		}
	});
}

// main thread
void wesnothd_connection::cancel()
{
	MPTEST_LOG;
	if(socket_.is_open()) {
		boost::system::error_code ec;
		socket_.cancel(ec);
		if(ec) {
			WRN_NW << "Failed to cancel network operations: " << ec.message() << std::endl;
		}
	}
}

// main thread
void wesnothd_connection::stop()
{
	// TODO: wouldn't cancel() have the same effect?
	MPTEST_LOG;
	io_service_.stop();
}

//worker thread
std::size_t wesnothd_connection::is_write_complete(const boost::system::error_code& ec, size_t bytes_transferred)
{
	MPTEST_LOG;
	if(ec)
	{
		{
			std::lock_guard<std::mutex> lock(last_error_mutex_);
			last_error_ = ec;
		}
		LOG_NW << __func__ << " Error: " << ec << "\n";
		io_service_.stop();
		return bytes_to_write_ - bytes_transferred;
	}
	bytes_written_ = bytes_transferred;
	return bytes_to_write_ - bytes_transferred;
}

//worker thread
void wesnothd_connection::handle_write(
	const boost::system::error_code& ec,
	std::size_t bytes_transferred
	)
{
	MPTEST_LOG;
	DBG_NW << "Written " << bytes_transferred << " bytes.\n";
	send_queue_.pop_front();
	if(ec)
	{
		{
			std::lock_guard<std::mutex> lock(last_error_mutex_);
			last_error_ = ec;
		}
		LOG_NW << __func__ << " Error: " << ec << "\n";
		io_service_.stop();
		return;
	}
	if (!send_queue_.empty()) {
		send();
	}
}

//worker thread
std::size_t wesnothd_connection::is_read_complete(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred
		)
{
	//We use custom is_write/read_complete function to be able to see the current progress of the upload/download
	MPTEST_LOG;
	if(ec)
	{
		{
			std::lock_guard<std::mutex> lock(last_error_mutex_);
			last_error_ = ec;
		}
		LOG_NW << __func__ << " Error: " << ec << "\n";
		io_service_.stop();
		return bytes_to_read_ - bytes_transferred;
	}
	bytes_read_ = bytes_transferred;
	if(bytes_transferred < 4) {
		return 4;
	} else {
		if(!bytes_to_read_) {
			std::istream is(&read_buf_);
			union { char binary[4]; uint32_t num; } data_size;
			is.read(data_size.binary, 4);
			bytes_to_read_ = ntohl(data_size.num) + 4;
			//Close immediately if we receive an invalid length
			if (bytes_to_read_ < 4)
				bytes_to_read_ = bytes_transferred;
		}
		return bytes_to_read_ - bytes_transferred;
	}
}

//worker thread
void wesnothd_connection::handle_read(
	const boost::system::error_code& ec,
	std::size_t bytes_transferred
	)
{
	MPTEST_LOG;
	DBG_NW << "Read " << bytes_transferred << " bytes.\n";
	bytes_to_read_ = 0;
	if(last_error_ && ec != boost::asio::error::eof)
	{
		{
			std::lock_guard<std::mutex> lock(last_error_mutex_);
			last_error_ = ec;
		}
		LOG_NW << __func__ << " Error: " << ec << "\n";
		io_service_.stop();
		return;
	}

	std::istream is(&read_buf_);
	config data;
	read_gz(data, is);
	{
		std::lock_guard<std::mutex> lock(recv_queue_mutex_);
		recv_queue_.emplace_back(std::move(data));
	}
	DBG_NW << "Received " << recv_queue_.back() << " bytes.\n";

	recv();
}

//worker thread
void wesnothd_connection::send()
{
	MPTEST_LOG;
	auto& buf = *send_queue_.front();
	size_t buf_size = buf.size();
	bytes_to_write_ = buf_size + 4;
	bytes_written_ = 0;
	payload_size_ = htonl(buf_size);

	boost::asio::streambuf::const_buffers_type gzipped_data = buf.data();
	std::deque<boost::asio::const_buffer> bufs(gzipped_data.begin(), gzipped_data.end());
	bufs.push_front(boost::asio::buffer(reinterpret_cast<const char*>(&payload_size_), 4));
	boost::asio::async_write(socket_, bufs,
		std::bind(&wesnothd_connection::is_write_complete, this, _1, _2),
		std::bind(&wesnothd_connection::handle_write, this, _1, _2)
	);
}
//worker thread
void wesnothd_connection::recv()
{
	MPTEST_LOG;
	boost::asio::async_read(socket_, read_buf_,
		std::bind(&wesnothd_connection::is_read_complete, this, _1, _2),
		std::bind(&wesnothd_connection::handle_read, this, _1, _2)
	);
}

// main thread, during handshake
std::size_t wesnothd_connection::poll()
{
	MPTEST_LOG;
	assert(!worker_thread_);
	try {
		return io_service_.poll();
	}
	catch (const boost::system::system_error& err) {
		if( err.code() == boost::asio::error::operation_aborted || err.code() == boost::asio::error::eof) {
			return 1;
		}
		WRN_NW << __func__ << " Rehrowing: " << err.code() << "\n";
		throw error(err.code());
	}
}

// main thread
bool wesnothd_connection::receive_data(config& result)
{
	MPTEST_LOG;

	{
		std::lock_guard<std::mutex> lock(recv_queue_mutex_);
		if (!recv_queue_.empty()) {
			result.swap(recv_queue_.front());
			recv_queue_.pop_front();
			return true;
		}
	}

	{
		std::lock_guard<std::mutex> lock(last_error_mutex_);
		if (last_error_) {
			throw error(last_error_);
		}
	}

	return false;
}

bool wesnothd_connection::wait_and_receive_data(config& data)
{
	while(!has_data_received()) {
		SDL_Delay(1);
	}

	return receive_data(data);
};


bool wesnothd_connection::fetch_data_with_loading_screen(config& cfg, loading_stage stage)
{
	assert(stage != loading_stage::none);

	bool res = false;
	gui2::dialogs::loading_screen::display(CVideo::get_singleton(), [&]() {
		gui2::dialogs::loading_screen::progress(stage);

		res = wait_and_receive_data(cfg);
	});

	return res;
}

wesnothd_connection::~wesnothd_connection()
{
	MPTEST_LOG;
}

//  wesnothd_connection_ptr


wesnothd_connection_ptr& wesnothd_connection_ptr::operator=(wesnothd_connection_ptr&& other)
{
	MPTEST_LOG;
	if(ptr_) {
		ptr_->stop();
		ptr_.reset();
	}
	ptr_ = std::move(other.ptr_);
	return *this;
}

wesnothd_connection_ptr wesnothd_connection::create(const std::string& host, const std::string& service)
{
	//can't use make_shared becasue the ctor is private
	return wesnothd_connection_ptr(std::shared_ptr<wesnothd_connection>(new wesnothd_connection(host, service)));
}

wesnothd_connection_ptr::~wesnothd_connection_ptr()
{
	MPTEST_LOG;
	if(ptr_) {
		ptr_->stop();
	}
}
