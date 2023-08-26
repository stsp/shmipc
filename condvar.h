#ifndef CONDVAR_H
#define CONDVAR_H

#include <stdint.h>
#include "waitq.h"

struct condvar {
  struct list_item waiter;  // keep this at the beginning of the struct
  struct wq_head *wqhead;
  struct rwlock *lock;
  ptrdiff_t ptroff;
};

void cond_init(struct condvar *cnd, struct wq_head *wqhead, ptrdiff_t ptroff);
void cond_destroy(struct condvar *cnd);
void cond_wait(struct condvar *cnd, struct rwlock *lock);
void cond_signal(struct condvar *cnd);
void cond_broadcast(struct condvar *cnd);

#define _WP(p, o) ((p) ? (__typeof(p))((char *)(p) + (o)) : NULL)
#define _RP(p, o) ((p) ? (__typeof(p))((char *)(p) - (o)) : NULL)

#endif
