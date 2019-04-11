#include <stdio.h>

#define MAXBUF 4028

int main(int argc, char *argv[]) {
	
	char ** line;
	printf(">> ");
	while( getline(line, MAXBUF) ) {
		if(judge(line) == QUIT) {
			printf("\n Hello World!\n");
			return 0;
		} else if(judge(line) == FUNC) {
			

		}
		else {


		}

	}

  return 0;
}
