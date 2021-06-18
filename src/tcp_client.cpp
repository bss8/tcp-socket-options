/**
 * @author Borislav Sabotinonv
 * CS5341 Assignment 1 
 * TCP Client-Server program to send data, modify socket option values, and print
 * 
 * This cpp file defines a TCP client, which sends a string payload to a server. 
 * The client takes in two command line arguments. The first is the hostname, the 
 * second is the port number. The payload contains a string representation of socket options 
 * and their values. The server will invert these values, modify its socket option values using them, and print. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../include/Common_Helpers.hpp"

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUF_SIZE];

    if (argc != 3)
    {
        std::cout << "Invoke with: " << argv[0] << " hostname port\n"
                  << std::endl;
        return EXIT_FAILURE;
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        std::cerr << ANSII_RED_START << "ERROR: opening socket!" << ANSII_END << std::endl;
        return EXIT_FAILURE;
    }

    server = gethostbyname(argv[1]);

    if (server == NULL)
    {
        fprintf(stderr, "ERROR: no such host!\n");
        return EXIT_FAILURE;
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << ANSII_RED_START << "ERROR connecting" << ANSII_END << std::endl;
        exit(EXIT_FAILURE);
    }

    // printf("Please enter the message: ");
    // bzero(buffer, BUF_SIZE);
    // fgets(buffer, BUF_SIZE - 1, stdin);

    // BEGIN MAIN SEND LOOP

    socklen_t len;
    struct sock_opts *ptr;

    std::cout << ANSII_YELLOW_COUT << "Sending socket options to server in format:"
              << ANSII_END << " " << ANSII_BLUE_START << "opt_str,opt_name,opt_val;" << ANSII_END << std::endl;
    std::cout << "  displayed below as opt_str(opt_name),opt_val ";
    std::cout << ", where opt_name is an integer value." << std::endl;

    std::string so_name_value = "";
    int opt_count = 0;
    for (ptr = sock_opts; ptr->opt_str != NULL; ptr++)
    {
        //
        if (ptr->opt_val_str == NULL)
        {
            std::cout << ptr->opt_str << ": " << ANSII_RED_START
                      << "(undefined) - skipping....." << ANSII_END << std::endl;
        }
        else
        {
            len = sizeof(val);
            if (getsockopt(sockfd, ptr->opt_level, ptr->opt_name,
                           &val, &len) == -1)
            {
                std::cout << ANSII_RED_START << "getsockopt did not find the option, skipping....."
                          << ANSII_END << std::endl;
            }
            else
            {
                // TODO print option string but send only the opt_name, which is actually an integer value
                std::string so_str = ptr->opt_str;
                std::string so_name = std::to_string(ptr->opt_name);
                std::string so_value = (*ptr->opt_val_str)(&val, len);
                so_name_value += so_str + "," + so_name + "," + so_value + ";";

                std::cout << so_str << "(" << so_name << ")"
                          << "," << so_value << std::endl;
                opt_count++;
            }
        }
    }

    strcpy(buffer, so_name_value.c_str());
    n = write(sockfd, buffer, strlen(buffer));

    std::cout << std::endl
              << ANSII_BLUE_COUT << "Sent " << opt_count
              << " options to the server for processing." << ANSII_END << std::endl;
    std::cout << "Please verify server reply below AND output on the server itself!" << std::endl
              << std::endl;

    // END MAIN SEND LOOP

    //n = write(sockfd, buffer, strlen(buffer));

    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }

    bzero(buffer, BUF_SIZE);
    n = read(sockfd, buffer, BUF_SIZE);

    if (n < 0)
    {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    std::cout << ANSII_GREEN_START << "Server reply is: " << ANSII_END;
    std::cout << buffer << std::endl;

    close(sockfd);
    goodbye();
    return EXIT_SUCCESS;
}
