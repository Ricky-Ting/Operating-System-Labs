#include <common.h>

int holding(struct spinlock_t *lock);
void pushcli(void);
void popcli(void);
