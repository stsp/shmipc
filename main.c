#include <stdio.h>
#include <assert.h>
#include "ipc.h"

struct sync {
  struct condvar cnd;
  struct rwlock lock;
  int *cpred;
};

static void do_accept(void *ipc)
{
  struct sync *s = ipc_alloc(ipc, sizeof(*s));
  ipc_cond_init(ipc, &s->cnd);
  ipc_rwlock_init(ipc, &s->lock);
  s->cpred = ipc_cpred(ipc);

  rwlock_rdlock(&s->lock);
  while (!*s->cpred)
    cond_wait(&s->cnd, &s->lock);
  rwlock_unlock(&s->lock);
  ipc_free(ipc, s);
  printf("client connected\n");
}

static void do_connect(void *ipc)
{
  struct sync *s = ipc_alloc(ipc, sizeof(*s));
  ipc_cond_init(ipc, &s->cnd);
  ipc_rwlock_init(ipc, &s->lock);
  s->cpred = ipc_cpred(ipc);

  rwlock_wrlock(&s->lock);
  *s->cpred = 1;
  rwlock_unlock(&s->lock);
  cond_broadcast(&s->cnd);
  ipc_free(ipc, s);
}

int main()
{
  void *ipc = ipc_attach("testipc", 1);
  assert(ipc);
  if (ipc_is_first(ipc))
    do_accept(ipc);
  else
    do_connect(ipc);
  ipc_detach(ipc);
  return 0;
}
