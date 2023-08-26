#ifndef SHM_H
#define SHM_H

void *shm_alloc(const char *name, unsigned pages);
void shm_free(void *handle);
void shm_lock(void *handle, int excl);
void shm_unlock(void *handle);

void *shm_addr(void *handle);

#endif
