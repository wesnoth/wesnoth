#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "asio_listener.hpp"
#include "mili/mili.h"

using boost::asio::ip::tcp;
using namespace mili;

asio_listener::asio_listener( boost::asio::io_service& io_service, tcp::socket& socket) :
    io_service_(io_service),
    socket_(socket),
    listener_( NULL )
{
}

asio_listener::~asio_listener()
{
}

void asio_listener::disconnect( ana::listener_handler* listener, boost::system::error_code error)
{
    listener->handle_disconnect( error, id() );
    disconnect_listener();
}

void asio_listener::handle_body( ana::detail::read_buffer buf,
                                const boost::system::error_code& ec, ana::listener_handler* listener )
{
    try
    {
        if (ec)
            disconnect(listener, ec);
        else
        {
            listener->handle_message( ec, id(), buf );

            listen_one_message();
        }
    }
    catch(const std::exception& e)
    {
        disconnect(listener, ec);
    }
}


void asio_listener::handle_header(char* header, const boost::system::error_code& ec, ana::listener_handler* listener )
{
    try
    {
        if (ec)
            disconnect(listener, ec);
        else
        {
            bistream input( std::string(header, ana::HeaderLength) );

            size_t   size;
            input >> size;

            if (size != 0)
            {
                ana::detail::read_buffer read_buf( new ana::detail::read_buffer_implementation( size ) );

                boost::asio::async_read(socket_, boost::asio::buffer( read_buf->base(), read_buf->size() ),
                                        boost::bind(&asio_listener::handle_body,
                                                    this, read_buf,
                                                    boost::asio::placeholders::error,
                                                    listener));
            }
        }
    }
    catch(const std::exception& e)
    {
        disconnect(listener, ec);
    }
}

void asio_listener::set_listener_handler( ana::listener_handler* listener )
{
    listener_ = listener;
}

void asio_listener::run_listener( )
{
    listen_one_message();
}

void asio_listener::listen_one_message()
{
    try
    {
        boost::asio::async_read(socket_, boost::asio::buffer(header_, ana::HeaderLength),
                                boost::bind(&asio_listener::handle_header,
                                            this, header_,
                                            boost::asio::placeholders::error,
                                            listener_));
    }
    catch(const std::exception& e)
    {
        disconnect(listener_, boost::system::error_code(1,boost::system::get_generic_category() ));
    }
}