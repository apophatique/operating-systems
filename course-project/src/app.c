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
            return "undefined";
    }
}

ssize_t get_string_length(char *buffer) {
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
int passengers_count_in_cities[4];
pthread_mutex_t plane_mutexes[4];

typedef struct {
    coord_t coord;
    int busy_places_counter;
    int current_city;
    int places[4];
    passenger_t *passengers[4];
} plane_t;
plane_t plane;

Display *display;
int screen;
Window window;
GC gc;
XGCValues values;

pthread_mutex_t plane_mutex;

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
    while (true) {
        sleep(3);

        if (plane.current_city == 0) {
            plane.current_city = -1;

            while (plane.coord.y > 90) {
                plane.coord.y--;
                usleep(2000);
            }

            plane.current_city = 1;
        } else if (plane.current_city == 1) {
            plane.current_city = -1;

            while (plane.coord.x > 85) {
                plane.coord.x--;
                usleep(2000);
            }

            plane.current_city = 2;
        } else if (plane.current_city == 2) {
            plane.current_city = -1;

            while (plane.coord.y < 570) {
                plane.coord.y++;
                usleep(2000);
            }

            plane.current_city = 3;
        } else if (plane.current_city == 3) {
            plane.current_city = -1;

            while (plane.coord.x < 840) {
                plane.coord.x++;
                usleep(2000);
            }

            plane.current_city = 0;
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
            pthread_mutex_lock(&plane_mutex);

            for (int i = 0; i < 4; i++) {
                if (plane.passengers[i] == NULL) {
                    if (pthread_mutex_trylock(&plane_mutexes[i]) != 0) {
                        continue;
                    }

                    printf("Seat is busy: %d\n", i);
                    plane.passengers[i] = passenger;
                    plane.places[i] = 1;
                    passenger_seat_id = i;

                    printf("Passenger id: %d, request: %d from city %d occurred in plane\n", passenger->id, passenger->requested_city, passenger->current_city);
                    passenger->is_in_plane = true;
                    passenger->last_city = passenger->current_city;
                    passengers_count_in_cities[passenger->current_city]--;
                    passenger->current_city = -2;
                    plane.busy_places_counter += 1;

                    break;
                }
            }
            pthread_mutex_unlock(&plane_mutex);
        }

        if (plane.current_city == passenger->requested_city && passenger->is_in_plane == true) {
            pthread_mutex_lock(&plane_mutex);

            plane.busy_places_counter -= 1;
            plane.passengers[passenger_seat_id] = NULL;
            plane.places[passenger_seat_id] = -1;

            passenger->current_city = passenger->requested_city;
            passengers_count_in_cities[passenger->current_city]++;
            passenger->is_in_plane = false;
            passenger->requested_city = get_passenger_requested_city(passenger->current_city);

            printf("Passenger id: %d, request: %d occurred in city %d\n", passenger->id, passenger->requested_city, passenger->current_city);

            pthread_mutex_unlock(&plane_mutexes[passenger_seat_id]);
            passenger_seat_id = -1;
            pthread_mutex_unlock(&plane_mutex);
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

int count_num_of_digits_in_positive_int(int positive_int) {
    if (positive_int < 0) {
        return 0;
    }

    int num_of_digits = 0;

    do {
        num_of_digits++;
        positive_int /= 10;
    } while (positive_int > 0);

    return num_of_digits;
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

    for (int i = 0; i < 4; i++) {
        if (plane.places[i] != -1) {
            char *buf = (char *)calloc(count_num_of_digits_in_positive_int(plane.passengers[i]->id + 1) + 1, sizeof(char));
            sprintf(buf, "%d", plane.passengers[i]->id + 1);

            XDrawString(
                    display,
                    window,
                    gc,
                    372,
                    230 + i * 15,
                    buf,
                    get_string_length(buf)
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
    }
}

void render_plane() {
    values.foreground = 0x99FBFF;
    XChangeGC(display, gc, GCForeground, &values);
    XFillArc(display, window, gc, plane.coord.x, plane.coord.y, 70, 70, 360 * 64, 360 * 64);
}

void render_passengers_count_in_cities() {
    int digits_count_of_moscow_passengers_number = passengers_count_in_cities[0] > 9 ? 2 : 1;
    int digits_count_of_beijing_passengers_number = passengers_count_in_cities[1] > 9 ? 2 : 1;
    int digits_count_of_new_york_passengers_number = passengers_count_in_cities[2] > 9 ? 2 : 1;
    int digits_count_of_london_passengers_number = passengers_count_in_cities[3] > 9 ? 2 : 1;

    char moscow_passengers_message[] = "Passengers count in Moscow: ";
    char beijing_passengers_message[] = "Passengers count in Beijing: ";
    char new_york_passengers_message[] = "Passengers count in New-York: ";
    char london_passengers_message[] = "Passengers count in London: ";

    char *moscow_passengers_count_buffer = (char *) calloc(((digits_count_of_moscow_passengers_number) + 1), sizeof(char));
    char *beijing_passengers_count_buffer = (char *) calloc(((digits_count_of_beijing_passengers_number) + 1), sizeof(char));
    char *new_york_passengers_count_buffer = (char *) calloc(((digits_count_of_new_york_passengers_number) + 1), sizeof(char));
    char *london_passengers_count_buffer = (char *) calloc(((digits_count_of_london_passengers_number) + 1), sizeof(char));

    sprintf(moscow_passengers_count_buffer, "%d", passengers_count_in_cities[0]);
    sprintf(beijing_passengers_count_buffer, "%d", passengers_count_in_cities[1]);
    sprintf(new_york_passengers_count_buffer, "%d", passengers_count_in_cities[2]);
    sprintf(london_passengers_count_buffer, "%d", passengers_count_in_cities[3]);

    moscow_passengers_count_buffer[digits_count_of_moscow_passengers_number] = 0;
    new_york_passengers_count_buffer[digits_count_of_new_york_passengers_number] = 0;
    beijing_passengers_count_buffer[digits_count_of_beijing_passengers_number] = 0;
    london_passengers_count_buffer[digits_count_of_london_passengers_number] = 0;

    coord_t moscow_message_coord;
    moscow_message_coord.x = 610;
    moscow_message_coord.y = 565;

    coord_t beijing_message_coord;
    beijing_message_coord.x = 610;
    beijing_message_coord.y = 65;

    coord_t new_york_message_coord;
    new_york_message_coord.x = 210;
    new_york_message_coord.y = 65;

    coord_t london_message_coord;
    london_message_coord.x = 210;
    london_message_coord.y = 565;

    XDrawString(
            display,
            window,
            gc,
            moscow_message_coord.x,
            moscow_message_coord.y,
            moscow_passengers_message,
            get_string_length(moscow_passengers_message)
    );
    XDrawString(
            display,
            window,
            gc,
            beijing_message_coord.x,
            beijing_message_coord.y,
            beijing_passengers_message,
            get_string_length(beijing_passengers_message)
    );
    XDrawString(
            display,
            window,
            gc,
            new_york_message_coord.x,
            new_york_message_coord.y,
            new_york_passengers_message,
            get_string_length(new_york_passengers_message)
    );
    XDrawString(
            display,
            window,
            gc,
            london_message_coord.x,
            london_message_coord.y,
            london_passengers_message,
            get_string_length(london_passengers_message)
    );

    XDrawString(
            display,
            window,
            gc,
            moscow_message_coord.x + 170,
            moscow_message_coord.y,
            moscow_passengers_count_buffer,
            get_string_length(moscow_passengers_count_buffer)
    );
    XDrawString(
            display,
            window,
            gc,
            beijing_message_coord.x + 170,
            beijing_message_coord.y,
            beijing_passengers_count_buffer,
            get_string_length(beijing_passengers_count_buffer)
    );
    XDrawString(
            display,
            window,
            gc,
            new_york_message_coord.x + 180,
            new_york_message_coord.y,
            new_york_passengers_count_buffer,
            get_string_length(new_york_passengers_count_buffer)
    );
    XDrawString(
            display,
            window,
            gc,
            london_message_coord.x + 170,
            london_message_coord.y,
            london_passengers_count_buffer,
            get_string_length(london_passengers_count_buffer)
    );

    free(moscow_passengers_count_buffer);
    free(beijing_passengers_count_buffer);
    free(new_york_passengers_count_buffer);
    free(london_passengers_count_buffer);
}

void iteration_render() {
    XClearWindow(display, window);

    render_background();
    render_cities();
    render_plane();
    render_information_table();
    render_passengers_count_in_cities();

    XFlush(display);
    usleep(20000);
}

void run_passengers_thread(int passengers_count) {
    pthread_t *passenger_threads = (pthread_t *) calloc(passengers_count, sizeof(pthread_t));
    passengers = (passenger_t *) calloc(passengers_count, sizeof(passenger_t));

    for (int i = 0; i < passengers_count; i++) {
        passengers[i].id = i;
        passengers[i].is_in_plane = false;
        passengers[i].current_city = 0;
        passengers[i].last_city = 0;
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
        plane.places[i] = -1;
    }

    pthread_t plane_thread;
    pthread_create(&plane_thread, NULL, plane_thread_routine, NULL);
}

void start() {
    for (int i = 0; i < 4; i++) {
        pthread_mutex_init(&plane_mutexes[i], NULL);
    }
    pthread_mutex_init(&plane_mutex, NULL);

    srand48((int64_t) time(NULL));
    initialize_window();
    int passengersCount = get_passengers_count();
    passengers_count_in_cities[0] += passengersCount;
    run_passengers_thread(passengersCount);
    run_plane_thread();

    while (true) {
        iteration_render();
    }
}

int main() {
    start();
    return EXIT_SUCCESS;
}
