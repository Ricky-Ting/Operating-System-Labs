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


int main(int argc, char *argv[]) {
	print(argv);
	argv[0] = "/bin/strace";	
	print(argv);
	pid_t pid = fork();
	if(pid == 0) {
		int ret = execve(argv[0], argv, environ);
		printf("Shouldn't be here! error=%d\n",ret);
	} else {
		wait(NULL);
		printf("End\n");
	}
	


  return 0;
}
