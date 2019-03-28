#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/wait.h>
#include <assert.h>
#include <regex.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_CALL 1000

extern char ** environ;

regmatch_t pmatch[1];
const size_t nmatch = 1;
char s[1000];
int ret;	
regex_t regex[2];
struct {
	char name[200];
	double time;
} call[MAX_CALL];
double sum = 0;
int counter = 0;
time_t current_time, previous_time;
double diff_time;

	

void print(char ** s) {
	char ** var = s;
	while(*var!=NULL) {
		printf("%s\n", *var);
		var++;
	}

}

void output() {
		for(int i=0; i<counter; i++) {
			printf("%s: \t \t %.5lf%%\n", call[i].name, call[i].time/sum*100);

		}
}


void sort() {
	double tmp_time = 0;
	char tmp_name[200];
	for(int i=0; i<counter-1; i++) {
		for(int j=i+1; j<counter; j++) {
			if(call[i].time < call[j].time) {
				tmp_time = call[j].time;
				call[j].time = call[i].time;
				call[i].time = tmp_time;

				sprintf(tmp_name,"%s",call[j].name);
				sprintf(call[j].name, "%s", call[i].name);
				sprintf(call[i].name, "%s", tmp_name);	
			}	
		}
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

	regcomp(&regex[0], "^[A-Za-z0-9_]*(", REG_NEWLINE);
		regcomp(&regex[1], "<[0-9/.]*>$", REG_NEWLINE);	
		
		time(&previous_time);
		while(fgets(s,800,stdin)) {			
			if(s[0]=='+')
				break;
			char name[100];
			char time_str[100]; double call_time;
			//printf("%s\n",s);
			ret = regexec(&regex[0], s, nmatch, pmatch, 0);
			//memcpy(name , s + pmatch[0].rm_so, (int)(pmatch[0].rm_eo - pmatch[0].rm_so));
			if(ret == 0) {
				strncpy(name, s + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
				name[pmatch[0].rm_eo-pmatch[0].rm_so - 1] = '\0';
				//printf("%s\n", name);
	
				ret = regexec(&regex[1], s, nmatch, pmatch, 0);
				if(ret == 0) {
						strncpy(time_str, s + pmatch[0].rm_so + 1, pmatch[0].rm_eo - pmatch[0].rm_so);
						time_str[pmatch[0].rm_eo - pmatch[0].rm_so - 2] = '\0';
						//printf("%s\n",time_str);
						sscanf(time_str,"%lf",&call_time);
						int tmp_counter = 0;
						while(tmp_counter<counter && (strcmp(call[tmp_counter].name, name)!=0))
								tmp_counter++;
						if(tmp_counter == counter) {
								strncpy(call[counter].name, name, 200);
								call[counter].time = call_time;
								counter++;
								sum += call_time;
						} else {
								call[tmp_counter].time += call_time;
								sum += call_time;
						}
				}
			}
		
			time(&current_time);	
			diff_time = difftime(current_time, previous_time);	
			if(diff_time > 1) {
				sort();
				system("clear");
				output();
				time(&previous_time);
			}
		}	
		wait(NULL);
		//printf("End\n");
		sort();
		system("clear");
		output();

	}
	


  return 0;
}
