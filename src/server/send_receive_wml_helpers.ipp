/*
   Copyright (C) 2016 by Sergey Popov <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

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

#include "server_base.hpp"
#include "simple_wml.hpp"
#include "filesystem.hpp"
#include "serialization/unicode_cast.hpp" //only used in windows specific code.

#include <memory>
#include <stdexcept>

template<typename Handler, typename ErrorHandler>
struct handle_doc
{
	Handler handler;
	ErrorHandler error_handler;
	socket_ptr socket;
	union DataSize
	{
		boost::uint32_t size;
		char buf[4];
	};
	std::shared_ptr<DataSize> data_size;
	std::shared_ptr<simple_wml::document> doc;
	boost::shared_array<char> buffer;
	handle_doc(socket_ptr socket, Handler handler, ErrorHandler error_handler, boost::uint32_t size, std::shared_ptr<simple_wml::document> doc) :
		handler(handler), error_handler(error_handler), socket(socket), data_size(new DataSize), doc(doc)
	{
		data_size->size = htonl(size);
	}
	handle_doc(socket_ptr socket, Handler handler, ErrorHandler error_handler) :
		handler(handler), error_handler(error_handler), socket(socket), data_size(new DataSize)
	{
	}
	void operator()(const boost::system::error_code& error, std::size_t)
	{
		if(check_error(error, socket)) {
			error_handler(socket);
			return;
		}
		handler(socket);
	}
};

template<typename Handler, typename ErrorHandler>
void async_send_doc(socket_ptr socket, simple_wml::document& doc, Handler handler, ErrorHandler error_handler)
{
	try {
		std::shared_ptr<simple_wml::document> doc_ptr(doc.clone());
		simple_wml::string_span s = doc_ptr->output_compressed();
		std::vector<boost::asio::const_buffer> buffers;

		handle_doc<Handler, ErrorHandler> handle_send_doc(socket, handler, error_handler, s.size(), doc_ptr);
		buffers.push_back(boost::asio::buffer(handle_send_doc.data_size->buf, 4));
		buffers.push_back(boost::asio::buffer(s.begin(), s.size()));
		async_write(*socket, buffers, handle_send_doc);
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
}

static void null_handler(socket_ptr)
{
}

#ifdef HAVE_SENDFILE

template <typename Handler, typename ErrorHandler>
struct sendfile_op
{
	socket_ptr sock_;
	int fd_;
	Handler handler_;
	ErrorHandler error_handler_;
	off_t offset_;
	std::size_t total_bytes_transferred_;

	// Function call operator meeting WriteHandler requirements.
	// Used as the handler for the async_write_some operation.
	void operator()(boost::system::error_code ec, std::size_t)
	{
		// Put the underlying socket into non-blocking mode.
		if (!ec)
			if (!sock_->native_non_blocking())
				sock_->native_non_blocking(true, ec);

		if (!ec)
		{
			for (;;)
			{
				// Try the system call.
				errno = 0;
				int n = ::sendfile(sock_->native_handle(), fd_, &offset_, 65536);
				ec = boost::system::error_code(n < 0 ? errno : 0,
											   boost::asio::error::get_system_category());
				total_bytes_transferred_ += ec ? 0 : n;

				// Retry operation immediately if interrupted by signal.
				if (ec == boost::asio::error::interrupted)
					continue;

				// Check if we need to run the operation again.
				if (ec == boost::asio::error::would_block
						|| ec == boost::asio::error::try_again)
				{
					// We have to wait for the socket to become ready again.
					sock_->async_write_some(boost::asio::null_buffers(), *this);
					return;
				}

				if (ec || n == 0)
				{
					// An error occurred, or we have reached the end of the file.
					// Either way we must exit the loop so we can call the handler.
					break;
				}

				// Loop around to try calling sendfile again.
			}
		}

		close(fd_);

		if(ec)
			error_handler_(sock_);
		else
			handler_(sock_);
	}
};

template<typename Handler, typename ErrorHandler>
void async_send_file(socket_ptr socket, const std::string& filename, Handler handler, ErrorHandler error_handler)
{
	std::vector<boost::asio::const_buffer> buffers;

	size_t filesize = filesystem::file_size(filename);
	int in_file(open(filename.c_str(), O_RDONLY));

	sendfile_op<Handler, ErrorHandler> op = { socket, in_file, handler, error_handler, 0, 0 };

	handle_doc<Handler, ErrorHandler> handle_send_doc(socket, handler, error_handler, filesize, nullptr);
	buffers.push_back(boost::asio::buffer(handle_send_doc.data_size->buf, 4));
	async_write(*socket, buffers, op);
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

	void operator()(boost::system::error_code ec, std::size_t)
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

	size_t filesize = filesystem::file_size(filename);
	std::wstring filename_ucs2 = unicode_cast<std::wstring>(filename);
	HANDLE in_file = CreateFileW(filename_ucs2.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	if (GetLastError() != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to open the file");
	}

	sendfile_op<Handler, ErrorHandler> op = { socket, in_file, OVERLAPPED(), handler, error_handler, false };

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

template<typename Handler>
inline void async_send_doc(socket_ptr socket, simple_wml::document& doc, Handler handler)
{
	async_send_doc(socket, doc, handler, null_handler);
}

inline void async_send_doc(socket_ptr socket, simple_wml::document& doc)
{
	async_send_doc(socket, doc, null_handler, null_handler);
}

template<typename Handler, typename ErrorHandler>
struct handle_receive_doc : public handle_doc<Handler, ErrorHandler>
{
	std::size_t buf_size;
	handle_receive_doc(socket_ptr socket, Handler handler, ErrorHandler error_handler) :
		handle_doc<Handler, ErrorHandler>(socket, handler, error_handler)
	{
	}
	void operator()(const boost::system::error_code& error, std::size_t size)
	{
		if(check_error(error, this->socket)) {
			this->error_handler(this->socket);
			return;
		}
		if(!this->buffer) {
			assert(size == 4);
			buf_size = ntohl(this->data_size->size);

			if(buf_size == 0) {
				ERR_SERVER <<
							  client_address(this->socket) <<
							  "\treceived invalid packet with payload size 0" << std::endl;
				this->error_handler(this->socket);
				return;
			}
			if(buf_size > simple_wml::document::document_size_limit) {
				ERR_SERVER <<
							  client_address(this->socket) <<
							  "\treceived packet with payload size over size limit" << std::endl;
				this->error_handler(this->socket);
				return;
			}

			this->buffer = boost::shared_array<char>(new char[buf_size]);
			async_read(*(this->socket), boost::asio::buffer(this->buffer.get(), buf_size), *this);
		} else {
			simple_wml::string_span compressed_buf(this->buffer.get(), buf_size);
			try {
				this->doc.reset(new simple_wml::document(compressed_buf));
			} catch (simple_wml::error& e) {
				ERR_SERVER <<
							  client_address(this->socket) <<
							  "\tsimple_wml error in received data: " << e.message << std::endl;
				async_send_error(this->socket, "Invalid WML received: " + e.message);
				this->error_handler(this->socket);
				return;
			}
			this->handler(this->socket, this->doc);
		}
	}
};

template<typename Handler, typename ErrorHandler>
inline void async_receive_doc(socket_ptr socket, Handler handler, ErrorHandler error_handler)
{
	handle_receive_doc<Handler, ErrorHandler> handle_receive_doc(socket, handler, error_handler);
	async_read(*socket, boost::asio::buffer(handle_receive_doc.data_size->buf, 4), handle_receive_doc);
}

template<typename Handler>
inline void async_receive_doc(socket_ptr socket, Handler handler)
{
	async_receive_doc(socket, handler, null_handler);
}

#endif
