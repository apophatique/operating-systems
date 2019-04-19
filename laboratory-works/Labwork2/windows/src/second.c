#include <stdio.h>
#include <Windows.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>

#define EXIT_WITH_SUCCESS 0
#define EXIT_WITH_ERROR_SENT_TO_H_ERROR -1
#define EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR -2

/**
 * In addition to simple writing to "output" using given output handle,
 * this function counts the length (in bytes) of the input data.
 * @param h_output Variable of type "HANDLE" that uniquely identifies "output" in Windows.
 * @param buffer Pointer to char buffer that stores data.
 * @param length_of_written_data Pointer to variable of type "DWORD" that will store length of written data.
 * @return A Boolean value that indicates success (or failure) of writing.
 */
BOOL write_file_with_auto_length_counting(const HANDLE h_output, const char *buffer, DWORD *length_of_written_data) {
    DWORD length_of_data_to_write = 0;

    while (buffer[length_of_data_to_write] != 0) {
        length_of_data_to_write++;
    }

    return WriteFile(h_output, buffer, length_of_data_to_write, length_of_written_data, NULL);
}

/**
 * This function takes n bytes from beginning of received string,
 * and returns pointer to a new null-terminated string containing only these bytes and '\0'.
 * @param string String from which that function should take bytes.
 * @param n Number of bytes to take from beginning of the string.
 * @return New null-terminated string of size: (n + 1) bytes.
 */
char *get_first_n_bytes_of_string(const char *string, const size_t n) {
    assert(n >= 0);
    assert(strlen(string) >= n);

    char *buffer = (char *)calloc(n + 1, sizeof(char));

    for (size_t i = 0; i < n; i++) {
        buffer[i] = string[i];
    }

    buffer[n] = 0;
    return buffer;
}

/**
 * This function retrieves last error code, then retrieves text of that error, 
 * puts it into a new text-size string, null-terminates that string,
 * converts it from "char" to "oem" and then returns a pointer to that string.
 * @return New null-terminated string that contains the last error message,
 *         or NULL if an error occured.
 */
char *get_last_error_message() {
    const DWORD ERROR_MESSAGE_BUFFER_SIZE = 300;
    char *error_message_long_size_buffer = (char *)calloc(ERROR_MESSAGE_BUFFER_SIZE + 1, sizeof(char));

    const DWORD length_of_error_message = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
                                                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                                                        error_message_long_size_buffer,
                                                        ERROR_MESSAGE_BUFFER_SIZE, NULL);

    if (length_of_error_message == 0) {
        return NULL;
    }

    char *error_message_actual_size_buffer = get_first_n_bytes_of_string(error_message_long_size_buffer, 
                                                                         length_of_error_message);
    char *correct_error_message = (char *)calloc(length_of_error_message + 1, sizeof(char));

    if (CharToOem(error_message_actual_size_buffer, correct_error_message) == 0) {
        return NULL;
    }

    free(error_message_long_size_buffer);
    free(error_message_actual_size_buffer);
    return correct_error_message;
}

/**
 * This function receives last error message and then immediately sends it to given error handle.
 * @param h_error Variable of type "HANDLE" that uniquely identifies "error" in Windows.
 * @param length_of_written_data Pointer to variable of type "DWORD" that will store length of written data.
 * @return A Boolean value that indicates success (or failure) of sending.
 */
BOOL send_last_error_message_to_h_error(const HANDLE h_error, DWORD *length_of_written_data) {
    char *error_message = get_last_error_message();

    if (error_message == NULL) {
        return 0;
    }

    BOOL result_of_writing = write_file_with_auto_length_counting(h_error, error_message, length_of_written_data);
    free(error_message);
    return result_of_writing; 
}

int main(void) {
    DWORD length_of_read_data,
          length_of_written_data;

    const HANDLE H_STD_ERROR = GetStdHandle(STD_ERROR_HANDLE);
    if (H_STD_ERROR == INVALID_HANDLE_VALUE) {
        return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
    }

    const HANDLE H_STD_INPUT = GetStdHandle(STD_INPUT_HANDLE);
    if (H_STD_INPUT == INVALID_HANDLE_VALUE) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            getchar();
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    const HANDLE H_STD_OUTPUT = GetStdHandle(STD_OUTPUT_HANDLE);
    if (H_STD_OUTPUT == INVALID_HANDLE_VALUE) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            getchar();
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    const char *FILE_PATH = "data.txt";
    const HANDLE H_FILE_1 = CreateFile(FILE_PATH, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (H_FILE_1 == INVALID_HANDLE_VALUE) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            getchar();
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    HANDLE H_FILE_2;

    if (DuplicateHandle(GetCurrentProcess(), H_FILE_1, GetCurrentProcess(), &H_FILE_2, 0, 0, DUPLICATE_SAME_ACCESS) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    HANDLE H_FILE_3;

    if (DuplicateHandle(GetCurrentProcess(), H_FILE_2, GetCurrentProcess(), &H_FILE_3, 0, 0, DUPLICATE_SAME_ACCESS) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    printf("H_FILE_1 is %d.\n\nH_FILE_2 is %d.\n\nH_FILE_3 is %d.\n\n",
           H_FILE_1, H_FILE_2, H_FILE_3);

    if (SetFilePointer(H_FILE_1, 10, NULL, FILE_BEGIN) == -1) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    const DWORD BUFFER_SIZE = 7;
    char *buffer_1 = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));
    char *buffer_2 = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));
    char *buffer_3 = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));

    if (ReadFile(H_FILE_1, buffer_1, BUFFER_SIZE, &length_of_read_data, NULL) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (ReadFile(H_FILE_2, buffer_2, BUFFER_SIZE, &length_of_read_data, NULL) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (ReadFile(H_FILE_3, buffer_3, BUFFER_SIZE, &length_of_read_data, NULL) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (write_file_with_auto_length_counting(H_STD_OUTPUT, "First buffer: ", &length_of_written_data) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (write_file_with_auto_length_counting(H_STD_OUTPUT, buffer_1, &length_of_written_data) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (write_file_with_auto_length_counting(H_STD_OUTPUT, "\n\nSecond buffer: ", &length_of_written_data) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (write_file_with_auto_length_counting(H_STD_OUTPUT, buffer_2, &length_of_written_data) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (write_file_with_auto_length_counting(H_STD_OUTPUT, "\n\nThird buffer: ", &length_of_written_data) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (write_file_with_auto_length_counting(H_STD_OUTPUT, buffer_3, &length_of_written_data) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            if (CloseHandle(H_FILE_1) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_2) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            if (CloseHandle(H_FILE_3) == 0) {
                if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
                } else {
                    getchar();
                    free(buffer_1);
                    free(buffer_2);
                    free(buffer_3);
                    return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
                }
            }

            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (CloseHandle(H_FILE_1) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (CloseHandle(H_FILE_2) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    if (CloseHandle(H_FILE_3) == 0) {
        if (send_last_error_message_to_h_error(H_STD_ERROR, &length_of_written_data) == 0) {
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_NOT_SENT_TO_H_ERROR;
        } else {
            getchar();
            free(buffer_1);
            free(buffer_2);
            free(buffer_3);
            return EXIT_WITH_ERROR_SENT_TO_H_ERROR;
        }
    }

    getchar();
    free(buffer_1);
    free(buffer_2);
    free(buffer_3);
    return EXIT_WITH_SUCCESS;
}
