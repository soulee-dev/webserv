#include <stdio.h>
#include <stdlib.h>

int main() {
    // Print the HTTP response header
    printf("Content-type: text/plain\n\n");

    // Read the CONTENT_LENGTH environment variable
    char *content_length_str = getenv("CONTENT_LENGTH");
    if (content_length_str == NULL) {
        printf("No POST data received.\n");
        return 0;
    }

    int content_length = atoi(content_length_str);
    if (content_length <= 0) {
        printf("Invalid CONTENT_LENGTH.\n");
        return 0;
    }

    // Allocate a buffer to hold the POST data
    char *post_data = malloc(content_length + 1);
    if (post_data == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Read the POST data from standard input
    fread(post_data, 1, content_length, stdin);
    post_data[content_length] = '\0'; // Null-terminate the string

    // Print the POST data
    printf("POST data received:\n");
    printf("%s\n", post_data);

    // Free the buffer
    free(post_data);

    exit(0);
}