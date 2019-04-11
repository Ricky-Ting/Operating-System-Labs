#include <stdio.h>
#include <stdlib.h>
#define QUIT 0
#define FUNC 1
#define EXPR 2 

int judge(char ** buf) {
	if( *(*buf+0) == 'q' && *(*buf+1) == 'u' &&  *(*buf+2) == 'i' &&  *(*buf+3) =='t' )
		return QUIT;
	else if( *(*buf+0) == 'i' && *(*buf+1) == 'n' &&  *(*buf+2) == 't' )
		return FUNC;
	else
		return EXPR;
}

int main(int argc, char *argv[]) {
	
	char ** line;
	int len;
	printf(">> ");
	while( getline(line, &len, stdin) ) {
		if(judge(line) == QUIT) {
			printf("\n Hello World!\n");
			return 0;
		} else if(judge(line) == FUNC) {
			

		}
		else {


		}

		free(line);

	}

  return 0;
}
