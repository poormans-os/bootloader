#include "mutex.h"

/**
 * @brief This function locks the given mutex
 * 
 * @param mutex Mutex to lock
 */
void acquireMutex(mutex_t *mutex)
{
    while (!__sync_bool_compare_and_swap(mutex, 0, 1))
        __asm__ volatile("pause");
}

/**
 * @brief This function unlocks the given mutex
 * 
 * @param mutex Mutex to unlock
 */
void releaseMutex(mutex_t *mutex)
{
    *mutex = 0;
}