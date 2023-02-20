#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main() {
    // create socket
    int socket_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_s == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // prepare server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // send data to server
    char buffer[BUFFER_SIZE] = "Hello, server!";
    ssize_t send_size = sendto(socket_s, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (send_size == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    // receive response from server
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    ssize_t recv_size = recvfrom(socket_s, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sender_addr, &sender_addr_len);
    if (recv_size == -1) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    // print received data
    printf("Received %zd bytes from %s:%d\n", recv_size, inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port));
    printf("Response from server: %s\n", buffer);

    // close socket
    close(socket_s);
    return 0;
}
