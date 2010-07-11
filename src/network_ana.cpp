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
    ana_network_manager ana_manager;
}

namespace {

// We store the details of a connection in a map that must be looked up by its handle.
// This allows a connection to be disconnected and then recovered,
// but the handle remains the same, so it's all seamless to the user.
struct connection_details {
    connection_details(TCPsocket sock, const std::string& host, int port)
        : sock(sock), host(host), port(port), remote_handle(0),
          connected_at( 0 ) // TODO: connected_at(SDL_GetTicks())
    {}

    TCPsocket sock;
    std::string host;
    int port;

    // The remote handle is the handle assigned to this connection by the remote host.
    // Is 0 before a handle has been assigned.
    int remote_handle;

    int connected_at;
};

typedef std::map<network::connection,connection_details> connection_map;
connection_map connections;


} // end anon namespace


static void check_error()
{
//     std::cout << "DEBUG: check_error\n";
}

namespace {

// SDLNet_SocketSet socket_set = 0;
typedef std::set<TCPsocket> socket_set_type;
socket_set_type socket_set;
std::set<network::connection> waiting_sockets;
typedef std::vector<network::connection> sockets_list;
sockets_list sockets;


struct partial_buffer {
    partial_buffer() :
        buf(),
        upto(0)
    {
    }

    std::vector<char> buf;
    size_t upto;
};

std::deque<network::connection> disconnection_queue;
std::set<network::connection> bad_sockets;

// network_worker_pool::manager* worker_pool_man = NULL;

} // end anon namespace

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
        if(socket) {
            bad_sockets.insert(socket);
        }
    }

    void error::disconnect()
    {
        std::cout << "DEBUG: error::disconnect\n";
        if(socket) network::disconnect(socket);
    }

    pending_statistics get_pending_stats()
    {
        throw std::runtime_error("TODO:Not implemented get_pending_stats");
    }

    manager::manager(size_t /*min_threads*/, size_t /*max_threads*/) : free_(true)
    {
        std::cout << "DEBUG: Creating a manager object.\n";
    }

    manager::~manager()
    {
    //     throw std::runtime_error("TODO:Not implemented");
        std::cout << "DEBUG: destroying the manager object.\n";
        ana_manager.close_connections_and_cleanup();
    }

    void set_raw_data_only()
    {
        std::cout << "DEBUG: error::disconnect\n";
        //     throw std::runtime_error("TODO:Not implemented");
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
        stop();
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

    namespace {

    connection accept_connection_pending(std::vector<TCPsocket>& /*pending_sockets*/,
                                        socket_set_type&        /*pending_socket_set*/)
    {
        throw std::runtime_error("TODO:Not implemented accept_connection_pending");
    }

    } //anon namespace

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
//         disconnection_queue.push_back(sock);
    }

    connection receive_data(config&           cfg,
                            connection        connection_num,
                            unsigned int      timeout,
                            bandwidth_in_ptr* /*bandwidth_in*/)
    {
        // comment next debug msg: too much output
//         std::cout << "DEBUG: Trying to read from connection in " << timeout << " milliseconds.\n";
        network::connection read_id = ana_manager.read_from( connection_num, cfg, timeout );

        // TODO: check timeout and return 0, or throw if error occured

        return read_id;
    }

    connection receive_data(config&           cfg,
                            connection        connection_num,
                            bool*             /*gzipped*/,
                            bandwidth_in_ptr* /*bandwidth_in*/) // TODO: use this pointer
    {
        return receive_data(cfg,connection_num, static_cast<unsigned>(0), NULL); // <- just call the previous version without timeouts
    }

    connection receive_data(std::vector<char>& buf, bandwidth_in_ptr* /*bandwidth_in*/)
    {
//         std::cout << "DEBUG: Trying to read to a vector<char>.\n";
        return ana_manager.read_from_all( buf );
    }

    struct bandwidth_stats {
        int out_packets;
        int out_bytes;
        int in_packets;
        int in_bytes;
        int day;
        const static size_t type_width = 16;
        const static size_t packet_width = 7;
        const static size_t bytes_width = 10;
        bandwidth_stats& operator+=(const bandwidth_stats& a)
        {
            out_packets += a.out_packets;
            out_bytes += a.out_bytes;
            in_packets += a.in_packets;
            in_bytes += a.in_bytes;

            return *this;
        }
    };

    typedef std::map<const std::string, bandwidth_stats> bandwidth_map;
    typedef std::vector<bandwidth_map> hour_stats_vector;
    hour_stats_vector hour_stats(24);

    static bandwidth_map::iterator add_bandwidth_entry(const std::string& packet_type)
    {
        time_t now = time(0);
        struct tm * timeinfo = localtime(&now);
        int hour = timeinfo->tm_hour;
        int day = timeinfo->tm_mday;
        assert(hour < 24 && hour >= 0);
        std::pair<bandwidth_map::iterator,bool> insertion = hour_stats[hour].insert(std::make_pair(packet_type, bandwidth_stats()));
        bandwidth_map::iterator inserted = insertion.first;
        if (!insertion.second && day != inserted->second.day)
        {
            // clear previuos day stats
            hour_stats[hour].clear();
            //insert again to cleared map
            insertion = hour_stats[hour].insert(std::make_pair(packet_type, bandwidth_stats()));
            inserted = insertion.first;
        }

        inserted->second.day = day;
        return inserted;
    }

    typedef boost::shared_ptr<bandwidth_stats> bandwidth_stats_ptr;


    struct bandwidth_stats_output {
        bandwidth_stats_output(std::stringstream& ss) : ss_(ss), totals_(new bandwidth_stats())
        {}
        void operator()(const bandwidth_map::value_type& stats)
        {
            // name
            ss_ << " " << std::setw(bandwidth_stats::type_width) <<  stats.first << "| "
                << std::setw(bandwidth_stats::packet_width)<< stats.second.out_packets << "| "
                << std::setw(bandwidth_stats::bytes_width) << stats.second.out_bytes/1024 << "| "
                << std::setw(bandwidth_stats::packet_width)<< stats.second.in_packets << "| "
                << std::setw(bandwidth_stats::bytes_width) << stats.second.in_bytes/1024 << "\n";
            *totals_ += stats.second;
        }
        void output_totals()
        {
            (*this)(std::make_pair(std::string("total"), *totals_));
        }
        private:
        std::stringstream& ss_;
        bandwidth_stats_ptr totals_;
    };

    std::string get_bandwidth_stats_all()
    {
        std::string result;
        for (int hour = 0; hour < 24; ++hour)
        {
            result += get_bandwidth_stats(hour);
        }
        return result;
    }

    std::string get_bandwidth_stats()
    {
        time_t now = time(0);
        struct tm * timeinfo = localtime(&now);
        int hour = timeinfo->tm_hour - 1;
        if (hour < 0)
            hour = 23;
        return get_bandwidth_stats(hour);
    }

    std::string get_bandwidth_stats(int hour)
    {
        assert(hour < 24 && hour >= 0);
        std::stringstream ss;

        ss << "Hour stat starting from " << hour << "\n " << std::left << std::setw(bandwidth_stats::type_width) <<  "Type of packet" << "| "
            << std::setw(bandwidth_stats::packet_width)<< "out #"  << "| "
            << std::setw(bandwidth_stats::bytes_width) << "out kb" << "| " /* Are these bytes or bits? base10 or base2? */
            << std::setw(bandwidth_stats::packet_width)<< "in #"  << "| "
            << std::setw(bandwidth_stats::bytes_width) << "in kb" << "\n";

        bandwidth_stats_output outputer(ss);
        std::for_each(hour_stats[hour].begin(), hour_stats[hour].end(), outputer);

        outputer.output_totals();
        return ss.str();
    }

    void add_bandwidth_out(const std::string& packet_type, size_t len)
    {
        bandwidth_map::iterator itor = add_bandwidth_entry(packet_type);
        itor->second.out_bytes += len;
        ++(itor->second.out_packets);
    }

    void add_bandwidth_in(const std::string& packet_type, size_t len)
    {
        bandwidth_map::iterator itor = add_bandwidth_entry(packet_type);
        itor->second.in_bytes += len;
        ++(itor->second.in_packets);
    }

    bandwidth_in::~bandwidth_in()
    {
        add_bandwidth_in(type_, len_);
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

        std::cout << "DEBUG: Sending: " << cfg << "\n";

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
        ana_manager.send_raw_data( buf, size_t( len ), connection_num );
//         throw std::runtime_error("TODO:Not implemented send_raw_data");
    }

    void process_send_queue(connection, size_t)
    {
        check_error();
    }

    /** @todo Note the gzipped parameter should be removed later. */
    void send_data_all_except(const config&       cfg,
                              connection          connection_num,
                              const bool          /*gzipped*/,
                              const std::string&  /*packet_type*/)
    {
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
        std::cout << "DEBUG: get_receive_stats\n";
        return ana_manager.get_receive_stats( handle );
    }

}// end namespace network
