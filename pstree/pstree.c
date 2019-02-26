#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>

struct pidtree {
	pid_t pid_num;
	pid_t pid_p;
	char * pid_name;
};

int counter = 0;
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
			
			File * F_name = fopen("pid_name_f",'r');
			assert(F_name);



			fclose(F_name);
			
			//printf("%s\n",strcat(tmp,ptr->d_name));
			//printf("%s,%s\n",tmp,pid_dir);
					
	

		}
	}
	closedir(dir);

}

