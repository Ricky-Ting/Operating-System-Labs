#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#define QUIT 0
#define FUNC 1
#define EXPR 2 
#define MAX_BUF 4028
#define MAX_F_LEN 4028




#define Assert(x,y) if(!x) {printf(y); return 0;}

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
			char file_tmplate[MAX_F_LEN] = "tmpXXXXXX.c";
			int tmpfd = mkstemps(file_tmplate, 2);
			Assert( (tmpfd!=-1),"\nCannot Create tmp File\n");	
			dprintf(tmpfd, "%s",line);	

			char GCC[MAX_BUF];
			sprintf(GCC, "gcc -shared -fPIC -m%d %s -o sl.so", (int)(8*(sizeof(void *)) ), file_tmplate);			
			system(GCC);
		
			void * handle = dlopen("sl.so", RTLD_NOW | RTLD_NODELETE);
	
			dlclose(handle);
			unlink(file_tmplate);

		}
		else {
			
			char file_tmplate[MAX_F_LEN] = "tmpXXXXXX.c";
			int tmpfd = mkstemps(file_tmplate, 2);
			Assert( (tmpfd!=-1),"\nCannot Create tmp File\n");	
			
			dprintf(tmpfd, "int tmp_func(void) { return %s;}",line);	

			char GCC[MAX_BUF];
			sprintf(GCC, "gcc -shared -fPIC -m%d %s -o sl.so", (int)(8*(sizeof(void *)) ), file_tmplate);			
			system(GCC);
			int (* func)();
			void * handle = dlopen("sl.so", RTLD_LAZY);
			func = dlsym(handle, "tmp_func");
			printf("%d\n", func());
			dlclose(handle);
			unlink(file_tmplate);


		}

		printf("\n>> ");
	}

  return 0;
}
