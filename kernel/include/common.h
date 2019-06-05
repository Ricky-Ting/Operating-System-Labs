#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
#include <trace.h>


/*BEGIN: FOR kmt*/

#define STACK_SIZE (4096*1024)
#define MAXCPU 32
typedef unsigned int uint;
#define TASK_READY 0
#define TASK_RUNNING 1
#define NOFILE 1024
#define TASK_SLEEP 2
#define MAXQ 20000
struct task {
	const char *name;
	_Context *context;
	int bind_cpu;
	int status; /* 0: ready  1: running 2: sleep*/
	uint8_t fence1[32];
	void * stack;
	uint8_t fence2[32];
	file_t *files[NOFILE];
	struct task * prev;
	struct task * next;	
};
struct spinlock {
	int locked; // Is the lock held?
	
	// For debugging:
	const char *name;				//Name of the lock
	int cpu;	// The cpu holding the lock;
	uint pcs[10]; 		// The call stack (an array of program counters)
										// that locked the lock.
	
};
struct semaphore {
	spinlock_t lock;
	int count;
	const char *name;
	task_t* queue[MAXQ];		
	int head;
	int tail;
};


struct handler_node {
	int seq;
	int event;
	handler_t handler;	
	struct handler_node* next;
};

/*END: FOR kmt*/



/*BEGIN: FOR vfs*/
#define MAXNAME 1024

struct vfilesystem{
	int mntcnt;				// the counter for mount point
	mnt_t* mnthead;
	spinlock_t mnt_lock;
};
vfilesystem_t *myvfs;

struct mnt {
	char path[MAXNAME];
	filesystem_t *fs;
	mnt_t *prev;
	mnt_t *next;
};

struct filesystem{
	char fsname[MAXNAME];
	char mount_path[MAXNAME];
	fsops_t *ops;
	device_t *dev;
	inodeops_t *iops;
	spinlock_t fs_lock;
};

struct fsops{
	void (*init)(struct filesystem *fs, const char *name, device_t *dev);
	inode_t *(*lookup)(struct filesystem *fs, const char *path, int flags);
	int (*close)(inode_t *inode);
};

struct inodeops{
	int (*open)(const char *name, file_t *file, int flags, filesystem_t *fs);
	int (*close)(file_t *file);
	ssize_t (*read)(file_t *file, char *buf, size_t size);
	ssize_t (*write)(file_t *file, const char *buf, size_t size);
	off_t (*lseek)(file_t *file, off_t offset, int whence);
	int (*mkdir)(const char *name, filesystem_t *fs);
	int (*rmdir)(const char *name, filesystem_t *fs);
	int (*link)(const char *name, inode_t *inode, filesystem_t *fs);
	int (*unlink)(const char *name, filesystem_t *fs);
};


struct inode{
	int refcnt;			// 有多少个file_t 指向inode，为0时free inode
	int id; 			//在磁盘上的inode编号
	//void *ptr;
	
	filesystem_t *fs;
	inodeops_t *ops;
};

struct file{
	int refcnt;
	inode_t *inode;
	uint64_t offset;
};

filesystem_t *create_blkfs(const char *fsname, device_t *dev);

/*END: FOR vfs*/


#endif
