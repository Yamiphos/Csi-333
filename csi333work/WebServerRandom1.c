#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PORT "8000"
#define BUFFER_SIZE 1024

// Global variable to store the base directory path
char* base_dir;

// Function prototype for handling requests
void* handle_request(void* arg);
int send_response(int client_fd, const char* status, const char* content_type, const char* body);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <base_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    base_dir = argv[1];
    printf("Starting server with base directory: %s\n", base_dir);

    struct addrinfo hints, *res;
    int server_fd;

    // Prepare address info hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get address info for the server
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo failed");
        exit(EXIT_FAILURE);
    }

    // Create the server socket
    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_fd == -1) {
        perror("Socket creation failed");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the address and port
    if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Bind failed");
        close(server_fd);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    // Start listening for incoming connections
    if (listen(server_fd, 10) == -1) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %s\n", PORT);

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
		
        if (client_fd == -1) {
            perror("Accept failed");
            continue;
        }

        // Spawn a new thread to handle the request
        pthread_t thread_id;
        int* client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        pthread_create(&thread_id, NULL, handle_request, client_fd_ptr);
        pthread_detach(thread_id);  // Automatically clean up the thread when done
    }

    close(server_fd);
    return 0;
}

// Function to handle each client request
void* handle_request(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    struct stat file_stat;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received < 1) {
        close(client_fd);
        return NULL;
    }

    buffer[bytes_received] = '\0';

    // Check if the request is a GET request
    if (strncmp(buffer, "GET ", 4) == 0) {
        char* path_start = buffer + 4;
        char* path_end = strchr(path_start, ' ');
        if (path_end) {
            *path_end = '\0';

            // Create the full file path
            char full_path[BUFFER_SIZE];
            snprintf(full_path, sizeof(full_path), "%s%s", base_dir, path_start);

            // Use stat instead of fstat to check if the file exists and get its stats
            if (stat(full_path, &file_stat) == 0) {
                int file_size = file_stat.st_size;

                // Format response with Content-Length
                char response_header[BUFFER_SIZE];
                snprintf(response_header, sizeof(response_header),
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Length: %d\r\n"
                         "Content-Type: text/html\r\n\r\n", 
                         file_size);

                // Send headers
                send(client_fd, response_header, strlen(response_header), 0);

                // Open the file and send its content
                int file_fd = open(full_path, O_RDONLY);
                if (file_fd != -1) {
                    char file_buffer[BUFFER_SIZE];
                    ssize_t bytes_read;
                    while ((bytes_read = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
                        send(client_fd, file_buffer, bytes_read, 0);
                    }
                    close(file_fd);
                }
            } else {
                // File does not exist, send a 404 Not Found response
                const char* not_found_body = "<html><body><h1>404 Not Found</h1></body></html>";
                send_response(client_fd, "404 Not Found", "text/html", not_found_body);
            }
        }
    }

    close(client_fd);
    return NULL;
}


// Function to send an HTTP response
int send_response(int client_fd, const char* status, const char* content_type, const char* body) {
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "HTTP/1.1 %s\r\nContent-Type: %s\r\n\r\n", status, content_type);
    send(client_fd, header, strlen(header), 0);

    if (body) {
        send(client_fd, body, strlen(body), 0);
    }

    return 0;
}
