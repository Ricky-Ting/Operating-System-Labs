#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/wait.h>

extern char ** environ;

int main(int argc, char *argv[]) {
	argv[0] = "strace";	
	if(pid == 0) {
		int ret = execve(argv[0], argv, environ);
		printf("Shouldn't be here! error=%d\n",ret);
	} else {
		wait(NULL);
		printf("End\n");
	}
	


  return 0;
}
