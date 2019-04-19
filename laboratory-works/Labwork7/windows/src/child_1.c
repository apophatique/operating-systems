#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Wrong number of command-line args (expected: > 3)");
        return;
    }

    char *name = argv[1];
    char *parent_name = argv[2];

    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_information;

    memset(&startup_info, 0, sizeof(STARTUPINFO));
    startup_info.cb = sizeof(startup_info);

    char buffer[30];
    sprintf(buffer, "%s %s %s", "child_1_1.exe", "CHILD_1_1", "CHILD_1");

    WINBOOL rc = CreateProcess(
            NULL,
            buffer,
            NULL,
            NULL,
            FALSE,
            NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &startup_info,
            &process_information
    );

    if (!rc) {
        printf("Error while trying to create process (error code: %ld)\n", GetLastError());
        getchar();
        return 0;
    }

    printf("CHILD_1\n");

    int iter;
    for (iter = 0; iter < 30; iter++) {
        printf("My name is %s, i am a child of %s (iteration number: %d)\n", name, parent_name, iter);
        Sleep(1000);
    }

    return 0;
}
