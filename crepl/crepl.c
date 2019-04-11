#include <stdio.h>
#include <stdlib.h>
#define QUIT 0
#define FUNC 1
#define EXPR 2 
#define MAX_BUF 4028

#define Assert(x,y) if(!x) {printf(y); return 0}

int judge(char * buf) {
	if( *(buf+0) == 'q' && *(buf+1) == 'u' &&  *(buf+2) == 'i' &&  *(buf+3) =='t' )
		return QUIT;
	else if( *(buf+0) == 'i' && *(buf+1) == 'n' &&  *(buf+2) == 't' )
		return FUNC;
	else
		return EXPR;
}

int main(int argc, char *argv[]) {
	
	char line[MAX_BUF];
	printf("Welcome to fantasy world.\n If you want to quit, type quit\n");
	printf(">> ");
	while( fgets(line, MAX_BUF, stdin) ) {
		if(judge(line) == QUIT) {
			printf("\n Hello World!\n");
			return 0;
		} else if(judge(line) == FUNC) {
			FILE *tmpfp = tmpfile();
			Assert(!tmpfp,"\nCannot Create tmp File\n");	

		}
		else {


		}

		printf("\n>> ");
	}

  return 0;
}
