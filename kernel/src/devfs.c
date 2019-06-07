#include <common.h>
#include <devices.h>



typedef struct devfs_inode {
	char dev_name[MAXNAME];
	inode_t inode;
	device_t *dev;
} devfs_inode_t;


#define NUM_OF_DEVS 6

#define DEVFS_RAMDISK0 0
#define DEVFS_RAMDISK1 1
#define DEVFS_TTY1 2
#define DEVFS_TTY2 3
#define DEVFS_TTY3 4
#define DEVFS_TTY4 5


char* devfs_name[NUM_OF_DEVS] = {"/ramdisk0","/ramdisk1","/tty1", "/tty2", "/tty3", "/tty4"};

devfs_inode_t	devfs_inodes[6]; 



void devfs_init(filesystem_t *fs, const char *name, device_t *dev){
	for(int i=0;i<NUM_OF_DEVS; i++) {
		strncpy(devfs_inodes[i].dev_name, devfs_name[i], MAXNAME);
		devfs_inodes[i].inode.refcnt = 0;
		devfs_inodes[i].inode.fs = fs;
		devfs_inodes[i].inode.ops = fs->iops;	
		devfs_inodes[i].dev = dev_lookup(devfs_name[i]);
	}
	strncpy(fs->fsname,name,MAXNAME);
	return ;
}


inode_t *devfs_lookup(struct filesystem *fs, const char *path, int flags) {
	for(int i=0; i<NUM_OF_DEVS; i++) {
		if(strcmp(path, devfs_name[i])==0) {
			return &devfs_inodes[i].inode;
		}
	} 
	return NULL;
}


int devfs_open(const char *name, file_t *file, int flags, filesystem_t *fs) {
	inode_t *node = fs->ops->lookup(fs, name, flags);
	if(node==NULL)
		return -1;
	file->offset = 0;
	file->refcnt = 1;
	file->inode = node;
	return 0;		
}

int	devfs_close(file_t *file) {
	return 0;
}


ssize_t devfs_read(file_t *file, char *buf, size_t size) {
	device_t *dev = devfs_inodes[file->inode->devfs_cnt].dev;
	int ret = dev->ops->read(dev, file->offset, buf, size);
	file->offset += ret;
	return ret;		
}


ssize_t devfs_write(file_t *file, const char *buf, size_t size) {
	device_t *dev = devfs_inodes[file->inode->devfs_cnt].dev;
	int ret = dev->ops->write(dev, file->offset, buf, size);
	file->offset += ret;
	return ret;		
}

off_t devfs_lseek(file_t *file, off_t offset, int whence) {

	int new_offset;
	if(whence == SEEK_SET) {
		new_offset = offset;
	}	else if(whence == SEEK_CUR) {
		new_offset = file->offset + offset;
	} else if(whence == SEEK_END) {
		panic("devfs don't support seek_end");
		//assert(0);
	} else {
		return -1;
	}
	
	if(new_offset<0 )
		return -1;
	file->offset = new_offset;
	return file->offset;
}


int devfs_mkdir(const char *name, filesystem_t *fs) {
	panic("devfs don't support mkdir");	
	return -1;
}

int devfs_rmdir(const char *name, filesystem_t *fs) {
	panic("devfs don't support rmdir");
	return -1;
}


int devfs_link(const char *name, inode_t *inode, filesystem_t *fs) {
	panic("devfs don't support link");
	return -1;
}


int devfs_unlink(const char *name, filesystem_t *fs) {

	panic("devfs don't support unlink");
	return -1;
}




inodeops_t devinodeops = {
	.open = devfs_open,
	.close = devfs_close,
	.read = devfs_read,
	.write = devfs_write,
	.lseek = devfs_lseek,
	.mkdir = devfs_mkdir,
	.rmdir = devfs_rmdir,
	.link = devfs_link,
	.unlink = devfs_unlink,
};


fsops_t devfsops = {
	.init = devfs_init,
	.lookup = devfs_lookup,
};




filesystem_t *create_devfs() {
	
	filesystem_t *fs = pmm->alloc(sizeof(filesystem_t));
	fs->dev = NULL;
	fs->ops = &devfsops;
	fs->iops = &devinodeops;	

	fs->ops->init(fs,"devfs",NULL);
	return fs;
}


