#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "../include/util.h"

int count_string_length(const char *string) {
    int count = 0;
    while (string[++count] != '\0');
    return count;
}

int main() {
    Display *display = XOpenDisplay(NULL);
    const int screen = XDefaultScreen(display);

    const Window window = XCreateSimpleWindow(
            display,
            RootWindow(display, screen),
            100, 50, 700, 500, 3,
            BlackPixel(display, screen),
            WhitePixel(display, screen)
    );

    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask);
    XStoreName(display, window, "Laboratory work 6");
    XMapWindow(display, window);

    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    GC gc = XDefaultGC(display, screen);
    XEvent event;   
    Bool looping = True;

    while (looping) {
        XNextEvent(display, &event);

        switch (event.type) {
            case KeyPress: {
                char *message = (char *)calloc(50, sizeof(char));
                sprintf(message, "Keyboard key press (%s)", XKeysymToString(XLookupKeysym(&event.xkey, 0)));
                printf("%s\n", message);
                XClearWindow(display, window);
                XDrawString(display, window, gc, 50, 50, message, count_string_length(message));
                free(message);
                break;
            }

            case ButtonPress: {
                if (event.xbutton.button >= 1 && event.xbutton.button <= 3) {
                    char *message = (char *)calloc(50, sizeof(char));

                    sprintf(
                            message,
                            "Mouse button click (x: %d, y: %d, type: %s)",
                            event.xbutton.x,
                            event.xbutton.y,

                            event.xbutton.button == 1 ? "left button" : (
                                    event.xbutton.button == 2 ? "middle button" : "right button"
                            )
                    );

                    printf("%s\n", message);
                    XClearWindow(display, window);

                    XDrawString(
                            display,
                            window,
                            gc,
                            event.xbutton.x,
                            event.xbutton.y,
                            message,
                            count_string_length(message)
                    );

                    free(message);
                }

                break;
            }

            case Expose: {
                char *message = (char *)calloc(50, sizeof(char));

                sprintf(
                        message,
                        "Change of window size (new size: %d px * %d px)",
                        event.xexpose.width,
                        event.xexpose.height
                );

                printf("%s\n", message);
                XClearWindow(display, window);
                XDrawString(display, window, gc, 50, 50, message, count_string_length(message));
                free(message);
                break;
            }

            case ClientMessage:
                if (event.xclient.data.l[0] == wmDeleteMessage) {
                    printf("Exit button click\n");
                    looping = False;
                }

                break;

            default: {
                printf("Unknown event (XEvent.type = %d)\n", event.type);
                break;
            }
        }
    }

    printf("Exiting...\n");
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return EXIT_SUCCESS;
}
