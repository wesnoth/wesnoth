/* $Id$ */

/*
Hello reader, welcome:

This document provides a proposal (in C++ header-file format) for
a new network API for Wesnoth.

The idea is to act as an intermediary to the ANA API & Library, which
is already under use in the game, but to implement the old API (thought
to work with SDLnet.)

The point is that the new API will use ANA's asynchronous capabilities,
thus reducing CPU usage by the network implementation and simplifying
the code.

An important point is that this new API would shift control in some cases:
e.g. instead of calling every now and then to see if a new client has
connected, the object implementing the corresponding interfaces will get a
call through the corresponding method.

References:

 - ANA : You can look at the ANA source code in src/ana. Some documentation
         about ANA can be found in doc/ana (LaTeX files).
 - asio : ANA is implemented using the boost::asio library, you can find
          documentation for it in boost.org.
 - Code : File network.hpp in trunk holds the current Wesnoth network API.
          This API has many deprecated methods and the implementation
          currently calls for many blocking calls (which is not desired.)

          Files network.cpp and network_worker.cpp hold the current SDLnet
          implementation of this API.

          Files network_ana.cpp and network_manager_ana.Xpp hold the current
          ANA implementation of the API.


Please submit all feature requests you deem relevant in the following section.

Feature Requests:

*/

//Code that needs to create a network component will implement this interface

struct wesnoth_network_handler
{
    // Called by the implementation when a client has connected to the server
    void handle_connect(ana::error_code, ana::net_id);

    // Called by the implementation when a client has received a message
    void handle_receive(ana::error_code, ana::net_id, operation_id, config&);

    // Called by the implementation when a client has disconnected from the server
    void handle_disconnect(ana::error_code, ana::net_id);

    // Called by the implementation when a client async send operation has completed
    void handle_send(ana::error_code, ana::net_id, operation_id);
};

//Code that needs to create network components will USE these objects

namespace network
{
    struct client
    {
        // Create a client, and associate a handler object to it
        client( wesnoth_client_interface* handler );

        // Example, set_timeout( network::SEND_OPERATIONS, ana::time::seconds( 10 ) );
        //          set_timeout( network::SEND_OPERATIONS, ana::KILOBYTES(1),
        //                                                 ana::time::seconds( 1 ) ); (?)
        //          set_timeout( network::CONNECT_OPERATIONS, ana::time::seconds( 30 ) );
        void set_timeout( ... );

        // Attempt async connection, will call handle_connect with results eventually
        operation_id async_connect( ... );

        // Attempt async connection through proxy, will call handle_connect with results eventually
        operation_id async_connect_through_proxy( ... );

        // Attempt to send a WML config to the server
        operation_id async_send( const cfg&, ana::SEND_TYPE );

        // Signal the client that you are waiting for a message from the server
        // The time parameter indicates how long you are willing to wait
        // If a message is received before this time period the appropiate call to
        // handle_receive will be made with this returning operation_id, otherwise
        // handle_receive will be called with ana::timeout_error
        operation_id waiting_for_message( time );

        // Get network stats for the client, the parameter indicates the time period
        // See the stats interface in ana/api/stats.hpp:53
        ana::stats* get_stats( ana::stat_type = ana::ACCUMULATED );

        // Cancel an operation
        void cancel( operation_id );

        // Force disconnect
        void disconnect();

        // Returns the string representing the IP address of the remote endpoint (server)
        std::string ip_address_of_server() const;
    };

    struct server
    {
        // Create a server, and associate a handler object to it, throws if can't open port(?)
        server( wesnoth_server_interface* handler, port );

        // Set timeouts for a given client
        // Example, set_timeout( 1, network::SEND_OPERATIONS, ana::time::seconds( 10 ) );
        //          set_timeout( 2, network::SEND_OPERATIONS, ana::KILOBYTES(1),
        //                                                    ana::time::seconds( 1 ) ); (?)
        void set_timeout( ana::net_id, ... );

        // Attempt to send a WML config to a client
        operation_id async_send( ana::net_id, const cfg&, ana::SEND_TYPE );

        // Attempt to send a WML config to every connected client
        operation_id async_send( const cfg&, ana::SEND_TYPE );

        // Attempt to send a WML config to every connected client in the container
        operation_id async_send( container_of_ids,  const cfg&, ana::SEND_TYPE );

        // Attempt to send a WML config to every connected client except one
        operation_id async_send_except( ana::net_id,  const cfg&, ana::SEND_TYPE );

        // Attempt to send a WML config to every connected client except those in the container
        operation_id async_send_except( container_of_ids,  const cfg&, ana::SEND_TYPE );

        // Signal the server that you are waiting for a message from a given client
        // The time parameter indicates how long you are willing to wait
        // If a message is received before this time period the appropiate call to
        // handle_receive will be made with this returning operation_id, otherwise
        // handle_receive will be called with ana::timeout_error
        operation_id waiting_for_message( ana::net_id, time );

        // Get network stats for the server, the parameter indicates the time period
        // See the stats interface in ana/api/stats.hpp:53
        ana::stats* get_stats( ana::stat_type = ana::ACCUMULATED );

        // Get network stats for a given client, the parameter indicates the time period
        // See the stats interface in ana/api/stats.hpp:53
        ana::stats* get_stats( ana::net_id, ana::stat_type = ana::ACCUMULATED );

        // Cancel an operation
        void cancel( operation_id );

        // Force disconnect on the server, will disconnect every connected client
        void disconnect();

        // Force the disconnection of a client
        void disconnect(ana::net_id);

        // Returns the string representing the IP address of a connected client
        std::string ip_address( ana::net_id ) const;
    };
}
