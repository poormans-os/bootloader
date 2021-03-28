#pragma once

#ifndef __MUTEX_H
#define __MUTEX_H

typedef volatile int mutex_t;

void acquireMutex(mutex_t *mutex);
void releaseMutex(mutex_t *mutex);

#endif