
/**
 * @file
 * @brief Server side chat application. Example for the ana project.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 - 2015 Guillermo Biset.
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
#include <sstream>

#include "ana.hpp"

using namespace ana;

const port DEFAULT_PORT = "30303";

class ChatServer : public listener_handler,
                   public send_handler,
                   public connection_handler
{
    public:
        ChatServer() :
            server_( ana::server::create() ),
            names_()
        {
        }

        void run(port pt)
        {
            server_->set_connection_handler( this );

            server_->set_listener_handler( this );

            server_->run(pt);

//             server_->set_timeouts(ana::FixedTime, ana::time::milliseconds(1));

            std::cout << "Server running, Enter to quit." << std::endl;

            std::string s;
            std::getline(std::cin, s); //yes, i've seen better code :)
        }

        ~ChatServer()
        {
            delete server_;
        }
    private:

        virtual void handle_connect(ana::error_code error, net_id client)
        {
            if (! error)
            {
                std::stringstream ss;
                ss << "Server: User " << client << " has joined from "
                   << server_->ip_address( client ) << ".";
                server_->send_all_except(client, ana::buffer( ss.str() ), this);
            }
        }

        virtual void handle_disconnect(ana::error_code error, net_id client)
        {
            std::stringstream ss;
            ss << names_[client] << " disconnected.";
            server_->send_all_except(client, ana::buffer( ss.str() ), this);
            names_.erase(client);
        }

        virtual void handle_send(ana::error_code error, net_id client, ana::operation_id id)
        {
            if ( error )
                std::cerr << "Error sending to client " << client << ". Timeout?" << std::endl;
        }

        std::string get_name(const std::string& msg)
        {
            size_t pos = msg.find(" ");

            return msg.substr(pos+1);
        }

        void parse_command(net_id client, const std::string& msg)
        {
            switch (msg[1]) //Lame
            {
                case 'n': //assume name command
                            names_[client] = get_name(msg);
                            break;
                case 'w': //assume who command
                            std::stringstream ss;
                            ss << "Connected clients: \n";

                            std::map<net_id, std::string>::iterator it;
                            for(it = names_.begin(); it != names_.end(); ++it)
                                ss << "    " << it->second << "\n";

                            server_->send_one(client, ana::buffer( ss.str() ), this);
                            break;
            }
        }

        virtual void handle_receive( ana::error_code          error,
                                     net_id                   client,
                                     ana::read_buffer         buffer)
        {
            if (! error)
            {
                std::string msg = buffer->string();

                if (msg.empty())
                    std::cout << "Received empty buffer. Size: " << buffer->size() << "\n";
                else if (msg[0] == '/')
                    parse_command(client, msg);
                else
                {
                    std::stringstream ss;
                    ss << names_[client] << " : " << msg;
                    server_->send_all_except(client, ana::buffer( ss.str() ), this);
                }
            }
            else
                handle_disconnect(error, client);
        }

        server*                       server_;
        std::map<net_id, std::string> names_;
};

int main(int argc, char **argv)
{
    std::cout << "Use " << argv[0] << " port_number to run on a specific port (default: "
              << DEFAULT_PORT << ".)\n\n";

    port pt(DEFAULT_PORT);

    if ( argc > 1 )
        pt = argv[1];

    ChatServer server;
    server.run(pt);
}
