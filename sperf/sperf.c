#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/wait.h>
#include <assert.h>
#include <regex.h>
#include <string.h>

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
	if( pipe(pipefd) != 0) {
		assert(0);
	}
	pid_t pid = fork();
	if(pid == 0) {
		FILE * trash_bin = fopen("/dev/null", "w+");
	
		dup2(pipefd[1], 2);
		dup2(fileno(trash_bin), 1);
		int ret = execve(myargv[0], myargv, environ);
		printf("Shouldn't be here! error=%d\n",ret);
	} else {
		dup2(pipefd[0], 0);
		regmatch_t pmatch[1];
		const size_t nmatch = 1;
		char s[1000];
		int ret;	
		regex_t regex[2];
		regcomp(&regex[0], "^[A-Za-z0-9_]*(", REG_NEWLINE);
		regcomp(&regex[1], "<[0-9/.]*>$", REG_NEWLINE);	
		
		while(fgets(s,800,stdin)) {			
			char name[100];
			//printf("%s\n",s);
			ret = regexec(&regex[0], s, nmatch, pmatch, 0);
			//memcpy(name , s + pmatch[0].rm_so, (int)(pmatch[0].rm_eo - pmatch[0].rm_so));
			if(ret == 0) {
				strncpy(name, s + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
				name[pmatch[0].rm_eo-pmatch[0].rm_so - 1] = '\0';
				printf("%s\n", name);
			}
		}	
		wait(NULL);
		printf("End\n");
	}
	


  return 0;
}
