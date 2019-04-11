#include <stdio.h>


int main(int argc, char *argv[]) {
	
	char ** line;
	int len;
	printf(">> ");
	while( getline(line, &len) ) {
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
