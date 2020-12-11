/*
   Copyright (C) 2016 by Sergey Popov <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License 2
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SEND_RECEIVE_WML_HELPERS_HPP
#define SEND_RECEIVE_WML_HELPERS_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SENDFILE
#include <sys/sendfile.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include "server/common/server_base.hpp"
#include "server/common/simple_wml.hpp"
#include "filesystem.hpp"
#include "serialization/unicode_cast.hpp" //only used in windows specific code.

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <memory>
#include <stdexcept>

/// Send a WML document from within a coroutine
/// @param socket
/// @param doc
/// @param yield The function will suspend on write operation using this yield context
inline void coro_send_doc(socket_ptr socket, simple_wml::document& doc, boost::asio::yield_context yield)
{
	try {
		simple_wml::string_span s = doc.output_compressed();

		union DataSize
		{
			uint32_t size;
			char buf[4];
		} data_size;
		data_size.size = htonl(s.size());

		std::vector<boost::asio::const_buffer> buffers {
			{ data_size.buf, 4 },
			{ s.begin(), std::size_t(s.size()) }
		};

		async_write(*socket, buffers, yield);
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
		throw;
	}
}

#ifdef HAVE_SENDFILE

/// Send contents of entire file directly to socket from within a coroutine
/// @param socket
/// @param filename
/// @param yield The function will suspend on write operations using this yield context
inline void coro_send_file(socket_ptr socket, const std::string& filename, boost::asio::yield_context yield)
{
	std::size_t filesize { std::size_t(filesystem::file_size(filename)) };
	int in_file { open(filename.c_str(), O_RDONLY) };
	off_t offset { 0 };
	std::size_t total_bytes_transferred { 0 };

	union DataSize
	{
		uint32_t size;
		char buf[4];
	} data_size;
	data_size.size = htonl(filesize);

	async_write(*socket, boost::asio::buffer(data_size.buf), yield);
	if(*(yield.ec_)) return;

	// Put the underlying socket into non-blocking mode.
	if(!socket->native_non_blocking())
		socket->native_non_blocking(true, *yield.ec_);
	if(*(yield.ec_)) return;

	for (;;)
	{
		// Try the system call.
		errno = 0;
		int n = ::sendfile(socket->native_handle(), in_file, &offset, 65536);
		*(yield.ec_) = boost::system::error_code(n < 0 ? errno : 0,
									   boost::asio::error::get_system_category());
		total_bytes_transferred += *(yield.ec_) ? 0 : n;

		// Retry operation immediately if interrupted by signal.
		if (*(yield.ec_) == boost::asio::error::interrupted)
			continue;

		// Check if we need to run the operation again.
		if (*(yield.ec_) == boost::asio::error::would_block
				|| *(yield.ec_) == boost::asio::error::try_again)
		{
			// We have to wait for the socket to become ready again.
			socket->async_write_some(boost::asio::null_buffers(), yield);
			continue;
		}

		if (*(yield.ec_) || n == 0)
		{
			// An error occurred, or we have reached the end of the file.
			// Either way we must exit the loop.
			break;
		}

		// Loop around to try calling sendfile again.
	}
}

#elif defined(_WIN32)

template<typename Handler, typename ErrorHandler>
struct sendfile_op
{
	socket_ptr sock_;
	HANDLE file_;
	OVERLAPPED overlap_;
	Handler handler_;
	ErrorHandler error_handler_;
	bool pending_;
	std::shared_ptr<handle_doc<Handler, ErrorHandler>> handle_send_doc_;

	void operator()(boost::system::error_code, std::size_t)
	{
		bool failed = false;
		if (!pending_)
		{
			BOOL success = TransmitFile(sock_->native_handle(), file_, 0, 0, &overlap_, nullptr, 0);
			if (!success)
			{
				int winsock_ec = WSAGetLastError();
				if (winsock_ec == WSA_IO_PENDING || winsock_ec == ERROR_IO_PENDING)
				{
					// The request is pending. Wait until it completes.
					pending_ = true;
					sock_->async_write_some(boost::asio::null_buffers(), *this);
					return;
				}
				else
				{
					failed = true;
				}
			}
		}
		else
		{
			DWORD win_ec = GetLastError();
			if (win_ec != ERROR_IO_PENDING && win_ec != ERROR_SUCCESS)
			{
				failed = true;
			}
			else if (!HasOverlappedIoCompleted(&overlap_))
			{
				// Keep waiting.
				sock_->async_write_some(boost::asio::null_buffers(), *this);
				return;
			}
		}

		CloseHandle(file_);
		CloseHandle(overlap_.hEvent);

		if (!failed)
		{
			handler_(sock_);
		}
		else
		{
			error_handler_(sock_);
		}
	}
};

template<typename Handler, typename ErrorHandler>
void async_send_file(socket_ptr socket, const std::string& filename, Handler handler, ErrorHandler error_handler)
{
	std::vector<boost::asio::const_buffer> buffers;

	SetLastError(ERROR_SUCCESS);

	std::size_t filesize = filesystem::file_size(filename);
	std::wstring filename_ucs2 = unicode_cast<std::wstring>(filename);
	HANDLE in_file = CreateFileW(filename_ucs2.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	if (GetLastError() != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to open the file");
	}

	sendfile_op<Handler, ErrorHandler> op = { socket, in_file, OVERLAPPED(), handler, error_handler, false, nullptr };

	HANDLE event = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	if (GetLastError() != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to create an event");
	}

	op.overlap_.hEvent = event;
	op.handle_send_doc_.reset(new handle_doc<Handler, ErrorHandler>(socket, handler, error_handler, filesize, nullptr));

	buffers.push_back(boost::asio::buffer(op.handle_send_doc_->data_size->buf, 4));
	async_write(*socket, buffers, op);
}

#else

// TODO: Implement this for systems without sendfile()
template<typename Handler, typename ErrorHandler>
void async_send_file(socket_ptr, const std::string&, Handler, ErrorHandler)
{
	assert(false && "Not implemented yet");
}

#endif

/// Receive WML document from a coroutine
/// @param socket
/// @param yield The function will suspend on read operation using this yield context
inline std::shared_ptr<simple_wml::document> coro_receive_doc(socket_ptr socket, boost::asio::yield_context yield)
{
	union DataSize
	{
		uint32_t size;
		char buf[4];
	} data_size;
	async_read(*socket, boost::asio::buffer(data_size.buf, 4), yield);
	if(*yield.ec_) return {};
	uint32_t size = ntohl(data_size.size);

	if(size == 0) {
		ERR_SERVER <<
					  client_address(socket) <<
					  "\treceived invalid packet with payload size 0" << std::endl;
		return {};
	}
	if(size > simple_wml::document::document_size_limit) {
		ERR_SERVER <<
					  client_address(socket) <<
					  "\treceived packet with payload size over size limit" << std::endl;
		return {};
	}

	boost::shared_array<char> buffer{ new char[size] };
	async_read(*socket, boost::asio::buffer(buffer.get(), size), yield);

	try {
		simple_wml::string_span compressed_buf(buffer.get(), size);
		return std::shared_ptr<simple_wml::document> { new simple_wml::document(compressed_buf) };
	}  catch (simple_wml::error& e) {
		ERR_SERVER <<
			client_address(socket) <<
			"\tsimple_wml error in received data: " << e.message << std::endl;
		//async_send_error(socket, "Invalid WML received: " + e.message);
		return {};
	}
}

#endif
