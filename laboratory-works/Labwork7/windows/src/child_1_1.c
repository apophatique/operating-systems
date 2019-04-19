#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Wrong number of command-line args (expected: > 3)");
        return;
    }

    char *name = argv[1];
    char *parent_name = argv[2];

    printf("CHILD_1_1\n");

    int iter;
    for (iter = 0; iter < 30; iter++) {
        printf("My name is %s, i am a child of %s (iteration number: %d)\n", name, parent_name, iter);
        Sleep(1000);
    }

    return 0;
}
