#include <netinet/ip.h>
#include <arpa/inet.h>

#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>

#include <netinet/tcp.h> /* for TCP_xxx defines */

#include "../include/Common_Helpers.hpp"

void print_socket_options();
void print_socket_options(int connfd);
void invert_socket_options(int connfd);

struct sock_opts
{
    const char *opt_str;
    int opt_level;
    int opt_name;
    char *(*opt_val_str)(union val *, int);
} sock_opts[] = {
    {"SO_BROADCAST", SOL_SOCKET, SO_BROADCAST, sock_str_flag},
    {"SO_DEBUG", SOL_SOCKET, SO_DEBUG, sock_str_flag},
    {"SO_DONTROUTE", SOL_SOCKET, SO_DONTROUTE, sock_str_flag},
    {"SO_ERROR", SOL_SOCKET, SO_ERROR, sock_str_int},
    {"SO_KEEPALIVE", SOL_SOCKET, SO_KEEPALIVE, sock_str_flag},
    {"SO_LINGER", SOL_SOCKET, SO_LINGER, sock_str_linger},
    {"SO_OOBINLINE", SOL_SOCKET, SO_OOBINLINE, sock_str_flag},
    {"SO_RCVBUF", SOL_SOCKET, SO_RCVBUF, sock_str_int},
    {"SO_SNDBUF", SOL_SOCKET, SO_SNDBUF, sock_str_int},
    {"SO_RCVLOWAT", SOL_SOCKET, SO_RCVLOWAT, sock_str_int},
    {"SO_SNDLOWAT", SOL_SOCKET, SO_SNDLOWAT, sock_str_int},
    {"SO_RCVTIMEO", SOL_SOCKET, SO_RCVTIMEO, sock_str_timeval},
    {"SO_SNDTIMEO", SOL_SOCKET, SO_SNDTIMEO, sock_str_timeval},
    {"SO_REUSEADDR", SOL_SOCKET, SO_REUSEADDR, sock_str_flag},
#ifdef SO_REUSEPORT
    {"SO_REUSEPORT", SOL_SOCKET, SO_REUSEPORT, sock_str_flag},
#else
    {"SO_REUSEPORT", 0, 0, NULL},
#endif
    {"SO_TYPE", SOL_SOCKET, SO_TYPE, sock_str_int},
    {"SO_USELOOPBACK", SOL_SOCKET, SO_USELOOPBACK, sock_str_flag},
    {"IP_TOS", IPPROTO_IP, IP_TOS, sock_str_int},
    {"IP_TTL", IPPROTO_IP, IP_TTL, sock_str_int},

#ifdef IPV6_DONTFRAG
    {"IPV6_DONTFRAG", IPPROTO_IPV6, IPV6_DONTFRAG, sock_str_flag},
#else
    {"IPV6_DONTFRAG", 0, 0, NULL},
#endif
#ifdef IPV6_UNICAST_HOPS
    {"IPV6_UNICAST_HOPS", IPPROTO_IPV6, IPV6_UNICAST_HOPS, sock_str_int},
#else
    {"IPV6_UNICAST_HOPS", 0, 0, NULL},
#endif
#ifdef IPV6_V6ONLY
    {"IPV6_V6ONLY", IPPROTO_IPV6, IPV6_V6ONLY, sock_str_flag},
#else
    {"IPV6_V6ONLY", 0, 0, NULL},
#endif
    {"TCP_MAXSEG", IPPROTO_TCP, TCP_MAXSEG, sock_str_int},
    {"TCP_NODELAY", IPPROTO_TCP, TCP_NODELAY, sock_str_flag},
#ifdef SCTP_AUTOCLOSE
    {"SCTP_AUTOCLOSE", IPPROTO_SCTP, SCTP_AUTOCLOSE, sock_str_int},
#else
    {"SCTP_AUTOCLOSE", 0, 0, NULL},
#endif
#ifdef SCTP_MAXBURST
    {"SCTP_MAXBURST", IPPROTO_SCTP, SCTP_MAXBURST, sock_str_int},
#else
    {"SCTP_MAXBURST", 0, 0, NULL},
#endif
#ifdef SCTP_MAXSEG
    {"SCTP_MAXSEG", IPPROTO_SCTP, SCTP_MAXSEG, sock_str_int},
#else
    {"SCTP_MAXSEG", 0, 0, NULL},
#endif
#ifdef SCTP_NODELAY
    {"SCTP_NODELAY", IPPROTO_SCTP, SCTP_NODELAY, sock_str_flag},
#else
    {"SCTP_NODELAY", 0, 0, NULL},
#endif
    {NULL, 0, 0, NULL}};


int main(int argc, char **argv)
{
    std::cout << "Hello! Starting TCP server on port " << PORT << ".....\n" << std::endl;
    std::cout << "Default option values (initial print): " << std::endl; 
    print_socket_options(); 
    std::cout << "\n Listening for connections....\n" << std::endl; 

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

        printf("server: got connection from internet address: %d\n", cliaddr.sin_addr.s_addr);

        in_addr client_in_addr = ((struct sockaddr_in *)&cliaddr)->sin_addr;
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_in_addr, str, INET_ADDRSTRLEN);
        printf("IP address is: %s\n", str);

        // struct sock_opts *ptr;
        // int opt = getsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &i_val, &len);

        // printf("%d\n", opt);
        // printf("%d", i_val);

        if ((childpid = fork()) == 0)
        {                    /* child process */
            close(listenfd); /* close listening socket */

            std::cout << "Default client socket options received are: " << std::endl; 
            print_socket_options(connfd);
            std::cout << std::endl; 

            send(connfd, "Server reply is: Hello! I will invert the socket options you sent me!\n", 69, 0);

            invert_socket_options(connfd);

            // Re-print the options to show their new values
            std::cout << "\n     *****" << std::endl; 
            std::cout << std::endl; 
            print_socket_options(connfd);
            
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
 * socket options provided by the client. 
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
                    newVal =  32248;
                else if (val.i_val == 0 ? newVal = 1 : newVal == 0); 

                setsockopt(connfd, ptr->opt_level, ptr->opt_name, &newVal, sizeof(newVal));
                //printf("default = %s\n", (*ptr->opt_val_str)(&val, len));
            }
            //close(sockfd);
        }
    }
}