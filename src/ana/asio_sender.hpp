#ifndef ASIO_SENDER_HPP
#define ASIO_SENDER_HPP

#include <boost/asio.hpp>
#include <memory>

#include "ana.hpp"

using boost::asio::ip::tcp;

class asio_listener : public virtual ana::listener
{
    public:
        asio_listener( tcp::socket& );

    private:
        virtual void disconnect_listener() {}

        void listen_one_message();

        void disconnect( ana::listener_handler* listener, boost::system::error_code error);

        void handle_header(char* header, const boost::system::error_code& , ana::listener_handler* );

        void handle_body(char* body, size_t , const boost::system::error_code& , ana::listener_handler* );

        /*attr*/
        boost::asio::io_service&   io_service_;
        tcp::socket&               socket_;
        ana::listener_handler*     listener_;
};

#endif