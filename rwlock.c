#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "rwlock.h"
#include "rwlock_p.h"

void rwlock_init(struct rwlock *lock, int *shaddr)
{
  lock->shaddr = shaddr;
  lock->lk = UNLCK;
}

void rwlock_destroy(struct rwlock *lock)
{
  assert(lock->lk == UNLCK);
}

void rwlock_rdlock(struct rwlock *lock)
{
  int val;
  assert(lock->lk == UNLCK);
  do {
    val = *lock->shaddr;
    if (val == -1) {
      usleep(1000);
      continue;
    }
  } while (!__sync_bool_compare_and_swap(lock->shaddr, val, val + 1));
  lock->lk = RDLCK;
}

void rwlock_wrlock(struct rwlock *lock)
{
  int val;
  assert(lock->lk == UNLCK);
  do {
    val = *lock->shaddr;
    if (val != 0) {
      usleep(1000);
      continue;
    }
  } while (!__sync_bool_compare_and_swap(lock->shaddr, 0, -1));
  lock->lk = WRLCK;
}

void rwlock_unlock(struct rwlock *lock)
{
  int ok;
  switch (lock->lk) {
    case UNLCK:
      abort();
      break;
    case RDLCK:
      ok = __sync_fetch_and_sub(lock->shaddr, 1);
      assert(ok > 0);
      break;
    case WRLCK:
      ok = __sync_bool_compare_and_swap(lock->shaddr, -1, 0);
      assert(ok);
      break;
  }
  lock->lk = UNLCK;
}

/* private interfaces for condvar */

void rwlock_signal(struct rwlock *lock, int *shaddr)
{
  int ok = __sync_fetch_and_add(shaddr, 1);
  assert(ok != -1);  // not awaking anyone
  ok = __sync_bool_compare_and_swap(&lock->lk, UNLCK, RDLCK);
  assert(ok);
}

void rwlock_wait(struct rwlock *lock)
{
  int ok = __sync_bool_compare_and_swap(&lock->lk, RDLCK, UNLCK);
  assert(ok);
  ok = __sync_fetch_and_sub(lock->shaddr, 1);
  assert(ok > 0);
  while (__sync_fetch_and_or(&lock->lk, 0) == UNLCK)
    usleep(1000);
  assert(lock->lk == RDLCK);
}
