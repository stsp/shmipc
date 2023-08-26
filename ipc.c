#include <stdlib.h>
#include <assert.h>
#include "pgalloc.h"
#include "shm.h"
#include "waitq.h"
#include "ipc.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#define P2ALIGN(x, y) (((x) + (y) - 1) & -(y))
#define PGA_LEN (128 + 1)

struct ipc_shm {
    const char *self;
    int used;
    struct wq_head wq;
    int cpred;
    int lvar;
    int pga[PGA_LEN];
    char *data;
};

struct ipc_desc {
    void *handle;
    struct ipc_shm *ishm;
    int oused;
    ptrdiff_t ptroff;
};

void *ipc_attach(const char *name, unsigned pages)
{
  struct ipc_desc *dsc;
  struct ipc_shm *ishm;
  int oused;
  void *shm = shm_alloc(name, pages);
  if (!shm)
    return NULL;
  ishm = shm_addr(shm);
  shm_lock(shm, 1);
  oused = ishm->used;
  if (!ishm->used) {
    pgainit_custom(ishm->pga, PGA_LEN, pages * 64);
    ishm->data = (char *)P2ALIGN((uintptr_t)(ishm + 1), 64);
    wq_init(&ishm->wq);
    ishm->self = (char *)ishm;
  }
  ishm->used++;
  shm_unlock(shm);

  dsc = malloc(sizeof(*dsc));
  assert(dsc);
  dsc->handle = shm;
  dsc->ishm = ishm;
  dsc->oused = oused;
  dsc->ptroff = ishm->self - (char *)ishm;
  return dsc;
}

void ipc_detach(void *handle)
{
  struct ipc_desc *dsc = handle;
  struct ipc_shm *ishm = dsc->ishm;
  void *shm = dsc->handle;
  shm_lock(shm, 1);
  ishm->used--;
//  if (!ishm->used)
//    smdestroy(&ishm->pool);
  shm_unlock(shm);
  shm_free(shm);
  free(dsc);
}

static void *smalloc(struct ipc_desc *dsc, unsigned size)
{
  struct ipc_shm *ishm = dsc->ishm;
  return _RP(ishm->data, dsc->ptroff) +
      (pgaalloc(ishm->pga, P2ALIGN(size, 64) >> 6, 0) << 6);
}

static void smfree(struct ipc_desc *dsc, void *addr)
{
  struct ipc_shm *ishm = dsc->ishm;
  pgafree(ishm->pga, ((char *)addr - _RP(ishm->data, dsc->ptroff)) >> 6);
}

void *ipc_alloc(void *handle, unsigned size)
{
  struct ipc_desc *dsc = handle;
  void *shm = dsc->handle;
  void *ret;

  shm_lock(shm, 1);
  ret = smalloc(dsc, size);
  shm_unlock(shm);
  return ret;
}

void ipc_free(void *handle, void *addr)
{
  struct ipc_desc *dsc = handle;
  void *shm = dsc->handle;

  shm_lock(shm, 1);
  smfree(dsc, addr);
  shm_unlock(shm);
}

int ipc_is_first(void *handle)
{
  struct ipc_desc *dsc = handle;
  return (dsc->oused == 0);
}

void ipc_cond_init(void *handle, struct condvar *cnd)
{
  struct ipc_desc *dsc = handle;
  cond_init(cnd, &dsc->ishm->wq, dsc->ptroff);
}

void ipc_rwlock_init(void *handle, struct rwlock *lock)
{
  struct ipc_desc *dsc = handle;
  rwlock_init(lock, &dsc->ishm->lvar);
}

int *ipc_cpred(void *handle)
{
  struct ipc_desc *dsc = handle;
  return &dsc->ishm->cpred;
}
