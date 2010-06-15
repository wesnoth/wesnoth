#include <iostream>

#include <SDL/SDL_net.h>

#include <ana.hpp>

using namespace ana;

const port DEFAULT_PORT = "12345";

int main(int argc, char **argv)
{
    TCPsocket sd, csd; /* Socket descriptor, Client socket descriptor */
    IPaddress ip, *remoteIP;
    int quit, quit2;
    char buffer[512];

    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Resolving the host using NULL make network interface to listen */
    if (SDLNet_ResolveHost(&ip, NULL, 2000) < 0)
    {
        fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Open a connection with the IP provided (listen on the host's port) */
    if (!(sd = SDLNet_TCP_Open(&ip)))
    {
        fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Wait for a connection, send data and term */
    quit = 0;
    while (!quit)
    {
        /* This check the sd if there is a pending connection.
        * If there is one, accept that, and open a new socket for communicating */
        if ((csd = SDLNet_TCP_Accept(sd)))
        {
            /* Now we can communicate with the client using csd socket
            * sd will remain opened waiting other connections */

            /* Get the remote address */
            if ((remoteIP = SDLNet_TCP_GetPeerAddress(csd)))
                /* Print the address, converting in the host format */
                printf("Host connected: %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
            else
                fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());

            quit2 = 0;
            while (!quit2)
            {
                if (SDLNet_TCP_Recv(csd, buffer, 512) > 0)
                {
                    printf("Client say: %s\n", buffer);

                    if(strcmp(buffer, "exit") == 0) /* Terminate this connection */
                    {
                        quit2 = 1;
                        printf("Terminate connection\n");
                    }
                    if(strcmp(buffer, "quit") == 0) /* Quit the program */
                    {
                        quit2 = 1;
                        quit = 1;
                        printf("Quit program\n");
                    }
                }
            }

            /* Close the client socket */
            SDLNet_TCP_Close(csd);
        }
    }

    SDLNet_TCP_Close(sd);
    SDLNet_Quit();

    return EXIT_SUCCESS;
}

/*
class test_server : public listener_handler,
                    public send_handler,
                    public connection_handler
{
    public:
        test_server() :
            server_( ana::server::create() )
        {
        }

        void run(port pt)
        {
            server_->set_connection_handler( this );

            server_->set_listener_handler( this );

            server_->run(pt);

            std::cout << "Server running, Enter to quit." << std::endl;

            std::string s;
            std::getline(std::cin, s); //yes, i've seen better code :)
        }

        ~test_server()
        {
            delete server_;
        }
    private:

        virtual void handle_connect(ana::error_code error, client_id client)
        {
            if (! error)
                std::cout << "Server: User " << client << " has joined.\n";
        }

        virtual void handle_disconnect(ana::error_code error, client_id client)
        {
            std::cout << "Server: User " << client << " disconnected.\n";
        }

        virtual void handle_send(ana::error_code error, client_id client)
        {
            if ( error )
                std::cerr << "Error sending to client " << client << ". Timeout?" << std::endl;
        }

        virtual void handle_message( ana::error_code error, client_id client, ana::detail::read_buffer buffer)
        {
            if (! error)
                std::cout << buffer->string() << std::endl;
            else
                handle_disconnect(error, client);
        }

        server*                          server_;
};


int main()
{
    test_server test;
    test.run(DEFAULT_PORT);
}
*/