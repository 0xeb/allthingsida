#ifdef _WIN32
    #ifdef __TESTING__
        #include <conio.h>
    #endif
#else
    #error Platform not supported
#endif

#include <windows.h>
#include "utils.hpp"

int main(int argc, char* argv[]) 
{
    // check arguments. expects an IDB file path
    if (argc < 2)
    {
        printf("Usage: %s <IDB file path>\n", argv[0]);
        return 1;
    }
    
#ifdef __TESTING__
    printf("eject_idb waiting...press any key to continue\n");
    _getch();
#endif
    char sem_name[64];
    make_semaphore_name(argv[1], sem_name, sizeof(sem_name));

    HANDLE hSemaphore = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, sem_name);
    if (hSemaphore == NULL)
    {
        printf("failed to eject %s\n", argv[1]);
        return 1;
    }
    ReleaseSemaphore(hSemaphore, 1, NULL);
    CloseHandle(hSemaphore);

    return 0;
}
