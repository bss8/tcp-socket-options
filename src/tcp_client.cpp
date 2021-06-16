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

    char buffer[256];
    if (argc != 3) {
       std::cout << "Invoke with: " << argv[0] << " hostname port\n" << std::endl; 
       return EXIT_FAILURE;
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        return EXIT_FAILURE;
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        return EXIT_FAILURE;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);

    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) 
    {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }

    bzero(buffer,256);
    n = read(sockfd, buffer, 255);
    
    if (n < 0) 
    {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", buffer);
 
    close(sockfd);
 
    return EXIT_SUCCESS;
}