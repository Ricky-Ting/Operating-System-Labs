#include <common.h>
#include <klib.h>


extern task_t* current_task[MAXCPU];
#define current (current_task[_cpu()])
filesystem_t *whichfs(const char *path) {

	//printf("In whichfs, %s\n", current->name);
	filesystem_t *ret = NULL;
	mnt_t* current_mnt = myvfs->mnthead;
	int current_max = 0;
	int len1,len2;
	len1 = strlen(path);
	//assert(current_mnt != NULL);
	//printf("In whichfs the path is %s\n",path);

  while(current_mnt!=NULL) {
		//printf("In whichfs current fs is %s\n",current_mnt->path);
		int iter = 0;
		len2 = strlen(current_mnt->path);
		while(iter<len1 && iter<len2 && path[iter]==current_mnt->path[iter])
			iter++;
		//printf("In whichfs iter = %d\n",iter);
		if( iter==len2 && iter>current_max ) {
			current_max = iter;
			ret = current_mnt->fs;
			assert(current_mnt->fs!=NULL);
		}	

		current_mnt = current_mnt->next;
	}	
	return ret;
}


void vfs_init() {
	myvfs = pmm->alloc(sizeof(vfilesystem_t));
	myvfs->mntcnt = 0;
	myvfs->mnthead = NULL;
	kmt->spin_init(&(myvfs->mnt_lock),"mount_lock");
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
		if(strlen(buf)==0) {
			buf[0] = '/';
			buf[1] = '\0';
		}
	} else {
		memcpy(buf, path, strlen(path));
		buf[strlen(path)] = '\0';
	}

	printf("In access buf is %s\n",buf);
	inode_t * p = fs->ops->lookup(fs, buf, 0);
	if(p!=NULL)
		return p->f_or_d;
	else
		return -1;
}


int vfs_mount(const char *path, filesystem_t *fs) {
	// Need mount LOCK
	kmt->spin_lock(&(myvfs->mnt_lock));
	mnt_t *new_mount = pmm->alloc(sizeof(mnt_t));
	assert(fs!=NULL);
	new_mount->fs = fs;
	strncpy(new_mount->path, path, MAXNAME);
	new_mount->prev = new_mount->next = NULL;
	
	if(myvfs->mnthead!=NULL) {
		myvfs->mnthead->prev = new_mount;
		new_mount->next = myvfs->mnthead;
	}
	sprintf(fs->mount_path, "%s",path);
	fs->mount_path[strlen(path)] = '\0';
	myvfs->mnthead = new_mount;
	kmt->spin_unlock(&(myvfs->mnt_lock));

	return 1;
	// Unlock	
}

int vfs_unmount(const char *path) {
	// Need mount LOCK
	kmt->spin_lock(&(myvfs->mnt_lock));
	mnt_t * iter = myvfs->mnthead;
	while(iter != NULL) {
		if(strncmp(iter->path, path, MAXNAME)==0) {
			if(iter->next!=NULL) {
				iter->next->prev = iter->prev;
			}
			if(iter->prev!=NULL) {
				iter->prev->next = iter->next;
			}
			if(iter == myvfs->mnthead) {
				myvfs->mnthead = iter->next;
			}
			iter->fs->mount_path[0] = '\0';
			pmm->free(iter);
	
			kmt->spin_unlock(&(myvfs->mnt_lock));
			return 0;
		}	
		iter = iter->next;
	}		
	kmt->spin_unlock(&(myvfs->mnt_lock));
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
	printf("In vfs_open, fd= %d, %d\n",iter, current->files[iter]->inode->fs);
	return iter;
}

ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
	if(fd<0 || fd>=NOFILE || current->files[fd]==NULL)
		return -1;	
	printf("In vfs_read, fd = %d\n",fd);
	
	printf("In vfs_read %d\n",current->files[fd]->inode->fs);
	int ret = current->files[fd]->inode->fs->iops->read(current->files[fd],buf,nbyte);
	return ret;

}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
	if(fd<0 || fd>=NOFILE || current->files[fd]==NULL)
		return -1;	
	printf("In vfs_write, fd =%d\n",fd);
	printf("In vfs_write %d\n",current->files[fd]->inode->fs);

	int ret = current->files[fd]->inode->fs->iops->write(current->files[fd],buf,nbyte);
	return ret;

}

off_t vfs_lseek(int fd, off_t offset, int whence) {
	if(fd<0 || fd>=NOFILE || current->files[fd]==NULL)
		return -1;	
	printf("In vfs_lseek, fd =%d\n",fd);
	printf("In vfs_lseek %d\n",current->files[fd]->inode->fs);

	int ret = current->files[fd]->inode->fs->iops->lseek(current->files[fd],offset,whence);
	printf("In vfs_lseek, fd =%d\n",fd);
	printf("In vfs_lseek %d\n",current->files[fd]->inode->fs);

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


int vfs_readdir(const char *path, void *buf) {
	filesystem_t* fs = whichfs(path); 
	if( fs == NULL ) {
		printf("No filesystem\n");
		return -1;
	}
	char tmpbuf[MAXNAME];
	int len = strlen(fs->mount_path);
	if(len!=1) {
		memcpy(tmpbuf, path+len, strlen(path)-len);
		tmpbuf[strlen(path)-len] = '\0';	
	} else {
		memcpy(tmpbuf, path, strlen(path));
		tmpbuf[strlen(path)] = '\0';
	}

	printf("In vfs_readdir fs name is %s\n",fs->fsname);
	return fs->iops->readdir(tmpbuf,buf, fs);
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
	.readdir = vfs_readdir,
};

