/**
 * @author Borislav Sabotinonv
 * CS5341 Assignment 1 
 * TCP Client-Server program to send data, modify socket option values, and print
 * 
 * This file defines a TCP server. It receives data from a client on port 5001. 
 * The expected data is a string payload, containing socket option names and values. 
 * The server will then invert the received values and modify its own socket option values 
 * using the new, inverted values and print the results. 
 */

#include <netinet/ip.h>
#include <arpa/inet.h>

#include "../include/Common_Helpers.hpp"

void print_socket_options();
void print_socket_options(int connfd);
void invert_socket_options(int connfd);
std::string process_client_msg(int connfd, in_addr client_in_addr, socklen_t clilen, sockaddr_in cliaddr);
void process_client_payload(std::string client_payload, int connfd);
void process_token(std::string token, int connfd);
bool is_number(const std::string &s);

int main(int argc, char **argv)
{
    //signal requires lam take an int parameter
    //this parameter is equal to the signals value
    auto lam =
        [](int i)
    {
        std::cout << "\nTermination command invoked. Shutting down server. " << std::endl;
        goodbye();
        /* "note that c++11 added a quick_exit which has an accompanying at_quick_exit which act the same as above. 
        *  But with quick_exit no clean up tasks are performed. In contrast, with exit object destructors are called 
        *  and C streams are closed, with only automatic storage variables not getting cleaned up." 
        *  https://stackoverflow.com/questions/9402254/how-do-you-run-a-function-on-exit-in-c 
        */
        exit(0);
    };

    // Handle termination conditions
    //^C or Ctrl + C
    signal(SIGINT, lam);
    //abort()
    signal(SIGABRT, lam);
    //sent by "kill" command
    signal(SIGTERM, lam);
    //^Z or Ctrl + Z
    signal(SIGTSTP, lam);

    // Print greeting message to the user.
    std::cout << ANSII_GREEN_START << "Hello! Starting TCP server on port " << PORT << ".....\n"
              << ANSII_END << std::endl;
    std::cout << ANSII_YELLOW_COUT << "Default server option values:" << ANSII_END << std::endl;
    print_socket_options();

    std::cout << "\n Listening for connections....\n"
              << std::endl;

    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    char s[INET6_ADDRSTRLEN];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, BACKLOG);

    while (true)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

        std::cout << ANSII_BLUE_COUT << "Server received a connection from internet address: "
                  << cliaddr.sin_addr.s_addr << ANSII_END << std::endl;

        in_addr client_in_addr = ((struct sockaddr_in *)&cliaddr)->sin_addr;

        // Get IP address of connecting client and display it
        char ip_addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_in_addr, ip_addr_str, INET_ADDRSTRLEN);
        std::cout << "IP address is: " << ip_addr_str << std::endl
                  << std::endl;

        if ((childpid = fork()) == 0)
        {                    /* child process */
            close(listenfd); /* close listening socket */

            std::string client_payload = process_client_msg(connfd, client_in_addr, clilen, cliaddr);

            process_client_payload(client_payload, connfd);

            // Re-print the server socket options to show their new values
            std::cout << "\n***********************************************************************************************" << std::endl
                      << std::endl;
            std::cout << ANSII_GREEN_START << "NEW server socket option values (after modifying client values): " << ANSII_END << std::endl;
            print_socket_options(connfd);

            std::string server_reply = "Hello - I inverted/modified the socket options you sent me and printed on my end!\n";
            int reply_size = 81;
            send(connfd, server_reply.c_str(), reply_size, 0);

            exit(0);
        }
        close(connfd); /* parent closes connected socket */
    }

    return EXIT_SUCCESS;
}

/**
 * print_socket_options - a helper function to print out the initial values of all socket options
 * prior to accepting a client connection. This is essentially a clone of checkopts.c from the text. 
 */
void print_socket_options()
{
    int fd;
    socklen_t len;
    struct sock_opts *ptr;

    for (ptr = sock_opts; ptr->opt_str != NULL; ptr++)
    {
        printf("%s: ", ptr->opt_str);
        if (ptr->opt_val_str == NULL)
            printf("(undefined)\n");
        else
        {
            switch (ptr->opt_level)
            {
            case SOL_SOCKET:
            case IPPROTO_IP:
            case IPPROTO_TCP:
                fd = socket(AF_INET, SOCK_STREAM, 0);
                break;
#ifdef IPV6
            case IPPROTO_IPV6:
                fd = socket(AF_INET6, SOCK_STREAM, 0);
                break;
#endif
#ifdef IPPROTO_SCTP
            case IPPROTO_SCTP:
                fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
                break;
#endif
            default:
                std::cout << "Cannot create fd for level: " << ptr->opt_level << std::endl;
            }

            len = sizeof(val);
            if (getsockopt(fd, ptr->opt_level, ptr->opt_name,
                           &val, &len) == -1)
            {
                std::cerr << ANSII_RED_START << "getsockopt error" << ANSII_END << std::endl;
            }
            else
            {
                printf("default = %s\n", (*ptr->opt_val_str)(&val, len));
            }
            close(fd);
        }
    }
}

/**
 * print_socket_options(int) - an overloaded version of print_socket_options()
 * A helper function to print the socket options given a socket connection with a client. 
 * @param int sockfd
 * @return none
 */
void print_socket_options(int sockfd)
{

    socklen_t len;
    struct sock_opts *ptr;

    for (ptr = sock_opts; ptr->opt_str != NULL; ptr++)
    {
        printf("  %s: ", ptr->opt_str);
        if (ptr->opt_val_str == NULL)
            printf("  (undefined)\n");
        else
        {
            len = sizeof(val);
            if (getsockopt(sockfd, ptr->opt_level, ptr->opt_name,
                           &val, &len) == -1)
            {
                std::cerr << ANSII_RED_START << "  getsockopt error" << ANSII_END << std::endl;
            }
            else
            {
                printf("default = %s\n", (*ptr->opt_val_str)(&val, len));
            }
            //close(sockfd);
        }
    }
}

/**
 * invert_socket_options is a helper function to invert the 
 * socket options of the server (NOT the ones sent by the client). 
 * This function is here as a proof of concept and is not used in the final program, 
 * as we need to invert the values sent to us by the client instead.  
 * @param int connfd - the socket from the accepted client connection
 * @return none
 */
void invert_socket_options(int connfd)
{
    socklen_t len;
    struct sock_opts *ptr;

    for (ptr = sock_opts; ptr->opt_str != NULL; ptr++)
    {
        //printf("  %s: ", ptr->opt_str);
        if (ptr->opt_val_str == NULL)
            printf("  (undefined)\n");
        else
        {
            len = sizeof(val);
            if (getsockopt(connfd, ptr->opt_level, ptr->opt_name,
                           &val, &len) == -1)
            {
                std::cerr << ANSII_RED_START << "  getsockopt error" << ANSII_END << std::endl;
            }
            else
            {
                //string test = (*ptr->opt_val_str)(&val, len);
                //printf("VALUE IS: %d  ||", val.i_val);
                //printf("VALUE IS: %d \n", val.l_val);

                int newVal;
                if (newVal > 1)
                    newVal = 32248;
                else if (val.i_val == 0 ? newVal = 1 : newVal == 0)
                    ;

                setsockopt(connfd, ptr->opt_level, ptr->opt_name, &newVal, sizeof(newVal));
                //printf("default = %s\n", (*ptr->opt_val_str)(&val, len));
            }
            //close(sockfd);
        }
    }
}

/** 
 * pricess_client_msg - a helper function to process the string payload of socket options and values from the client
 * Function will receive the message from client in the buffer. It will print details on how many bytes 
 * it received, who sent the message, and then will print the message.
 * Finally, it will return the client payload as a string. 
 * @param int connfd - the socket
 * @param in_addr client_in_addr - the client address
 * @param in_addr actual size of of client address 
 * @return string client payload
 */
std::string process_client_msg(int connfd, in_addr client_in_addr, socklen_t clilen, sockaddr_in cliaddr)
{
    char host[NI_MAXHOST], service[NI_MAXSERV];
    char buf[BUF_SIZE];

    // Receive the client message into buffer
    int nread = recvfrom(connfd, buf, BUF_SIZE, 0, (struct sockaddr *)&client_in_addr, &clilen);

    if (nread == -1)
    {
        std::cout << "There were issues with the request but will continue....." << std::endl;
        return ""; /* Ignore request as it failed, will continue loop and skip logic below */
    }

    // Get the hostname
    int getname_err = getnameinfo((struct sockaddr *)&cliaddr,
                                  sizeof(cliaddr), host, NI_MAXHOST,
                                  service, NI_MAXSERV, NI_NUMERICSERV);
    if (getname_err == 0)
    {
        std::cout << "Received " << nread << " bytes from " << host << ":" << service << std::endl;
        std::cout << "  format should be: option string, option name (int value), option value(s); " << std::endl;
        std::cout << "  comma is used to delimit values within a socket option group. Semicolon delimits between option groups." << std::endl;
    }
    else
    {
        //fprintf(stderr, "getnameinfo: %s\n", gai_strerror(getname_err));
        std::cerr << ANSII_RED_START << stderr << " getnameinfo: " << gai_strerror(getname_err) << ANSII_END << std::endl;
    }

    // Display the message
    std::cout << ANSII_GREEN_START << "Msg received from client is: " << ANSII_END;
    std::cout << buf << std::endl;

    return buf;
}

/**
 * process_client_payload - a helper function to do the following: 
 *      1. Parse the client payload
 *      2. Set server socket options to the opposite (or in non-binary cases, just different) values
 * @param string client_payload
 * @param int connfd - the socket
 */
void process_client_payload(std::string client_payload, int connfd)
{
    std::cout << std::endl
              << ANSII_BLUE_COUT << "PROCESSING CLIENT PAYLOAD....."
              << ANSII_END << std::endl;
    std::string delimiter = ";";

    size_t pos = 0;
    std::string token;
    int num_tokens = 0;
    while ((pos = client_payload.find(delimiter)) != std::string::npos)
    {
        token = client_payload.substr(0, pos);
        std::cout << token << std::endl;
        process_token(token, connfd);
        client_payload.erase(0, pos + delimiter.length());
        num_tokens++;
    }

    std::cout << ANSII_BLUE_START << "Processed " << num_tokens << " received from the client." << ANSII_END << std::endl;
}

/**
 * process_token - a helper function to deal with each comma delimited token. 
 * After parsing the client payload we end up with one of four variants: 
 *      Binary:     SO_KEEPALIVE,off
 *      Linger:     SO_LINGER,l_onoff = 0, l_linger = 0
 *      Integer:    SO_RCVBUF,131072
 *      Time:       SO_SNDTIMEO,0 sec, 0 usec
 *      
 * I intentionally ignore linger.  
 * This program will invert binary options and modify integer ones to some predefined value. 
 * It will set a timeval option to sec = 1 and usec = 500000 as some implementations do not accept tv_usec above 10^6
 * Linger option value will be kept as is intentionally. 
 * 
 * @param string token - the string to process. This will be a single option group in the form: 
 *                          opt_str,opt_name,opt_val_1,opt_val_2; 
 * @param int connfd - the socket for which to change options
 * @return none
 */
void process_token(std::string token, int connfd)
{
    std::string delimiter = ",";
    int pos = 0, idx = 0;
    std::string components[4];

    while ((pos = token.find(delimiter)) != std::string::npos)
    {
        components[idx] = token.substr(0, pos);
        //std::cout << components[idx] << std::endl;
        token.erase(0, pos + delimiter.length());
        idx++;
    }

    if (token.length() > 0)
    {
        components[idx] = token;
        //std::cout << token << std::endl;
    }

    // NOTE:    components[0] is opt_str
    //          components[1] is opt_name
    //          components[2] is either binary value or 1st timeval/linger val
    //          components[3] is 2nd timeval/linger val

    int opt_name = stoi(components[1]);

    // Determine and set the socket option level
    // One of three choices will be received from client: SO, IP, or TCP
    // The corresponding levels are SOL_SOCKET, IPPROTO_IP, and IPPROTO_TCP
    int opt_lvl;
    if (components[0].find("SO_") != std::string::npos)
    {
        opt_lvl = SOL_SOCKET;
    }
    else if (components[0].find("IP_") != std::string::npos)
    {
        opt_lvl = IPPROTO_IP;
    }
    else if (components[0].find("TCP_") != std::string::npos)
    {
        opt_lvl = IPPROTO_TCP;
    }

    // Invert the client value and set it as the new server socket option value
    int inverted_binary_val = 0;
    // BINARY
    if (components[2].compare("off") == 0 || components[2].compare("0") == 0)
    {
        std::cout << "  binary value is off, inverting to on..." << std::endl;
        inverted_binary_val = 1;

        if (setsockopt(connfd, opt_lvl, opt_name, &inverted_binary_val, sizeof(inverted_binary_val)) == -1)
        {
            std::cerr << ANSII_RED_START << stderr << " Error setting socket option! " << strerror(errno) << ANSII_END << std::endl;
        }
    }
    else if (components[2].compare("on") == 0 || components[2].compare("1") == 0)
    {
        std::cout << "  binary value is on, inverting to off..." << std::endl;

        // inverted_binary_val is already zero by default so we use it as-is
        if (setsockopt(connfd, opt_lvl, opt_name, &inverted_binary_val, sizeof(inverted_binary_val)) == -1)
        {
            std::cerr << ANSII_RED_START << stderr << " Error setting socket option! " << strerror(errno) << ANSII_END << std::endl;
        }
    }
    // NUMERIC
    else if (is_number(components[2]) && stoi(components[2]) > 1)
    {
        int value = stoi(components[2]);

        std::cout << "  value is numeric, modifying it ";

        // shift left by 1 is equivalent to multiplying by 2 but more efficient.
        int new_value;
        if (value > 10000) // check if this value is a large one, so we can treat it differently
        {
            new_value = 30000; // be mindful of upper limits, keep this value lower
            std::cout << "from " << value << " to "
                      << new_value << " (displays as 2x when printed, so 60000, except for TCP_MAXSEG)" << std::endl;
        }
        else
        {
            new_value = value << 1; // otherwise multiply by 2 by shifting left once
            std::cout << "from " << value << " to " << new_value << std::endl;
        }

        if (setsockopt(connfd, opt_lvl, opt_name, &new_value, sizeof(new_value)) == -1)
        {
            std::cerr << ANSII_RED_START << stderr << " Error setting socket option! " << strerror(errno) << ANSII_END << std::endl;
        }
    }
    else if (components[2].find("sec") != std::string::npos)
    {
        struct timeval time_val;
        time_val.tv_sec = 1;
        time_val.tv_usec = 500000;

        if (setsockopt(connfd, opt_lvl, opt_name, &time_val, sizeof(time_val)) == -1)
        {
            std::cerr << ANSII_RED_START << stderr << " Error setting socket option! " << strerror(errno) << ANSII_END << std::endl;
        }
    }
    else
    {
        std::cout << "  opt type is linger, leaving it as-is intentionally..." << std::endl;
    }
}

/**
 * is_number - a helper function using C++ features to determine if a string contains only digits or not
 * @param const string &s - the string to evaluate
 * @return bool true if all digits, false otherwise 
 * https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
 */
bool is_number(const std::string &s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](unsigned char c)
                                      { return !std::isdigit(c); }) == s.end();
}
