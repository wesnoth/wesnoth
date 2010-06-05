#include <sstream>

#include "asio_proxy_connection.hpp"

proxy_connection::proxy_connection(tcp::socket& socket, proxy_information pi, ana::address address,
                                   ana::port port, proxy_connection_manager* manager) :
    socket_(socket),
    proxy_info_(pi),
    address_(address),
    port_(port),
    manager_(manager)
{
}


std::string* proxy_connection::generate_connect_request() const
{
    return new std::string
    (
        "CONNECT " + address_ + ":" + port_ + " HTTP/1.0\n"
        "User-agent: ana 0.1 \n"
        "\n"
    );
}

void proxy_connection::handle_response(boost::asio::streambuf*          buf,
                                       const boost::system::error_code& ec,
                                       ana::connection_handler*         handler)
{
    //TODO: interpret the response and act accordingly
    if ( ec )
        handler->handle_connect(ec, 0);
    else
    {
        std::stringstream ss;
        ss << buf;

        const size_t find_pos = ss.str().find( std::string( "200 Connection established" ) );

        if ( find_pos < ss.str().size() )
        {
            handler->handle_connect( ec, 0 );
            manager_->handle_proxy_connection();
        }
    }
    delete buf;
}

void proxy_connection::handle_sent_request(const boost::system::error_code& ec,
                                           std::string*                     request,
                                           ana::connection_handler*         handler)
{
    delete request;

    boost::asio::streambuf* buf = new boost::asio::streambuf( 500 );

    boost::asio::async_read_until(socket_, *buf,
                                  "\r\n\r\n",
                                  boost::bind(&proxy_connection::handle_response, this,
                                              buf, boost::asio::placeholders::error, handler));

}

void proxy_connection::handle_connect(const boost::system::error_code& ec,
                                      tcp::resolver::iterator          endpoint_iterator,
                                      ana::connection_handler*         handler)
{
    if ( ! ec )
    {
        std::string* request( generate_connect_request() );

        socket_.async_send(boost::asio::buffer( *request ),
                                 boost::bind(&proxy_connection::handle_sent_request,this,
                                             boost::asio::placeholders::error,
                                             request, handler));
    }
    else
    {
        if ( endpoint_iterator == tcp::resolver::iterator() ) // could not connect to proxy
            handler->handle_connect( ec, 0 );
        else
        {
            //retry
            socket_.close();

            tcp::endpoint endpoint = *endpoint_iterator;
            socket_.async_connect(endpoint,
                                boost::bind(&proxy_connection::handle_connect, this,
                                            boost::asio::placeholders::error, ++endpoint_iterator, handler));
        }
    }
}

void proxy_connection::connect( ana::connection_handler* handler )
{
    try
    {
        tcp::resolver resolver( socket_.get_io_service() );
        tcp::resolver::query query(proxy_info_.proxy_address.c_str(), proxy_info_.proxy_port.c_str() );
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::endpoint endpoint = *endpoint_iterator;
        socket_.async_connect(endpoint,
                              boost::bind(&proxy_connection::handle_connect, this,
                                          boost::asio::placeholders::error, ++endpoint_iterator, handler));
    }
    catch (const std::exception& e)
    {
        handler->handle_connect( boost::system::error_code(1,boost::system::get_generic_category() ), 0 );
        std::cerr << "Client: An error ocurred, " << e.what() << std::endl;
    }
}
