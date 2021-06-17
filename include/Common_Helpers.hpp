#ifndef COMMON_HELPERS_H
#define COMMON_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <netinet/tcp.h> /* for TCP_xxx defines */

#include <csignal>
#include <cstdlib>

// for streams - cout, cerr, etc.
#include <iostream>

#define PORT 5001
#define BACKLOG 10
#define BUF_SIZE 16384

#define SO_USELOOPBACK 100

// ANSII codes for colored text, to improve UI and readability
std::string ANSII_BLUE_START = "\033[0;30;46m";
std::string ANSII_RED_START = "\033[0;31m";
std::string ANSII_RED_HIGH = "\033[9;37;41m";
std::string ANSII_END = "\033[0m";
std::string ANSII_GREEN_START = "\033[0;32m";
std::string ANSII_BLUE_COUT = "\033[0;30;46m";
std::string ANSII_RED_COUT = "\033[41;1m";
std::string ANSII_GREEN_COUT = "\033[0;30;42m";
std::string ANSII_YELLOW_COUT = "\033[30;48;5;3m";

// The purpose of union is to save memory by using the same
// memory space for storage of different objects at different times
union val
{
    int i_val;
    long l_val;
    struct linger linger_val;
    struct timeval timeval_val;
} val;

static char *sock_str_flag(union val *, int);
static char *sock_str_int(union val *, int);
static char *sock_str_linger(union val *, int);
static char *sock_str_timeval(union val *, int);

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

/* include checkopts3 */
static char strres[128];

static char *
sock_str_flag(union val *ptr, int len)
{
    /* *INDENT-OFF* */
    if (len != sizeof(int))
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
    else
        snprintf(strres, sizeof(strres),
                 "%s", (ptr->i_val == 0) ? "off" : "on");
    return (strres);
    /* *INDENT-ON* */
}
/* end checkopts3 */

static char *
sock_str_int(union val *ptr, int len)
{
    if (len != sizeof(int))
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
    else
        snprintf(strres, sizeof(strres), "%d", ptr->i_val);
    return (strres);
}

static char *
sock_str_linger(union val *ptr, int len)
{
    struct linger *lptr = &ptr->linger_val;

    if (len != sizeof(struct linger))
        snprintf(strres, sizeof(strres),
                 "size (%d) not sizeof(struct linger)", len);
    else
        snprintf(strres, sizeof(strres), "l_onoff = %d, l_linger = %d",
                 lptr->l_onoff, lptr->l_linger);
    return (strres);
}

static char *
sock_str_timeval(union val *ptr, int len)
{
    struct timeval *tvptr = &ptr->timeval_val;

    if (len != sizeof(struct timeval))
        snprintf(strres, sizeof(strres),
                 "size (%d) not sizeof(struct timeval)", len);
    else
        snprintf(strres, sizeof(strres), "%d sec, %d usec",
                 tvptr->tv_sec, tvptr->tv_usec);
    return (strres);
}

/**
 * Displays a compatible, pleasant graphic to the user as a goodbye
 */
void goodbye()
{
    std::cout << std::endl;
    std::cout << ANSII_BLUE_START << " __  __  __  __  __     __  " << ANSII_END << std::endl;
    std::cout << ANSII_BLUE_START << "/ _ /  \\/  \\|  \\|__)\\_/|_ | " << ANSII_END << std::endl;
    std::cout << ANSII_BLUE_START << "\\__)\\__/\\__/|__/|__) | |__. " << ANSII_END << std::endl;
    std::cout << ANSII_BLUE_START << "                            " << ANSII_END << std::endl;
    std::cout << std::endl;
}

#endif
