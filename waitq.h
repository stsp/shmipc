#ifndef WAITQ_H
#define WAITQ_H

#include <stddef.h>
#include "rwlock.h"

struct list_item {
  struct list_item *next;
};

struct wq_head {
  struct list_item *list;
  struct rwlock lock;
  int lockvar;
};

static inline void wq_init(struct wq_head *head)
{
  head->lockvar = 0;
  rwlock_init(&head->lock, &head->lockvar);
  head->list = NULL;
}

static inline void wq_add(struct wq_head *head, struct list_item *waiter)
{
  struct list_item *tmp = head->list;
  head->list = waiter;
  waiter->next = tmp;
}

static inline void wq_del(struct wq_head *head, struct list_item *waiter)
{
  head->list = waiter->next;
}

static inline void wq_lock(struct wq_head *head)
{
  rwlock_wrlock(&head->lock);
}

static inline void wq_unlock(struct wq_head *head)
{
  rwlock_unlock(&head->lock);
}

#endif
