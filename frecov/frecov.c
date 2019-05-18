#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#define BytesPerSectorOff 0x0B
#define SectorsPerClusterOff 0x0D
#define ReservedSectorOff 0x0e
#define NumberofFATOff 0x10
#define SectorsPerFATOff 0x24

uint16_t BytesPerSector;
uint8_t SectorsPerCluster;
uint16_t ReservedSector;
uint8_t NumberofFAT;
uint32_t SectorsPerFAT;


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
	assert(img_start != NULL);

	/*
	for(int i=0;i<10;i++) {
		printf("%x ", (unsigned int) ( *(unsigned char *)(img_start + i)) );
	}
	*/


	/* Collect basic info*/
	BytesPerSector = *((uint16_t *)(img_start + BytesPerSectorOff));
	SectorsPerCluster = *((uint8_t *)(img_start + SectorsPerClusterOff));
	ReservedSector = *((uint16_t *) (img_start + ReservedSectorOff));
	NumberofFAT = *((uint8_t *)(img_start + NumberofFATOff));
	SectorsPerFAT = *((uint32_t *)(img_start + SectorsPerFATOff));		
		
	printf("%x %x %x %x %x", BytesPerSector, SectorPerCluster, reservedSector, NumberofFAT, SectorsPerFAT);



	close(fd);
  return 0;
}
