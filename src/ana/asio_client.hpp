#include <boost/asio.hpp>
#include <memory>

using boost::asio::ip::tcp;

#include "ana.hpp"

#include "asio_listener.hpp"
#include "mili/mili.h"

#ifndef ASIO_CLIENT_HPP
#define ASIO_CLIENT_HPP

class asio_client : public ana::client,
                    public asio_listener
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
        virtual ~asio_client() {}

        virtual void connect( ana::connection_handler* );

        virtual void run();

        virtual ana::client_id id() const;

        virtual void send( boost::asio::const_buffer, ana::send_handler*, ana::send_type );

        virtual void disconnect_listener();

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
};

#endif