#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024


struct sockaddr_in bind_socket(int socket){
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    return server_addr;
}


int main() {
    // create socket to receive data
    int socket_r = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    if (socket_r == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // bind socket to local address and port
    server_addr = bind_socket(socket_r);

    // receive and process data
    char buffer[MAX_BUFFER_SIZE];
    
    socklen_t client_addr_len = sizeof(client_addr);
    ssize_t recv_size;
    puts("Listening..");
    while ((recv_size = recvfrom(socket_r, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len)) != -1) {
        // process received data
        printf("Received %zd bytes from %s:%d\n", recv_size, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("Received this message from client: %s\n", buffer);
        
        // send response back to client
        char buffer_cli[MAX_BUFFER_SIZE] = "Hello, client!";
        ssize_t send_size = sendto(socket_r, buffer_cli, recv_size, 0, (struct sockaddr*)&client_addr, client_addr_len);
        if (send_size == -1) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
        
        printf("Sent %zd bytes back to %s:%d\n", send_size, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    // close socket
    close(socket_r);
    return 0;
}
