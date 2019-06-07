#include <common.h>
#include <devices.h>


#define MAXTHREAD 1024

inode_t proc_inodes[MAXTHREAD];

extern int pid_counter;
extern task_t* task_head[MAXCPU];

void procfs_init(filesystem_t *fs, const char *name, device_t *dev){
	strncpy(fs->fsname,name,MAXNAME);
	return ;
}


inode_t *procfs_lookup(struct filesystem *fs, const char *path, int flags) {
	int len = strlen(path);
	char buf[MAXNAME];
	memcpy(buf,path+1, len-1);
	buf[len-1] = '\0';
	int pid = -1;
	//sscanf(path, "%d",&pid);
	int it = len-2;
	int ans = 0;
	int jie = 1;
	while(it>=0) {
		if(buf[it] - '0'<0 || buf[it] - '0'>9)
			return NULL;
		ans += jie * (buf[it] - '0');
		jie *= 10;
		it--;
	}
	pid = ans;

	if(pid<0 || pid>=MAXTHREAD || pid>=pid_counter)
		return NULL;
	proc_inodes[pid].refcnt = 1;
	proc_inodes[pid].proc_cnt = pid;
	proc_inodes[pid].fs = fs;
	proc_inodes[pid].ops = fs->iops;

	task_t *iter = task_head[pid%_ncpu()];
	while(iter!=NULL && iter->pid!=pid)
		iter = iter->next;
	assert(iter!=NULL);
	proc_inodes[pid].task = iter;
	return &(proc_inodes[pid]);
}


int procfs_open(const char *name, file_t *file, int flags, filesystem_t *fs) {
	inode_t *node = fs->ops->lookup(fs, name, flags);
	if(node==NULL)
		return -1;
	file->offset = 0;
	file->refcnt = 1;
	file->inode = node;
	return 0;		
}

int	procfs_close(file_t *file) {
	return 0;
}


ssize_t procfs_read(file_t *file, char *buf, size_t size) {
	char mybuf[2048];
	task_t *task = file->inode->task;
	sprintf(mybuf,"task name: %s\ntotal cpu: %d\ncurrent cpu: %d\nstack start: %d\nstack_end: %d\nstack_size: %d\n", task->name, _ncpu(), task->bind_cpu, task->stack, (int)(task->stack) + STACK_SIZE, STACK_SIZE );


	int read_start = file->offset;
	int read_end = file->offset + size;
	if(read_end>strlen(mybuf))
		read_end = strlen(mybuf);

	memcpy(buf, mybuf+read_start, read_end-read_start);
	file->offset += (read_end - read_start);
	return read_end - read_start;		
}


ssize_t procfs_write(file_t *file, const char *buf, size_t size) {
	panic("procfs dont't support write");
	return -1;		
}

off_t procfs_lseek(file_t *file, off_t offset, int whence) {

	char mybuf[2048];
	task_t *task = file->inode->task;
	sprintf(mybuf,"task name: %s\ntotal cpu: %d\ncurrent cpu: %d\nstack start: %d\nstack_end: %d\nstack_size: %d\n", task->name, _ncpu(), task->bind_cpu, task->stack, (int)(task->stack) + STACK_SIZE, STACK_SIZE );
	

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
	
	if(new_offset<0 || new_offset>strlen(mybuf))
		return -1;
	file->offset = new_offset;
	return file->offset;
}


int procfs_mkdir(const char *name, filesystem_t *fs) {
	panic("procfs don't support mkdir");	
	return -1;
}

int procfs_rmdir(const char *name, filesystem_t *fs) {
	panic("procfs don't support rmdir");
	return -1;
}


int procfs_link(const char *name, inode_t *inode, filesystem_t *fs) {
	panic("procfs don't support link");
	return -1;
}


int procfs_unlink(const char *name, filesystem_t *fs) {

	panic("procfs don't support unlink");
	return -1;
}

inodeops_t procinodeops = {
	.open = procfs_open,
	.close = procfs_close,
	.read = procfs_read,
	.write = procfs_write,
	.lseek = procfs_lseek,
	.mkdir = procfs_mkdir,
	.rmdir = procfs_rmdir,
	.link = procfs_link,
	.unlink = procfs_unlink,
};


fsops_t procfsops = {
	.init = procfs_init,
	.lookup = procfs_lookup,
};




filesystem_t *create_procfs() {
	
	filesystem_t *fs = pmm->alloc(sizeof(filesystem_t));
	fs->dev = NULL;
	fs->ops = &procfsops;
	fs->iops = &procinodeops;	

	fs->ops->init(fs,"procfs",NULL);
	return fs;
}


