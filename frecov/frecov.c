#include <stdio.h>
#include <sys/mman.h>

int fd;

int main(int argc, char *argv[]) {

	/*Open the img*/
	fd = open(argv[1], O_RDONLY);
	assert(fd>=0);

	/*Get the size of the img*/
	struct stat st;
	int ret = fstat(fd,&st);
	assert(ret!=-1);
	int len = st.st_size;
	
	/*Map the file to mem*/

	mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);




	close(fd);
  return 0;
}
