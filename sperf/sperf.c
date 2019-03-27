#include <sys/types.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
	pid_t pid = fork(); 
	if(pid == 0) {
		execve(argv[1], argv+2);
	} else {

				

	}
	


  return 0;
}
