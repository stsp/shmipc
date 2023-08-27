#ifndef PGALLOC_H
#define PGALLOC_H

void *pgainit(unsigned npages);
void pgadone(void *pool);
void pgainit_custom(int *array, unsigned len, unsigned npages);
void pgareset(void *pool);
int pgaalloc(void *pool, unsigned npages, unsigned id);
void pgafree(void *pool, unsigned page);
int pgaresize(void *pool, unsigned page, unsigned oldpages, unsigned newpages);
int pgaavail_largest(void *pool);
struct pgrm {
    int id;
    int pgoff;
};
struct pgrm pgarmap(void *pool, unsigned page);

#endif
