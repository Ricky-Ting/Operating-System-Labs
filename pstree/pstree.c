#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#define MAXPID 10000

struct node {
	pid_t pid_num;
	pid_t pid_p;
	char pid_name[100] ;
	int child_num;
	int child_index[200];
} pidtree[MAXPID];

int counter = 0;
int root;
void pstree(void);

int main(int argc, char *argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    assert(argv[i]); // always true
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]); // always true
	pstree();

  return 0;
}

int is_number(char* s) {
	while(*s) {
		if( (*s < '0' ) || (*s > '9') )
			return 0;
		s++;
	}
	return 1;

}

void pstree() {
	DIR *dir = opendir("/proc");
	struct dirent * ptr;
	while( (ptr=readdir(dir)) ) {
		if( (ptr->d_type==DT_DIR) && (is_number(ptr->d_name)) ) {
			//printf("%s\n",ptr->d_name);
			//DIR *pid_dir = opendir(  strcat("/proc",ptr->d_name) );
			char tmp[10] = "/proc";
			char pid_dir[300], pid_name_f[300], pid_pa_f[300];
			sprintf(pid_dir,"%s/%s",tmp,ptr->d_name);
			sprintf(pid_name_f, "%s/comm",pid_dir);
			sprintf(pid_pa_f, "%s/stat",pid_dir);
			
			//printf("%s\n",pid_name_f);
			FILE * F_name = fopen(pid_name_f,"r");
			assert(F_name);
			fscanf(F_name,"%s",pidtree[counter].pid_name);	
			fclose(F_name);
	
			FILE * F_pa = fopen(pid_pa_f, "r");
			assert(F_pa);
			char omit1[100];
			char omit2;
			fscanf(F_pa,"%d %s %c %d",&pidtree[counter].pid_num, omit1, &omit2 ,&pidtree[counter].pid_p);	
			fclose(F_pa);
			
			//printf("%s\n",strcat(tmp,ptr->d_name));
			//printf("%s,%s\n",tmp,pid_dir);
			counter++;		
	

		}
	}
	closedir(dir);

	/*	
	for(int i=0;i<counter;i++) {
		printf("pid:%d,name:%s,parent:%d\n",pidtree[i].pid_num,pidtree[i].pid_name, pidtree[i].pid_p);
	}
	
}
	*/
	for(int i=0;i<counter;i++) {
		pidtree[i].child_num=0;
	}	

	for(int i=0;i<counter;i++) {	
		if(pidtree[i].pid_p==0) {
			root = i;
		}
		else {
			for(int j=0;j<counter;j++){
				if(pidtree[j].pid_num==pidtree[i].pid_p) {
					pidtree[j].child_index[pidtree[j].child_num]=i;
					pidtree[j].child_num++;
				}
			}

		}
	}



}
