#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/wait.h>

extern char ** environ;

void print(char ** s) {
	char ** var = s;
	while(*var!=NULL) {
		printf("%s\n", *var);
		var++;
	}

}

int pipefd[2];

int main(int argc, char *argv[]) {
	char * myargv[100];
	myargv[0] = "/usr/bin/strace";	
	myargv[1] = "-T";
	int tmp = 1;
	while(argv[tmp]!=NULL) {
		myargv[tmp+1] = argv[tmp];
		tmp++;
	}
	myargv[tmp+1] = NULL;
	//print(myargv);	
	int pipe(pipefd);
	pid_t pid = fork();
	if(pid == 0) {
		int ret = execve(myargv[0], myargv, environ);
		printf("Shouldn't be here! error=%d\n",ret);
	} else {
		wait(NULL);
		printf("End\n");
	}
	


  return 0;
}
