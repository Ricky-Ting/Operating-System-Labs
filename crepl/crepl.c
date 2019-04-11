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

			char GCC[MAX_BUF], dl_name[MAX_BUF];
			sprintf(dl_name,"./%s",file_tmplate);
			dl_name[12] = 's';
			dl_name[13] = 'o';
			dl_name[14] = '\0';
			sprintf(GCC, "gcc -shared -fPIC -m%d   %s -o %s", (int)(8*(sizeof(void *)) ), file_tmplate, dl_name);			
			system(GCC);
			
			/*void * handle*/  dlopen(dl_name, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE );
	
			unlink(file_tmplate);
			dlclose(handle);

		}
		else {
			
			char file_tmplate[MAX_F_LEN] = "tmpXXXXXX.c";
			int tmpfd = mkstemps(file_tmplate, 2);
			Assert( (tmpfd!=-1),"\nCannot Create tmp File\n");	
			
			dprintf(tmpfd, "int tmp_func(void) { return %s;}",line);	
			
			char GCC[MAX_BUF], dl_name[MAX_BUF];
			sprintf(dl_name,"./%s",file_tmplate);
			dl_name[12] = 's';
			dl_name[13] = 'o';
			dl_name[14] = '\0';
			sprintf(GCC, "gcc -shared -fPIC -m%d   %s -o %s", (int)(8*(sizeof(void *)) ), file_tmplate, dl_name);			
			
			
			system(GCC);
	

			int (* func)(void);
			void * handle = dlopen(dl_name, RTLD_LAZY | RTLD_LOCAL);
			//fprintf(stderr,"%s", dlerror());
			Assert(handle, "\nCannot dlopen sl.so\n");
			func = dlsym(handle, "tmp_func");	
			char * error;
			if( (error = dlerror())!=NULL)
				fprintf(stderr,"%s", error);
			int ret = (*func)();
			printf("%d\n", ret);
			unlink(file_tmplate);
			dlclose(handle);	

		}

		printf("\n>> ");
	}

  return 0;
}
