#include <common.h>
#include <klib.h>
#include <devices.h>

#define MAX_INODE_NUM 1024
#define MAX_BLOCK_NUM 1024
#define INODE_SIZE 128
#define BLOCK_SIZE 1024
#define INODE_BITMAP_SIZE MAX_INODE_NUM
#define BLOCK_BITMAP_SIZE MAX_BLOCK_NUM

#define INODE_BITMAP_OFF 0
#define BLOCK_BITMAP_OFF (INODE_BITMAP_OFF + INODE_BITMAP_SIZE)
#define INODE_OFF (BLOCK_BITMAP_OFF + BLOCK_BITMAP_SIZE)
#define BLOCK_OFF (INODE_OFF + INODE_SIZE*MAX_INODE_NUM)

#define ISFILE 1
#define ISDIRE 0



struct blkinode{
	uint32_t filesize;		// 4B
	uint32_t block_id[20];	// 20*4 = 80B
	int32_t refcnt;			// 4B
	int32_t f_or_d;			// 4B
	int32_t has_inode_t; 	// 4B
	char buf[32];		// 32B
};  					//128B
typedef struct blkinode blkinode_t;

struct blkdire{
	uint32_t inode_id; 		// 4B
	uint32_t filesize;		// 4B
	char filename[20];	// 20B
	int32_t f_or_d;			// 4B
};
typedef struct blkdire blkdire_t;

void blkfs_init(struct filesystem *fs, const char *name, device_t *dev) {
	
	//kmt->spin_init(&(fs->fs_lock),"fs_lock");
	strncpy(fs->fsname, name, MAXNAME);
	fs->dev = dev;		
	// 实现根目录 根目录的inode编号为0
	assert(INODE_SIZE == sizeof(blkinode_t));
	assert(sizeof(blkdire_t) == 32);
	blkinode_t tmp1;
	blkinode_t* root = &tmp1;
	root->filesize = 64;
	root->block_id[0] = 0;
	root->refcnt = 1;
	root->f_or_d = ISDIRE;	
	root->has_inode_t = 0;

	//kmt->spin_lock(&(fs->fs_lock));
	dev->ops->write(dev, INODE_OFF, (void *)root, sizeof(blkinode_t));
	//kmt->spin_unlock(&(fs->fs_lock));

	char inode_bitmap[MAX_INODE_NUM];
	memset(inode_bitmap,0,MAX_INODE_NUM);
   	inode_bitmap[0] = 1;	

	//kmt->spin_lock(&(fs->fs_lock));
	dev->ops->write(dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
	//kmt->spin_unlock(&(fs->fs_lock));

	char block_bitmap[MAX_BLOCK_NUM];
	memset(block_bitmap,0,MAX_BLOCK_NUM);
	block_bitmap[0] = 1;
	
	//kmt->spin_lock(&(fs->fs_lock));
	dev->ops->write(dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);
	//kmt->spin_unlock(&(fs->fs_lock));


	//blkdire* dot = malloc()
	// . 和..有需要再补
}

inode_t *blkfs_lookup(struct filesystem *fs, const char *path, int flags) {
	inode_t *ret = NULL;
	int flag = 1;
	uint32_t current_inode_id = 0;
	uint32_t current_block_id = 0;
	uint32_t iter = 0;
	char buf[1024];
	int len = strlen(path);
	blkinode_t tmp1; blkdire_t tmp3;

	blkinode_t* current_inode = &tmp1;
	fs->dev->ops->read(fs->dev,	INODE_OFF + INODE_SIZE*current_inode_id, current_inode, INODE_SIZE);

	blkdire_t* current_dire = &tmp3;
	
	//printf("In blkfs_lookup, path is %s\n",path);
	while(iter<len) {
		if(path[iter]=='/') {
			iter++;
			continue;
		}	
		int iter2 = iter;
		while(iter2<len-1 && path[iter2+1]!='/')
			iter2++;
		memcpy(buf, path+iter, iter2-iter+1);	
		buf[iter2-iter+1] = '\0';
		//printf("In blkfs_lookup, buf is %s\n",buf);
			
		int found = 0;	
		for(int i=2; i< (current_inode->filesize / 32); i++) {
			fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE*current_block_id + i*32, current_dire, sizeof(blkdire_t));
			if(strcmp(current_dire->filename,buf)==0) {
				current_inode_id = current_dire->inode_id;
				fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*current_inode_id, current_inode, INODE_SIZE);
				current_block_id = current_inode->block_id[0];
				found = 1;
				break;
			}
		}

		if(!found || (iter2!=len-1 && current_inode->f_or_d==ISFILE)) {
			flag = 0;
			break;	
		}
		iter = iter2+1;
	}
	if(flag) {
		if(!(current_inode->has_inode_t)) {
			ret = pmm->alloc(sizeof(inode_t));
			ret->id = current_dire->inode_id;
			ret->f_or_d = current_dire->f_or_d;
			ret->fs = fs;
			ret->refcnt = 0;
			ret->ops = fs->iops;
			memcpy(current_inode->buf,(void *) (&ret), sizeof(ret));
			current_inode->has_inode_t = 1;
			fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*(ret->id), current_inode, INODE_SIZE );
		}else {

			memcpy( (void *)(&ret),current_inode->buf, sizeof(ret) );	
		}
	}

	return ret;
}

int blkfs_close(inode_t *inode) {
	return 0;	
}

int blkfs_inode_open(const char *name, file_t *file, int flags, filesystem_t *fs) {
	inode_t *node = fs->ops->lookup(fs, name, 0);
	file->offset = 0;
	file->refcnt = 1;
	if(node!=NULL) {
		file->inode = node;
		return 0;	
	}

	int len = strlen(name);
	int iter = len-1;
	while(iter>0 && name[iter]!='/') 
		iter--;
	assert(name[iter]=='/');
	
	char buf[MAXNAME];
	memcpy(buf, name, iter+1);
	buf[iter+1] = '\0';	
	
	node = fs->ops->lookup(fs, buf, 0);
	if(node == NULL)
		return -1;

	blkinode_t tmp1,tmp2; blkdire_t tmp3;
	blkinode_t *upper_inode = &tmp1;
	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*node->id, upper_inode, INODE_SIZE);


	char inode_bitmap[MAX_INODE_NUM], block_bitmap[MAX_BLOCK_NUM];
	fs->dev->ops->read(fs->dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
	fs->dev->ops->read(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);

	int inode_iter = 0;
	int block_iter = 0;
	while(inode_iter<MAX_INODE_NUM && inode_bitmap[inode_iter]!=0)
		inode_iter++;
	while(block_iter<MAX_BLOCK_NUM && block_bitmap[block_iter]!=0) 
		block_iter++;
	assert(inode_iter<MAX_INODE_NUM);
	assert(block_iter<MAX_BLOCK_NUM);
	inode_bitmap[inode_iter] = 1;
	block_bitmap[block_iter] = 1;
	

	//kmt->spin_lock(&(fs->fs_lock));
	fs->dev->ops->write(fs->dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
	fs->dev->ops->write(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);
	//kmt->spin_unlock(&(fs->fs_lock));

	

	blkdire_t *new_dire = &tmp3;
	blkinode_t *new_file = &tmp2;

	new_dire->inode_id = inode_iter;
  new_dire->filesize = 0;
	memcpy(new_dire->filename, name+iter+1, strlen(name) - iter -1 );
	new_dire->filename[strlen(name)- iter - 1] = '\0';
	new_dire->f_or_d = ISFILE;

	new_file->filesize = 0;
	new_file->block_id[0] = block_iter;
	for(int i=1;i<20;i++) 
		new_file->block_id[i] = -1;
	new_file->refcnt = 0;
	new_file->f_or_d = ISFILE;
	new_file->has_inode_t = 0;


	//kmt->spin_lock(&(fs->fs_lock));

	fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE*upper_inode->block_id[0] + upper_inode->filesize, new_dire, sizeof(blkdire_t));
	upper_inode->filesize += 32;

	fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*node->id, upper_inode, INODE_SIZE);
	fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*inode_iter, new_file, INODE_SIZE);
	
	//kmt->spin_unlock(&(fs->fs_lock));

	
	file->inode = fs->ops->lookup(fs,name,0);
	assert(file->inode!=NULL);
	return 0;
}

int blkfs_inode_close(file_t *file) {
	return 0;	
}

ssize_t blkfs_inode_read(file_t *file, char *buf, size_t size) {
	//printf("Enter blkfs_inode_read\n");
	ssize_t ret = 0;
	filesystem_t *fs = file->inode->fs;
	assert(fs!=NULL);
	uint64_t off = file->offset;
	uint32_t inode_id = file->inode->id;
	blkinode_t tmp1;
	blkinode_t *read_inode = &tmp1;
	assert(read_inode!=NULL);
	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*inode_id, read_inode, INODE_SIZE);
	uint32_t filesize = read_inode->filesize;
	uint32_t read_start = off;
	uint32_t read_end = off + size;	 read_end = (read_end<=filesize)?read_end:filesize;
	assert(read_start<=filesize);
	if(read_start == read_end)
		return 0;

	//printf("IN blkfs_read, read_start = %d, read_end = %d\n",read_start, read_end);
	uint32_t block_start = read_start/BLOCK_SIZE;
	uint32_t block_end = (read_end-1)/BLOCK_SIZE;
	
	//printf("IN blkfs_read, block_start = %d, block_end = %d\n",block_start, block_end);


	if(block_start == block_end) {
		fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE * read_inode->block_id[block_start] + read_start%BLOCK_SIZE, buf, size);
		file->offset = read_end;
		//printf("In blkfs_read, off = %d\n", BLOCK_OFF + BLOCK_SIZE * read_inode->block_id[block_start] + read_start%BLOCK_SIZE);
		return read_end - read_start;
	}

	for(int i=block_start; i<=block_end; i++) {
		if(i==block_start) {
			fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE * read_inode->block_id[i] + read_start%BLOCK_SIZE, buf+ret, BLOCK_SIZE - (read_start % BLOCK_SIZE));
			ret += BLOCK_SIZE - (read_start%BLOCK_SIZE);
		} else if(i == block_end) {
			fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE * read_inode->block_id[i], buf+ret, read_end%BLOCK_SIZE );
			ret += read_end%BLOCK_SIZE;
		} else {
			fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE * read_inode->block_id[i], buf+ret, BLOCK_SIZE);
			ret += BLOCK_SIZE;
		}
	}
	assert( ret == (read_end-read_start) );
	file->offset = read_end;
	return ret;
}

ssize_t blkfs_inode_write(file_t *file, const char *buf, size_t size) {
	filesystem_t *fs = file->inode->fs;
	uint32_t write_start = file->offset;
	uint32_t write_end = write_start + size;
	uint32_t ret = 0;
	if(size==0)
		return 0;
	uint32_t start_block = write_start / BLOCK_SIZE;
	uint32_t end_block = (write_end-1) / BLOCK_SIZE;	
		
	uint32_t inode_id = file->inode->id;
	blkinode_t tmp1;
	blkinode_t *current_inode = &tmp1;
	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*inode_id, current_inode, INODE_SIZE);

	char block_bitmap[MAX_BLOCK_NUM]; 
	int block_iter = 0;
	fs->dev->ops->read(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);
	//printf("In blkfs_inode_write: start_block = %d, end_block= %d \n",start_block, end_block);

	for(int i=start_block; i<=end_block; i++) {
		if(current_inode->block_id[i]==-1) {
			while(block_iter<MAX_BLOCK_NUM && block_bitmap[block_iter]!=0) 
				block_iter++;
			assert(block_iter<MAX_BLOCK_NUM);
			block_bitmap[block_iter] = 1;
			current_inode->block_id[i] = block_iter;
		}

		if(start_block == end_block) {
			assert(ret == 0);
			//kmt->spin_lock(&(fs->fs_lock));
			fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE * current_inode->block_id[i] + write_start%BLOCK_SIZE, buf+ ret, write_end - write_start);
			//kmt->spin_unlock(&(fs->fs_lock));
			ret += write_end - write_start;
			/*	
			char buf1[200];
			fs->dev->ops->read(fs->dev, BLOCK_OFF + current_inode->block_id[i] + write_start%BLOCK_SIZE, buf1, write_end - write_start);
			printf("In blkfs_write, %s\n",buf1);
			*/
			//printf("In blkfs_read, off = %d\n",BLOCK_OFF + BLOCK_SIZE *current_inode->block_id[i] + write_start%BLOCK_SIZE);
			break;
		}
		//kmt->spin_lock(&(fs->fs_lock));
		if(i == start_block) {
			assert(ret==0);
			fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE * current_inode->block_id[i] + write_start%BLOCK_SIZE, buf+ret, BLOCK_SIZE - (write_start%BLOCK_SIZE));
			ret += BLOCK_SIZE - (write_start%BLOCK_SIZE);
		}else if(i == end_block) {
			fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE * current_inode->block_id[i], buf+ret, write_end%BLOCK_SIZE);
			ret += write_end%BLOCK_SIZE;
		}else {
			fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE * current_inode->block_id[i], buf+ret, BLOCK_SIZE);
			ret += BLOCK_SIZE;
		}
		//kmt->spin_unlock(&(fs->fs_lock));

	}					
	//printf("In blk_inode_write: ret:%d, write_end:%d, write_start:%d\n",ret, write_end,write_start);		
	assert(ret == write_end - write_start);
	if(write_end > current_inode->filesize)
		current_inode->filesize = write_end;		
	file->offset = write_end;
	
	//kmt->spin_lock(&(fs->fs_lock));

	fs->dev->ops->write(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);
	fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*inode_id, current_inode, INODE_SIZE);
	//kmt->spin_unlock(&(fs->fs_lock));

	return ret;
}

off_t blkfs_inode_lseek(file_t *file, off_t offset, int whence) {
	uint32_t inode_id = file->inode->id;
	//printf("In blkfs_inode_lseek, inode_id = %d\n", inode_id);
	filesystem_t *fs = file->inode->fs;
	//printf("In blkfs_lseek, h%d\n",file->inode->fs);

	blkinode_t tmp1;
  blkinode_t * current_inode = &tmp1;
	assert(current_inode!=NULL);
	//printf("In blkfs_lseek, cuinode = %d, inode = %d, INODE_SIZE = %d\n", current_inode, file->inode, INODE_SIZE);
	//printf("In blkfs_lseek, h%d\n",file->inode->fs);

	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*inode_id, current_inode, INODE_SIZE);
	//printf("In blkfs_lseek, h%d\n",file->inode->fs);

	uint32_t filesize = current_inode->filesize;
	off_t new_offset = 0;
		


	if(whence == SEEK_SET) {
		new_offset = offset;
	}else if(whence == SEEK_CUR) {
		new_offset = (off_t)(file->offset) + offset;	
	}else if(whence == SEEK_END) {
		new_offset = (off_t)(filesize) + offset;
	}else {
		return -1;
	}
	
	if(new_offset<0 || new_offset>filesize)
		return -1;

	//printf("In blkfs_lseek, h%d\n",file->inode->fs);

	return file->offset = new_offset;
}

int blkfs_inode_mkdir(const char *name, filesystem_t *fs) {
	/*
	 *	Create only one directory
	 */
	int len = strlen(name);
	int iter = len-1;
	while(iter>0 && name[iter]!='/') 
		iter--;
	assert(name[iter]=='/');
	
	char buf[MAXNAME];
	memcpy(buf, name, iter+1);
	buf[iter+1] = '\0';	
	
	inode_t *node = fs->ops->lookup(fs, buf, 0);
	if(node == NULL)
		return -1;

	blkinode_t tmp1,tmp2;
	blkinode_t *current_inode = &tmp1;
	fs->dev->ops->read(fs->dev, INODE_OFF + node->id*INODE_SIZE, current_inode, INODE_SIZE);
	
	if(current_inode->f_or_d == ISFILE) {
		return -1;
	}
	
	char inode_bitmap[MAX_INODE_NUM], block_bitmap[MAX_BLOCK_NUM];
	fs->dev->ops->read(fs->dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
	fs->dev->ops->read(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);

	int inode_iter = 0;
	int block_iter = 0;
	while(inode_iter<MAX_INODE_NUM && inode_bitmap[inode_iter]!=0)
		inode_iter++;
	while(block_iter<MAX_BLOCK_NUM && block_bitmap[block_iter]!=0) 
		block_iter++;
	assert(inode_iter<MAX_INODE_NUM);
	assert(block_iter<MAX_BLOCK_NUM);
	inode_bitmap[inode_iter] = 1;
	block_bitmap[block_iter] = 1;
	

	//kmt->spin_lock(&(fs->fs_lock));

	fs->dev->ops->write(fs->dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
	fs->dev->ops->write(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);
		


	
	blkdire_t tmp3;
	blkdire_t *new_dire = &tmp3;
	new_dire->inode_id = inode_iter;
	new_dire->filesize = 64;
	memcpy(new_dire->filename, name+iter+1, strlen(name) - iter -1 );
	new_dire->filename[strlen(name)- iter - 1] = '\0';
	new_dire->f_or_d = ISDIRE;
	fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE*current_inode->block_id[0] + current_inode->filesize, new_dire, sizeof(blkdire_t));
		
	current_inode->filesize += 32;
	fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*node->id, current_inode, INODE_SIZE);

	blkinode_t *new_inode = &tmp2;
	new_inode->filesize = 64;
	new_inode->block_id[0] = block_iter;
	new_inode->refcnt = 1;
	new_inode->f_or_d = ISDIRE;
	new_inode->has_inode_t = 0;
	fs->dev->ops->write(fs->dev, INODE_OFF + inode_iter*INODE_SIZE, new_inode, INODE_SIZE);	
	//kmt->spin_unlock(&(fs->fs_lock));




	return 0;
}

int blkfs_inode_rmdir(const char *name, filesystem_t *fs) {
	int len = strlen(name);
	int iter = len-1;
	while(iter>0 && name[iter]!='/') 
		iter--;
	assert(name[iter]=='/');
	
	char buf[MAXNAME];
	memcpy(buf, name, iter+1);
	buf[iter+1] = '\0';	
	
	inode_t *node1 = fs->ops->lookup(fs, buf, 0);
	inode_t *node2 = fs->ops->lookup(fs, name, 0);
	if(node1 == NULL || node2 == NULL)
		return -1;

	blkinode_t tmp1,tmp2;
	blkinode_t *current_inode = &tmp1;
	fs->dev->ops->read(fs->dev, INODE_OFF + node1->id*INODE_SIZE, current_inode, INODE_SIZE);

	blkinode_t * delete_inode = &tmp2;
   	fs->dev->ops->read(fs->dev, INODE_OFF + node2->id*INODE_SIZE, delete_inode, INODE_SIZE);	
	
	if(current_inode->f_or_d == ISFILE || delete_inode->f_or_d == ISFILE || delete_inode->filesize>64 ) {
		return -1;
	}
	
	char inode_bitmap[MAX_INODE_NUM], block_bitmap[MAX_BLOCK_NUM];
	fs->dev->ops->read(fs->dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
	fs->dev->ops->read(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);

	int inode_iter = node2->id;
	int block_iter = delete_inode->block_id[0];
	inode_bitmap[inode_iter] = 0;
	block_bitmap[block_iter] = 0;
	
	//kmt->spin_lock(&(fs->fs_lock));

	fs->dev->ops->write(fs->dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
	fs->dev->ops->write(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);
	//kmt->spin_unlock(&(fs->fs_lock));


	
	blkdire_t tmp3;
	blkdire_t *delete_dire = &tmp3;
			

	//kmt->spin_lock(&(fs->fs_lock));

	for(int i=2; i<(current_inode->filesize/32); i++) {
		fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE *current_inode->block_id[0] + i*32, delete_dire, sizeof(blkdire_t));
		if(delete_dire->inode_id == node2->id) {
			if(i != (current_inode->filesize/32 - 1)) {
				fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE*current_inode->block_id[0] + 32 *(current_inode->filesize/32 -1), delete_dire, sizeof(blkdire_t) );
				fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE*current_inode->block_id[0] + i*32, delete_dire, sizeof(blkdire_t));
			}
			current_inode->filesize -= 32;
			break;
		}
	}
	fs->dev->ops->write(fs->dev, INODE_OFF + node1->id*INODE_SIZE, current_inode, INODE_SIZE);
	//kmt->spin_unlock(&(fs->fs_lock));
	
	pmm->free(node2);
	return 0;

}

int blkfs_inode_link(const char *name, inode_t *inode, filesystem_t *fs) {
	int len = strlen(name);
	int iter = len-1;
	while(iter>0 && name[iter]!='/') 
		iter--;
	assert(name[iter]=='/');
	
	char buf[MAXNAME];
	memcpy(buf, name, iter+1);
	buf[iter+1] = '\0';	
	
	inode_t * node = fs->ops->lookup(fs,buf,0);
	
	blkinode_t tmp1, tmp2; blkdire_t tmp3;

	blkinode_t* current_inode = &tmp1;
	blkinode_t* link_inode = &tmp2;
	blkdire_t* new_dire = &tmp3;
	
	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*node->id, current_inode, INODE_SIZE);
	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*inode->id, link_inode, INODE_SIZE);
	new_dire->inode_id = inode->id;
	new_dire->filesize = link_inode->filesize;
	new_dire->f_or_d = ISFILE;
	memcpy(new_dire->filename, name+iter+1, strlen(name)-iter-1);
	new_dire->filename[strlen(name)-iter-1] = '\0';


	//kmt->spin_lock(&(fs->fs_lock));

	fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE*current_inode->block_id[0] + current_inode->filesize, new_dire, sizeof(blkdire_t));
	//printf("In blkfs_link, %d\n", current_inode->filesize);
	current_inode->filesize += 32;	
	fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*node->id, current_inode, INODE_SIZE);

	link_inode->refcnt += 1;
	fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*inode->id, link_inode, INODE_SIZE);
	//kmt->spin_unlock(&(fs->fs_lock));

	return 0;
}

int blkfs_inode_unlink(const char *name, filesystem_t *fs){
	inode_t * node1 = fs->ops->lookup(fs, name, 0);
	if(node1 == NULL)
		return -1;

	int len = strlen(name);
	int iter = len-1;
	while(iter>0 && name[iter]!='/') 
		iter--;
	assert(name[iter]=='/');
	
	char buf[MAXNAME], filename[MAXNAME];
	memcpy(buf, name, iter+1);
	buf[iter+1] = '\0';	
	memcpy(filename, name+iter+1, strlen(name)-iter-1);
	filename[strlen(name)-iter-1] = '\0';
	
	inode_t * node2 = fs->ops->lookup(fs,buf,0);
	if(node2 == NULL)
		return -1;
	
	blkinode_t tmp1, tmp2; blkdire_t tmp3;
	
	blkinode_t *delete_inode = &tmp1;
	blkinode_t *upper_inode = &tmp2;
	blkdire_t *delete_dire = &tmp3;

	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*node1->id, delete_inode, INODE_SIZE);
	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE*node2->id, upper_inode, INODE_SIZE);
	

	for(int i=2; i<(upper_inode->filesize/32); i++) {
		fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE*upper_inode->block_id[0] + i*32, delete_dire, sizeof(blkdire_t));
		if(strcmp(filename, delete_dire->filename)==0) {
			if(i != (upper_inode->filesize/32)-1) {
				fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE*upper_inode->block_id[0] + 32 * (upper_inode->filesize/32 -1), delete_dire, sizeof(blkdire_t));
				//kmt->spin_lock(&(fs->fs_lock));

				fs->dev->ops->write(fs->dev, BLOCK_OFF + BLOCK_SIZE*upper_inode->block_id[0] + i*32, delete_dire, sizeof(blkdire_t));
				//kmt->spin_unlock(&(fs->fs_lock));

			}		
			upper_inode->filesize -=32;
			break;
		}
	
	}

	//kmt->spin_lock(&(fs->fs_lock));

	fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*node2->id, upper_inode, INODE_SIZE);

	delete_inode->refcnt--;
	if(delete_inode->refcnt>0) {
		fs->dev->ops->write(fs->dev, INODE_OFF + INODE_SIZE*node1->id, delete_inode, INODE_SIZE);
	} else {
		char inode_bitmap[MAX_INODE_NUM], block_bitmap[MAX_BLOCK_NUM];
		fs->dev->ops->read(fs->dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
		fs->dev->ops->read(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);

		inode_bitmap[node1->id] = 0;
		if(delete_inode->filesize ==0)
			block_bitmap[delete_inode->block_id[0]] = 0;
		else
			for(int i=0; i<=( (delete_inode->filesize-1)/BLOCK_SIZE); i++) {
				block_bitmap[delete_inode->block_id[i]] = 0;
			}
		fs->dev->ops->write(fs->dev, INODE_BITMAP_OFF, inode_bitmap, MAX_INODE_NUM);
		fs->dev->ops->write(fs->dev, BLOCK_BITMAP_OFF, block_bitmap, MAX_BLOCK_NUM);
		pmm->free(node1);			
	}
	//kmt->spin_unlock(&(fs->fs_lock));

	return 0;
}

int blkfs_readdir(const char *path, void *buf, filesystem_t *fs) {
	int off = 0;
	char tmpbuf[200];
	blkinode tmp1; blkdire tmp2;
	blkinode *current_inode = &tmp1;
	blkdire *current_dire = &tmp2;

	inode_t *node = fs->ops->lookup(fs,path,0);
	if(node == NULL)
		return -1;
	assert(node->f_or_d == ISDIRE);

	fs->dev->ops->read(fs->dev, INODE_OFF + INODE_SIZE * (node->id), current_inode, INODE_SIZE);
	for(int i=2; i<(current_inode->filesize/32); i++) {
		fs->dev->ops->read(fs->dev, BLOCK_OFF + BLOCK_SIZE * (current_inode->block_id[0])+ i*32, current_dire, sizeof(blkdire_t));
		sprintf(tmpbuf, "%s\n", current_dire->filename);
		memcpy(buf+off, tmpbuf, sizeof(tmpbuf));
		off += sizeof(tmpbuf);
	}
	buf[off] = '\0';
	return 0;
}




inodeops_t blkinodeops = {
	.open = blkfs_inode_open,
	.close = blkfs_inode_close,
	.read = blkfs_inode_read,
	.write = blkfs_inode_write,
	.lseek = blkfs_inode_lseek,
	.mkdir = blkfs_inode_mkdir,
	.rmdir = blkfs_inode_rmdir,
	.link = blkfs_inode_link,
	.unlink = blkfs_inode_unlink,
	.readdir = blkfs_readdir,
};

fsops_t blkfsops = {
	.init = blkfs_init,
	.lookup = blkfs_lookup,
	.close = blkfs_close,
};

filesystem_t *create_blkfs(const char *fsname, device_t *dev) {
	assert(dev!=NULL);
	filesystem_t *fs = pmm->alloc(sizeof(filesystem_t));
	fs->ops = &blkfsops;
	fs->iops = &blkinodeops;
	fs->ops->init(fs, fsname, dev);
	return fs;
}

