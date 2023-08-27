#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ipc.h"
#include "pid.h"

struct sync {
  struct condvar cnd;
  struct rwlock lock;
  int *cpred;
};

static void do_accept(void *ipc)
{
  struct sync *s = ipc_alloc(ipc, sizeof(*s));
  const char *msg;

  ipc_cond_init(ipc, &s->cnd);
  ipc_rwlock_init(ipc, &s->lock);
  s->cpred = ipc_cpred(ipc);

  rwlock_rdlock(&s->lock);
  while (!*s->cpred) {
    printf("waiting for client...\n");
    cond_wait(&s->cnd, &s->lock);
  }
  rwlock_unlock(&s->lock);

  msg = ipc_msg(ipc);
  assert(msg);
  printf("client connected: %s\n", msg);
  ipc_msg_put(ipc);

  ipc_free(ipc, s);
}

static void do_connect(void *ipc, const char *arg0)
{
  struct sync *s = ipc_alloc(ipc, sizeof(*s));
  char *msg;
  unsigned pid = my_pid();

  ipc_cond_init(ipc, &s->cnd);
  ipc_rwlock_init(ipc, &s->lock);
  s->cpred = ipc_cpred(ipc);

  msg = ipc_msg_get(ipc, 256);
  assert(msg);
  sprintf(msg, "Hello World from %s:%i", arg0, pid);
  ipc_msg_put(ipc);

  printf("connecting to server from pid %i\n", pid);
  rwlock_wrlock(&s->lock);
  *s->cpred = 1;
  rwlock_unlock(&s->lock);
  cond_broadcast(&s->cnd);
  ipc_free(ipc, s);
}

int main(int argc, char *argv[])
{
  void *ipc = ipc_attach("testipc", 1);
  assert(ipc);
  if (ipc_is_first(ipc))
    do_accept(ipc);
  else
    do_connect(ipc, argv[0]);
  ipc_detach(ipc);
  return 0;
}
