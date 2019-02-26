#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>

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

void pstree() {
	DIR *dir = opendir("/proc");
	struct dirent * ptr;
	while( ptr=readdir(dir) ) {
		if(ptr->d_type==DT_DIR)
			printf("%s\n",ptr->d_name);

	}

}

