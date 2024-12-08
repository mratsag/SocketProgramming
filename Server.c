#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <curl/curl.h>
#include "dotenv.h" // dotenv için gerekli başlık dosyası
#include "cJSON.h"

#define PORT 8080
#define BUFFER_SIZE 1024

// CURL write callback function
struct Memory {
    char *response;
    size_t size;
};

size_t write_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;

    char *ptr = realloc(mem->response, mem->size + total_size + 1);
    if (ptr == NULL) return 0; // Out of memory

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, total_size);
    mem->size += total_size;
    mem->response[mem->size] = '\0';

    return total_size;
}

// Function to fetch weather data from OpenWeather API
char *fetch_weather_data(const char *city) {
    CURL *curl;
    CURLcode res;
    struct Memory chunk = {0};

    // Load API key from environment variables
    dotenv_load(".env"); // .env dosyasını yükle
    const char *api_key = getenv("OPENWEATHER_API_KEY");
    if (!api_key) {
        fprintf(stderr, "API key not found in environment variables\n");
        return NULL;
    }

    char url[BUFFER_SIZE];
    snprintf(url, BUFFER_SIZE, "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric", city, api_key);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.response);
            chunk.response = NULL;
        }

        curl_easy_cleanup(curl);
    }

    return chunk.response;
}

// Function to parse weather data and extract useful information
char *parse_weather_data(const char *json_response) {
    cJSON *json = cJSON_Parse(json_response);
    if (json == NULL) return strdup("Failed to parse weather data.");

    cJSON *main = cJSON_GetObjectItemCaseSensitive(json, "main");
    cJSON *weather = cJSON_GetObjectItemCaseSensitive(json, "weather");
    cJSON *temp = cJSON_GetObjectItemCaseSensitive(main, "temp");
    cJSON *description = cJSON_GetArrayItem(weather, 0);

    char *weather_info = malloc(BUFFER_SIZE);
    snprintf(weather_info, BUFFER_SIZE, "Current weather: %s, Temperature: %.1f°C",
             cJSON_GetObjectItemCaseSensitive(description, "description")->valuestring,
             temp->valuedouble);

    cJSON_Delete(json);
    return weather_info;
}

// Thread function to handle client requests
void *handle_client(void *socket_desc) {
    int client_socket = *(int *)socket_desc;
    free(socket_desc);

    char city_name[BUFFER_SIZE];
    int read_size = read(client_socket, city_name, BUFFER_SIZE);
    if (read_size <= 0) {
        close(client_socket);
        return NULL;
    }

    city_name[read_size] = '\0'; // Null-terminate the string

    char *weather_data = fetch_weather_data(city_name);
    if (weather_data == NULL) {
        char *error_msg = "Failed to fetch weather data.";
        send(client_socket, error_msg, strlen(error_msg), 0);
        close(client_socket);
        return NULL;
    }

    char *parsed_data = parse_weather_data(weather_data);
    send(client_socket, parsed_data, strlen(parsed_data), 0);

    free(weather_data);
    free(parsed_data);
    close(client_socket);
    return NULL;
}

int main() {
    int server_fd, client_socket, *new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        new_socket = malloc(1);
        *new_socket = client_socket;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_socket) < 0) {
            perror("Could not create thread");
            return 1;
        }

        pthread_detach(thread_id);
    }

    return 0;
}