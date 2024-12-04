#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in server_address;
    char city_name[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    // Get city name from user
    printf("Enter the city name: ");
    fgets(city_name, BUFFER_SIZE, stdin);
    city_name[strcspn(city_name, "\n")] = 0; // Remove newline character

    // Send city name to server
    send(sock, city_name, strlen(city_name), 0);

    // Read weather data from server
    read(sock, buffer, BUFFER_SIZE);
    printf("Weather data for %s:\n%s\n", city_name, buffer);

    // Close the socket
    close(sock);

    return 0;
}