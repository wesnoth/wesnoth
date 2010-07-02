// c/* $Id$ */

/**
 * @file network_ana.hpp
 * @brief Network implementation using ana.
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

#include <boost/variant.hpp>

#include "ana/api/ana.hpp"
#include "network.hpp"

#include "global.hpp"

#include "config.hpp"

#include "gettext.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/binary_or_text.hpp"
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

struct send_stats_logger
{
    virtual void update_send_stats( size_t ) = 0;
};

class ana_handler : public ana::send_handler
{
    public:
        ana_handler( boost::mutex& mutex, send_stats_logger* logger, size_t buf_size, size_t calls = 1 ) :
            mutex_(mutex),
            target_calls_( calls ),
            error_code_(),
            logger_( logger ),
            buf_size_( buf_size )
        {
            std::cout << "DEBUG: Constructing a new ana_handler...\n";
            if ( calls > 0 )
                mutex_.lock();
        }

        ~ana_handler()
        {
            std::cout << "DEBUG: Terminating an ana_handler...\n";
            if ( target_calls_ > 0 )
                throw std::runtime_error("Handler wasn't called enough times.");
        }

    private:
        virtual void handle_send(ana::error_code error_code, ana::net_id /*client*/)
        {
            if ( ! error_code )
                logger_->update_send_stats( buf_size_ );

            error_code_ = error_code;

            if ( --target_calls_ == 0 )
                mutex_.unlock();
        }

        boost::mutex&      mutex_;
        size_t             target_calls_;
        ana::error_code    error_code_;
        send_stats_logger* logger_;
        size_t             buf_size_;
};

class ana_connect_handler : public ana::connection_handler
{
    public:
        ana_connect_handler( boost::mutex& mutex, ana::timer* timer) :
            mutex_(mutex),
            timer_(timer),
            error_code_(),
            connected_(false)
        {
            std::cout << "DEBUG: Constructing a new ana_connect_handler...\n";
            mutex_.lock();
        }

        void handle_timeout(ana::error_code error_code)
        {
            if ( ! connected_ ) // disregard this call after a connect termination (regardless of result)
            {
                if (error_code)
                    std::cout << "DEBUG: Connection attempt timed out\n";
                else
                    std::cout << "DEBUG: Shouldn't reach here\n";

                error_code_ = error_code;
                mutex_.unlock();
            }
        }

        ~ana_connect_handler()
        {
            std::cout << "DEBUG: Terminating an ana_connect_handler...\n";
        }

        const ana::error_code& error() const
        {
            return error_code_;
        }

    private:
        virtual void handle_connect(ana::error_code error_code, ana::net_id /*client*/)
        {
            connected_ = true;
            timer_->cancel();

            if (! error_code)
                std::cout << "DEBUG: Connected.\n";
            else
                std::cout << "DEBUG: Can't connect.\n";

            error_code_ = error_code;
            mutex_.unlock();
        }

        boost::mutex&      mutex_;
        ana::timer*        timer_;
        ana::error_code    error_code_;
        bool               connected_;
};


class ana_component : public send_stats_logger
{
    public:
        ana_component( ) :
            base_( ana::server::create() ),
            is_server_( true ),
            id_( server()->id() ),
            send_stats_(),
            receive_stats_(),
            mutex_(),
            condition_(),
            buffers_()
        {
        }

        ana_component( const std::string& host, const std::string& port) :
            base_( ana::client::create(host,port) ),
            is_server_( false ),
            id_(  client()->id() ),
            send_stats_(),
            receive_stats_(),
            mutex_(),
            condition_(),
            buffers_()
        {
        }

        network::statistics get_send_stats() const
        {
            return send_stats_;
        }

        network::statistics get_receive_stats() const
        {
            return receive_stats_;
        }

        ana::server* server() const
        {
            if( ! is_server_ )
                throw std::runtime_error("Component is not a server.");

            return boost::get<ana::server*>(base_);
        }

        ana::client* client() const
        {
            if( is_server_ )
                throw std::runtime_error("Component is not a client.");

            return boost::get<ana::client*>(base_);
        }

        bool is_server() const
        {
            return is_server_;
        }

        bool is_client() const
        {
            return ! is_server_;
        }

        ana::net_id get_id() const
        {
            return id_;
        }

        const ana::stats* get_stats() const
        {
            if ( is_server_)
                return server()->get_stats();
            else
                return client()->get_stats();
        }

        void add_buffer(ana::detail::read_buffer buffer)
        {
            {
                boost::lock_guard<boost::mutex> lock(mutex_);
                buffers_.push( buffer );
            }
            condition_.notify_all();
        }

        ana::detail::read_buffer wait_for_element()
        {
            boost::unique_lock<boost::mutex> lock(mutex_);

            while(buffers_.empty())
                condition_.wait(lock);

            const ana::detail::read_buffer buffer_ret = buffers_.front();

            buffers_.pop();

            return buffer_ret;
        }


        void update_receive_stats( size_t buffer_size )
        {
            receive_stats_.current_max = ( buffer_size > receive_stats_.current_max) ? buffer_size : receive_stats_.current_max;
            receive_stats_.total      += buffer_size;
            receive_stats_.current     = buffer_size;
        }

    private:
        virtual void update_send_stats( size_t buffer_size)
        {
            send_stats_.current_max = ( buffer_size > send_stats_.current_max) ? buffer_size : send_stats_.current_max;
            send_stats_.total      += buffer_size;
            send_stats_.current     = buffer_size;
        }

        boost::variant<ana::server*, ana::client*> base_;

        bool        is_server_;
        ana::net_id id_;

        network::statistics send_stats_;
        network::statistics receive_stats_;

        //Buffer queue attributes
        boost::mutex                   mutex_;
        boost::condition_variable      condition_;

        std::queue< ana::detail::read_buffer > buffers_;
};

class clients_manager : public ana::connection_handler
{
    public:
        clients_manager() :
            ids_()
        {
        }

        size_t client_amount() const
        {
            return ids_.size();
        }

    private:
        virtual void handle_connect(ana::error_code error, ana::net_id client)
        {
            if (! error )
                ids_.insert(client);
        }

        virtual void handle_disconnect(ana::error_code /*error*/, ana::net_id client)
        {
            ids_.erase(client);
        }

        std::set<ana::net_id> ids_;
};

class ana_network_manager : public ana::listener_handler,
                            public ana::send_handler
{
    public:
        ana_network_manager() :
            connect_timer_( NULL ),
            components_(),
            server_manager_()
        {
        }

        ana::net_id create_server( )
        {
            std::cout << "DEBUG: Creating server.\n";

            ana_component* new_component = new ana_component( );
            components_.insert( new_component );

            ana::server* server = new_component->server();

            clients_manager* manager = new clients_manager();
            server_manager_[ server ] = manager;

            server->set_connection_handler( manager );
            server->set_listener_handler( this );

            return server->id();
        }

        network::connection create_client_and_connect(std::string host, int port)
        {
            std::cout << "DEBUG: Creating client and connecting...\n";

            try
            {
                std::stringstream ss;
                ss << port;

                ana_component* new_component = new ana_component( host, ss.str() );
                components_.insert( new_component );

                ana::client* const client = new_component->client();

                boost::mutex mutex;

                connect_timer_ = new ana::timer();

                ana_connect_handler handler(mutex, connect_timer_);

                connect_timer_->wait( ana::time::seconds(10), // 10 seconds to connection timeout
                                    boost::bind(&ana_connect_handler::handle_timeout, &handler,
                                                boost::asio::error::make_error_code( boost::asio::error::timed_out ) ) );

                client->connect( &handler );

                client->set_listener_handler( this );
                client->run();
                client->start_logging();

                mutex.lock();   // just wait for handler to release it
                mutex.unlock(); // unlock for destruction

                delete connect_timer_;

                if( ! handler.error() )
                {
                    //Send handshake
                    const std::string empty_str;
                    client->send( ana::buffer( empty_str ), this );

                    return network::connection( client->id() );
                }
                else
                    return 0;
            }
            catch( const std::exception& e )
            {
                std::cout << "DEBUG: Caught an exception while trying to connect.\n";
                return 0;
            }
        }

        const ana::stats* get_stats( network::connection connection_num )
        {
            ana::net_id id( connection_num );

            std::set<ana_component*>::iterator it;

            it = std::find_if( components_.begin(), components_.end(),
                               boost::bind(&ana_component::get_id, _1) == id );

            if ( it != components_.end())
                return (*it)->get_stats();

            return NULL;
        }

        void run_server(ana::net_id id, int port)
        {
            std::stringstream ss;
            ss << port;

            std::set<ana_component*>::iterator it;

            it = std::find_if( components_.begin(), components_.end(),
                               boost::bind(&ana_component::get_id, _1) == id );

            if ( it != components_.end())
                return (*it)->server()->run( ss.str() );
        }

        std::string ip_address( network::connection id )
        {
            std::set<ana_component*>::iterator it;

            for (it = components_.begin(); it != components_.end(); ++it)
            {
                if ( (*it)->is_server() )
                {
                    const std::string ip = (*it)->server()->ip_address( ana::net_id( id ) );
                    if  (ip != "")
                        return ip;
                }
            }
            return "";
        }

        size_t number_of_connections() const
        {
            return components_.size(); // TODO:check if this is the intention, guessing not
        }

        size_t send_all( const config& cfg, bool zipped )
        {
            std::cout << "DEBUG: Sending to everybody...\n";
            std::stringstream out;
            config_writer cfg_writer(out, zipped);
            cfg_writer.write(cfg);

            std::set<ana_component*>::iterator it;

            for (it = components_.begin(); it != components_.end(); ++it)
            {
                if ( (*it)->is_server() )
                {
                    boost::mutex mutex;
                    const size_t necessary_calls = server_manager_[ (*it)->server() ]->client_amount();

                    ana_handler handler( mutex, *it, out.str().size(), necessary_calls );

                    (*it)->server()->send_all( ana::buffer( out.str() ), &handler, ana::ZERO_COPY);

                    mutex.lock(); // the handler will release the mutex after necessary_calls calls
                }
            }
            return out.str().size();
        }

        size_t send( network::connection connection_num , const config& cfg, bool zipped )
        {
            std::cout << "DEBUG: Single send...\n";
            ana::net_id id( connection_num );

            std::stringstream out;
            config_writer cfg_writer(out, zipped);
            cfg_writer.write(cfg);

            std::set<ana_component*>::iterator it;

            for (it = components_.begin(); it != components_.end(); ++it)
            {
                if ( (*it)->is_server() )
                {
                    boost::mutex mutex;
                    ana_handler handler( mutex, *it, out.str().size() );
                    (*it)->server()->send_one( id, ana::buffer( out.str() ), &handler, ana::ZERO_COPY);
                    mutex.lock(); // this should work just fine
                }
            }
            return out.str().size();
        }

        ana::detail::read_buffer read_from( network::connection connection_num )
        {
            std::set<ana_component*>::iterator it;

            if ( connection_num == 0 )
            {
                if ( components_.empty() )
                    throw std::runtime_error("Trying to read but nothing was running.");

                if ( components_.size() == 1 )
                {
                    it = components_.begin();
                    return (*it)->wait_for_element();
                }
                else
                    throw std::runtime_error("Global Buffer Queue here?");
            }
            else
            {
                ana::net_id id( connection_num );

                std::cout << "DEBUG: Trying to read something from (" << connection_num << " = " << id << ")\n";

                it = std::find_if( components_.begin(), components_.end(),
                                boost::bind(&ana_component::get_id, _1) == id );

                if ( it != components_.end())
                    return (*it)->wait_for_element();
                else
                    throw std::runtime_error("Trying a network read from an invalid component id.");
            }
        }

        network::statistics get_send_stats(network::connection handle)
        {
            std::cout << "DEBUG: in get_send_stats to " << handle << "\n";

            if ( handle != 0 )
            {
                ana::net_id id( handle );
                std::set< ana_component* >::iterator it;

                it = std::find_if( components_.begin(), components_.end(),
                                    boost::bind(&ana_component::get_id, _1) == id );

                if ( it != components_.end() )
                    return (*it)->get_send_stats( );
                else
                    throw std::runtime_error("Received message from a non connected component.");
            }
            else if( ! components_.empty() )
            {
                std::set< ana_component* >::iterator it = components_.begin();
                return (*it)->get_send_stats();
            }
            else
                return network::statistics();
        }

        network::statistics get_receive_stats(network::connection handle)
        {
            if ( handle != 0 )
            {
                ana::net_id id( handle );
                std::set< ana_component* >::iterator it;

                it = std::find_if( components_.begin(), components_.end(),
                                    boost::bind(&ana_component::get_id, _1) == id );

                if ( it != components_.end() )
                    return (*it)->get_receive_stats( );
                else
                    throw std::runtime_error("Received message from a non connected component.");
            }
            else if( ! components_.empty() )
            {
                std::set< ana_component* >::iterator it = components_.begin();
                return (*it)->get_receive_stats();
            }
            else
                return network::statistics();
        }

    private:
        virtual void handle_send(ana::error_code error_code, ana::net_id client)
        {
            if ( error_code )
                network::disconnect( client );
        }

        virtual void handle_message( ana::error_code          error,
                                     ana::net_id              client,
                                     ana::detail::read_buffer buffer)
        {
            if (! error)
            {
                std::cout << "DEBUG: Buffer received with size " << buffer->size() << " from " << client << "\n";

                std::set< ana_component* >::iterator it;

                for (it = components_.begin(); it != components_.end(); ++it)
                    std::cout << "DEBUG: Component id : " << (*it)->get_id() << "\n";

                it = std::find_if( components_.begin(), components_.end(),
                                   boost::bind(&ana_component::get_id, _1) == client );

                if ( it != components_.end() )
                {
                    (*it)->update_receive_stats( buffer->size() );
                    (*it)->add_buffer( buffer );
                }
                else
                    throw std::runtime_error("Received message from a non connected component.");
            }
        }

        virtual void handle_disconnect(ana::error_code /*error_code*/, ana::net_id client)
        {
            std::cout << "DEBUG: Disconnected from server.\n";

            std::set< ana_component* >::iterator it;

            it = std::find_if( components_.begin(), components_.end(),
                                boost::bind(&ana_component::get_id, _1) == client );

            if ( it != components_.end() )
            {
                std::cout << "DEBUG: Removing bad component.\n";
                components_.erase(it);
            }
        }

        ana::timer*                connect_timer_;
        std::set< ana_component* > components_;

        std::map< ana::server*, const clients_manager* > server_manager_;
};

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
//     throw std::runtime_error("TODO:Not implemented");
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
    {}

    connection_stats get_connection_stats(connection connection_num)
    {
        const ana::stats* stats = ana_manager.get_stats( connection_num );

        return connection_stats( stats->bytes_out(),
                                 stats->bytes_in(),
                                 stats->uptime() ); //TODO: is uptime ok?
    }

    error::error(const std::string& msg, connection sock) : message(msg), socket(sock)
    {
        if(socket) {
            bad_sockets.insert(socket);
        }
    }

    void error::disconnect()
    {
        if(socket) network::disconnect(socket);
    }

    pending_statistics get_pending_stats()
    {
        throw std::runtime_error("TODO:Not implemented get_pending_stats");
    }

    manager::manager(size_t /*min_threads*/, size_t /*max_threads*/) : free_(true)
    {
    //     throw std::runtime_error("TODO:Not implemented");
    }

    manager::~manager()
    {
    //     throw std::runtime_error("TODO:Not implemented");
    }

    void set_raw_data_only()
    {
    //     throw std::runtime_error("TODO:Not implemented");
    }

    server_manager::server_manager(int port, CREATE_SERVER create_server) : free_(false), connection_(0)
    {
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
        return ana_manager.create_client_and_connect( host, port );
    }

    connection connect(const std::string& host, int port, threading::waiter& /*waiter*/)
    {
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
        throw std::runtime_error("TODO:Not implemented accept_connection");
    }

    bool disconnect(connection /*s*/)
    {
        throw std::runtime_error("TODO:Not implemented disconnect");
    }

    void queue_disconnect(network::connection sock)
    {
//         throw("TODO:Not implemented queue_disconnect");
        disconnection_queue.push_back(sock);
    }

    connection receive_data(config&           /*cfg*/,
                            connection        connection_num,
                            unsigned int      /*timeout*/,
                            bandwidth_in_ptr* /*bandwidth_in*/)
    {
        ana::detail::read_buffer buffer = ana_manager.read_from( connection_num );

        std::cout << "DEBUG: Read a buffer of size " << buffer->size() << "\n";

        throw std::runtime_error("TODO:Not implemented receive_data0");
    }

    connection receive_data(config&           /*cfg*/,
                            connection        /*connection_num*/,
                            bool*             /*gzipped*/,
                            bandwidth_in_ptr* /*bandwidth_in*/)
    {
        throw std::runtime_error("TODO:Not implemented receive_data1");
    }

    connection receive_data(std::vector<char>& /*buf*/, bandwidth_in_ptr* /*bandwidth_in*/)
    {
        throw std::runtime_error("TODO:Not implemented receive_data2");
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

        if( connection_num == 0 )
            return ana_manager.send_all( cfg, gzipped );
        else
            return ana_manager.send( connection_num, cfg, gzipped );
    }

    void send_raw_data(const char*        /*buf*/,
                       int                /*len*/,
                       connection         /*connection_num*/,
                       const std::string& /*packet_type*/)
    {
        throw std::runtime_error("TODO:Not implemented send_raw_data");
    }

    void process_send_queue(connection, size_t)
    {
        check_error();
    }

    /** @todo Note the gzipped parameter should be removed later. */
    void send_data_all_except(const config&       /*cfg*/,
                              connection          /*connection_num*/,
                              const bool          /*gzipped*/,
                              const std::string&  /*packet_type*/)
    {
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
