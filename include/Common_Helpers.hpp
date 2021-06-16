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

#include <cstdlib>

// for streams - cout, cerr, etc.
#include <iostream>

#define PORT 5001
#define BACKLOG 10
#define SO_USELOOPBACK 100

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



#endif