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

#include "server_base.hpp"
#include "simple_wml.hpp"

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
        boost::shared_ptr<DataSize> data_size;
        boost::shared_ptr<simple_wml::document> doc;
        boost::shared_array<char> buffer;
        handle_doc(socket_ptr socket, Handler handler, ErrorHandler error_handler, boost::uint32_t size, boost::shared_ptr<simple_wml::document> doc) :
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
                boost::shared_ptr<simple_wml::document> doc_ptr(doc.clone());
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

template<typename Handler>
void async_send_doc(socket_ptr socket, simple_wml::document& doc, Handler handler)
{
        async_send_doc(socket, doc, handler, null_handler);
}

static void async_send_doc(socket_ptr socket, simple_wml::document& doc)
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
void async_receive_doc(socket_ptr socket, Handler handler, ErrorHandler error_handler)
{
        handle_receive_doc<Handler, ErrorHandler> handle_receive_doc(socket, handler, error_handler);
        async_read(*socket, boost::asio::buffer(handle_receive_doc.data_size->buf, 4), handle_receive_doc);
}

template<typename Handler>
void async_receive_doc(socket_ptr socket, Handler handler)
{
        async_receive_doc(socket, handler, null_handler);
}

#endif
