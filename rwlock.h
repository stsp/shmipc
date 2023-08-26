#ifndef RWLOCK_H
#define RWLOCK_H

enum LkMode { UNLCK, RDLCK, WRLCK };

struct rwlock {
  int *shaddr;
  enum LkMode lk;
};

void rwlock_init(struct rwlock *lock, int *shaddr);
void rwlock_destroy(struct rwlock *lock);
void rwlock_rdlock(struct rwlock *lock);
void rwlock_wrlock(struct rwlock *lock);
void rwlock_unlock(struct rwlock *lock);

#endif
