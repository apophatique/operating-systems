#include <windows.h>
#include <stdio.h>

BOOL clear_window(HWND hwnd) {
    RECT rect;

    if (GetClientRect(hwnd, &rect) == 0) {
        return 0;
    }

    HDC hdc = GetDC(hwnd);

    if (hdc == 0) {
        return 0;
    }

    return FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
}

char *get_key_representation_by_code(const WPARAM key_code) {
    switch (key_code) {
        case VK_BACK:
            return ("BACKSPACE");

        case VK_TAB:
            return ("TAB");

        case VK_RETURN:
            return ("ENTER");

        case VK_SHIFT:
            return ("SHIFT");

        case VK_CONTROL:
            return ("CTRL");

        case VK_MENU:
            return ("ALT");

        case VK_CAPITAL:
            return ("CAPS LOCK");

        case VK_ESCAPE:
            return ("ESCAPE");

        case VK_SPACE:
            return ("SPACE");

        case VK_PRIOR:
            return ("PAGE UP");

        case VK_NEXT:
            return ("PAGE DOWN");

        case VK_END:
            return ("END");

        case VK_HOME:
            return ("HOME");

        case VK_LEFT:
            return ("LEFT ARROW");

        case VK_RIGHT:
            return ("RIGHT ARROW");

        case VK_UP:
            return ("UP ARROW");

        case VK_DOWN:
            return ("DOWN ARROW");

        case VK_DELETE:
            return ("DELETE");

        default: {
            char *buffer = (char *)calloc(3, sizeof(char));

            if (key_code >= 0x30 && key_code <= 0x39) {
                sprintf(buffer, "%d", key_code - 48);
            } else if (key_code >= 0x41 && key_code <= 0x5A) {
                sprintf(buffer, "%c", (char)key_code);
            } else if (key_code >= 0x70 && key_code <= 0x7B) {
                sprintf(buffer, "F%d", key_code - 0x6F);
            } else {
                free(buffer);
                return "unknown virtual key code";
            }

            return buffer;
        }
    }
}

LRESULT WINAPI win_proc(HWND hwnd, const UINT tmsg, const WPARAM w_param, const LPARAM l_param) {
    switch (tmsg) {
        case WM_SIZE: {
            clear_window(hwnd);
            HDC hdc = GetDC(hwnd);
            char *msg_buffer = (char *)calloc(39, sizeof(char));
            sprintf(msg_buffer, "Changed window size (w: %d, h: %d)", LOWORD(l_param), HIWORD(l_param));
            RECT window_rect;
            GetWindowRect(hwnd, &window_rect);

            TextOut(
                    hdc,
                    (window_rect.right - window_rect.left) / 2 - 125,
                    (window_rect.bottom - window_rect.top) / 2 - 28,
                    msg_buffer,
                    39
            );

            ReleaseDC(hwnd, hdc);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            clear_window(hwnd);
            HDC hdc = GetDC(hwnd);
            char *msg_buffer = (char *)calloc(45, sizeof(char));
            sprintf(msg_buffer, "Pressed left mouse button (x: %d, y: %d)", LOWORD(l_param), HIWORD(l_param));
            TextOut(hdc, LOWORD(l_param), HIWORD(l_param), msg_buffer, 45);
            ReleaseDC(hwnd, hdc);
            return 0;
        }

        case WM_RBUTTONDOWN: {
            clear_window(hwnd);
            HDC hdc = GetDC(hwnd);
            char *msg_buffer = (char *)calloc(46, sizeof(char));
            sprintf(msg_buffer, "Pressed right mouse button (x: %d, y: %d)", LOWORD(l_param), HIWORD(l_param));
            TextOut(hdc, LOWORD(l_param), HIWORD(l_param), msg_buffer, 46);
            ReleaseDC(hwnd, hdc);
            return 0;
        }

        case WM_KEYDOWN: {
            clear_window(hwnd);
            HDC hdc = GetDC(hwnd);
            char *msg_buffer = (char *)calloc(31, sizeof(char));
            sprintf(msg_buffer, "Pressed key (%s)", get_key_representation_by_code(w_param));
            RECT window_rect;
            GetWindowRect(hwnd, &window_rect);

            TextOut(
                    hdc,
                    (window_rect.right - window_rect.left) / 2 - 100,
                    (window_rect.bottom - window_rect.top) / 2 - 28,
                    msg_buffer,
                    31
            );

            ReleaseDC(hwnd, hdc);
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        default:
            return DefWindowProc(hwnd, tmsg, w_param, l_param);
    }
}

int main() {
    HINSTANCE h_instance;
    STARTUPINFO startup_info;
    int n_cmd_show;
    HWND hwnd;
    MSG msg;
    WNDCLASS wnd_class;

    GetStartupInfo(&startup_info);

    if ((startup_info.dwFlags & STARTF_USESHOWWINDOW) == 1) {
        n_cmd_show = startup_info.wShowWindow;
    } else {
        n_cmd_show = SW_SHOWDEFAULT;
    }

    h_instance = GetModuleHandle(0);
    
    if (h_instance == 0) {
        return EXIT_FAILURE;
    }
    
    memset(&wnd_class, 0, sizeof(wnd_class));
    wnd_class.lpszClassName = "Window";
    wnd_class.lpfnWndProc = win_proc;
    wnd_class.hCursor = LoadCursor(0, IDC_ARROW);

    if (wnd_class.hCursor == 0) {
        return EXIT_FAILURE;
    }

    wnd_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wnd_class.hInstance = h_instance;

    if (RegisterClass(&wnd_class) == 0) {
        return EXIT_FAILURE;
    }

    hwnd = CreateWindow(
            "Window",
            "Labwork5",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            h_instance,
            0
    );

    if (hwnd == 0) {
        return EXIT_FAILURE;
    }

    ShowWindow(hwnd, n_cmd_show);

    BOOL getting_message_result;

    while ((getting_message_result = GetMessage(&msg, 0, 0, 0 )) != 0) {
        if (getting_message_result == -1) {
            return EXIT_FAILURE;
        } else {
            DispatchMessage(&msg);
        }
    }

    return EXIT_SUCCESS;
}
