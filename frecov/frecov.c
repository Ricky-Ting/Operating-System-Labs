#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

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
uint32_t len;
void * img_start;
void * data_start;
uint32_t EntryPerCluster;
static inline int search_in_entry(void * entry_start); 
static inline int search_in_cluster(int NO); 
int main(int argc, char *argv[]) {

	/*Open the img*/
	fd = open(argv[1], O_RDONLY);
	assert(fd>=0);

	/*Get the size of the img*/
	struct stat st;
	int ret = fstat(fd,&st);
	assert(ret!=-1);
	len = st.st_size;
	
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
	
	data_start = img_start + ReservedSector*BytesPerSector + NumberofFAT*SectorsPerFAT*BytesPerSector;	
	EntryPerCluster = SectorsPerCluster * BytesPerSector / 32;
	//printf("%p\n %p\n",data_start, (void *)(data_start - img_start));
	//printf("%x %x %x %x %x", BytesPerSector, SectorsPerCluster, ReservedSector, NumberofFAT, SectorsPerFAT);

	
	for(int i=0;;i++){
		int ret = search_in_cluster(i);
		if(!ret)
			break;
	}
	
	close(fd);
  return 0;
}

static inline int search_in_cluster(int NO) {
	void * this_cluster = data_start + NO*SectorsPerCluster*BytesPerSector;  
	int ret;
	for(int i=0; i< EntryPerCluster; i++) {
		ret = search_in_entry(this_cluster + i*32);
		if(!ret)
			return 0;
	}
	return 1;
} 

static inline int search_in_entry(void * entry_start) {
	if(entry_start+32>img_start+len)
		return 0;
		
	if(*((uint8_t *)(entry_start + 0xB)) != 0x0f) {
		if(!( *((uint8_t *)(entry_start + 0x8)) == 0x42 &&  *((uint8_t *)(entry_start + 0x9)) == 0x4D && *((uint8_t *)(entry_start + 0xa)) == 0x50 )  ) {
			return 1;
		}

		char filename[MAXBUF];
		filename[0] = '\0';
		char ch;
		for(int i=0;i<8;i++) {
			ch = (char)(*(uint8_t*)(entry_start + i));	
			if(ch == ' ')
				break;
			filename[i] = ch;
			filename[i+1] = '\0';
		}
		printf("%s.bmp\n",filename);
	
	}


	return 1;
}





