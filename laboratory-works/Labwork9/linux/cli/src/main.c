#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>


int n = 0;
char *data_buffer;
pthread_mutex_t reading_mutex, writing_mutex, n_mutex;

const char data_to_write[3][12] = {
        {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'},
        {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L'},
        {'+', '_', '!', '%', '^', '&', '*', '\\', '|', '@', '~', '$'}
};
short get_height_of_term() {
    struct winsize windowSize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize);
    return windowSize.ws_row;
}

long get_random(long min, long max) {
    return lrand48() % (max + 1 - min) + (min);
}
struct output_args {
    int row, column;
    char *data_to_output;
};

void output(const struct output_args args) {
    printf(
            "\033[%d;%dH%s\n",
            args.row,
            20 * ((int) args.column - 1),
            args.data_to_output
    );
}

void writer_func(void *arg) {
    if (arg == NULL) {
        return;
    }

    const intptr_t thread_id = (intptr_t) arg;
    int repeats = 0;

    while (1) {
        int i = 0;
        pthread_mutex_lock(&writing_mutex);

        while (i < 12) {
            data_buffer[i] = data_to_write[thread_id - 1][i];

            if (i == 5) {
                pthread_mutex_unlock(&writing_mutex);

                usleep(get_random(0, 1000000));

                pthread_mutex_lock(&writing_mutex);
            }

            i++;
        }

        data_buffer[i] = 0;

        pthread_mutex_unlock(&writing_mutex);

        sleep(get_random(2, 3));
        repeats++;
    }
}

void reader_func(void *arg) {
    if (arg == NULL) {
        return;
    }

    const intptr_t thread_id = (intptr_t) arg;
    int repeats = 0;

    while (repeats < get_height_of_term() - 1) {
        pthread_mutex_lock(&n_mutex);
        n++;

        if (n == 1) {
            pthread_mutex_lock(&writing_mutex);
        }

        pthread_mutex_unlock(&n_mutex);

        char *data_to_output = (char *) calloc(13, sizeof(char));
        strcpy(data_to_output, data_buffer);

        pthread_mutex_lock(&n_mutex);
        n--;

        if (n == 0) {
            pthread_mutex_unlock(&writing_mutex);
        }

        pthread_mutex_unlock(&n_mutex);

        struct output_args args;

        args.row = repeats + 1;
        args.column = (int) thread_id;
        args.data_to_output = data_to_output;

        output(args);

        free(data_to_output);

        usleep(get_random(250000, 350000));
        repeats++;
    }
}



int main() {
    printf("\033[2J\n");

    data_buffer = (char *) calloc(13, sizeof(char));

    pthread_t reader_threads[3], writer_threads[3];

    pthread_mutex_init(&reading_mutex, NULL);
    pthread_mutex_init(&writing_mutex, NULL);
    pthread_mutex_init(&n_mutex, NULL);

    for (int i = 0; i < 3; i++) {
        pthread_create(&writer_threads[i], NULL, (void *) writer_func, (void *) (intptr_t) i + 1);
    }

    for (int i = 0; i < 3; i++) {
        pthread_create(&reader_threads[i], NULL, (void *) reader_func, (void *) (intptr_t) i + 1);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(reader_threads[i], NULL);
    }

    free(data_buffer);

    pthread_mutex_destroy(&n_mutex);
    pthread_mutex_destroy(&writing_mutex);
    pthread_mutex_destroy(&reading_mutex);

    return EXIT_SUCCESS;
}
