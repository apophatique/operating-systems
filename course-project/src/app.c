#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#define WINDOW_X 100
#define WINDOW_Y 50
#define WINDOW_WIDTH 1030
#define WINDOW_HEIGHT 800
#define WINDOW_BORDER_WIDTH 3
#define NUMBER_OF_PASSENGER_SEATS 4

/**
 * Extended random numbers generation function
 */
long get_rand(const long min, const long max) {
    return lrand48() % (max + 1 - min) + (min);
}

int get_passenger_requested_city(int current_city) {
    int rand_city_num = current_city;

    while (rand_city_num == current_city) {
        rand_city_num = get_rand(0, 3);
    }

    return rand_city_num;
}

char *get_city_name_by_his_number(const int number) {
    switch (number) {
        case 0:
            return "Moscow";
        case 1:
            return "Beijing";
        case 2:
            return "New-York";
        case 3:
            return "London";
        default:
            return NULL;
    }
}

char *get_city_string_number_by_int_number(const int number) {
    switch (number) {
        case 0:
            return "1";
        case 1:
            return "2";
        case 2:
            return "3";
        case 3:
            return "4";
        default:
            return NULL;
    }
}

ssize_t get_string_length(const char *buffer) {
    ssize_t length_of_data_to_write = 0;

    while (buffer[length_of_data_to_write] != 0) {
        length_of_data_to_write++;
    }

    return length_of_data_to_write;
}

typedef struct {
    int x;
    int y;
} coord_t;

typedef struct {
    int id;
    int current_city;
    int requested_city;
    int last_city;
    _Bool is_in_plane;
} passenger_t;

passenger_t *passengers;

typedef struct {
    coord_t coord;
    int busy_places_counter;
    int current_city;
    int passengers_id[4];
    passenger_t *passengers[4];
} plane_t;

plane_t plane;

Display *display;
int screen;
Window window;
GC gc;
XGCValues values;

pthread_mutex_t plane_mutexes[4];

void initialize_window() {
    XInitThreads();
    display = XOpenDisplay(NULL);

    if (display == NULL) {
        printf("Error XOpenDisplay\n");
        exit(1);
    }

    screen = XDefaultScreen(display);

    window = XCreateSimpleWindow(
            display,
            RootWindow(display, screen),
            WINDOW_X,
            WINDOW_Y,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            WINDOW_BORDER_WIDTH,
            BlackPixel(display, screen),
            BlackPixel(display, screen)
    );

    if (window == 0) {
        printf("Error XCreateSimpleWindow\n");
        exit(1);
    }

    XMapWindow(display, window);
    gc = XDefaultGC(display, screen);
}

void *plane_thread_routine() {
    sleep(1);
    while (true) {
        if (plane.current_city == 0) {
            plane.current_city = -1;

            while (plane.coord.y > 90) {
                plane.coord.y--;
                usleep(2000);
            }

            plane.current_city = 1;
            sleep(2);
        }

        if (plane.current_city == 1) {
            plane.current_city = -1;

            while (plane.coord.x > 85) {
                plane.coord.x--;
                usleep(2000);
            }

            plane.current_city = 2;
            sleep(2);
        }

        if (plane.current_city == 2) {
            plane.current_city = -1;

            while (plane.coord.y < 570) {
                plane.coord.y++;
                usleep(2000);
            }

            plane.current_city = 3;
            sleep(2);
        }

        if (plane.current_city == 3) {
            plane.current_city = -1;

            while (plane.coord.x < 840) {
                plane.coord.x++;
                usleep(2000);
            }

            plane.current_city = 0;
            sleep(2);
        }
    }
}

void passengers_thread_routine(void *data) {
    int passenger_seat_id = -1;
    passenger_t *passenger = (passenger_t *) data;

    while (true) {
        if (
                plane.current_city == passenger->current_city &&
                plane.busy_places_counter < NUMBER_OF_PASSENGER_SEATS &&
                passenger->is_in_plane == false
        ) {
            for (int i = 0; i < 4; i++) {
                if (plane.passengers[i] == NULL) {

                    if (pthread_mutex_trylock(&plane_mutexes[i]) != 0) {
                        continue;
                    }
                    printf("Seat is busy: %d\n", i);
                    plane.passengers[i] = passenger;
                    plane.passengers_id[i] = passenger->id;
                    passenger_seat_id = i;

                    printf("Passenger id: %d, request: %d from city %d occurred in plane\n", passenger->id, passenger->requested_city, passenger->current_city);
                    passenger->is_in_plane = true;
                    passenger->last_city = passenger->current_city;
                    passenger->current_city = -2;
                    plane.busy_places_counter+=1;

                    break;
                }
            }
        }

        if (plane.current_city == passenger->requested_city && passenger->is_in_plane == true) {

            plane.busy_places_counter-=1;
            plane.passengers[passenger_seat_id] = NULL;
            plane.passengers_id[passenger->id] = -1;
            passenger_seat_id = -1;

            passenger->current_city = passenger->requested_city;
            passenger->is_in_plane = false;
            passenger->requested_city = get_passenger_requested_city(passenger->current_city);

            printf("Passenger id: %d, request: %d occurred in city %d\n", passenger->id, passenger->requested_city, passenger->current_city);
            pthread_mutex_unlock(&plane_mutexes[passenger_seat_id]);
            sleep(get_rand(3, 4));
        }
    }
}

int get_passengers_count() {
    system("clear");
    printf("Enter the passengers count: ");

    int passengersCount;
    scanf("%d", &passengersCount);

    return passengersCount;
}

void render_background() {
    values.foreground = 0xFFFFFF;
    XChangeGC(display, gc, GCForeground, &values);
    XFillRectangle(display, window, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void render_cities() {
    values.foreground = 0x000000;
    XChangeGC(display, gc, GCForeground, &values);
    XFillArc(display, window, gc, 50, 50, 140, 140, 360 * 64, 360 * 64);
    values.foreground = 0xffffff;
    XChangeGC(display, gc, GCForeground, &values);
    XDrawString(display, window, gc, 99, 120, "New-York", 8);

    values.foreground = 0x000000;
    XChangeGC(display, gc, GCForeground, &values);
    XFillArc(display, window, gc, 50, 550, 140, 140, 360 * 64, 360 * 64);
    values.foreground = 0xffffff;
    XChangeGC(display, gc, GCForeground, &values);
    XDrawString(display, window, gc, 102, 610, "London", 6);

    values.foreground = 0x000000;
    XChangeGC(display, gc, GCForeground, &values);
    XFillArc(display, window, gc, 800, 550, 140, 140, 360 * 64, 360 * 64);
    values.foreground = 0xffffff;
    XChangeGC(display, gc, GCForeground, &values);
    XDrawString(display, window, gc, 852, 610, "Moscow", 6);

    values.foreground = 0x000000;
    XChangeGC(display, gc, GCForeground, &values);
    XFillArc(display, window, gc, 800, 50, 140, 140, 360 * 64, 360 * 64);
    values.foreground = 0xffffff;
    XChangeGC(display, gc, GCForeground, &values);
    XDrawString(display, window, gc, 855, 115, "Beijing", 7);

    values.foreground = 0x000000;
    XChangeGC(display, gc, GCForeground, &values);
    XDrawLine(display, window, gc, 870, 115, 870, 590);
    XDrawLine(display, window, gc, 870, 610, 50, 610);
    XDrawLine(display, window, gc, 120, 590, 120, 80);
    XDrawLine(display, window, gc, 120, 120, 840, 120);
}

void render_information_table() {
    values.foreground = 0x141924;
    XChangeGC(display, gc, GCForeground, &values);

    XDrawLine(display, window, gc, 370, 220, 620, 220);
    XDrawLine(display, window, gc, 370, 235, 620, 235);
    XDrawLine(display, window, gc, 370, 250, 620, 250);
    XDrawLine(display, window, gc, 370, 265, 620, 265);
    XDrawLine(display, window, gc, 370, 280, 620, 280);

    XDrawLine(display, window, gc, 620, 220, 620, 280);
    XDrawLine(display, window, gc, 520, 220, 520, 280);
    XDrawLine(display, window, gc, 420, 220, 420, 280);
    XDrawLine(display, window, gc, 370, 220, 370, 280);

    XDrawString(display, window, gc, 370, 215, "Passenger", 9);
    XDrawString(display, window, gc, 462, 215, "From", 4);
    XDrawString(display, window, gc, 560, 215, "To", 2);

    values.foreground = 0x000000;
    XChangeGC(display, gc, GCForeground, &values);
    /*
    for (int i = 0; i < plane.busy_places_counter; i++) {
            XDrawString(
                    display,
                    window,
                    gc,
                    372,
                    230 + i * 15,
                    get_city_string_number_by_int_number(plane.passengers[i]->id),
                    1
            );

            XDrawString(
                    display,
                    window,
                    gc,
                    462,
                    230 + i * 15,
                    get_city_name_by_his_number(plane.passengers[i]->last_city),
                    get_string_length(get_city_name_by_his_number(plane.passengers[i]->last_city))
                    );

            XDrawString(
                    display,
                    window,
                    gc,
                    560,
                    230 + i * 15,
                    get_city_name_by_his_number(plane.passengers[i]->requested_city),
                    get_string_length(get_city_name_by_his_number(plane.passengers[i]->requested_city))
                    );
        }
        */
}

void render_plane() {
    values.foreground = 0x99FBFF;
    XChangeGC(display, gc, GCForeground, &values);
    XFillArc(display, window, gc, plane.coord.x, plane.coord.y, 70, 70, 360 * 64, 360 * 64);
}

void iteration_render() {
    XClearWindow(display, window);

    render_background();
    render_cities();
    render_plane();
    render_information_table();

    XFlush(display);
    usleep(20000);
}

void run_passengers_thread(int passengers_count) {
    pthread_t *passenger_threads = (pthread_t *) malloc(passengers_count * sizeof(pthread_t));
    passengers = (passenger_t *) malloc(passengers_count * sizeof(passenger_t));

    for (int i = 0; i < passengers_count; i++) {
        passengers[i].id = i;
        passengers[i].is_in_plane = false;
        passengers[i].current_city = 0;
        passengers[i].last_city = -1;
        passengers[i].requested_city = get_passenger_requested_city(0);
        printf("start request: %d\n", passengers[i].requested_city);

        pthread_create(&(passenger_threads[i]), NULL, (void *) passengers_thread_routine, &passengers[i]);
    }
}

void run_plane_thread() {
    plane.coord.x = 836;
    plane.coord.y = 580;
    plane.busy_places_counter = 0;
    plane.current_city = 0;

    for (int i = 0; i < 4; i++) {
        plane.passengers[i] = NULL;
        plane.passengers_id[i] = -1;
    }

    pthread_t plane_thread;
    pthread_create(&plane_thread, NULL, plane_thread_routine, NULL);
}

void start(int passengersCount) {
    for (int i = 0; i < 4; i++) {
        pthread_mutex_init(&plane_mutexes[i], NULL);
    }
    srand48((int64_t) time(NULL));
    initialize_window();
    run_passengers_thread(passengersCount);
    run_plane_thread();

    while (true) {
        iteration_render();
    }
}

int main() {
    start(2);
    return EXIT_SUCCESS;
}
