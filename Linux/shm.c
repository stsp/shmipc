#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "shlock.h"
#include "../shm.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

struct shm_s {
  void *addr;
  unsigned pages;
  char *name;
  char *shname;
  void *shlock;
  void *ulock;
};

void *shm_alloc(const char *name, unsigned pages)
{
#define SHLOCK_DIR "shl"
#define EXLOCK_DIR "exl"
  int rc;
  void *addr, *shlock, *exlock;
  char shname[256];
  struct stat st;
  int fd;
  struct shm_s *ret;

  exlock = shlock_open(EXLOCK_DIR, name, 1, 1);
  assert(exlock);
  snprintf(shname, sizeof(shname), "/%s", name);
  fd = shm_open(shname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("shm_open()");
    exit(1);
  }
  rc = fstat(fd, &st);
  assert(!rc);
  if (!st.st_size) {
    rc = ftruncate(fd, pages * PAGE_SIZE);
    assert(!rc);
  } else {
    assert(pages * PAGE_SIZE <= st.st_size);
  }
  shlock = shlock_open(SHLOCK_DIR, name, 0, 1);
  assert(shlock);
  shlock_close(exlock);
  addr = mmap(NULL, pages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
              fd, 0);
  close(fd);
  if (addr == MAP_FAILED)
    return NULL;

  ret = malloc(sizeof(*ret));
  assert(ret);
  ret->addr = addr;
  ret->pages = pages;
  ret->name = strdup(name);
  ret->shname = strdup(shname);
  ret->shlock = shlock;
  ret->ulock = NULL;
  return ret;
}

void shm_free(void *handle)
{
  struct shm_s *s = handle;
  void *exlock;
  int rc;

  exlock = shlock_open(EXLOCK_DIR, s->name, 1, 1);
  assert(exlock);
  rc = shlock_close(s->shlock);
  if (rc)
    shm_unlink(s->shname);
  shlock_close(exlock);
  munmap(s->addr, s->pages * PAGE_SIZE);
  free(s->name);
  free(s->shname);
  free(s);
}

void shm_lock(void *handle, int excl)
{
#define ULOCK_DIR "ulk"
  struct shm_s *s = handle;
  void *lock = shlock_open(ULOCK_DIR, s->name, excl, 1);
  assert(lock);
  s->ulock = lock;
}

void shm_unlock(void *handle)
{
  struct shm_s *s = handle;
  shlock_close(s->ulock);
  s->ulock = NULL;
}

void *shm_addr(void *handle)
{
  struct shm_s *s = handle;
  return s->addr;
}
