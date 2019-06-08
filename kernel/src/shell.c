#include <common.h>
#include <klib.h>

void shell_thread(void *tty_id) {
	
	char buf[128];

	char pwd[1024], prompt[1024];
	pwd[0] = '/'; pwd[1] = '\0';	
	sprintf(buf, "/dev/tty%d",tty_id);
	int stdin = vfs->open(buf, 0);
	int stdout = vfs->open(buf, 0);

	char line[2048];
	char output[2048];
	while(1) {
		sprintf(prompt, "%s>", pwd);
		vfs->write(stdout, prompt, strlen(prompt) );	
		int nread = vfs->read(stdin, line, sizeof(line));
		line[nread - 1] = '\0';
		
		if(strcmp(line,"ls")==0) {
			output[0] = '\0';
			printf("In sheell, buf is %s\n",output);
			int ret = vfs->readdir(pwd,output);
			if(ret<0)
				printf("ls Error\n");
			else {
				//int tmplen = strlen(output);
				//output[tmplen] = '\n'; output[tmplen +1 ] = '\0'; 
				vfs->write(stdout, output, strlen(output));
			}
		} else if(line[0] == 'c' && line[1] == 'd') {
			int len = strlen(line);
			memmove(line, line+3, len-3);
			line[len-3] = '\0';
			printf("In shell cd %s\n",line);
			int ret = vfs->access(line,0);
			if(ret != ISDIRE) {
				printf("cd Error ret = %d\n",ret);
			} else {
				strcpy(pwd, line);
			}
		} else if(line[0] =='l' && line[1] == 'n') {
			int len = strlen(line);
			memmove(line, line+3, len-3);
			line[len-3] = '\0';
			
			char oldpath[200];
			char newpath[200];
			int it =0;
			while(it<len-3 && line[it]!=' ')
				it++;
			
			memcpy(oldpath, line, it);
			oldpath[it] = '\0';
			memcpy(newpath, line+it+1, len-3 - it -1);
			newpath[len-3-it-1] = '\0';
			int ret = vfs->link(oldpath, newpath);
			if(ret<0)
				printf("link %s %s failed\n", oldpath, newpath);
		} else if(line[0] == 'u') {
				printf("Mor\n");

		} else if(line[0] == 't' && line[1] == 'o' && line[2] == 'u' && line[3] == 'c' && line[4] == 'h') {
			int len = strlen(line);
			memmove(line, line+6, len-6);
			line[len-6] = '\0';
			int ret = vfs->open(line,0);
			if(ret<0) {
				printf("touch %s failed\n",line);
			} else{
				
			} 
		} else if(line[0] == 'm' && line[1] == 'k' && line[2] == 'd' && line[3] == 'i' && line[4] == 'r'){
			int len = strlen(line);
			memmove(line, line+6, len-6);
			line[len-6] = '\0';
			int ret = vfs->mkdir(line);
			if(ret<0) {
				printf("mkdir %s failed\n",line);
			}
		} else if (line[0] == 'r' && line[1] == 'm' && line[2] == 'd' && line[3] == 'i' && line[4] == 'r') {
			int len = strlen(line);
			memmove(line, line+6, len-6);
			line[len-6] = '\0';
			int ret = vfs->rmdir(line);
			if(ret<0) {
				printf("rmdir %s failed\n",line);
			}

		} else if(line[0] == 'r' && line[1] == 'm') {
			int len = strlen(line);
			memmove(line, line+3, len-3);
			line[len-3] = '\0';

			int ret = vfs->unlink(line);
			if(ret != ISDIRE) {
				printf("rm %s failed\n",line);
			}
		} else if(line[0] == 'c' && line[1] == 'a' && line[2] == 't') {
			int len = strlen(line);
			memmove(line, line+4, len-4);
			line[len-4] = '\0';

			char readbuf[128];
			int fd = vfs->open(line,0);
			int nread = 0;
			do{
				nread = vfs->read(fd, readbuf,128);
				if(nread<0) {
					printf("cat %s failed\n",line);
					break;
				}	
				vfs->write(stdout, readbuf, nread);
			}while( nread==128);

		} else if(line[0]=='w' && line[1]=='r' && line[2]=='i' && line[3] == 't' && line[4] == 'e') {
			// write filename offset content
			int len = strlen(line);
			memmove(line, line+6, len-6);
			line[len-6] = '\0';
			len = len -6;			
	
			int it1=0; int it2 = 0;
			char filename[200];
			char off_string[20];
			int off = 0;

			while(it1<len && line[it1]!=' ')
				it1++;
			it2 = it1+1;
			while(it2<len && line[it2]!=' ')
				it2++;						
				
			memcpy(filename, line, it1);
			line[it1] = '\0';

			memcpy(off_string, line+it1+1, it2-it1-1);
			off_string[it2-it1-1] = '\0';
		
			memmove(line, line+it2+1, len - it2 -1);
			line[len - it2 - 1] = '\0';
			
			int len2 = strlen(off_string);
			int it3 = len2-1;
			int jie =1;
			while(it3>=0) {
				off += (jie*(off_string[it3]-'0'));
				jie*=10;
				it3--;
			}			
			int fd = vfs->open(filename, 0);
			vfs->lseek(fd,off, SEEK_SET);
			vfs->write(fd, line, strlen(line));
		}



	}
}
