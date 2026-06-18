#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "connection_handler.h"


#define PORT 8080


char* getlocalhostipaddress() {
    struct hostent *host;
    struct in_addr **addr_list;

    host = gethostbyname("localhost");
    if (host == NULL) {
        perror("Error getting localhost");
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    return inet_ntoa(*addr_list[0]);
}

int main() {

    char* local_host_address = getlocalhostipaddress();

    struct sockaddr_in address = {
        AF_INET,
        htons(PORT),
        {inet_addr(local_host_address)}, {0}
    };


    int file_descriptor;

    if ((file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
    }

    // Set the SO_REUSEPORT option
    int flag = 1;
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag)) < 0) {
        perror("Error in trying to setup Reuseport");
        exit(EXIT_FAILURE);
    }

    printf("file_descriptor: %d\n", file_descriptor);

    // Bind socket to port
    int result = bind(file_descriptor, (struct sockaddr *) &address, sizeof(address));
    if (result < 0) {
        perror("Error in binding socket to port");
        exit(EXIT_FAILURE);
    }

    int listen_result = listen(file_descriptor, 10);
    if (listen_result < 0) {
        perror("Error in listening");
        exit(EXIT_FAILURE);
    }

    // Define passive socket that listens for connections
    while (1) {
        const int new_connection = accept(file_descriptor, NULL,NULL);
        if (new_connection < 0) {
            perror("Accept connection Failed");
            exit(EXIT_FAILURE);
        }

        // Call handler
        handle_connection(new_connection);
    }

    close(file_descriptor);

    return 0;
}

