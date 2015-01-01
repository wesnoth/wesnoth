
/**
 * @file
 * @brief Network API implementation using ana.
 *
 * Copyright (C) 2010 - 2015 Guillermo Biset.
 *
 * Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 *
 * See the COPYING file for more details.
 */

#include "network.hpp"
#include "network_manager_ana.hpp"

#include "global.hpp"

#include "config.hpp"

#include "gettext.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/parser.hpp"
#include "thread.hpp"
#include "util.hpp"

#include <cerrno>
#include <queue>
#include <iomanip>
#include <set>
#include <cstring>

#include <signal.h>
#if defined(_WIN32) || defined(__WIN32__) || defined (WIN32)
#undef INADDR_ANY
#undef INADDR_BROADCAST
#undef INADDR_NONE
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>  // for TCP_NODELAY
#ifdef __BEOS__
#include <socket.h>
#else
#include <fcntl.h>
#endif
#define SOCKET int
#endif

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)
// Only warnings and not errors to avoid DoS by log flooding
namespace
{
    ana_network_manager        ana_manager;
    network::bandwidth_in_ptr  global_bandwidth_in_ptr( new network::bandwidth_in(4) );
    //TODO: no global bandwidth

    size_t instances_using_the_network_module( 0 );
} // end namespace

namespace network {

    /**
    * Amount of seconds after the last server ping when we assume to have timed out.
    * When set to '0' ping timeout isn't checked.
    * Gets set in preferences::manager according to the preferences file.
    */
    unsigned int ping_timeout = 0;

    connection_stats::connection_stats(int sent, int received, int connected_at)
        : bytes_sent(sent), bytes_received(received), time_connected(0 - connected_at)
                                                        // TODO: s/SDLGetTicks/0/
    {
    }

    connection_stats get_connection_stats(connection connection_num)
    {
        const ana::stats* stats = ana_manager.get_stats( connection_num );

        if ( stats == NULL )
            throw std::runtime_error("Invalid connection ID to get stats from.");
        else
            return connection_stats( stats->bytes_out(),
                                     stats->bytes_in(),
                                     stats->uptime() );
    }

    error::error(const std::string& msg, connection sock) : game::error(msg), socket(sock)
    {
    }

    void error::disconnect()
    {
    }

    // --- Proxy methods
    void enable_connection_through_proxy()
    {
        ana_manager.enable_connection_through_proxy();
    }
    void set_proxy_address ( const std::string& address  )
    {
        ana_manager.set_proxy_address( address );
    }

    void set_proxy_port    ( const std::string& port     )
    {
        ana_manager.set_proxy_port( port );
    }

    void set_proxy_user    ( const std::string& user     )
    {
        ana_manager.set_proxy_user( user );
    }

    void set_proxy_password( const std::string& password )
    {
        ana_manager.set_proxy_password( password );
    }
    // --- End Proxy methods


    pending_statistics get_pending_stats()
    {
        //TODO: implement this feature, this is only to avoid segfaults when /query netstats is sent
        pending_statistics result;

        result.npending_sends       = 0;
        result.nbytes_pending_sends = 0;

        return result;
    }

    manager::manager(size_t /*min_threads*/, size_t /*max_threads*/) : free_(true)
    {
        ++instances_using_the_network_module;
    }

    manager::~manager()
    {
        if ( --instances_using_the_network_module == 0 )
            ana_manager.close_connections_and_cleanup();
    }

    void set_raw_data_only()
    {
    }

    server_manager::server_manager(int port, CREATE_SERVER create_server)
        : free_(false),
          connection_(0)
    {
        if ( create_server != NO_SERVER )
        {
            ana::net_id server_id = ana_manager.create_server( );
            ana_manager.run_server( server_id, port);
        }
    }

    server_manager::~server_manager()
    {
    }

    void server_manager::stop()
    {
        throw std::runtime_error("TODO:Not implemented stop");
    }

    bool server_manager::is_running() const
    {
        throw std::runtime_error("TODO:Not implemented is_running");
    }

    size_t nconnections()
    {
        return ana_manager.number_of_connections();
    }

    bool is_server()
    {
        throw std::runtime_error("TODO:Not implemented is_server");
    }

    connection connect(const std::string& host, int port)
    {
        return ana_manager.create_client_and_connect( host, port );
    }

    connection connect(const std::string& host, int port, threading::waiter& /*waiter*/)
    {
        return connect(host,port);
    }

    connection accept_connection()
    {
        return ana_manager.new_connection_id();
    }

    bool disconnect(connection handle)
    {
        return ana_manager.disconnect( handle );
    }

    void queue_disconnect(network::connection handle )
    {
        ana_manager.disconnect( handle );
    }

    connection receive_data(config&           cfg,
                            connection        connection_num,
                            unsigned int      timeout,
                            bandwidth_in_ptr* bandwidth_in)
    {
        ana_manager.throw_if_pending_disconnection();

        //TODO: temporary fix
        if ( bandwidth_in != NULL )
            *bandwidth_in = global_bandwidth_in_ptr;

        network::connection read_id = ana_manager.read_from( connection_num, cfg, timeout );

        // TODO: check timeout and return 0, or throw if error occurred

        return read_id;
    }

    connection receive_data(config&           cfg,
                            connection        connection_num,
                            bandwidth_in_ptr* bandwidth_in) // TODO: use this pointer
    {
        // <- just call the previous version without timeouts
        return receive_data(cfg,connection_num, static_cast<unsigned>(0), bandwidth_in);
    }

    connection receive_data(std::vector<char>& buf, bandwidth_in_ptr* bandwidth_in)
    {
        ana_manager.throw_if_pending_disconnection();

        //TODO: temporary fix
        if ( bandwidth_in != NULL )
            *bandwidth_in = global_bandwidth_in_ptr;

        return ana_manager.read_from_all( buf );
    }


    void add_bandwidth_out(const std::string& /*packet_type*/, size_t /*len*/)
    {
        //TODO: implement? (apparently called from network_worker only
    }

    void add_bandwidth_in(const std::string& /*packet_type*/, size_t /*len*/)
    {
        //TODO: implement? (apparently called from network_worker only
    }


    std::string get_bandwidth_stats_all()
    {
        //TODO: packet_type and widths should be modifiable
        const char*  packet_type  = "network";

        const size_t field_width  = 8;
        const size_t packet_width = 8;
        const size_t bytes_width  = 8;

        const ana::stats* stats = ana_manager.get_stats( );

        std::stringstream ss;
        ss  << " " << std::setw(field_width) <<  packet_type << "| "
            << std::setw(packet_width)<< stats->packets_out()<< "| "
            << std::setw(bytes_width) << stats->bytes_out() /1024 << "| "
            << std::setw(packet_width)<< stats->packets_in()<< "| "
            << std::setw(bytes_width) << stats->bytes_in() /1024 << "\n";

        return ss.str();
    }

    std::string get_bandwidth_stats()
    {
        //TODO: packet_type and widths should be modifiable
        const char*  packet_type  = "network";

        const size_t field_width  = 8;
        const size_t packet_width = 8;
        const size_t bytes_width  = 8;

        const ana::stats* stats = ana_manager.get_stats( 0, ana::HOURS );


        std::stringstream ss;
        ss  << " " << std::setw(field_width) <<  packet_type << "| "
            << std::setw(packet_width)<< stats->packets_out()<< "| "
            << std::setw(bytes_width) << stats->bytes_out() /1024 << "| "
            << std::setw(packet_width)<< stats->packets_in()<< "| "
            << std::setw(bytes_width) << stats->bytes_in() /1024 << "\n";

        return ss.str();
    }

    std::string get_bandwidth_stats(int /*hour*/)
    {
        return std::string(""); //TODO: implement
    }

    bandwidth_in::~bandwidth_in()
    {
    }

    void send_file(const std::string& /*filename*/,
                   connection /*connection_num*/,
                   const std::string& /*packet_type*/)
    {
        throw std::runtime_error("TODO:Not implemented send_file");
    }

    size_t send_data(const config&                cfg,
                     connection                   connection_num,
                     const std::string&           /*packet_type*/)
    {
        if(cfg.empty())
            return 0;

        if( connection_num == 0 )
            return ana_manager.send_all( cfg );
        else
            return ana_manager.send( connection_num, cfg );
    }

    void send_raw_data(const char*        buf,
                       int                len,
                       connection         connection_num,
                       const std::string& /*packet_type*/)
    {
        ana_manager.send_raw_data( buf, size_t( len ), connection_num );
    }

    void process_send_queue(connection, size_t)
    {
        ana_manager.throw_if_pending_disconnection();
    }

    void send_data_all_except(const config&       cfg,
                              connection          connection_num,
                              const std::string&  /*packet_type*/)
    {
        ana_manager.send_all_except(cfg, connection_num);
    }

    std::string ip_address(connection connection_num)
    {
        return ana_manager.ip_address( connection_num );
    }

    statistics get_send_stats(connection handle)
    {
        return ana_manager.get_send_stats( handle );
    }

    statistics get_receive_stats(connection handle)
    {
        return ana_manager.get_receive_stats( handle );
    }
}// end namespace network
