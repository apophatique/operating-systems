#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <sys/stat.h>

#define STD_INPUT_HANDLE 0
#define STD_OUTPUT_HANDLE 1
#define STD_ERROR_HANDLE 2

#define EXIT_WITH_SUCCESS 0
#define EXIT_WITH_ERROR -1

/**
 * In addition to simple writing to "output" using given file descriptor,
 * this function counts the length (in bytes) of input data.
 * @param buffer Pointer to char buffer that stores data.
 * @param file_descriptor Number that uniquely identifies "output" in Linux.
 * @return Length (in bytes) of written data.
 */
ssize_t write_with_auto_length_counting(const int file_descriptor, const char *buffer) {
    ssize_t length_of_data_to_write = 0;

    while (buffer[length_of_data_to_write] != 0) {
        length_of_data_to_write++;
    }

    return write(file_descriptor, buffer, length_of_data_to_write);
}

/**
 * This function takes n bytes from beginning of received string
 * and returns pointer to a new null-terminated string containing only these bytes and '\0'.
 * @param string String from which that function should take bytes.
 * @param n Number of bytes to take from beginning of the string.
 * @return New null-terminated string of size: (n + 1) bytes.
 */
char *get_first_n_bytes_of_string(const char *string, const ssize_t n) {
    assert(n >= 0);
    assert(strlen(string) >= n);

    char *buffer = (char *)calloc(n + 1, sizeof(char));

    for (ssize_t i = 0; i < n; i++) {
        buffer[i] = string[i];
    }

    buffer[n] = 0;
    return buffer;
}

int main(void) {
    const ssize_t BUFFER_SIZE = 1000;
    char *buffer = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));

    ssize_t length_of_read_data,
            length_of_written_data;

    if ((length_of_read_data = read(STD_INPUT_HANDLE, buffer, BUFFER_SIZE)) == -1) {
        perror("\n\nREAD ERROR");
        free(buffer);
        return EXIT_WITH_ERROR;
    }

    char *correct_data = get_first_n_bytes_of_string(buffer, length_of_read_data);
    free(buffer);

    const mode_t PERMISSION = 0600;
    const char *FILE_PATH = "./data.txt";
    int file_descriptor;

    if ((file_descriptor = open(FILE_PATH, O_WRONLY | O_CREAT, PERMISSION)) == -1) {
        perror("\n\nOPEN ERROR");
        free(correct_data);
        return EXIT_WITH_ERROR;
    }

    if ((length_of_written_data = write_with_auto_length_counting(file_descriptor, correct_data)) == -1) {
        perror("\n\nWRITE ERROR");

        if (close(file_descriptor) == -1) {
            perror("\n\nCLOSE ERROR");
        }

        free(correct_data);
        return EXIT_WITH_ERROR;
    }

    free(correct_data);
    return EXIT_WITH_SUCCESS;
}
