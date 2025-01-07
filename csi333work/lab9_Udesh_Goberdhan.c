#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345 
#define BUFFER_SIZE 1024

void run_server();
void run_client(const char* address, int port);

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("Starting in server mode...\n");
        run_server();
    } else if (argc == 3) {
        const char* address = argv[1];
        int port = atoi(argv[2]);
        printf("Starting in client mode. Connecting to %s:%d...\n", address, port);
        run_client(address, port);
    } else { //incase the program isnt run properly this is to clarify how to run it
        fprintf(stderr, "Usage: %s [address] [port]\n", argv[0]);
        return 1;
    }
    return 0;
}

void run_server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int exchanges = 10; // limiting to 10 exchanges
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d\n", PORT);
    
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Client connected\n");

    for (int i = 0; i < exchanges; i++) {
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) break;
        buffer[bytes_received] = '\0';
        printf("Received from client: %s\n", buffer);
        
        const char* pong = "pong";
        send(client_fd, pong, strlen(pong), 0);
        printf("Sent to client: pong\n");

        sleep(1); // 1 second delay between each exchange
    }
    
    printf("Reached 10 exchanges, closing connection. Server Side.\n");
    close(client_fd);
    close(server_fd);
}

void run_client(const char* address, int port) {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int exchanges = 10; // limiting to 10 exchanges might not be needed for client side
    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < exchanges; i++) {
        const char* ping = "ping";
        send(client_fd, ping, strlen(ping), 0);
        printf("Sent to server: ping\n");
        
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) break;
        buffer[bytes_received] = '\0';
        printf("Received from server: %s\n", buffer);

        sleep(1); // 1 second delay between each exchange
    }

    printf("Reached 10 exchanges, closing connection. Client Side.\n");
    close(client_fd);
}

