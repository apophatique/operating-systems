#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_SYSTEM_ERROR_LOGGED -1
#define EXIT_SYSTEM_ERROR_NOT_LOGGED -2
#define EXIT_BAD_USER_INPUT -3

BOOL write_file_with_auto_length_counting(const HANDLE h_output, const char *buffer, DWORD *length_of_written_data) {
    DWORD length_of_data_to_write = 0;

    while (buffer[length_of_data_to_write] != 0) {
        length_of_data_to_write++;
    }

    return WriteFile(h_output, buffer, length_of_data_to_write, length_of_written_data, NULL);
}

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

BOOL clear_console_buffer(const HANDLE h_console, DWORD *length_of_written_data) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (GetConsoleScreenBufferInfo(h_console, &csbi) == 0) {
        return 0;
    }

    COORD screen_size = csbi.dwSize;
    WORD length = (WORD)(screen_size.Y * screen_size.X);

    COORD start_position;
    start_position.Y = 0;
    start_position.X = 0;

    return FillConsoleOutputCharacter(h_console, ' ', length, start_position, length_of_written_data);
}

BOOL write_to_center_of_console_with_auto_length_counting(const HANDLE h_console, const char *buffer,
                                                          DWORD *length_of_written_data, WORD attribute) {
    DWORD length_of_data_to_write = 0;

    while (buffer[length_of_data_to_write] != 0) {
        length_of_data_to_write++;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(h_console, &csbi) == 0) {
        return 0;
    }

    COORD start_coord;
    start_coord.X = (csbi.srWindow.Right - csbi.srWindow.Left + 1) / 2 - length_of_data_to_write / 2 - 1;
    start_coord.Y = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1) / 2 - 1;

    WORD *attributes = (WORD *)calloc(length_of_data_to_write, sizeof(WORD));
    for (int i = 0; i < length_of_data_to_write; i++) {
        attributes[i] = attribute;
    }

    if (WriteConsoleOutputAttribute(h_console, attributes, length_of_data_to_write,
                                    start_coord, length_of_written_data) == 0) {
        return 0;
    }

    return WriteConsoleOutputCharacter(h_console, buffer, length_of_data_to_write,
                                       start_coord, length_of_written_data);
}

char *get_last_error_message() {
    const DWORD ERROR_MESSAGE_FILE_SIZE = 300;
    char *error_message_long_size_buffer = (char *)calloc(ERROR_MESSAGE_FILE_SIZE + 1, sizeof(char));

    const DWORD length_of_error_message = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                                                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                                        error_message_long_size_buffer,
                                                        ERROR_MESSAGE_FILE_SIZE, NULL);

    if (length_of_error_message == 0) {
        return NULL;
    }

    char *error_message_actual_size_buffer = get_first_n_bytes_of_string(error_message_long_size_buffer,
                                                                         length_of_error_message);

    free(error_message_long_size_buffer);
    return error_message_actual_size_buffer;
}

BOOL send_last_error_message_to_some_handle(const HANDLE handle, DWORD *length_of_written_data) {
    char *error_message = get_last_error_message();

    if (error_message == NULL) {
        return 0;
    }

    error_message = get_first_n_bytes_of_string(error_message, strlen(error_message) - 2);
    BOOL result_of_writing = write_to_center_of_console_with_auto_length_counting(handle, error_message,
                                                                                  length_of_written_data,
                                                                                  FOREGROUND_RED);
    free(error_message);
    return result_of_writing;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return EXIT_BAD_USER_INPUT;
    }

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    DWORD length_of_read_data,
          length_of_written_data;

    const HANDLE H_STD_OUTPUT = GetStdHandle(STD_OUTPUT_HANDLE);
    if (H_STD_OUTPUT == INVALID_HANDLE_VALUE) {
        return EXIT_SYSTEM_ERROR_NOT_LOGGED;
    }

    const HANDLE H_STD_INPUT = GetStdHandle(STD_INPUT_HANDLE);
    if (H_STD_INPUT == INVALID_HANDLE_VALUE) {
        if (clear_console_buffer(H_STD_OUTPUT, &length_of_written_data) == 0) {
            if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
                return EXIT_SYSTEM_ERROR_NOT_LOGGED;
            } else {
                getchar();
                return EXIT_SYSTEM_ERROR_LOGGED;
            }
        }

        if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
            return EXIT_SYSTEM_ERROR_NOT_LOGGED;
        } else {
            getchar();
            return EXIT_SYSTEM_ERROR_LOGGED;
        }
    }

    const char *FILE_PATH = argv[1];
    const DWORD FILE_SIZE = (DWORD)atoi(argv[2]);

    const HANDLE H_FILE = CreateFile(FILE_PATH, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL, NULL);
    if (H_FILE == INVALID_HANDLE_VALUE) {
        if (clear_console_buffer(H_STD_OUTPUT, &length_of_written_data) == 0) {
            if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
                return EXIT_SYSTEM_ERROR_NOT_LOGGED;
            } else {
                getchar();
                return EXIT_SYSTEM_ERROR_LOGGED;
            }
        }

        if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
            return EXIT_SYSTEM_ERROR_NOT_LOGGED;
        } else {
            getchar();
            return EXIT_SYSTEM_ERROR_LOGGED;
        }
    }

    OVERLAPPED overlapped;
    overlapped.Offset = 0;
    overlapped.OffsetHigh = 0;

    if (LockFileEx(H_FILE, LOCKFILE_EXCLUSIVE_LOCK, 0, FILE_SIZE, 0, &overlapped) == 0) {
        if (clear_console_buffer(H_STD_OUTPUT, &length_of_written_data) == 0) {
            if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
                return EXIT_SYSTEM_ERROR_NOT_LOGGED;
            } else {
                getchar();
                return EXIT_SYSTEM_ERROR_LOGGED;
            }
        }

        if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
            return EXIT_SYSTEM_ERROR_NOT_LOGGED;
        } else {
            getchar();
            return EXIT_SYSTEM_ERROR_LOGGED;
        }
    }

    char *buffer = (char *)calloc(FILE_SIZE + 1, sizeof(char));

    if (ReadFile(H_FILE, buffer, FILE_SIZE, &length_of_read_data, NULL) == 0) {
        if (clear_console_buffer(H_STD_OUTPUT, &length_of_written_data) == 0) {
            if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
                return EXIT_SYSTEM_ERROR_NOT_LOGGED;
            } else {
                getchar();
                return EXIT_SYSTEM_ERROR_LOGGED;
            }
        }

        if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
            free(buffer);
            return EXIT_SYSTEM_ERROR_NOT_LOGGED;
        } else {
            getchar();
            free(buffer);
            return EXIT_SYSTEM_ERROR_LOGGED;
        }
    }

    char *correct_data = get_first_n_bytes_of_string(buffer, length_of_read_data);
    free(buffer);

    if (clear_console_buffer(H_STD_OUTPUT, &length_of_written_data) == 0) {
        if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
            return EXIT_SYSTEM_ERROR_NOT_LOGGED;
        } else {
            getchar();
            return EXIT_SYSTEM_ERROR_LOGGED;
        }
    }

    if (write_to_center_of_console_with_auto_length_counting(H_STD_OUTPUT, correct_data, &length_of_written_data,
                                                             FOREGROUND_GREEN) == 0) {
        if (clear_console_buffer(H_STD_OUTPUT, &length_of_written_data) == 0) {
            if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
                return EXIT_SYSTEM_ERROR_NOT_LOGGED;
            } else {
                getchar();
                return EXIT_SYSTEM_ERROR_LOGGED;
            }
        }

        if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
            free(correct_data);
            return EXIT_SYSTEM_ERROR_NOT_LOGGED;
        } else {
            getchar();
            free(correct_data);
            return EXIT_SYSTEM_ERROR_LOGGED;
        }
    }

    Sleep(7000);
    free(correct_data);

    if (UnlockFileEx(H_FILE, 0, FILE_SIZE, 0, &overlapped) == 0) {
        if (clear_console_buffer(H_STD_OUTPUT, &length_of_written_data) == 0) {
            if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
                return EXIT_SYSTEM_ERROR_NOT_LOGGED;
            } else {
                getchar();
                return EXIT_SYSTEM_ERROR_LOGGED;
            }
        }

        if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
            free(correct_data);
            return EXIT_SYSTEM_ERROR_NOT_LOGGED;
        } else {
            getchar();
            free(correct_data);
            return EXIT_SYSTEM_ERROR_LOGGED;
        }
    }

    if (CloseHandle(H_FILE) == 0) {
        if (clear_console_buffer(H_STD_OUTPUT, &length_of_written_data) == 0) {
            if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
                return EXIT_SYSTEM_ERROR_NOT_LOGGED;
            } else {
                getchar();
                return EXIT_SYSTEM_ERROR_LOGGED;
            }
        }

        if (send_last_error_message_to_some_handle(H_STD_OUTPUT, &length_of_written_data) == 0) {
            free(correct_data);
            return EXIT_SYSTEM_ERROR_NOT_LOGGED;
        } else {
            getchar();
            free(correct_data);
            return EXIT_SYSTEM_ERROR_LOGGED;
        }
    }

    return EXIT_SUCCESS;
}
