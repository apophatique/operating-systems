#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <time.h>

HANDLE std_in,
       std_out,
       std_err;

DWORD length_of_read_data,
      length_of_written_data,
      old_con_mode;

_Bool should_listen_to_events = 1;

BOOL fill_handle_vars() {
    std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    std_in = GetStdHandle(STD_INPUT_HANDLE);
    std_err = GetStdHandle(STD_ERROR_HANDLE);

    if (
            std_out == INVALID_HANDLE_VALUE ||
            std_in == INVALID_HANDLE_VALUE ||
            std_err == INVALID_HANDLE_VALUE
    ) {
        return 0;
    }

    return 1;
}

int random_number(const int lower_bound, const int upper_bound) {
    int result = 0,
        l_num = 0,
        h_num = 0;

    if (lower_bound < upper_bound) {
        l_num = lower_bound;
        h_num = upper_bound + 1;
    } else {
        l_num = upper_bound + 1;
        h_num = lower_bound;
    }

    srand((unsigned int)time(0));
    result = (rand() % (h_num - l_num)) + l_num;
    return result;
}

char *get_first_n_bytes_of_string(const char *string, const size_t n) {
    if (n < 0 || strlen(string) < n) {
        return 0;
    }

    char *buffer = (char *)calloc(n + 1, sizeof(char));

    for (size_t i = 0; i < n; i++) {
        buffer[i] = string[i];
    }

    buffer[n] = 0;
    return buffer;
}

VOID handle_key_event(const KEY_EVENT_RECORD key_event_record) {
    printf("Key event: ");
    key_event_record.bKeyDown == 1 ? printf("key pressed (") : printf("key released (");

    switch (key_event_record.wVirtualKeyCode) {
        case VK_BACK:
            printf("BACKSPACE");
            break;

        case VK_TAB:
            printf("TAB");
            break;

        case VK_RETURN:
            printf("ENTER");
            break;

        case VK_SHIFT:
            printf("SHIFT");
            break;

        case VK_CONTROL:
            printf("CTRL");
            break;

        case VK_MENU:
            printf("ALT");
            break;

        case VK_CAPITAL:
            printf("CAPS LOCK");
            break;

        case VK_ESCAPE:
            printf("ESCAPE");
            break;

        case VK_SPACE:
            printf("SPACE");
            break;

        case VK_PRIOR:
            printf("PAGE UP");
            break;

        case VK_NEXT:
            printf("PAGE DOWN");
            break;

        case VK_END:
            printf("END");
            break;

        case VK_HOME:
            printf("HOME");
            break;

        case VK_LEFT:
            printf("LEFT ARROW");
            break;

        case VK_RIGHT:
            printf("RIGHT ARROW");
            break;

        case VK_UP:
            printf("UP ARROW");
            break;

        case VK_DOWN:
            printf("DOWN ARROW");
            break;

        case VK_DELETE:
            printf("DELETE");
            break;

        default: {
            if (key_event_record.wVirtualKeyCode >= 0x30 && key_event_record.wVirtualKeyCode <= 0x39) {
                printf("%d", key_event_record.wVirtualKeyCode - 48);
            } else if (key_event_record.wVirtualKeyCode >= 0x41 && key_event_record.wVirtualKeyCode <= 0x5A) {
                printf("%c", (char)key_event_record.wVirtualKeyCode);
            } else if (key_event_record.wVirtualKeyCode >= 0x70 && key_event_record.wVirtualKeyCode <= 0x7B) {
                printf("F%d", key_event_record.wVirtualKeyCode - 0x6F);
            } else {
                printf("unknown virtual key code");
            }
        }
    }

    printf(")\n");
}

BOOL write_char_with_attr(const char *_char, const WORD attr, const COORD pos) {
    if (WriteConsoleOutputAttribute(std_out, &attr, 1, pos, &length_of_written_data) == 0) {
        return 0;
    }

    return WriteConsoleOutputCharacter(std_out, _char, 1, pos, &length_of_written_data);
}

BOOL read_and_transform_char_from_console(char *buffer, COORD pos) {
    char read_character;
    DWORD read_chars_count;

    if (ReadConsoleOutputCharacter(std_out, &read_character, 1, pos, &read_chars_count) == 0) {
        return 0;
    }

    if (read_character >= 65 && read_character <= 90) {
        read_character += 32;
    } else if (read_character >= 97 && read_character <= 122) {
        read_character -= 32;
    }

    buffer[0] = read_character;
    return 1;
}

WORD generate_random_color() {
    int choice_helper = random_number(0, 5);

    if (choice_helper == 0) {
        return FOREGROUND_RED;
    } else if (choice_helper == 1) {
        return FOREGROUND_GREEN;
    } else if (choice_helper == 2) {
        return FOREGROUND_BLUE;
    } else if (choice_helper == 3) {
        return FOREGROUND_RED | FOREGROUND_GREEN;
    } else if (choice_helper == 4) {
        return FOREGROUND_RED | FOREGROUND_BLUE;
    } else if (choice_helper == 5) {
        return FOREGROUND_BLUE | FOREGROUND_GREEN;
    }

    return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
}

BOOL handle_mouse_event(const MOUSE_EVENT_RECORD mer) {
    const COORD mouse_pos = mer.dwMousePosition;
    char *str_mouse_pos = (char *)calloc(11, sizeof(char));
    sprintf(str_mouse_pos, "x: %d, y: %d", mouse_pos.X, mouse_pos.Y);

    char *character;
    switch (mer.dwEventFlags) {
        case 0:
            switch (mer.dwButtonState) {
                case FROM_LEFT_1ST_BUTTON_PRESSED:
                    character = (char *)calloc(1, sizeof(char));

                    if (read_and_transform_char_from_console(character, mouse_pos) == 0) {
                        return 0;
                    }

                    if (write_char_with_attr(
                            character,
                            generate_random_color(),
                            mouse_pos
                    ) == 0) {
                        return 0;
                    }

                    free(character);
                    break;

                case RIGHTMOST_BUTTON_PRESSED:
                    printf("Mouse event: right button press (%s)\n", str_mouse_pos);
                    should_listen_to_events = 0;
                    break;

                default:
                    break;
            }

            break;

        case DOUBLE_CLICK:
            if (SetConsoleCursorPosition(std_out, mouse_pos) == 0) {
                return 0;
            }

            printf("Mouse event: double click (%s)\n", str_mouse_pos);
            break;

        case MOUSE_MOVED:
            printf("Mouse event: mouse moved (%s)\n", str_mouse_pos);
            break;

        case MOUSE_WHEELED:
            printf("Mouse event: mouse wheeled (%s)\n", str_mouse_pos);
            break;

        default:
            printf("Mouse event: unknown mouse event (%s)\n", str_mouse_pos);
            break;
    }

    return 1;
}

int main(const int argc, const char *argv[]) {
    if (fill_handle_vars() == 0) {
        return EXIT_FAILURE;
    }

    if (argc != 3) {
        fprintf(std_err, "incorrect number of args (expected: 3)");
        getchar();
        return EXIT_FAILURE;
    }

    const char *FILE_PATH = argv[1];
    const DWORD FILE_SIZE = (DWORD)strtol(argv[2], 0, 10);

    HANDLE H_FILE = CreateFile(
            FILE_PATH,
            GENERIC_READ,
            FILE_SHARE_READ,
            0,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0
    );

    if (H_FILE == INVALID_HANDLE_VALUE) {
        fprintf(std_err, "unable to get file handle");
        getchar();
        return EXIT_FAILURE;
    }

    char *buffer = (char *)calloc(FILE_SIZE + 1, sizeof(char));
    if (ReadFile(H_FILE, buffer, FILE_SIZE, &length_of_read_data, 0) == 0) {
        fprintf(std_err, "unable to read from file handle");
        free(buffer);
        getchar();
        return EXIT_FAILURE;
    }

    char *correct_data = get_first_n_bytes_of_string(buffer, length_of_read_data);
    free(buffer);
    printf("%s", correct_data);
    printf("\n\n");
    free(correct_data);

    if (GetConsoleMode(std_in, &old_con_mode) == 0) {
        fprintf(std_err, "unable to get console mode");
        getchar();
        return EXIT_FAILURE;
    }

    if (SetConsoleMode(std_in, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT) == 0) {
        fprintf(std_err, "unable to set console mode");
        getchar();
        return EXIT_FAILURE;
    }

    DWORD read_counter = 0;

    while (should_listen_to_events == 1) {
        INPUT_RECORD input_record;

        if (ReadConsoleInput(std_in, &input_record, 1, &read_counter) == 0) {
            fprintf(std_err, "unable to read console input");
            getchar();
            return EXIT_FAILURE;
        }

        switch (input_record.EventType) {
            case KEY_EVENT:
                handle_key_event(input_record.Event.KeyEvent);
                break;

            case MOUSE_EVENT:
                if (handle_mouse_event(input_record.Event.MouseEvent) == 0) {
                    fprintf(std_err, "unable to handle mouse event");
                    return EXIT_FAILURE;
                }

                break;

            case FOCUS_EVENT:
                printf("Focus event: ");
                input_record.Event.FocusEvent.bSetFocus == 1 ? printf("focus is set\n") : printf("focus is unset\n");
                break;

            default:
                printf("unknown event type");
                break;
        }
    }

    if (SetConsoleMode(std_in, old_con_mode) == 0) {
        fprintf(std_err, "unable to set console mode");
        getchar();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
