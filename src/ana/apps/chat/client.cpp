
/**
 * @file
 * @brief Client side chat application. Example for the ana project.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 - 2014 Guillermo Biset.
 *
 * This file is part of the ana project.
 *
 * System:         ana
 * Language:       C++
 *
 * Author:         Guillermo Biset
 * E-Mail:         billybiset AT gmail DOT com
 *
 * ana is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * ana is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ana.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <iomanip>

#include "ana.hpp"

using namespace ana;

const port DEFAULT_PORT = "30303";

const char* const DEFAULT_ADDRESS = "127.0.0.1";
const char* const DEFAULT_NAME    = "Unnamed";


void show_help(char* const command)
{
    std::cout << "The following options should be given in order and are all optional:\n"
        "\tName:            [recommended] Set name. Default=" << DEFAULT_NAME << std::endl <<
        "\tAddress:         [optional]    Set address. Default=" << DEFAULT_ADDRESS << std::endl <<
        "\tPort:            [optional]    Set port. Default=" << DEFAULT_PORT << std::endl <<
        "\tProxy address:   [optional]    Set proxy address." << std::endl <<
        "\tProxy port:      [optional]    Set proxy port." << std::endl <<
        "\tProxy user:      [optional]    Set proxy authentication user name." << std::endl <<
        "\tProxy password:  [optional]    Set proxy authentication password." << std::endl <<
        "Examples:\n" <<
        "\t" << command << " billy localhost 12345\n" <<
        "\t\tConnect to a local server on port 12345 with nick billy.\n" <<
        "\t" << command << " billy X.K.C.D 12345 localhost 3128 foo bar\n" <<
        "\t\tConnect to a remote server at X.K.C.D:12345 with proxy credentials foo:bar.\n\n";
        ;
}

struct connection_info
{
    connection_info() :
        pt(DEFAULT_PORT),
        address(DEFAULT_ADDRESS),
        name(DEFAULT_NAME),
        proxyaddr(),
        proxyport(),
        user(),
        password()
    {
    }

    bool use_proxy() const
    {
        return proxyaddr != "" && proxyport != "";
    }

    port        pt;
    std::string address;
    std::string name;
    std::string proxyaddr;
    std::string proxyport;
    std::string user;
    std::string password;
};

class ChatClient : public ana::listener_handler,
                   public ana::send_handler,
                   public ana::connection_handler
{
    public:
        ChatClient(connection_info ci) :
            conn_info_( ci ),
            continue_(true),
            client_( ana::client::create(ci.address, ci.pt) ),
            name_(ci.name)
        {
        }

        std::string get_name(const std::string& msg)
        {
            size_t pos = msg.find(" ");

            return msg.substr(pos+1);
        }

        double get_seconds(const std::string& msg)
        {
            size_t pos = msg.find(" ");

            std::stringstream ss( msg.substr(pos+1) );

            double result;

            ss >> result;

            return result;
        }


        void parse_command(const std::string& msg)
        {
            if (msg[1] == 'n') //Lame: assume name command
                name_ = get_name(msg);
            else if (msg[1] == 's')
            {
                const ana::stats* acum_stats = client_->get_stats( ana::ACCUMULATED );
                const ana::stats* sec_stats  = client_->get_stats( ana::SECONDS );
                const ana::stats* min_stats  = client_->get_stats( ana::MINUTES );
                const ana::stats* hour_stats = client_->get_stats( ana::HOURS );
                const ana::stats* day_stats  = client_->get_stats( ana::DAYS );

                std::cout << "                  Network  Statistics\n\n"
                    << "Uptime : " << acum_stats->uptime() << " seconds.\n\n"
                    << "+-----------------+-----------------+-----------------+\n"
                    << "|                 |     Packets     |      Bytes      |\n"
                    << "|                 +-----------------+--------+--------+\n"
                    << "|                 |   In   |  Out   |   In   |   Out  |\n"
                    << "+-----------------+--------+--------+--------+--------+\n"
                    << "|   Accumulated   |" << std::setw(8) << acum_stats->packets_in() <<"|"
                                            << std::setw(8) << acum_stats->packets_out() <<"|"
                                            << std::setw(8) << acum_stats->bytes_in() <<"|"
                                            << std::setw(8) << acum_stats->bytes_out() <<"|\n"
                    << "+-----------------+-----------------+-----------------+\n"
                    << "|   Last second   |" << std::setw(8) << sec_stats->packets_in() <<"|"
                                            << std::setw(8) << sec_stats->packets_out() <<"|"
                                            << std::setw(8) << sec_stats->bytes_in() <<"|"
                                            << std::setw(8) << sec_stats->bytes_out() <<"|\n"
                    << "+-----------------+-----------------+-----------------+\n"
                    << "|   Last minute   |" << std::setw(8) << min_stats->packets_in() <<"|"
                                            << std::setw(8) << min_stats->packets_out() <<"|"
                                            << std::setw(8) << min_stats->bytes_in() <<"|"
                                            << std::setw(8) << min_stats->bytes_out() <<"|\n"
                    << "+-----------------+-----------------+-----------------+\n"
                    << "|    Last hour    |" << std::setw(8) << hour_stats->packets_in() <<"|"
                                            << std::setw(8) << hour_stats->packets_out() <<"|"
                                            << std::setw(8) << hour_stats->bytes_in() <<"|"
                                            << std::setw(8) << hour_stats->bytes_out() <<"|\n"
                    << "+-----------------+-----------------+-----------------+\n"
                    << "|    Last  day    |" << std::setw(8) << day_stats->packets_in() <<"|"
                                            << std::setw(8) << day_stats->packets_out() <<"|"
                                            << std::setw(8) << day_stats->bytes_in() <<"|"
                                            << std::setw(8) << day_stats->bytes_out() <<"|\n"
                    << "+-----------------+-----------------+-----------------+\n";
            }
            else if ( msg[1] == 'h' )
            {
                double seconds = get_seconds( msg );
                client_->expecting_message( ana::time::seconds( seconds ) );
            }
        }

        void run_input()
        {
            std::string msg;
            do
            {
                std::cout << name_ << " : ";
                std::getline(std::cin, msg);

                if (msg[0] == '/')
                    parse_command(msg);

                if (msg == "/quit")
                    std::cout << "\nExiting.\n";
                else if (msg == "/empty")
                {
                    const std::string empty_str;

                    client_->send( ana::buffer( empty_str ), this );
                }
                else
                    if (!msg.empty())
                        client_->send( ana::buffer( msg ), this);

            } while ( (msg != "/quit") && continue_);

            client_->disconnect();
        }

        void run()
        {
            try
            {
                if ( ! conn_info_.use_proxy() )
                    client_->connect( this );
                else
                    client_->connect_through_proxy(conn_info_.proxyaddr,
                                                   conn_info_.proxyport,
                                                   this,
                                                   conn_info_.user,
                                                   conn_info_.password);

                client_->set_listener_handler( this );
                client_->run();
                client_->set_timeouts(ana::FixedTime, ana::time::seconds(10));

                std::cout << "Available commands: \n"  <<
                             "    '/quit'      : Quit. \n"
                             "    '/who'       : List connected users. \n" <<
                             "    '/stats'     : Print full network stats. \n" <<
                             "    '/hold secs' : Wait secs for a new message. \n" <<
                             "    '/name name' : Change name." << std::endl;

                run_input();

            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }

            delete client_;
        }

    private:

        virtual void handle_connect( ana::error_code error, net_id server_id )
        {
            if ( error )
            {
                std::cerr << "\nError connecting." << std::endl;
                continue_ = false;
            }
            else
                client_->send( ana::buffer( std::string("/name ") + name_) , this);
        }

        virtual void handle_disconnect( ana::error_code error, net_id server_id)
        {
            std::cerr << "\nServer disconnected. Enter to exit." << std::endl;
            continue_ = false;
        }

        virtual void handle_receive( ana::error_code error, net_id, ana::read_buffer msg)
        {
            if (! error)
            {
                std::cout << std::endl << msg->string()
                          << std::endl << name_ << " : ";
                std::cout.flush();
            }
            else if ( error == ana::timeout_error )
                std::cerr << "\nTimeout for waiting message.\n";
        }

        virtual void handle_send( ana::error_code error, net_id client, ana::operation_id op_id)
        {
            if ( error )
                std::cout << "Error. Timeout?" << std::endl;
        }

        connection_info     conn_info_;
        bool                continue_;
        ana::client*        client_;
        std::string         name_;
};

int main(int argc, char **argv)
{
    show_help(argv[0]);

    connection_info ci;

    if ( argc > 1 )
        ci.name      = argv[1];
    if ( argc > 2 )
        ci.address   = argv[2];
    if ( argc > 3 )
        ci.pt        = argv[3];
    if ( argc > 4 )
        ci.proxyaddr = argv[4];
    if ( argc > 5 )
        ci.proxyport = argv[5];
    if ( argc > 6 )
        ci.user      = argv[6];
    if ( argc > 7 )
        ci.password  = argv[7];

    std::cout << "Main client app.: Starting client" << std::endl;

    ChatClient client(ci);
    client.run();
}
