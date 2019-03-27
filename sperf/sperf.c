#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

extern char ** environ;

int main(int argc, char *argv[]) {
	pid_t pid = fork(); 
	printf("%s\n",argv[0]);
	argv++;
	printf("%s\n",argv[0]);

	if(pid == 0) {
		int ret = execve("/bin/ls", argv++, environ);
		printf("Shouldn't be here! error=%d\n",ret);
	} else {
		printf("End\n");
	}
	


  return 0;
}
