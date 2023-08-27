#ifndef IPC_H
#define IPC_H

#include "condvar.h"
#include "rwlock.h"

void *ipc_attach(const char *name, unsigned pages);
void ipc_detach(void *handle);
void *ipc_alloc(void *handle, unsigned size);
void ipc_free(void *handle, void *addr);

int ipc_is_first(void *handle);
void ipc_cond_init(void *handle, struct condvar *cnd);
void ipc_rwlock_init(void *handle, struct rwlock *lock);
int *ipc_cpred(void *handle);

char *ipc_msg_get(void *handle, unsigned size);
void ipc_msg_put(void *handle);
const char *ipc_msg(void *handle);

#endif
