#include <dpmi.h>
#include <sys/segments.h>
#include <sys/nearptr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "../shm.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

struct shm_s {
  uint32_t handle;
  void *addr;
  unsigned pages;
  char *name;
};

#define SHM_NOEXEC 1
#define SHM_EXCL 2

void *shm_alloc(const char *name, unsigned pages)
{
  __dpmi_shminfo shm = {};
  int rc;
  struct shm_s *ret;

  shm.name_selector = _my_ds();
  shm.name_offset = (uintptr_t)name;
  shm.size_requested = pages * PAGE_SIZE;
  shm.reserved2 = SHM_NOEXEC;
  rc = __dpmi_allocate_shared_memory(&shm);
  if (rc)
    return NULL;
  __djgpp_nearptr_enable();
  ret = malloc(sizeof(*ret));
  assert(ret);
  ret->handle = shm.handle;
  ret->addr = (void *)(shm.address - __djgpp_base_address);
  ret->pages = shm.size / PAGE_SIZE;
  ret->name = strdup(name);
  return ret;
}

void shm_free(void *handle)
{
  struct shm_s *s = handle;
  int rc;

  rc = __dpmi_free_shared_memory(s->handle);
  assert(!rc);
  free(s->name);
  free(s);
}

void shm_lock(void *handle, int excl)
{
  struct shm_s *s = handle;
  int rc;

  rc = __dpmi_serialize_on_shared_memory(s->handle, 0);
  assert(!rc);
}

void shm_unlock(void *handle)
{
  struct shm_s *s = handle;
//  int rc;

  __dpmi_free_serialization_on_shared_memory(s->handle, 0);
#if 0
  /* disable due to djgpp bug */
  assert(!rc);
#endif
}

void *shm_addr(void *handle)
{
  struct shm_s *s = handle;
  return s->addr;
}
