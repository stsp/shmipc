#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "rwlock_p.h"
#include "waitq.h"
#include "condvar.h"

void cond_init(struct condvar *cnd, struct wq_head *wqhead, ptrdiff_t ptroff)
{
  cnd->wqhead = wqhead;
  cnd->lock = NULL;
  cnd->ptroff = ptroff;
}

void cond_destroy(struct condvar *cnd)
{
  assert(!cnd->lock);
}

void cond_wait(struct condvar *cnd, struct rwlock *lock)
{
  wq_lock(cnd->wqhead);
  wq_add(cnd->wqhead, _WP(&cnd->waiter, cnd->ptroff));
  wq_unlock(cnd->wqhead);
  assert(!cnd->lock);
  cnd->lock = lock;
  rwlock_wait(lock);
}

#if 0
void cond_signal(struct condvar *cnd)
{
  /* no need for wq_lock here, as at this point the predicate
   * is set so no new waiters are possible */
  if (cnd->lock) {
    struct rwlock *lock = cnd->lock;
    cnd->lock = NULL;
    wq_del(cnd->wqhead, &cnd->waiter);
    rwlock_signal(lock);
  }
}
#endif

void cond_broadcast(struct condvar *cnd)
{
  struct list_item *item;
  while ((item = _RP(cnd->wqhead->list, cnd->ptroff))) {
    struct condvar *cnd1 = (struct condvar *)item;
    struct rwlock *lock = _RP(cnd1->lock, cnd->ptroff);
    int *shaddr = _RP(lock->shaddr, cnd->ptroff);
    cnd1->lock = NULL;
    wq_del(cnd->wqhead, item);
    rwlock_signal(lock, shaddr);
  }
}
