#ifndef RWLOCK_P_H
#define RWLOCK_P_H

#include "rwlock.h"

void rwlock_signal(struct rwlock *lock, int *shaddr);
void rwlock_wait(struct rwlock *lock);

#endif
