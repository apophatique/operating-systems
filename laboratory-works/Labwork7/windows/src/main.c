#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdio.h>

int main() {
    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_information;

    printf("MAIN\n");
    memset(&startup_info, 0, sizeof(STARTUPINFO));
    startup_info.cb = sizeof(startup_info);

    HANDLE h_job = CreateJobObject(NULL, NULL);

    WINBOOL rc = CreateProcess(
            NULL,
            "child_1.exe CHILD_1 MAIN",
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
        return EXIT_FAILURE;
    }

    AssignProcessToJobObject(h_job, process_information.hProcess);

    rc = CreateProcess(
            NULL,
            "child_2.exe CHILD_2 MAIN",
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
        return EXIT_FAILURE;
    }

    printf(
            "Child process:\nhProcess = %p; hThread = %p; dwProcessId = %ld; dwThreadId = %ld\n",
            process_information.hProcess,
            process_information.hThread,
            process_information.dwProcessId,
            process_information.dwThreadId
    );

    int iter;
    for (iter = 0; iter < 20; iter++) {
        printf("My name is MAIN (iteration number: %d)\n", iter);

        if (iter == 7) {
            printf("DELETE JOB\n");
            TerminateJobObject(h_job, 1);
        } else if (iter == 11) {
            printf("DELETE PROCESS\n");
            TerminateProcess(process_information.hProcess, 1);
        }

        Sleep(2000);
    }

    CloseHandle(process_information.hProcess);
    CloseHandle(process_information.hThread);
    return 0;
}
