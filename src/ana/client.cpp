#include <iostream>

#include "getopt_pp.h"

#include "ana.hpp"

using namespace GetOpt;
using namespace ana;

const port DEFAULT_PORT = "30303";

const char* const DEFAULT_ADDRESS = "127.0.0.1";

void show_help()
{
    std::cout << "Valid options are:\n"
        "\t-p --port        [optional] Set port. Default=" << DEFAULT_PORT << std::endl <<
        "\t-a --address     [optional] Set address. Default=" << DEFAULT_ADDRESS << std::endl <<
        "\t-c --connections [optional] Set connection amount (for multiplexing). Default = 1" << std::endl;
    ;
}

class ChatClient : public ana::listener_handler,
                   public ana::send_handler,
                   public ana::connection_handler
{
    public:
        ChatClient(const std::string& address, port pt, std::string name) :
            continue_(true),
            client_( ana::client::create(address,pt) ),
            name_(name)
        {
        }

        std::string get_name(const std::string& msg)
        {
            size_t pos = msg.find(" ");

            return msg.substr(pos+1);
        }

        void parse_command(const std::string& msg)
        {
            if (msg[1] == 'n') //Lame: assume name command
            {
                name_ = get_name(msg);
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
                else
                    if (msg.size() > 0)
                        client_->send( ana::buffer( msg ), this);

            } while ( (msg != "/quit") && continue_);
        }

        void run()
        {
            try
            {
                client_->connect( this );
                client_->set_listener_handler( this );
                client_->run();

                std::cout << "Connected. Available commands: \n"  <<
                             "    '/quit'      : Quit. \n"
                             "    '/who'       : List connected users. \n" <<
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

        virtual void handle_connect( ana::error_code error, client_id server_id )
        {
            if ( error )
                std::cerr << "Error connecting." << std::endl;
            else
                client_->send( ana::buffer( std::string("/name ") + name_) , this);
        }

        virtual void handle_disconnect( ana::error_code error, client_id server_id)
        {
            std::cerr << "\nServer disconnected. Enter to exit." << std::endl;
            continue_ = false;
        }

        virtual void handle_message( ana::error_code error, client_id, ana::detail::read_buffer msg)
        {
            if (! error)
            {
                std::cout << std::endl << msg->string()
                          << std::endl << name_ << " : ";
                std::cout.flush();
            }
        }

        virtual void handle_send( ana::error_code error, client_id client)
        {
            if ( error )
                std::cout << "Error. Timeout?" << std::endl;
        }

        bool                continue_;
        ana::client*        client_;
        std::string         name_;
};

int main(int argc, char **argv)
{
    GetOpt_pp options(argc, argv);

    if (options >> OptionPresent('h', "help"))
        show_help();
    else
    {
        port pt(DEFAULT_PORT);
        std::string address(DEFAULT_ADDRESS);
        std::string name("Unnamed");

        options >> Option('p', "port", pt) >> Option('a',"address",address) >> Option('n',"name",name);
        std::cout << "Main client app.: Starting client" << std::endl;

        ChatClient client(address,pt,name);
        client.run();
    }
}
