#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 8080


char* getlocalhostipaddress() {
    struct hostent *host;
    struct in_addr **addr_list;

    host = gethostbyname("localhost");
    if (host == NULL) {
        perror("Error getting localhost");
    }

    int i;
    addr_list = (struct in_addr **) host->h_addr_list;
    return inet_ntoa(*addr_list[0]);
}

int main() {

    char* local_host_address = getlocalhostipaddress();

    struct sockaddr_in address = {
        AF_INET,
        htons(PORT),
        inet_addr(local_host_address),
        {0}
    };


    int file_descriptor;

    if ((file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
    }

    printf("file_descriptor: %d\n", file_descriptor);

    // Bind socket to port
    int result = bind(file_descriptor, (struct sockaddr *) &address, sizeof(address));
    if (result == -1) {
        perror("Error in binding socket to port");
        exit(EXIT_FAILURE);
    }

    // Listen for new connections
    const int status = listen(file_descriptor,10);
    if (status < 0) {
        perror("Error in passive socket listening");
        exit(EXIT_FAILURE);
    }

    // Define handler function

    // Accept a new connection
    const int client_connection = accept(file_descriptor, NULL, NULL);
    if (client_connection < 0) {
        perror("Error in accepting new connection");
        exit(EXIT_FAILURE);
    }

    close(file_descriptor);
    return 0;
}

