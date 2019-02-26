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
			char tmp[100] = "/proc/";
			char* tmp2 = strcat(tmp,ptr->d_name);
			//printf("%s\n",strcat(tmp,ptr->d_name));
			DIR *pid_dir = opendir(tmp2);
			
			closedir(pid_dir);

		}
	}

}

