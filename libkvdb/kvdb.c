#include "kvdb.h"
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int kvdb_open(kvdb_t *db, const char *filename) {
	/* What if the opened db isn't closed */
	memcpy(db->filename, filename, strlen(filename) + 1);

	db->fp = fopen(filename, O_RDWR | O_CREAT);	 
	if(db->fp==NULL) {
		perror("Open file %s failed", filename);
		return -1;
	}
	db->fd = fileno(db->fp);

	return 0;
}
 
int kvdb_close(kvdb_t *db) {
	int ret = close(db->fd);
	if(ret < 0) {
		perror("Close file failed");
		return -1;
	}
	db->fd = -1;
	return 0;
}

int kvdb_put(kvdb_t *db, const char * key, const char *value) {
	int ret = 0;
	if(db->fd<0) {
		perror("No file opened");
		return -1;
	}
	ret = flock(db->fd, LOCK_EX);
	if(ret<0) {
		perror("lock file failed");
		return -1;
	}		

	lseek(db->fd, 0, SEEK_END);
	write(fd,key,sizeof(key));
	sync();
	write(fd,value,sizeof(value));
	sync();
	write(fd,"\n",1);
	sync();
		
	ret = flock(db->fd, LOCK_UN);
	if(ret<0) {
		perror("unlock file failed");
		return -1;
	}
	return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
	char * value = NULL;
	int ret  = 0;
	
	if(db->fd<0) {
		perror("No file opened");
		return NULL;
	}

	ret = flock(db->fd, LOCK_SH);
	if(ret<0) {
		perror("lock file failed");
		return NULL;
	}
	
	char buf[MAXBUF];
	char valuebuf[16MB];
	char line[17MB];
	lseek(db->fd,0,SEEK_SET);
	while(!feof(fp)) {
		fgets(line,17MB,fp);
		sscanf(line,"%s%s",buf,valuebuf);	
		if(strcmp(buf,key)==0) {
			value = malloc(strlen(valuebuf)+1);
			memcpy(value,valuebuf,sizeof(value));
			break;	
		}
	}		


	

	ret = flock(db->fd, LOCK_UN);
	if(ret<0) {
		perror("unlock file failed");
		return NULL;
	}	

	return value;
}


