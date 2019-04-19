#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

struct thread_output_execute_args {
    intptr_t thread_id;
    int row;
    int column;
};

enum constants {
    FIRST_THREAD_ID = 1,
    SECOND_THREAD_ID = 2,
    THIRD_THREAD_ID = 3,

    ITERATIONS_NUMBER = 20,

    SLEEP_TIME_MICROSECONDS = 100000,
    SLEEP_TIME_IN_THE_PROGRAM_END_MICROSECONDS = 300000,

    ITER_NUM_ON_WHICH_THREAD_1_MUST_BE_CANCELLED_BY_MAIN = 6,
    ITER_NUM_ON_WHICH_THREAD_3_MUST_BE_CANCELLED_BY_MAIN = 11,
    ITER_NUM_ON_WHICH_THREAD_3_MUST_ALLOW_TO_CANCEL_ITSELF = 16
};

void thread_output_execute(const struct thread_output_execute_args args) {
    printf(
            "\033[%d;%dH\033[1;%dm%c\n",
            args.row + 1,
            (int) (args.column + 20 * args.thread_id),
            (int) (30 + args.thread_id),
            'a' + args.row
    );
}

void thread_function(void *probably_thread_id) {
    if (probably_thread_id == NULL) {
        return;
    }

    const intptr_t thread_id = (intptr_t) probably_thread_id;

    if (thread_id == FIRST_THREAD_ID || thread_id == THIRD_THREAD_ID) {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    }

    int i = 0;

    while (i < ITERATIONS_NUMBER) {
        if (thread_id == THIRD_THREAD_ID && i == ITER_NUM_ON_WHICH_THREAD_3_MUST_ALLOW_TO_CANCEL_ITSELF) {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
        }

        int j = 0;

        while (j < thread_id * 2) {
            struct thread_output_execute_args args;

            args.row = i;
            args.column = j;
            args.thread_id = thread_id;

            thread_output_execute(args);
            usleep(SLEEP_TIME_MICROSECONDS);
            j++;
        }

        i++;
    }
}

void *(*thread_func_ptr)(void *) = (void *) thread_function;

int main() {
    printf("\033[2J\n");

    pthread_t thread_1, thread_2, thread_3;

    pthread_create(&thread_1, NULL, thread_func_ptr, (void *) (intptr_t) FIRST_THREAD_ID);
    pthread_create(&thread_2, NULL, thread_func_ptr, (void *) (intptr_t) SECOND_THREAD_ID);
    pthread_create(&thread_3, NULL, thread_func_ptr, (void *) (intptr_t) THIRD_THREAD_ID);

    int i = 0;

    while (i < ITERATIONS_NUMBER) {
        if (i == ITER_NUM_ON_WHICH_THREAD_1_MUST_BE_CANCELLED_BY_MAIN) {
            pthread_cancel(thread_1);
        }

        if (i == ITER_NUM_ON_WHICH_THREAD_3_MUST_BE_CANCELLED_BY_MAIN) {
            pthread_cancel(thread_3);
        }

        printf(
                "\033[%d;%dH\033[37m%d) %c\n",
                i + 1,
                i < 9 ? 2 : 1,
                i + 1,
                'a' + i
        );

        usleep(SLEEP_TIME_MICROSECONDS);
        i++;
    }

    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);
    pthread_join(thread_3, NULL);

    usleep(SLEEP_TIME_IN_THE_PROGRAM_END_MICROSECONDS);
    printf("\033[2J\033[1;1HPress ENTER to exit...");
    getchar();
    printf("\033[2J\033[1;1H");
    return EXIT_SUCCESS;
}
