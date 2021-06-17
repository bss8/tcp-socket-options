#include <netinet/ip.h>
#include <arpa/inet.h>

// #include <sys/wait.h>
// #include <signal.h>
// #include <sys/select.h>
// #include <sys/time.h>

#include "../include/Common_Helpers.hpp"

void print_socket_options();
void print_socket_options(int connfd);
void invert_socket_options(int connfd);
std::string process_client_msg(int connfd, in_addr client_in_addr, socklen_t clilen, sockaddr_in cliaddr);
void process_client_payload(std::string client_payload);

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

    //^C
    signal(SIGINT, lam);
    //abort()
    signal(SIGABRT, lam);
    //sent by "kill" command
    signal(SIGTERM, lam);
    //^Z
    signal(SIGTSTP, lam);

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

            process_client_payload(client_payload);

            // Re-print the server socket options to show their new values
            std::cout << "\n     *****" << std::endl
                      << std::endl;
            print_socket_options(connfd);

            send(connfd, "Hello - I inverted/modified the socket options you sent me and printed on my end!\n", 72, 0);

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
                printf("Can't create fd for level %d\n", ptr->opt_level);
            }

            len = sizeof(val);
            if (getsockopt(fd, ptr->opt_level, ptr->opt_name,
                           &val, &len) == -1)
            {
                printf("getsockopt error");
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
                printf("  getsockopt error");
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
                printf("  getsockopt error");
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
    }
    else
    {
        //fprintf(stderr, "getnameinfo: %s\n", gai_strerror(getname_err));
        std::cerr << stderr << " getnameinfo: " << gai_strerror(getname_err) << std::endl;
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
 */
void process_client_payload(std::string client_payload)
{
    std::cout << ANSII_BLUE_COUT << "PROCESSING CLIENT PAYLOAD....." << ANSII_END << std::endl;
    std::string delimiter = ";";

    size_t pos = 0;
    std::string token;
    while ((pos = client_payload.find(delimiter)) != std::string::npos)
    {
        token = client_payload.substr(0, pos);
        std::cout << token << std::endl;
        client_payload.erase(0, pos + delimiter.length());
    }
    std::cout << client_payload << std::endl;
}
