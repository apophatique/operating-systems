#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <asm/ioctls.h>
#include <time.h>

#define STD_INPUT_HANDLE 0
#define STD_OUTPUT_HANDLE 1
#define STD_ERROR_HANDLE 2

#define EXIT_WITH_SUCCESS 0
#define EXIT_WITH_ERROR -1

ssize_t write_to_handle(const int file_descriptor, const char *buffer) {
    ssize_t length_of_data_to_write = 0;

    while (buffer[length_of_data_to_write] != 0) {
        length_of_data_to_write++;
    }

    return write(file_descriptor, buffer, length_of_data_to_write);
}

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

ssize_t clear_screen(const int file_descriptor) {
    if (write_to_handle(file_descriptor, "\x1b[2J") == -1) {
        return -1;
    }

    dprintf(STD_OUTPUT_HANDLE, "\x1b[1;1H");
    dprintf(STD_OUTPUT_HANDLE, "\x1b[0m");
    return 0;
}

ssize_t get_terminal_size(int *rows, int *cols) {
    struct winsize window_size;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1) {
        return -1;
    }

    *rows = window_size.ws_row;
    *cols = window_size.ws_col;
    return 0;
}

ssize_t write_to_screen_center_of_handle(const int file_descriptor, const char *buffer) {
    ssize_t length_of_data_to_write = 0;

    while (buffer[length_of_data_to_write] != 0) {
        length_of_data_to_write++;
    }

    int term_size_rows, term_size_cols;
    if (get_terminal_size(&term_size_rows, &term_size_cols) == -1) {
        return -1;
    }

    if (clear_screen(file_descriptor) == -1) {
        return -1;
    }

    const short COLORS[] = { 31, 32, 33, 34, 35, 36 };
    dprintf(file_descriptor, "\x1b[%dm", COLORS[rand() % 5]);
    dprintf(file_descriptor, "\x1b[%d;%dH", term_size_rows / 2, term_size_cols / 2 - length_of_data_to_write / 2);
    return write(file_descriptor, buffer, length_of_data_to_write);
}

int main(int argc, char *argv[]) {
    assert(argc == 3);

    ssize_t length_of_read_data,
            length_of_written_data;

    const char *FILE_PATH = argv[1];
    const size_t FILE_SIZE = (size_t)atoi(argv[2]);

    const mode_t PERMISSION = 0600;
    int file_descriptor;

    srand((unsigned int)time(NULL));

    if ((file_descriptor = open(FILE_PATH, O_RDONLY, PERMISSION)) == -1) {
        write_to_screen_center_of_handle(STD_OUTPUT_HANDLE, strerror(errno));
        getchar();
        clear_screen(STD_OUTPUT_HANDLE);
        return EXIT_WITH_ERROR;
    }

    if (flock(file_descriptor, LOCK_EX) == -1) {
        write_to_screen_center_of_handle(STD_OUTPUT_HANDLE, strerror(errno));
        getchar();
        clear_screen(STD_OUTPUT_HANDLE);
        close(file_descriptor);
        return EXIT_WITH_ERROR;
    }

    char *buffer = (char *)calloc(FILE_SIZE + 1, sizeof(char));

    if ((length_of_read_data = read(file_descriptor, buffer, FILE_SIZE)) == -1) {
        write_to_screen_center_of_handle(STD_OUTPUT_HANDLE, strerror(errno));
        getchar();
        clear_screen(STD_OUTPUT_HANDLE);
        close(file_descriptor);
        free(buffer);
        return EXIT_WITH_ERROR;
    }

    buffer[FILE_SIZE] = 0;

    if ((length_of_written_data = write_to_screen_center_of_handle(STD_OUTPUT_HANDLE, buffer)) == -1) {
        write_to_screen_center_of_handle(STD_OUTPUT_HANDLE, strerror(errno));
        getchar();
        clear_screen(STD_OUTPUT_HANDLE);
        close(file_descriptor);
        free(buffer);
        return EXIT_WITH_ERROR;
    }

    sleep(7);
    free(buffer);

    if (flock(file_descriptor, LOCK_UN) == -1) {
        write_to_screen_center_of_handle(STD_OUTPUT_HANDLE, strerror(errno));
        getchar();
        clear_screen(STD_OUTPUT_HANDLE);
        return EXIT_WITH_ERROR;
    }

    if (clear_screen(STD_OUTPUT_HANDLE) == -1) {
        return EXIT_WITH_ERROR;
    }

    if (close(file_descriptor) == -1) {
        return EXIT_WITH_ERROR;
    }

    return EXIT_SUCCESS;
}
