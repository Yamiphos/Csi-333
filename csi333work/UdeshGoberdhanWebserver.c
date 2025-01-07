#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PORT 8000
#define BUFFER_SIZE 1024
#define RESPONSE_HEADER_SIZE 512

void *handle_client(void *client_socket_ptr);
void send_response(int client_socket, const char *status, const char *content_type, const char *body, size_t body_length);
void serve_file(int client_socket, const char *file_path);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <base_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *base_directory = argv[1];
    chdir(base_directory);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread_id;
        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        if (pthread_create(&thread_id, NULL, handle_client, client_socket_ptr) != 0) {
            perror("Thread creation failed");
            free(client_socket_ptr);
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}

void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    free(client_socket_ptr);

    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE - 1);

    char method[16], path[256];
    sscanf(buffer, "%s %s", method, path);

    if (strcmp(method, "GET") == 0) {
        if (path[0] == '/') memmove(path, path + 1, strlen(path));  // Remove leading '/'
        if (strlen(path) == 0) strcpy(path, "index.html");         // Default file

        serve_file(client_socket, path);
    } else {
        send_response(client_socket, "405 Method Not Allowed", "text/plain", "Method Not Allowed", 18);
    }

    close(client_socket);
    return NULL;
}

void send_response(int client_socket, const char *status, const char *content_type, const char *body, size_t body_length) {
    char header[RESPONSE_HEADER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "\r\n",
             status, content_type, body_length);

    write(client_socket, header, strlen(header));
    write(client_socket, body, body_length);
}

void serve_file(int client_socket, const char *file_path) {
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        send_response(client_socket, "404 Not Found", "text/plain", "File Not Found", 13);
        return;
    }

    struct stat file_stat;
    fstat(file_fd, &file_stat);

    char *file_content = malloc(file_stat.st_size);
    read(file_fd, file_content, file_stat.st_size);

    send_response(client_socket, "200 OK", "text/html", file_content, file_stat.st_size);

    free(file_content);
    close(file_fd);
}
