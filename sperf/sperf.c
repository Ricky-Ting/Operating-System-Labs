#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

extern char **environ;
int main(int argc, char *argv[]) {
	print("%s\n",argv[0]);
	pid_t pid = fork(); 
	if(pid == 0) {
		execve("/bin/ls", argv, environ);
	} else {

				

	}
	


  return 0;
}
