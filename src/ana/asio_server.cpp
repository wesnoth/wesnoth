#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "asio_server.hpp"

#include "mili/mili.h"

using namespace ana;

using boost::asio::ip::tcp;

using namespace mili;

client_id server::client_proxy::last_client_id_ = 0;

asio_server::asio_server() :
    io_service_(),
    io_thread_(),
    acceptor_( NULL ),
    client_proxies_(),
    listening_(false),
    listener_( NULL ),
    connection_handler_( NULL ),
    last_client_proxy_( NULL )
{
}

asio_server::~asio_server()
{
    io_service_.stop();
    io_thread_.join();

    /* Since the asio_client_proxy destuctor removes the client from client_proxies_
       I'll just delete every proxy from a different list. */
    std::list<client_proxy*> copy( client_proxies_ );

    for (std::list<client_proxy*>::iterator it = copy.begin();
         it != copy.end();
         ++it)
    {
        delete *it;
    }

    assert( client_proxies_.empty() );

    delete last_client_proxy_;
}

server* ana::server::create()
{
    return new asio_server();
}

void asio_server::set_connection_handler( connection_handler* handler )
{
    connection_handler_ = handler;
}

void asio_server::run(port pt)
{
    tcp::acceptor* new_acceptor(new tcp::acceptor( io_service_,
                                                   tcp::endpoint(tcp::v4(), atoi( pt.c_str() ))));

    acceptor_.reset(new_acceptor);

    async_accept( connection_handler_ );

    run_listener( );

    io_thread_ = boost::thread( boost::bind( &boost::asio::io_service::run, &io_service_) );
}

client_id asio_server::id() const
{
    return 0;
}

void asio_server::async_accept( connection_handler* handler )
{
    try
    {
        last_client_proxy_ = new asio_client_proxy(io_service_, this);

        acceptor_->async_accept(last_client_proxy_->socket(),
                                boost::bind(&asio_server::handle_accept,
                                            this,
                                            boost::asio::placeholders::error,
                                            last_client_proxy_,
                                            handler));
    }
    catch( const std::exception& e )
    {
        delete last_client_proxy_;
    }
}

void asio_server::register_client(client_proxy* client)
{
    client_proxies_.push_back(client);

    if (listening_)
    {
        client->set_listener_handler( listener_ );
        client->run_listener( );
    }
}

void asio_server::deregister_client(client_proxy* client)
{
    client_proxies_.remove( client );
}

void asio_server::handle_accept(const boost::system::error_code& ec,
                               asio_client_proxy* client,
                               connection_handler* handler )
{
    if (! ec)
    {
        register_client(client);
        handler->handle_connect( ec, client->id() );
    }
    else
    {
        std::cerr << "Server: Error accepting client connection." << std::endl;
        delete client;
    }

    async_accept( handler );
}


void asio_server::run_listener( )
{
    for (std::list<client_proxy*>::iterator it( client_proxies_.begin() ); it != client_proxies_.end(); ++it)
    {
        (*it)->set_listener_handler( listener_ );
        (*it)->run_listener( );
    }
}

void asio_server::set_listener_handler( listener_handler* listener )
{
    listening_ = true;
    listener_  = listener;
}

void asio_server::send_one(client_id id,boost::asio::const_buffer buffer, send_handler* handler, send_type copy_buffer)
{
    send_if(buffer, handler, create_predicate ( boost::bind( std::equal_to<client_id>(), id, _1) ), copy_buffer );
}

void asio_server::send_all_except(client_id id,boost::asio::const_buffer buffer, send_handler* handler, send_type copy_buffer)
{
    send_if(buffer, handler, create_predicate ( boost::bind( std::not_equal_to<client_id>(), id, _1) ), copy_buffer );
}


void asio_server::send_if(boost::asio::const_buffer buffer, send_handler* handler,
                          const client_predicate&  predicate, send_type copy_buffer)
{
    // This allows me to copy the buffer only once for many send operations
    ana::detail::shared_buffer s_buf( new ana::detail::copying_buffer( buffer, copy_buffer ) ); // it's a boost::shared_ptr

    for (std::list<client_proxy*>::iterator it(client_proxies_.begin()); it != client_proxies_.end(); ++it)
        if ( predicate.selects( (*it)->id() ) )
            (*it)->send(s_buf, handler, this);
}

void asio_server::send_all(boost::asio::const_buffer buffer, send_handler* handler, send_type copy_buffer )
{
    // This allows me to copy the buffer only once for many send operations
    ana::detail::shared_buffer s_buf(new ana::detail::copying_buffer( buffer, copy_buffer ) ); // it's a boost::shared_ptr

    std::for_each(client_proxies_.begin(), client_proxies_.end(), boost::bind(&client_proxy::send, _1,
                                                                              s_buf, handler, this));
}

asio_server::asio_client_proxy::asio_client_proxy(boost::asio::io_service& io_service, asio_proxy_manager* server) :
    client_proxy(),
    socket_(io_service),
    asio_listener(io_service, socket_),
    manager_(server)
{
}

asio_server::asio_client_proxy::~asio_client_proxy()
{
    manager_->deregister_client( this );
}

tcp::socket& asio_server::asio_client_proxy::socket()
{
    return socket_;
}

void asio_server::asio_client_proxy::handle_sent_header(const boost::system::error_code& ec,
                                                        mili::bostream* bos, ana::detail::shared_buffer buffer,
                                                        send_handler* handler, timer* running_timer)
{
    delete bos;

    if ( ! ec )
    {
        boost::asio::async_write(socket_, boost::asio::buffer(buffer->base(), buffer->size() ),
                                    boost::bind(&asio_server::asio_client_proxy::handle_send,this,
                                                boost::asio::placeholders::error,
                                                buffer, handler, running_timer));

    }
    else
    {
        disconnect_listener();
        running_timer->cancel();
    }
}


void asio_server::asio_client_proxy::handle_send(const boost::system::error_code& ec,
                                                 ana::detail::shared_buffer buffer,
                                                 send_handler* handler, timer* running_timer)
{
    running_timer->cancel();

    handler->handle_send( ec, id() );

    if ( ec )
        disconnect_listener();
}

void asio_server::asio_client_proxy::handle_timeout(const boost::system::error_code& ec, ana::send_handler* handler)
{
    if ( ec != boost::asio::error::operation_aborted) // The timer wasn't cancelled. So: inform this and disconnect
        handler->handle_send( boost::asio::error::make_error_code( boost::asio::error::timed_out ) , id() );
}

void asio_server::asio_client_proxy::send(ana::detail::shared_buffer buffer,
                                          send_handler* handler,
                                          ana::detail::timed_sender* sender)
{
    timer* running_timer( NULL );
    //timer is either running or wasn't created
    try
    {
        running_timer = sender->start_timer( buffer,
                                             boost::bind(&asio_server::asio_client_proxy::handle_timeout, this,
                                                         boost::asio::placeholders::error, handler ) );

        bostream* output_stream = new bostream();
        (*output_stream) << buffer->size();

        //write the header first in a separate operation, then send the full buffer
        boost::asio::async_write(socket_, boost::asio::buffer( output_stream->str() ),
                                    boost::bind(&asio_server::asio_client_proxy::handle_sent_header,this,
                                                boost::asio::placeholders::error, output_stream,
                                                buffer, handler, running_timer));
    }
    catch(std::exception& e)
    {
        disconnect_listener();
        running_timer->cancel();
    }
}

void asio_server::asio_client_proxy::disconnect_listener()
{
    socket_.cancel();
    delete this;
}

