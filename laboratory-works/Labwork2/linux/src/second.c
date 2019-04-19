#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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

/**
 * This function doesn't return a file descriptor, but rather puts it into buffer by pointer which is @param.
 * This is done because if both conditions in the body of function are false, it should not return anything.
 * @param buffer Pointer to buffer that will store required file descriptor.
 * @return Function returns: 0 if getting was successful;
 *                           -1 if isatty(...) error occurred;
 *                           -2 if ttyname(...) error occured;
 *                           -3 if open(...) error occured.
 */
int get_tty_output_file_descriptor(int *buffer) {
    if (isatty(STD_OUTPUT_HANDLE) == 1) {
        *buffer = STD_OUTPUT_HANDLE;
        return 0;
    } else if (errno == EBADF) {
        return -1;
    } else if (isatty(STD_INPUT_HANDLE) == 1) {
        int internal_buffer;
        char *name_of_tty;

        if ((name_of_tty = ttyname(STD_INPUT_HANDLE)) == NULL) {
            return -2;
        }

        if ((internal_buffer = open(name_of_tty, O_WRONLY)) == -1) {
            return -3;
        }

        *buffer = internal_buffer;
        return 0;    
    } else if (errno == EBADF) {
        return -1;
    }
}

int main(void) {
    const mode_t PERMISSION = 0600;
    const char *FILE_PATH = "./data.txt";
    int file_descriptors[3];

    if ((file_descriptors[0] = open(FILE_PATH, O_RDONLY, PERMISSION)) == -1) {
        perror("\n\nOPEN ERROR");
        return EXIT_WITH_ERROR;
    }

    if ((file_descriptors[1] = dup(file_descriptors[0])) == -1) {
        perror("\n\nDUP ERROR");

        for (int i = 0; i < 1; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        return EXIT_WITH_ERROR;        
    }

    if ((file_descriptors[2] = dup(file_descriptors[1])) == -1) {
        perror("\n\nDUP ERROR");

        for (int i = 0; i < 2; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        return EXIT_WITH_ERROR;
    }

    off_t offset;

    if ((offset = lseek(file_descriptors[0], (off_t)10, SEEK_SET)) == (off_t)-1) {
        perror("\n\nLSEEK ERROR");

        for (int i = 0; i < 3; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        return EXIT_WITH_ERROR;
    }

    int tty_file_descriptor;
    ssize_t length_of_read_data,
            length_of_written_data;

    if (isatty(STD_INPUT_HANDLE) == 1) {        
        const int result_of_getting_tty_fd = get_tty_output_file_descriptor(&tty_file_descriptor);

        if (result_of_getting_tty_fd == -1) {
            perror("\n\nISATTY ERROR");

            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            return EXIT_WITH_ERROR;
        } else if (result_of_getting_tty_fd == -2) {
            perror("\n\nTTYNAME ERROR");

            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            return EXIT_WITH_ERROR;
        } else if (result_of_getting_tty_fd == -3) {
            perror("\n\nOPEN ERROR");

            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            return EXIT_WITH_ERROR;
        }

        if ((length_of_written_data = write_with_auto_length_counting(tty_file_descriptor,
                                                                      "\n1nd file descriptor: ")) == -1) {
            perror("\n\nWRITE ERROR");

            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            if (close(tty_file_descriptor) == -1) {
                perror("\n\nCLOSE ERROR");
            }

            return EXIT_WITH_ERROR;
        }

        if ((length_of_written_data = dprintf(tty_file_descriptor, "%d", file_descriptors[0])) < 0) {
            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            if (close(tty_file_descriptor) == -1) {
                perror("\n\nCLOSE ERROR");
            }

            return EXIT_WITH_ERROR;
        }

        if ((length_of_written_data = write_with_auto_length_counting(tty_file_descriptor,
                                                                      "\n\n2nd file descriptor: ")) == -1) {
            perror("\n\nWRITE ERROR");

            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            if (close(tty_file_descriptor) == -1) {
                perror("\n\nCLOSE ERROR");
            }

            return EXIT_WITH_ERROR;
        }

        if ((length_of_written_data = dprintf(tty_file_descriptor, "%d", file_descriptors[1])) < 0) {
            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            if (close(tty_file_descriptor) == -1) {
                perror("\n\nCLOSE ERROR");
            }

            return EXIT_WITH_ERROR;
        }

        if ((length_of_written_data = write_with_auto_length_counting(tty_file_descriptor,
                                                                      "\n\n3rd file descriptor: ")) == -1) {
            perror("\n\nWRITE ERROR");

            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            if (close(tty_file_descriptor) == -1) {
                perror("\n\nCLOSE ERROR");
            }

            return EXIT_WITH_ERROR;
        }

        if ((length_of_written_data = dprintf(tty_file_descriptor, "%d", file_descriptors[2])) < 0) {
            for (int i = 0; i < 3; i++) {
                if (close(file_descriptors[i]) == -1) {
                    perror("\n\nCLOSE ERROR");
                }
            }

            if (close(tty_file_descriptor) == -1) {
                perror("\n\nCLOSE ERROR");
            }

            return EXIT_WITH_ERROR;
        }
    } else if (errno == EBADF) {
        perror("\n\nISATTY ERROR");

        for (int i = 0; i < 1; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        if (close(tty_file_descriptor) == -1) {
            perror("\n\nCLOSE ERROR");
        }

        return EXIT_WITH_ERROR;
    }

    const ssize_t BUFFER_SIZE = 7;
    char *buffer_1 = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));
    char *buffer_2 = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));
    char *buffer_3 = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));

    if ((length_of_read_data = read(file_descriptors[0], buffer_1, 7)) == -1) {
        perror("\n\nREAD ERROR");

        for (int i = 0; i < 3; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        if (close(tty_file_descriptor) == -1) {
            perror("\n\nCLOSE ERROR");
        }

        free(buffer_1);
        free(buffer_2);
        free(buffer_3);
        return EXIT_WITH_ERROR;
    }

    if ((length_of_written_data = dprintf(tty_file_descriptor, "\n\n%s", buffer_1)) < 0) {
        for (int i = 0; i < 3; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        if (close(tty_file_descriptor) == -1) {
            perror("\n\nCLOSE ERROR");
        }

        free(buffer_1);
        free(buffer_2);
        free(buffer_3);
        return EXIT_WITH_ERROR;
    }
    
    if ((length_of_read_data = read(file_descriptors[1], buffer_2, 7)) == -1) {
        perror("\n\nREAD ERROR");

        for (int i = 0; i < 3; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        if (close(tty_file_descriptor) == -1) {
            perror("\n\nCLOSE ERROR");
        }

        free(buffer_1);
        free(buffer_2);
        free(buffer_3);
        return EXIT_WITH_ERROR;
    }

    if ((length_of_written_data = dprintf(tty_file_descriptor, "\n\n%s", buffer_2)) < 0) {
        for (int i = 0; i < 3; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        if (close(tty_file_descriptor) == -1) {
            perror("\n\nCLOSE ERROR");
        }

        free(buffer_1);
        free(buffer_2);
        free(buffer_3);
        return EXIT_WITH_ERROR;
    }

    if ((length_of_read_data = read(file_descriptors[2], buffer_3, 7)) == -1) {
        perror("\n\nREAD ERROR");

        for (int i = 0; i < 3; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        if (close(tty_file_descriptor) == -1) {
            perror("\n\nCLOSE ERROR");
        }

        free(buffer_1);
        free(buffer_2);
        free(buffer_3);
        return EXIT_WITH_ERROR;
    }

    if ((length_of_written_data = dprintf(tty_file_descriptor, "\n\n%s\n\n", buffer_3)) < 0) {
        for (int i = 0; i < 3; i++) {
            if (close(file_descriptors[i]) == -1) {
                perror("\n\nCLOSE ERROR");
            }
        }

        if (close(tty_file_descriptor) == -1) {
            perror("\n\nCLOSE ERROR");
        }

        free(buffer_1);
        free(buffer_2);
        free(buffer_3);
        return EXIT_WITH_ERROR;
    }

    free(buffer_1);
    free(buffer_2);
    free(buffer_3);
    return EXIT_WITH_SUCCESS;
}
