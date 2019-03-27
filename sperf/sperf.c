#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

extern char **environ;
int main(int argc, char *argv[]) {
	//printf("%s\n",argv[0]);
	pid_t pid = fork(); 
	char * arg[] = {"ls"};
	if(pid == 0) {
		execve("/bin/ls", arg, environ);
	} else {

				

	}
	


  return 0;
}
