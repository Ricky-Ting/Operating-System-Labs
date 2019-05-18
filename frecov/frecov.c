#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

int fd;
void * img_start;

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

	img_start = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
	for(int i=0;i<10;i++) {
		printf("%x ", (unsigned int) ( *(char *)(img_start + i)) );
	}




	close(fd);
  return 0;
}
