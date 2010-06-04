#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#include "ana.hpp"

#ifndef ASIO_PROXY_CONNECTION
#define ASIO_PROXY_CONNECTION

struct proxy_information
{
    proxy_information() :
        auth_type(ana::proxy::none),
        proxy_address(),
        proxy_port(),
        user_name(),
        password()
    {
    }

    ana::proxy::authentication_type auth_type;
    std::string                     proxy_address;
    std::string                     proxy_port;
    std::string                     user_name;
    std::string                     password;
};

class proxy_connection
{
    public:
        proxy_connection(tcp::socket& socket, proxy_information pi, ana::address address, ana::port port);

        void connect( ana::connection_handler* handler );

    private:
        std::string* generate_connect_request() const;

        void handle_connect(const boost::system::error_code& ec,
                            tcp::resolver::iterator endpoint_iterator,
                            ana::connection_handler* handler);

        void handle_sent_request(const boost::system::error_code& ec,
                                 std::string* request, ana::connection_handler* handler);

        void handle_response( ana::detail::read_buffer ,
                              const boost::system::error_code& ,
                              ana::connection_handler* );

        // Attributes
        tcp::socket&      socket_;

        const proxy_information proxy_info_;

        ana::address      address_;
        ana::port         port_;
};

#endif