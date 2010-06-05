#include <boost/asio.hpp>
#include <memory>

using boost::asio::ip::tcp;

#include "ana.hpp"

#include "asio_proxy_connection.hpp"
#include "asio_listener.hpp"
#include "mili/mili.h"

#ifndef ASIO_CLIENT_HPP
#define ASIO_CLIENT_HPP

class asio_client : public ana::client,
                    public asio_listener,
                    private proxy_connection_manager
{
    public:
        /**
         * Standard constructor.
         *
         * @param address : Address to try to connect to. The server should be there.
         * @param port : port to use for the connection. The server should have opened it.
         */
        asio_client(std::string address, ana::port port);

    private:
        virtual ~asio_client();

        virtual void connect( ana::connection_handler* );

        virtual void connect_through_proxy(ana::proxy::authentication_type auth_type,
                                           std::string                     proxy_address,
                                           std::string                     proxy_port,
                                           ana::connection_handler*        handler,
                                           std::string                     user_name = "",
                                           std::string                     password = "");

        virtual void run();

        virtual ana::client_id id() const;

        virtual void send( boost::asio::const_buffer, ana::send_handler*, ana::send_type );

        virtual void disconnect_listener();

        virtual void handle_proxy_connection();

        void handle_sent_header(const boost::system::error_code& ec,
                                mili::bostream*, ana::detail::shared_buffer,
                                ana::send_handler*);

                                void handle_send(const boost::system::error_code& ec,
                                                 ana::detail::shared_buffer,
                                                 ana::send_handler*);

        void handle_connect(const boost::system::error_code& ec,
                            tcp::resolver::iterator endpoint_iterator,
                            ana::connection_handler* );

        /* Override, as per -Weffc++ */
        asio_client(const asio_client& other);
        asio_client& operator= (const asio_client& other);

        /*attr*/
        boost::asio::io_service   io_service_;
        tcp::socket               socket_;

        std::string               address_;
        ana::port                 port_;

        proxy_connection*         proxy_;
        bool                      use_proxy_;
};

#endif