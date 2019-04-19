#include <windows.h>
#include <stdio.h>

int main() {
    system("cls");
    HANDLE mutex_handle, shared_memory_handle;
    char *shared_memory_ptr;

    if ((shared_memory_handle = OpenFileMapping(FILE_MAP_READ, 0, "memory")) == NULL) {
        printf("Error while trying to open file mapping (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("File mapping has been opened!\n");
    }

    printf("Memory handle: %d\n", (int) shared_memory_handle);

    if ((shared_memory_ptr = MapViewOfFile(shared_memory_handle, FILE_MAP_READ, 0, 0, 1024)) == NULL) {
        printf("Error while trying to map view of file (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("View of file has been mapped!\n");
    }

    printf("Reading data from buffer without waiting: %s\n", shared_memory_ptr);

    if ((mutex_handle = OpenMutex(SYNCHRONIZE, 0, "mutex")) == NULL) {
        printf("Error while trying to open mutex (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Mutex has been opened!\n");
    }

    if (WaitForSingleObject(mutex_handle, INFINITE) == (DWORD) 0xFFFFFFFF) {
        printf("Error while trying to wait for single object (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Wait for single object was successful!\n");
    }

    printf("Reading data from buffer after waiting: %s\n", shared_memory_ptr);

    if (CloseHandle(mutex_handle) == 0 || CloseHandle(shared_memory_handle) == 0) {
        printf("Error while trying to close handle (error code: %ld)\n", GetLastError());
        getchar();
        return EXIT_FAILURE;
    } else {
        printf("Handles were closed!\n");
    }

    printf("\nPress ENTER to exit...");
    getchar();
    return EXIT_SUCCESS;
}
