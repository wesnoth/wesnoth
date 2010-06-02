#ifndef ASIO_LISTENER_HPP
#define ASIO_LISTENER_HPP

#include <boost/asio.hpp>
#include <memory>

#include "ana.hpp"

using boost::asio::ip::tcp;

class asio_listener : public virtual ana::detail::listener
{
    public:
        asio_listener( boost::asio::io_service&, tcp::socket&);

        virtual void set_listener_handler( ana::listener_handler* listener);
        virtual void run_listener();

        virtual ~asio_listener();
    private:
        virtual void disconnect_listener() {}

        void listen_one_message();

        void disconnect( ana::listener_handler* listener, boost::system::error_code error);

        void handle_header(char* header, const boost::system::error_code& , ana::listener_handler* );

        void handle_body( ana::detail::read_buffer , const boost::system::error_code& , ana::listener_handler* );

        /*attr*/
        boost::asio::io_service&   io_service_;
        tcp::socket&               socket_;
        ana::listener_handler*     listener_;
        char                       header_[ana::HeaderLength];
};

#endif