/*
 * Copyright (c) 1998-2015 Erez Zadok
 * Copyright (c) 2009	   Shrikar Archak
 * Copyright (c) 2003-2015 Stony Brook University
 * Copyright (c) 2003-2015 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _TRFS_H_
#define _TRFS_H_

#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/seq_file.h>
#include <linux/statfs.h>
#include <linux/fs_stack.h>
#include <linux/magic.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/xattr.h>
#include <linux/exportfs.h>
#include "record.h"
#define TRFS_PAGE_SIZE 4096
/* the file system name */
#define TRFS_NAME "trfs"

//#define EXTRA_CREDIT

/* trfs root inode number */
#define TRFS_ROOT_INO     1

/* useful for tracking code reachability */
#define UDBG printk(KERN_DEFAULT "DBG:%s:%s:%d\n", __FILE__, __func__, __LINE__)

//#define RECORD_CHECKSUM_SIZE 4
/* operations vectors defined in specific files */
extern const struct file_operations trfs_main_fops;
extern const struct file_operations trfs_dir_fops;
extern const struct inode_operations trfs_main_iops;
extern const struct inode_operations trfs_dir_iops;
extern const struct inode_operations trfs_symlink_iops;
extern const struct super_operations trfs_sops;
extern const struct dentry_operations trfs_dops;
extern const struct address_space_operations trfs_aops, trfs_dummy_aops;
extern const struct vm_operations_struct trfs_vm_ops;
extern const struct export_operations trfs_export_ops;

extern int trfs_init_inode_cache(void);
extern void trfs_destroy_inode_cache(void);
extern int trfs_init_dentry_cache(void);
extern void trfs_destroy_dentry_cache(void);
extern int new_dentry_private_data(struct dentry *dentry);
extern void free_dentry_private_data(struct dentry *dentry);
extern struct dentry *trfs_lookup(struct inode *dir, struct dentry *dentry,
		unsigned int flags);
extern struct inode *trfs_iget(struct super_block *sb,
		struct inode *lower_inode);
extern int trfs_interpose(struct dentry *dentry, struct super_block *sb,
		struct path *lower_path);
/*
typedef enum {TRFS_OP_CREATE,
	TRFS_OP_OPEN,
	TRFS_OP_CLOSE,
	TRFS_OP_MKDIR,
	TRFS_OP_RMDIR,
	TRFS_OP_UNLINK,
	
	
	TRFS_OP_READ,
	TRFS_OP_WRITE,
	TRFS_OP_SYMLINK,

	TRFS_OP_HARDLINK,
	TRFS_OP_RENAME} trfs_operation;
*/
struct trfs_list_inode{
	struct inode* inode;
	struct trfs_list_inode* next;
};

struct trfs_list_dentry{
	struct dentry* dentry;
	struct trfs_list_dentry* next; 
};
struct trfs_record_struct{
	int record_no;
	unsigned int flags;
	struct trfs_list_inode* trfs_list_inode;
	struct trfs_list_dentry* trfs_list_dentry;
	trfs_operation type;
	int success_flag;
	struct file* k_fd;
	void* data;

};
extern struct task_struct *task;


#ifdef EXTRA_CREDIT
struct toggle_ops
{
    int add_bits;
    int rem_bits;
};


#endif

bool is_enabled(unsigned long bitmap, trfs_operation type);
/*
struct trfs_record_struct_file{ //NEW_TRFS
        int record_no; //4bytes
        short int buf_size; //2 bytes 
	trfs_operation type;
        unsigned int flags;
        unsigned int permission_mode;
        short int path_len;
	int success_flag;
	struct file*  k_fd;
	char path_name[1];
	
};
*/
//AAAAAAAAAAAAAA extern struct trfs_record_struct *trfs_record_struct;//kmalloc(sizeof(struct trfs_record_struct), GFP_KERNEL);
extern char* write_file_buff;
int trfs_append_to_list(void* node, int x, struct trfs_record_struct *trfs_record_struct);
int trfs_add_record(struct trfs_record_struct *trfs_record_struct);
struct trfs_record_struct* get_trfs_record_struct(trfs_operation type);
extern int vacant;
/* file private data */
struct trfs_file_info {
	struct file *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
};

/* trfs inode data in memory */
struct trfs_inode_info {
	struct inode *lower_inode;
	struct inode vfs_inode;
};

/* trfs dentry data in memory */
struct trfs_dentry_info {
	spinlock_t lock;	/* protects lower_path */
	struct path lower_path;
};

/* trfs super-block data in memory */
struct trfs_sb_info {
	struct super_block *lower_sb;
	struct file *out_fd;
        unsigned int bitmap;
	struct mutex trfs_record_lock;
};

/*
 * inode to private data
 *
 * Since we use containers and the struct inode is _inside_ the
 * trfs_inode_info structure, TRFS_I will always (given a non-NULL
 * inode pointer), return a valid non-NULL pointer.
 */
static inline struct trfs_inode_info *TRFS_I(const struct inode *inode)
{
	return container_of(inode, struct trfs_inode_info, vfs_inode);
}

/* dentry to private data */
#define TRFS_D(dent) ((struct trfs_dentry_info *)(dent)->d_fsdata)

/* superblock to private data */
#define TRFS_SB(super) ((struct trfs_sb_info *)(super)->s_fs_info)

/* file to private Data */
#define TRFS_F(file) ((struct trfs_file_info *)((file)->private_data))

/* file to lower file */
static inline struct file *trfs_lower_file(const struct file *f)
{
	return TRFS_F(f)->lower_file;
}

static inline void trfs_set_lower_file(struct file *f, struct file *val)
{
	TRFS_F(f)->lower_file = val;
}

/* inode to lower inode. */
static inline struct inode *trfs_lower_inode(const struct inode *i)
{
	return TRFS_I(i)->lower_inode;
}

static inline void trfs_set_lower_inode(struct inode *i, struct inode *val)
{
	TRFS_I(i)->lower_inode = val;
}

/* superblock to lower superblock */
static inline struct super_block *trfs_lower_super(
		const struct super_block *sb)
{
	return TRFS_SB(sb)->lower_sb;
}

static inline void trfs_set_lower_super(struct super_block *sb,
		struct super_block *val)
{
	TRFS_SB(sb)->lower_sb = val;
}

/* path based (dentry/mnt) macros */
static inline void pathcpy(struct path *dst, const struct path *src)
{
	dst->dentry = src->dentry;
	dst->mnt = src->mnt;
}
/* Returns struct path.  Caller must path_put it. */
static inline void trfs_get_lower_path(const struct dentry *dent,
		struct path *lower_path)
{
	spin_lock(&TRFS_D(dent)->lock);
	pathcpy(lower_path, &TRFS_D(dent)->lower_path);
	path_get(lower_path);
	spin_unlock(&TRFS_D(dent)->lock);
	return;
}
static inline void trfs_put_lower_path(const struct dentry *dent,
		struct path *lower_path)
{
	path_put(lower_path);
	return;
}
static inline void trfs_set_lower_path(const struct dentry *dent,
		struct path *lower_path)
{
	spin_lock(&TRFS_D(dent)->lock);
	pathcpy(&TRFS_D(dent)->lower_path, lower_path);
	spin_unlock(&TRFS_D(dent)->lock);
	return;
}
static inline void trfs_reset_lower_path(const struct dentry *dent)
{
	spin_lock(&TRFS_D(dent)->lock);
	TRFS_D(dent)->lower_path.dentry = NULL;
	TRFS_D(dent)->lower_path.mnt = NULL;
	spin_unlock(&TRFS_D(dent)->lock);
	return;
}
static inline void trfs_put_reset_lower_path(const struct dentry *dent)
{
	struct path lower_path;
	spin_lock(&TRFS_D(dent)->lock);
	pathcpy(&lower_path, &TRFS_D(dent)->lower_path);
	TRFS_D(dent)->lower_path.dentry = NULL;
	TRFS_D(dent)->lower_path.mnt = NULL;
	spin_unlock(&TRFS_D(dent)->lock);
	path_put(&lower_path);
	return;
}

/* locking helpers */
static inline struct dentry *lock_parent(struct dentry *dentry)
{
	struct dentry *dir = dget_parent(dentry);
	inode_lock_nested(d_inode(dir), I_MUTEX_PARENT);
	return dir;
}

static inline void unlock_dir(struct dentry *dir)
{
	inode_unlock(d_inode(dir));
	dput(dir);
}
#endif	/* not _TRFS_H_ */
