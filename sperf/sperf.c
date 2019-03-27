#include <sys/types.h>
#include <unistd.h>

extern char **environ;
int main(int argc, char *argv[]) {
	pid_t pid = fork(); 
	if(pid == 0) {
		execve("/bin/ls", NULL, environ);
	} else {

				

	}
	


  return 0;
}
