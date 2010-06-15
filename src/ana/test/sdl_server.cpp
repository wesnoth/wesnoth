#include <iostream>

#include <SDL/SDL_net.h>

const int DEFAULT_PORT = 2000;

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
    if (SDLNet_ResolveHost(&ip, NULL, DEFAULT_PORT) < 0)
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
                // Added this, from here
                char buf[4];
                int len = SDLNet_TCP_Recv(csd,buf,4);

                const int message_length = *( reinterpret_cast<int*>(buf) );

                printf("Next message length %d\n", message_length);
                // ... to here

                if (SDLNet_TCP_Recv(csd, buffer, 512) > 0)
                {
                    buffer[message_length] = '\0';     // Added this, interpret the message correctly ugly fix
                    printf("Client says: %s\n", buffer);

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
