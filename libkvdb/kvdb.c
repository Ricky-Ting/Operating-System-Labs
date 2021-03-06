#include "kvdb.h"
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void __may_crash() {
	if( rand()%2 == 0) {
		printf("Crash\n");
		exit(0);
	}
}

int kvdb_open(kvdb_t *db, const char *filename) {
	/* What if the opened db isn't closed */
	//memcpy(db->filename, filename, strlen(filename) + 1);

	srand(time(NULL));

	char logname[MAXBUF];
	sprintf(logname,"%s.log",filename);

	FILE * tmp = fopen(filename, "a+");
	fclose(tmp);

	tmp = fopen(logname,"a+");
	fclose(tmp);
	
	db->fp = fopen(filename, "rw+");	 
	

	db->log = fopen(logname, "rw+");
	if(db->fp==NULL || db->log==NULL) {
		perror("Open file failed");
		return -1;
	}

	//printf("%d\n",(int)lseek(fileno(db->log),0,SEEK_END));	
	db->fd = fileno(db->fp);
	db->logfd = fileno(db->log);
	pthread_mutex_init(&db->lk,NULL);
	return 0;
}
 
int kvdb_close(kvdb_t *db) {
	int ret = close(db->fd);
	int ret2 = close(db->logfd);
	if(ret < 0 || ret2 < 0) {
		perror("Close file failed");
		return -1;
	}
	db->fd = -1;
	db->fp = NULL;
	db->log = NULL;
	db->logfd = -1;
	return 0;
}

int kvdb_put(kvdb_t *db, const char * key, const char *value) {
	//printf("Put %s %s\n",key, value);
	int ret = 0;
	if(db->fd<0) {
		perror("No file opened");
		return -1;
	}
	pthread_mutex_lock(&db->lk);
	ret = flock(db->fd, LOCK_EX);
	if(ret<0) {
		perror("lock file failed");
		pthread_mutex_unlock(&db->lk);
		return -1;
	}		
	if(lseek(db->logfd,0,SEEK_END)==0) {
		lseek(db->logfd,0,SEEK_SET);
		write(db->logfd,"n",1);
	}else {
		lseek(db->logfd,0,SEEK_SET);
		char buf[MAXBUF];
		read(db->logfd,buf,1);
		if(buf[0]!='y') { 
			recover(db);
			//printf("%sh\n",buf);
		}
		else {
			lseek(db->logfd,0,SEEK_SET);
			write(db->logfd,"n",1);
		}
	}
	
	sync();
	
	lseek(db->fd, 0, SEEK_END);
	write(db->fd,key,strlen(key));
	//printf("%d\n",(int)(strlen(key)));
	write(db->fd," ",1);

	//__may_crash();

	write(db->fd,value,strlen(value));
	//printf("%d\n",(int)(strlen(value)));
	//__may_crash();

	write(db->fd,"\n",1);
	//__may_crash();
	sync();
		
	lseek(db->logfd,0,SEEK_SET);
	write(db->logfd,"y",1);
	sync();

	flock(db->fd, LOCK_UN);
	pthread_mutex_unlock(&db->lk);
	return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
	//printf("get %s\n",key);
	char * value = NULL;
	int ret  = 0;
	
	if(db->fd<0) {
		perror("No file opened");
		return NULL;
	}

	pthread_mutex_lock(&db->lk);
	ret = flock(db->fd, LOCK_EX);
	if(ret<0) {
		perror("lock file failed");
		pthread_mutex_unlock(&db->lk);	
		return NULL;
	}

	if(lseek(db->logfd,0,SEEK_END)!=0) {
		lseek(db->logfd,0,SEEK_SET);
		char buf[MAXBUF];
		read(db->logfd,buf,1);
		if(buf[0]!='y') { 
			recover(db);
			lseek(db->logfd,0,SEEK_SET);
			write(db->logfd,"y",1);
		}
	}
	
	sync();
	char buf[MAXBUF];
	char * valuebuf = malloc(16 MB);
	//char * line = malloc(17 MB);
	lseek(db->fd,0,SEEK_SET);
	while(~fscanf(db->fp,"%s%s",buf,valuebuf)) {
		//printf("%s %s \n", buf, valuebuf);
		if(strcmp(buf,key)==0) {
			if(value!=NULL)
				free(value);
			value = malloc(strlen(valuebuf)+1);
			memcpy(value,valuebuf,strlen(valuebuf)+1);
			//printf("In get,%s\n",value);

		}
	}		
	free(valuebuf);
	ret = flock(db->fd, LOCK_UN);
	pthread_mutex_unlock(&db->lk);

	return value;
}

void recover(kvdb_t *db) {
	//printf("Here recoer\n");
	char *line = malloc(17 MB);
	lseek(db->fd,0,SEEK_SET);
	while((fgets(line, 17 MB, db->fp))!=NULL) {
		;
	}		
	int len = strlen(line);
	memset(line,'0',17 MB);
	sprintf(line,"~!@#$&*()$DELETEDNOSENSE~!@#$&*()NOUSE!!!!");
	line[strlen(line)+1] = ' ';
	line[len+60] = 'h'; line[len+61] = '\n';	
	len = -len;
	lseek(db->fd,len,SEEK_END);
	write(db->fd, line , -len+62);
	free(line);	
	sync();
}
