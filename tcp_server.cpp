#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>

#include <cstdlib>
#include <string.h>
#include <unistd.h>

// for streams - cout, cerr, etc.
#include <iostream>

using namespace std;

#define PORT 5001
#define BACKLOG 10

int main(int argc, char **argv)
{
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
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

        printf("server: got connection from internet address: %d\n", cliaddr.sin_addr.s_addr);

        in_addr client_in_addr = ((struct sockaddr_in *)&cliaddr)->sin_addr;
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_in_addr, str, INET_ADDRSTRLEN);
        printf("IP address is: %s", str);

        if ((childpid = fork()) == 0)
        {                    /* child process */
            close(listenfd); /* close listening socket */

            exit(0);
        }
        close(connfd); /* parent closes connected socket */
    }
}
