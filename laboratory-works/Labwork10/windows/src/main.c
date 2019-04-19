#include <windows.h>
#include <stdio.h>
#include <time.h>

int get_random(int min, int max) {
    srand((u_int) time(NULL));
    return rand() % (max + 1 - min) + min;
}

int main() {
    system("cls");
    HANDLE mutex_handle, shared_memory_handle;

    if ((shared_memory_handle = CreateFileMapping(
            (HANDLE) 0xFFFFFFFF,
            NULL,
            PAGE_READWRITE,
            0,
            1024,
            "memory"
    )) == NULL) {
        printf("Error while trying to create file mapping (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("File mapping was created!\n");
    }

    printf("Memory handle: %d\n", (int) shared_memory_handle);
    char *shared_memory_ptr;

    if ((shared_memory_ptr = MapViewOfFile(shared_memory_handle, FILE_MAP_WRITE, 0, 0, 1024)) == NULL) {
        printf("Error while trying to map view of file (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("View of file has been mapped!\n");
    }

    if ((mutex_handle = CreateMutex(NULL, 1, "mutex")) == NULL) {
        printf("Error while trying to create mutex (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Mutex was created!\n");
    }

    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_information;

    memset(&startup_info, 0, sizeof(STARTUPINFO));
    startup_info.cb = sizeof(startup_info);

    if (!CreateProcess(
            NULL,
            "child.exe",
            NULL,
            NULL,
            0,
            NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &startup_info,
            &process_information
    )) {
        printf("Error while trying to create process (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Child process was created!\n");
    }

    Sleep(get_random(10000, 15000));

    char string_to_write[] = "I_AM_DATA_FROM_MAIN_PROCESS!";

    int i;
    for (i = 0; i < strlen(string_to_write) + 1; i++) {
        shared_memory_ptr[i] = string_to_write[i];
    }

    printf("Data written to buffer: %s\n", string_to_write);

    if (ReleaseMutex(mutex_handle) == 0) {
        printf("Error while trying to release mutex (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Mutex has been released!\n");
    }

    if (
            CloseHandle(mutex_handle) == 0 ||
            CloseHandle(shared_memory_handle) == 0 ||
            CloseHandle(process_information.hProcess) == 0 ||
            CloseHandle(process_information.hThread) == 0
    ) {
        printf("Error while trying to close handle (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Handles were closed!\n");
    }

    if (UnmapViewOfFile(shared_memory_ptr) == 0) {
        printf("Error while trying to unmap view of file (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("View of file has been unmapped!\n");
    }

    char *allocated_memory_ptr;

    if ((allocated_memory_ptr = VirtualAlloc(NULL, 1000, MEM_COMMIT, PAGE_READWRITE)) == NULL) {
        printf("Error while trying to allocate memory using VirtualAlloc (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Memory has been allocated using VirtualAlloc!\n");
    }

    for (i = 0; i < 26; i++) {
        const char char_to_write = (char) (65 + i);
        char *ptr_for_writing = allocated_memory_ptr + 400 * i;

        printf(
                "Character %c was written to memory address %p...\n",
                char_to_write,
                ptr_for_writing
        );

        memset(ptr_for_writing, char_to_write, 1);
    }

    if (VirtualFree(allocated_memory_ptr, 0, MEM_RELEASE) == 0) {
        printf("Error while trying to free memory using VirtualFree (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Memory has been freed using VirtualFree!\n");
    }

    printf("\nPress ENTER to exit...");
    getchar();
    return EXIT_SUCCESS;
}
