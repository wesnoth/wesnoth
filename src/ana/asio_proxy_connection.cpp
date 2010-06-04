#include "asio_proxy_connection.hpp"

proxy_connection::proxy_connection(tcp::socket& socket, proxy_information pi, ana::address address, ana::port port) :
    socket_(socket),
    proxy_info_(pi),
    address_(address),
    port_(port)
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

void proxy_connection::handle_response(ana::detail::read_buffer         buf,
                                       const boost::system::error_code& ec,
                                       ana::connection_handler*         handler)
{
    //TODO: interpret the response and act accordingly
    std::cout << "Received: " << buf->string() << "\n"; //to debug for now!
}

void proxy_connection::handle_sent_request(const boost::system::error_code& ec,
                                           std::string*                     request,
                                           ana::connection_handler*         handler)
{
    delete request;

    //TODO: use a meaningful constant instead of 100
    ana::detail::read_buffer read_buf( new ana::detail::read_buffer_implementation( 100 ) );

    boost::asio::async_read(socket_, boost::asio::buffer( read_buf->base(), read_buf->size() ),
                            boost::bind(&proxy_connection::handle_response, this,
                                        read_buf, boost::asio::placeholders::error, handler));

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
