#ifndef __KVDB_H__
#define __KVDB_H__

#define MAXBUF 1024

#define MB *1024*1024

#include <stdio.h>

struct kvdb {
	char filename[MAXBUF];
	int fd;
	FILE * fp;
		
};
typedef struct kvdb kvdb_t;

int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);

#endif