#include "mutex.h"

void acquireMutex(mutex_t *mutex)
{
    while (!__sync_bool_compare_and_swap(mutex, 0, 1))
        __asm__ volatile("pause");
}

int checkMutex(mutex_t *mutex)
{
    return *mutex;
}

void releaseMutex(mutex_t *mutex)
{
    *mutex = 0;
}