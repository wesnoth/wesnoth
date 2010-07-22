/* $Id$ */

/**
 * @file network_ana.cpp
 * @brief Network API implementation using ana.
 *
 * Copyright (C) 2010 Guillermo Biset.
 *
 * Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * or at your option any later version.
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

#include "filesystem.hpp"

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
}

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
        std::cout << "DEBUG: get_connection_stats\n";
        const ana::stats* stats = ana_manager.get_stats( connection_num );

        return connection_stats( stats->bytes_out(),
                                 stats->bytes_in(),
                                 stats->uptime() ); //TODO: is uptime ok?
    }

    error::error(const std::string& msg, connection sock) : message(msg), socket(sock)
    {
        std::cout << "DEBUG: error::error\n";
    }

    void error::disconnect()
    {
        std::cout << "DEBUG: error::disconnect\n";
    }

    pending_statistics get_pending_stats()
    {
        throw std::runtime_error("TODO:Not implemented get_pending_stats");
    }

    manager::manager(size_t /*min_threads*/, size_t /*max_threads*/) : free_(true)
    {
        ++instances_using_the_network_module;
        std::cout << "DEBUG: Creating a manager object.\n";
    }

    manager::~manager()
    {
        std::cout << "DEBUG: destroying the manager object.\n";

        if ( --instances_using_the_network_module == 0 )
            ana_manager.close_connections_and_cleanup();
    }

    void set_raw_data_only()
    {
    }

    server_manager::server_manager(int port, CREATE_SERVER create_server) : free_(false), connection_(0)
    {
        std::cout << "DEBUG: server_manager\n";
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
        std::cout << "DEBUG: connect\n";
        return ana_manager.create_client_and_connect( host, port );
    }

    connection connect(const std::string& host, int port, threading::waiter& /*waiter*/)
    {
        std::cout << "DEBUG: connect2\n";
        return connect( host, port );
    }

    connection accept_connection()
    {
        return ana_manager.new_connection_id();
    }

    bool disconnect(connection /*s*/)
    {
        throw std::runtime_error("TODO:Not implemented disconnect");
    }

    void queue_disconnect(network::connection /*sock*/)
    {
        throw("TODO:Not implemented queue_disconnect");
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

//         if ( read_id != 0 )
//             std::cout << "Read: " << cfg << "\n";

        // TODO: check timeout and return 0, or throw if error occured

        return read_id;
    }

    connection receive_data(config&           cfg,
                            connection        connection_num,
                            bool*             /*gzipped*/,
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
        std::cout << "DEBUG: get_bandwidth_stats_all() for "
                  << ana_manager.number_of_connections() << " components.\n";



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
        //TODO: check if this is the intention
        return get_bandwidth_stats_all();
    }

    std::string get_bandwidth_stats(int hour)
    {
        std::cout << "DEBUG: get_bandwidth_stats with hour " << hour << "\n";
        return std::string(""); //TODO: implement
    }

    bandwidth_in::~bandwidth_in()
    {
    }

    void send_file(const std::string& /*filename*/, connection /*connection_num*/, const std::string& /*packet_type*/)
    {
        throw std::runtime_error("TODO:Not implemented send_file");
    }

    /**
    * @todo Note the gzipped parameter should be removed later, we want to send
    * all data gzipped. This can be done once the campaign server is also updated
    * to work with gzipped data.
    */
    size_t send_data(const config&                cfg,
                     connection                   connection_num,
                     const bool                   gzipped,
                     const std::string&           /*packet_type*/)
    {
        if(cfg.empty())
            return 0;

//         std::cout << "DEBUG: Sending: " << cfg << "\n";

        if( connection_num == 0 )
            return ana_manager.send_all( cfg, gzipped );
        else
            return ana_manager.send( connection_num, cfg, gzipped );
    }

    void send_raw_data(const char*        buf,
                       int                len,
                       connection         connection_num,
                       const std::string& /*packet_type*/)
    {
//         std::cout << "DEBUG: Sending Raw: " << std::string( buf, len ) << "\n";
        ana_manager.send_raw_data( buf, size_t( len ), connection_num );
    }

    void process_send_queue(connection, size_t)
    {
        ana_manager.throw_if_pending_disconnection();
    }

    /** @todo Note the gzipped parameter should be removed later. */
    void send_data_all_except(const config&       cfg,
                              connection          connection_num,
                              const bool          /*gzipped*/,
                              const std::string&  /*packet_type*/)
    {
//         std::cout << "DEBUG: Sending all except " << connection_num << " - " << cfg << "\n";
        ana_manager.send_all_except(cfg, connection_num);
    }

    std::string ip_address(connection connection_num)
    {
        std::cout << "DEBUG: ip_address\n";
        return ana_manager.ip_address( connection_num );
    }

    statistics get_send_stats(connection handle)
    {
        std::cout << "DEBUG: get_send_stats\n";
        return ana_manager.get_send_stats( handle );
    }

    statistics get_receive_stats(connection handle)
    {
        return ana_manager.get_receive_stats( handle );
    }
}// end namespace network
