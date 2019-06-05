#include <common.h>
#include <klib.h>


extern task_t* current_task[MAXCPU];
#define current (current_task[_cpu()])
filesystem_t *whichfs(const char *path) {
	filesystem_t *ret = NULL;
	mnt_t* current_mnt = vfilesystem.mnthead;
	int current_max = 0;
	int len1,len2;
	len1 = strlen(path);
	//assert(current_mnt != NULL);
	printf("In whichfs the path is %s\n",path);

  while(current_mnt!=NULL) {
		printf("In whichfs current fs is %s\n",current_mnt->path);
		int iter = 0;
		len2 = strlen(current_mnt->path);
		while(iter<len1 && iter<len2 && path[iter]==current_mnt->path[iter])
			iter++;
		printf("In whichfs iter = %d\n",iter);
		if( iter==len2 && iter>current_max ) {
			current_max = iter;
			ret = current_mnt->fs;
		}	

		current_mnt = current_mnt->next;
	}	
	return ret;
}


void vfs_init() {
	vfilesystem.mntcnt = 0;
	vfilesystem.mnthead = NULL;
}

int vfs_access(const char *path, int mode) {
	/* 
	 * 	check the existence of the file
	 */

	filesystem_t* fs = whichfs(path); 
	if( fs == NULL ) {
		printf("In vfs_access No fs matched\n");
		return -1;
	}

	char buf[MAXNAME];
	int len = strlen(fs->mount_path);
	if(len!=1) {
		memcpy(buf, path+len, strlen(path)-len);
		buf[strlen(path)-len] = '\0';	
	} else {
		memcpy(buf, path, strlen(path));
		buf[strlen(path)] = '\0';
	}

	inode_t * p = fs->ops->lookup(fs, buf, 0);
	if(p!=NULL)
		return 0;
	else
		return -1;
}


int vfs_mount(const char *path, filesystem_t *fs) {
	// Need mount LOCK
	mnt_t *new_mount = pmm->alloc(sizeof(sizeof(mnt_t)));
	new_mount->fs = fs;
	strncpy(new_mount->path, path, MAXNAME);
	new_mount->prev = new_mount->next = NULL;
	
	if(vfilesystem.mnthead!=NULL) {
		vfilesystem.mnthead->prev = new_mount;
		new_mount->next = vfilesystem.mnthead;
	}
	sprintf(fs->mount_path, "%s",path);
	fs->mount_path[strlen(path)] = '\0';
	vfilesystem.mnthead = new_mount;
	return 1;
	// Unlock	
}

int vfs_unmount(const char *path) {
	// Need mount LOCK
	mnt_t * iter = vfilesystem.mnthead;
	while(iter != NULL) {
		if(strncmp(iter->path, path, MAXNAME)==0) {
			if(iter->next!=NULL) {
				iter->next->prev = iter->prev;
			}
			if(iter->prev!=NULL) {
				iter->prev->next = iter->next;
			}
			if(iter == vfilesystem.mnthead) {
				vfilesystem.mnthead = iter->next;
			}
			iter->fs->mount_path[0] = '\0';
			pmm->free(iter);
			return 0;
		}	
		iter = iter->next;
	}		
	return -1;
}

int vfs_mkdir(const char *path) {
	filesystem_t* fs = whichfs(path); 
	if( fs == NULL ) {
		return -1;
	}

	char buf[MAXNAME];
	int len = strlen(fs->mount_path);
	if(len!=1) {
		memcpy(buf, path+len, strlen(path)-len);
		buf[strlen(path)-len] = '\0';	
	} else {
		memcpy(buf, path, strlen(path));
		buf[strlen(path)] = '\0';
	}

	int ret = fs->iops->mkdir(buf,fs);
	return ret;

}

int vfs_rmdir(const char *path) {
	filesystem_t* fs = whichfs(path); 
	if( fs == NULL ) {
		return -1;
	}

	char buf[MAXNAME];
	int len = strlen(fs->mount_path);
	if(len!=1) {
		memcpy(buf, path+len, strlen(path)-len);
		buf[strlen(path)-len] = '\0';	
	} else {
		memcpy(buf, path, strlen(path));
		buf[strlen(path)] = '\0';
	}

	int ret = fs->iops->rmdir(buf,fs);
	return ret;

}

int vfs_link(const char *oldpath, const char *newpath) {
	filesystem_t* fs1 = whichfs(oldpath);
	filesystem_t* fs2 = whichfs(newpath);
	if(fs1==NULL)
		return -1;
	if(fs1!=fs2)
		return -1;
	char buf1[MAXNAME], buf2[MAXNAME];
	int len = strlen(fs1->mount_path);
	if(len!=1) {
		memcpy(buf1, oldpath+len, strlen(oldpath)-len);
		memcpy(buf2, newpath+len, strlen(newpath)-len);
		buf1[strlen(oldpath)-len] = '\0';	
		buf2[strlen(newpath)-len] = '\0';	
	} else {
		memcpy(buf1, oldpath, strlen(oldpath));
		buf1[strlen(oldpath)] = '\0';
		memcpy(buf2, newpath, strlen(newpath));
		buf2[strlen(newpath)] = '\0';

	}


	inode_t * node = fs1->ops->lookup(fs1, buf1, 0);	
	inode_t * node2 = fs2->ops->lookup(fs2,buf2, 0);
	if(node2!=NULL)
		return -1;
	int ret = fs1->iops->link(buf2, node, fs1);
	return ret;
}	

int vfs_unlink(const char *path) {
	filesystem_t* fs = whichfs(path); 
	if( fs == NULL ) {
		return -1;
	}

	char buf[MAXNAME];
	int len = strlen(fs->mount_path);
	if(len!=1) {
		memcpy(buf, path+len, strlen(path)-len);
		buf[strlen(path)-len] = '\0';	
	} else {
		memcpy(buf, path, strlen(path));
		buf[strlen(path)] = '\0';
	}


	int ret = fs->iops->unlink(buf,fs);
	return ret;


}

int vfs_open(const char *path, int flags) {
	filesystem_t* fs = whichfs(path); 
	if( fs == NULL ) {
		printf("No filesystem\n");
		return -1;
	}
	char buf[MAXNAME];
	int len = strlen(fs->mount_path);
	if(len!=1) {
		memcpy(buf, path+len, strlen(path)-len);
		buf[strlen(path)-len] = '\0';	
	} else {
		memcpy(buf, path, strlen(path));
		buf[strlen(path)] = '\0';
	}		
	
	file_t *open_file = pmm->alloc(sizeof(file_t));
	int ret = fs->iops->open(buf, open_file, flags, fs);
	if(ret != 0)
		return -1;

	int iter = 0;
	while(iter<NOFILE && current->files[iter]!=NULL)
		iter++;
	assert(iter<NOFILE);
	current->files[iter] = open_file;
	return iter;
}

ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
	if(fd<0 || fd>=NOFILE || current->files[fd]==NULL)
		return -1;	
	int ret = current->files[fd]->inode->fs->iops->read(current->files[fd],buf,nbyte);
	return ret;

}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
	if(fd<0 || fd>=NOFILE || current->files[fd]==NULL)
		return -1;	
	int ret = current->files[fd]->inode->fs->iops->write(current->files[fd],buf,nbyte);
	return ret;

}

off_t vfs_lseek(int fd, off_t offset, int whence) {
	if(fd<0 || fd>=NOFILE || current->files[fd]==NULL)
		return -1;	
	int ret = current->files[fd]->inode->fs->iops->lseek(current->files[fd],offset,whence);
	return ret;


}

int vfs_close(int fd) {
	if(fd<0 || fd>=NOFILE)
		return -1;
	if(current->files[fd]==NULL)
		return 0;
	//filesystem *fs = current->file[fd]->inode->fs;
	//fs->iops->close(current->file[fd])	
	current->files[fd]->refcnt--;
	if(current->files[fd]->refcnt>0)
		return 0;
	else {
		current->files[fd]->inode->refcnt--;
		pmm->free(current->files[fd]);
		current->files[fd] = NULL;
		return 0;
	}
	
}






















MODULE_DEF(vfs) {
	.init = vfs_init,
	.access = vfs_access,
	.mount = vfs_mount,
	.unmount = vfs_unmount,
	.mkdir = vfs_mkdir,
	.rmdir = vfs_rmdir,
	.link = vfs_link,
	.unlink = vfs_unlink,
	.open = vfs_open,
	.read = vfs_read,
	.write = vfs_write,
	.lseek = vfs_lseek,
	.close = vfs_close,
};

