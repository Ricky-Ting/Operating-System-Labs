#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	pid_t pid = fork(); 
	char * arg[] = {"ls", NULL};
	char * env[] = {"PATH=/bin", NULL};
	if(pid == 0) {
		int ret = execve("/bin/ls", arg, env);
		printf("Shouldn't be here! error=%d\n",ret);
	} else {
		wait(NULL);
		printf("End\n");
				

	}
	


  return 0;
}
