#include <common.h>
#include <klib.h>

void shell_thread() {
	int tty_id = 1;
	char buf[128];

	char pwd[1024], prompt[1024];
	pwd[0] = '/'; pwd[1] = '\0';	
	sprintf(buf, "/dev/tty%d",tty_id);
	int stdin = vfs->open(buf, 0);
	int stdout = vfs->open(buf, 0);

	char line[2048];

	while(1) {
		sprintf(prompt, "%s>", pwd);
		vfs->write(stdout, prompt, strlen(prompt) );	
		int nread = vfs->read(stdin, line, sizeof(line));
		line[nread - 1] = '\0';
		
		if(strcmp(line,"ls")==0) {
			int ret = vfs->readdir(pwd,buf);
			if(ret<0)
				printf("ls Error\n");
			else {
				vfs->write(stdout, buf, strlen(buf));
			}
		} else if(line[0] == 'c' && line[1] == 'd') {
			int len = strlen(line);
			memmove(line, line+3, len-3);
			line[len-3] = '\0';
			int ret = vfs->access(line,0);
			if(ret != ISDIRE) {
				printf("cd Error\n");
			} else {
				strcpy(pwd, line);
			}
		} else if(line[0] == 'r' && line[1] == 'm') {
			int len = strlen(line);
			memmove(line, line+3, len-3);
			line[len-3] = '\0';

			int ret = vfs->unlink(line);
			if(ret != ISDIRE) {
				printf("rm Error\n");

		} else if(line[0] =='l' && line[1] == 'n') {
		
		} else if(line[0] == 'u')

	}


}
