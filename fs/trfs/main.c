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
#include "trfs.h"
#include <linux/module.h>
#include <linux/crc32.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

char* write_file_buff = NULL;
int vacant = 0;
int parse_raw_data(char *raw_data);
char *parsed_file_path = NULL;
//struct trfs_record_struct *trfs_record_struct= NULL; AAAAAAAAAAAAAAA
u32 trfs_checksum(char *check_buff);
bool is_enabled(unsigned long bitmap, trfs_operation type);
void write_to_file(struct file* myfile, char* buf, int size);
struct task_struct *task = NULL;

int thread_function(void *write_file)
{
        while(!kthread_should_stop())
        {
                if (write_file != NULL && write_file_buff != NULL)      {
                        write_to_file((struct file *)write_file, write_file_buff, TRFS_PAGE_SIZE - vacant);
                        write_file_buff[0]='\0';
                        vacant = TRFS_PAGE_SIZE ;
                }
                ssleep(3);
        }
        return 0;
}


bool is_enabled(unsigned long bitmap, trfs_operation type){
	//printk("-------> Bitmap value: %x\n",bitmap);
	switch ( type ) {
                case TRFS_OP_CREATE:
                        if(bitmap & (1 << TRFS_OP_CREATE ))
                                return true;
                        break;
                case TRFS_OP_OPEN:
                        if(bitmap & (1 << TRFS_OP_OPEN ))
                                return true;
                        break;
                case TRFS_OP_CLOSE:
                         if(bitmap & (1 << TRFS_OP_CLOSE  ))
                                return true;
                         break;
                case TRFS_OP_MKDIR:
                        if(bitmap & (1 << TRFS_OP_MKDIR ))
                                return true;
                        break;
		case TRFS_OP_RMDIR:
			if(bitmap & (1 << TRFS_OP_RMDIR))
				return true;
			break;
                case TRFS_OP_UNLINK:
                        if(bitmap & (1 << TRFS_OP_UNLINK))
                                return true;
			break;
                case TRFS_OP_READ:
                        if(bitmap & (1 << TRFS_OP_READ))
                                return true;
			break;
                case TRFS_OP_WRITE:
                        if(bitmap & (1 << TRFS_OP_WRITE))
                                return true;
			break;
                case TRFS_OP_SYMLINK:
                        if(bitmap & (1 << TRFS_OP_SYMLINK))
                                return true;
			break;
                case TRFS_OP_HARDLINK:
                        if(bitmap & (1 << TRFS_OP_HARDLINK))
                                return true;
			break;
                case TRFS_OP_RENAME:
                        if(bitmap & (1 << TRFS_OP_RENAME))
                                return true;
			break;
		
                }
		printk("NOT TRACING %d", type);
		return false;
}

EXPORT_SYMBOL(is_enabled);

struct trfs_record_struct* get_trfs_record_struct(trfs_operation type){
	struct trfs_record_struct* trfs_record_struct = NULL;	
	trfs_record_struct = (struct trfs_record_struct*)kmalloc(sizeof(struct trfs_record_struct), GFP_KERNEL);
        if(trfs_record_struct == NULL){
                printk("\nUnable to assign memory to record");
        }
        trfs_record_struct->type = -1;
        trfs_record_struct->trfs_list_inode = NULL;
        trfs_record_struct->trfs_list_dentry = NULL;
        trfs_record_struct->data = NULL;        
	
	return trfs_record_struct;
}

EXPORT_SYMBOL(get_trfs_record_struct);

int trfs_append_to_list(void* node, int x,struct trfs_record_struct* trfs_record_struct){
	if(trfs_record_struct == NULL)
		return -EINVAL;
	switch(x){
		case 0:
			//printk("\nADDING NODE TO INODE LIST");
			if(trfs_record_struct->trfs_list_inode == NULL){
				trfs_record_struct->trfs_list_inode = kmalloc(sizeof(struct trfs_list_inode), GFP_KERNEL);
				if(trfs_record_struct->trfs_list_inode == NULL){
					return -ENOMEM;
				}
				trfs_record_struct->trfs_list_inode->inode = (struct inode*)node;
				trfs_record_struct->trfs_list_inode->next = NULL;
			}	
			else{
				trfs_record_struct->trfs_list_inode->next = kmalloc(sizeof(struct trfs_list_inode), GFP_KERNEL);
				if(trfs_record_struct->trfs_list_inode->next == NULL){
					return -ENOMEM;
				}
				trfs_record_struct->trfs_list_inode->next->inode = (struct inode*)node;
				trfs_record_struct->trfs_list_inode->next->next = NULL;
			}
			break;
		case 1:
			//printk("\nADDING NODE TO DENTRY LIST");
			if(trfs_record_struct->trfs_list_dentry == NULL){
				trfs_record_struct->trfs_list_dentry = kmalloc(sizeof(struct trfs_list_dentry), GFP_KERNEL);
				if(trfs_record_struct->trfs_list_dentry == NULL){
					return -ENOMEM;
				}
				trfs_record_struct->trfs_list_dentry->dentry = (struct dentry*)node;
				trfs_record_struct->trfs_list_dentry->next = NULL;
			}
			else{
				trfs_record_struct->trfs_list_dentry->next = kmalloc(sizeof(struct trfs_list_dentry), GFP_KERNEL);
				if(trfs_record_struct->trfs_list_dentry->next == NULL){
					return -ENOMEM;
				}
				trfs_record_struct->trfs_list_dentry->next->dentry = (struct dentry*)node;
				trfs_record_struct->trfs_list_dentry->next->next = NULL;
			}
			break;
	}
	return 0;
}
void write_to_file(struct file* myfile, char* buf, int size){
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	vfs_write(myfile, buf, size, &myfile->f_pos);
	set_fs(oldfs);
}	
struct trfs_record_struct_file* get_file_writable_record_helper(struct trfs_record_struct* trfs_record_struct, int buf_size){
	struct trfs_record_struct_file *trfs_record_struct_file = NULL;
	char* pathname = NULL, *data = NULL;
	trfs_record_struct_file = (struct trfs_record_struct_file *)kmalloc(buf_size,GFP_KERNEL);//TODO
	if(trfs_record_struct_file == NULL)
		return NULL;
	pathname = kmalloc(PAGE_SIZE, GFP_KERNEL);
	data = dentry_path_raw(trfs_record_struct->trfs_list_dentry->dentry, pathname, PAGE_SIZE);
	trfs_record_struct_file->buf_size = buf_size;
	//printk("\nCHECK 11 %d", trfs_record_struct_file->buf_size);
	trfs_record_struct_file->record_no = trfs_record_struct->record_no;
	//printk("trfs_record_struct_file----> ", trfs_record_struct_file->path_len );
	trfs_record_struct_file->path_len = strlen(data);
	//printk("\ntrfs_record_struct_file----> %d", trfs_record_struct_file->path_len );
	trfs_record_struct_file->type = trfs_record_struct->type;
	trfs_record_struct_file->success_flag = trfs_record_struct->success_flag;
	trfs_record_struct_file->permission_mode = trfs_record_struct->trfs_list_dentry->dentry->d_inode->i_mode;
	trfs_record_struct_file->flags = trfs_record_struct->flags;
	trfs_record_struct_file->k_fd = trfs_record_struct->k_fd;
	strcpy(trfs_record_struct_file->path_name, data);
	trfs_record_struct_file->path_name[trfs_record_struct_file->path_len] = '\0';
	if(pathname)
		kfree(pathname);
	return trfs_record_struct_file;

	//printk("\nNAME ----> %s", trfs_record_struct_file->path_name);	
}

struct trfs_record_struct_file* get_file_writable_record(struct trfs_record_struct* trfs_record_struct){
	struct trfs_record_struct_file *trfs_record_struct_file = NULL;
	int buf_size = 0; //SIZE OF THIS RECORD
	char* data = NULL, *pathname = NULL, *pathname2 = NULL, *data2 = NULL;
	pathname = kmalloc(PAGE_SIZE, GFP_KERNEL);
        pathname2 = kmalloc(PAGE_SIZE, GFP_KERNEL);
	data = dentry_path_raw(trfs_record_struct->trfs_list_dentry->dentry, pathname, PAGE_SIZE);
	

	switch(trfs_record_struct->type)
	{	
		case TRFS_OP_CREATE:
		case TRFS_OP_OPEN:
		case TRFS_OP_CLOSE:
		case TRFS_OP_MKDIR:
		case TRFS_OP_RMDIR:
		case TRFS_OP_UNLINK:
			buf_size = sizeof(struct trfs_record_struct_file)
				+ strlen(data)
				+ ((trfs_record_struct->data != NULL) ? strlen(trfs_record_struct->data)+1 : 0) + 1;

			//	printk("\nsizeof(struct trfs_record_struct_file) -----> %d", sizeof(struct trfs_record_struct_file));
			//	printk("\nstrlen(data) ----> %d", strlen(data));
			//	printk("\n buf_size ----> %d", buf_size);
			trfs_record_struct_file =  get_file_writable_record_helper(trfs_record_struct, buf_size);		
			if(trfs_record_struct_file == NULL){
				printk("\n NO MEMORY FOUND");
				return NULL;
			}		
			break;
		case TRFS_OP_READ:
		case TRFS_OP_WRITE:
		case TRFS_OP_SYMLINK:

			buf_size = sizeof(struct trfs_record_struct_file)
				+ strlen(data)
				+ ((trfs_record_struct->data != NULL) ? strlen(trfs_record_struct->data)+1 : 0) + 1;
			trfs_record_struct_file =  get_file_writable_record_helper(trfs_record_struct, buf_size);
			if(trfs_record_struct_file == NULL){
				printk("\n NO MEMORY FOUND");
				return NULL;
			}
			//	printk("\nAFTER ADDING SPACE ---------> PATH LEN %d", trfs_record_struct_file->path_len);

			//	printk("\n TRFS_RECORD_STRUCT_FILE->PATH_NAME %s", trfs_record_struct_file->path_name);
			trfs_record_struct_file->path_name[trfs_record_struct_file->path_len] = ' ';
			//	printk("\n TRFS_RECORD_STRUCT->DATA %s",(char*)trfs_record_struct->data);

			//printk("\n TRFS_RECORD_STRUCT_FILE->PATH_NAME %s", trfs_record_struct_file->path_name);
			if(trfs_record_struct->data == NULL)
				printk("\nSOMETHING IS WRONG");

			strcpy(trfs_record_struct_file->path_name + trfs_record_struct_file->path_len + 1, trfs_record_struct->data);
			break;


		case TRFS_OP_HARDLINK:
		case TRFS_OP_RENAME:
			memset(pathname2, '\0', sizeof(char)*PAGE_SIZE);
			data2 = dentry_path_raw(trfs_record_struct->trfs_list_dentry->next->dentry, pathname2, PAGE_SIZE);
			buf_size = sizeof(struct trfs_record_struct_file)
				+ strlen(data)+1
				+ strlen(data2)+1;
			trfs_record_struct_file =  get_file_writable_record_helper(trfs_record_struct, buf_size);
			if(trfs_record_struct_file == NULL){
				printk("\n NO MEMORY FOUND");
				return NULL;
			}
			//printk("\nSize data ( %zu, %zu ) ",strlen(data), strlen(data2));
			//printk("\nSize of struct: %zu", sizeof(struct trfs_record_struct_file));
			//printk("\nSecond Path name: (%s  %s)", data2, pathname2);
			trfs_record_struct_file->path_name[trfs_record_struct_file->path_len] = ' ';
			strcpy(trfs_record_struct_file->path_name + trfs_record_struct_file->path_len + 1, data2);
	}
	if(pathname)
		kfree(pathname);
	if(pathname2)
		kfree(pathname2);
	return trfs_record_struct_file;	

}


int trfs_add_record(struct trfs_record_struct* trfs_record_struct){
	
	static int record_no = 0; 
	int checksum = -1, i = 0;
	char* write_buf = NULL;
	struct file* write_file_fd = NULL;
	struct trfs_record_struct_file *trfs_record_struct_file = NULL;
	struct super_block *sb = NULL;	
	int remaining_data = 0;
	//record_no++;
	//printk("\n RECORD NUMBER IS ---> %d", record_no);
	if(trfs_record_struct == NULL || trfs_record_struct->type < 0){
		printk("\nNOT NULL");
		return -EINVAL;
	}
	sb = trfs_record_struct->trfs_list_inode->inode->i_sb;
	//trfs_record_struct->record_no = record_no;
	mutex_lock(&TRFS_SB(sb)->trfs_record_lock);	
	trfs_record_struct_file = get_file_writable_record(trfs_record_struct);
	
	if(trfs_record_struct_file == NULL){
		printk("\n NO MEMORY EXIT");
		return -ENOMEM;
	}
	record_no++;
	trfs_record_struct_file->record_no = record_no;
	/*printk("\nFILE DATA: \nRecord no: %d \nRecord size: %d \nType:%d \nFlags: %d \nPermissions: %d\nPath len: %d\nSuccess flag: %d\nPath name: %s" 
			,trfs_record_struct_file->record_no
			,trfs_record_struct_file->buf_size
			,trfs_record_struct_file->type
			,trfs_record_struct_file->flags
			,trfs_record_struct_file->permission_mode
			,trfs_record_struct_file->path_len
			,trfs_record_struct_file->success_flag
			//,trfs_record_struct_file->k_fd
			,trfs_record_struct_file->path_name);
	*/	
	write_file_fd = TRFS_SB(sb)->out_fd;
	
	if(write_file_fd == NULL){
		return -ENOMEM;
	}

	write_buf = (char *)kmalloc(trfs_record_struct_file->buf_size, GFP_KERNEL);
	memcpy(write_buf,trfs_record_struct_file,trfs_record_struct_file->buf_size);
	checksum = trfs_checksum(write_buf);
	//printk("\n Checksum %d",checksum);
	//printk("\n VACAN BEFORE CHECKSUM ----> %d", vacant);
	if(RECORD_CHECKSUM_SIZE){
		if( RECORD_CHECKSUM_SIZE >= vacant){
			if(write_file_fd != NULL)
				write_to_file(write_file_fd, write_file_buff, TRFS_PAGE_SIZE - vacant);
			vacant = TRFS_PAGE_SIZE;
			write_file_buff[0] = '\0';
		}
		memcpy(write_file_buff + TRFS_PAGE_SIZE - vacant , &checksum, RECORD_CHECKSUM_SIZE);
		vacant -= RECORD_CHECKSUM_SIZE;
	}	
	//printk("\n VACAN AFTER CHECKSUM ----> %d   CHECKSUM ----> %d", vacant, checksum);	
	
	if(trfs_record_struct_file->buf_size >= vacant){
	//	printk("\nActually Writing here");
		write_to_file(write_file_fd, write_file_buff, TRFS_PAGE_SIZE - vacant);
		remaining_data = trfs_record_struct_file->buf_size;
	//	printk("\nRemaining Data %d", remaining_data);
		vacant = TRFS_PAGE_SIZE;
		write_file_buff[0] = '\0';
		while(remaining_data >= TRFS_PAGE_SIZE ){
			write_to_file(write_file_fd, write_buf + i*TRFS_PAGE_SIZE, TRFS_PAGE_SIZE);
			i++;
			remaining_data -= TRFS_PAGE_SIZE;
			write_file_buff[0] = '\0'; 
			
		}
		if(i > 0){
			if(remaining_data > 0){
			 	write_to_file(write_file_fd, write_buf + i*TRFS_PAGE_SIZE, remaining_data);	
					
			}
			write_file_buff[0] = '\0';
			goto get_out;
			
		}		
	//	printk("\n vacant = %d", vacant);

	}
	

	//printk("\n before vacant = %d ", vacant);
	memcpy(write_file_buff + TRFS_PAGE_SIZE - vacant, trfs_record_struct_file, trfs_record_struct_file->buf_size);
	vacant -= trfs_record_struct_file->buf_size;
	write_file_buff[TRFS_PAGE_SIZE - vacant] = '\0';
	//printk("\n vacant finally---> %d  trfs_record_struct_file->buf_size---> %d", vacant, trfs_record_struct_file->buf_size);
	//printk("TRFS_RECORD_STRUCT_FILE->PATH_NAME %s", trfs_record_struct_file->path_name);
get_out:	
	if(write_buf){
		kfree(write_buf);
		write_buf = NULL;
	}

	if(trfs_record_struct_file){
		kfree(trfs_record_struct_file);
		trfs_record_struct_file = NULL;
	}	

	//CODE TO FREE THE LIST INSIDE STRUCTURE
	if(trfs_record_struct->trfs_list_inode){
		if(trfs_record_struct->trfs_list_inode->next){
			kfree(trfs_record_struct->trfs_list_inode->next);
			trfs_record_struct->trfs_list_inode->next = NULL;
		}
		kfree(trfs_record_struct->trfs_list_inode);
		trfs_record_struct->trfs_list_inode = NULL;
	}
	if(trfs_record_struct->trfs_list_dentry){
		if(trfs_record_struct->trfs_list_dentry->next){
			kfree(trfs_record_struct->trfs_list_dentry->next);
			trfs_record_struct->trfs_list_dentry->next = NULL;
		}
		kfree(trfs_record_struct->trfs_list_dentry);
		trfs_record_struct->trfs_list_dentry = NULL;
	}
	if(trfs_record_struct->data){
		kfree(trfs_record_struct->data);
		trfs_record_struct->data = NULL;
	}
        mutex_unlock(&TRFS_SB(sb)->trfs_record_lock);
	//printk("\n\n----------------------------------------------------\n\n");	
	return 1;	
}

EXPORT_SYMBOL(trfs_add_record);
/*
 * There is no need to lock the trfs_super_info's rwsem as there is no
 * way anyone can have a reference to the superblock at this point in time.
 */

static int trfs_read_super(struct super_block *sb, void *raw_data, int silent)
{
	int err = 0;
	struct super_block *lower_sb;
	struct path lower_path;
	char *dev_name = (char *) raw_data;
	struct inode *inode;
	struct file *filp_out;
	if (!dev_name) {
		printk(KERN_ERR
				"trfs: read_super: missing dev_name argument\n");
		err = -EINVAL;
		goto out;
	}



	/* parse lower path */
	err = kern_path(dev_name, LOOKUP_FOLLOW | LOOKUP_DIRECTORY,
			&lower_path);
	if (err) {
		printk(KERN_ERR	"trfs: error accessing "
				"lower directory '%s'\n", dev_name);
		goto out;
	}

	/* allocate superblock private data */
	sb->s_fs_info = kzalloc(sizeof(struct trfs_sb_info), GFP_KERNEL);
	if (!TRFS_SB(sb)) {
		printk(KERN_CRIT "trfs: read_super: out of memory\n");
		err = -ENOMEM;
		goto out_free;
	}

	/* set the lower superblock field of upper superblock */
	lower_sb = lower_path.dentry->d_sb;
	atomic_inc(&lower_sb->s_active);
	trfs_set_lower_super(sb, lower_sb);

	/* inherit maxbytes from lower file system */
	sb->s_maxbytes = lower_sb->s_maxbytes;

	/*
	 * Our c/m/atime granularity is 1 ns because we may stack on file
	 * systems whose granularity is as good.
	 */
	sb->s_time_gran = 1;

	sb->s_op = &trfs_sops;

	sb->s_export_op = &trfs_export_ops; /* adding NFS support */

	/* get a new inode and allocate our root dentry */
	inode = trfs_iget(sb, d_inode(lower_path.dentry));
	printk("\nRoot inode %zu",inode->i_ino);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out_sput;
	}
	sb->s_root = d_make_root(inode);
	if (!sb->s_root) {
		err = -ENOMEM;
		goto out_iput;
	}
	d_set_d_op(sb->s_root, &trfs_dops);

	/* link the upper and lower dentries */
	sb->s_root->d_fsdata = NULL;
	err = new_dentry_private_data(sb->s_root);
	if (err)
		goto out_freeroot;

	/* if get here: cannot have error */

	/* set the lower dentries for s_root */
	trfs_set_lower_path(sb->s_root, &lower_path);

	/*
	 * No need to call interpose because we already have a positive
	 * dentry, which was instantiated by d_make_root.  Just need to
	 * d_rehash it.
	 */
	d_rehash(sb->s_root);
	if (!silent)
		printk(KERN_INFO
				"trfs: mounted on top of %s type %s\n",
				dev_name, lower_sb->s_type->name);
	
        filp_out  = filp_open(parsed_file_path, O_WRONLY|O_CREAT, 0644);
	if (!filp_out  || IS_ERR(filp_out)) {
		printk("trfs_read_file err %d\n", (int) PTR_ERR(filp_out));
		goto out_free;

	}
	//Assign trfs file fd to the trfs superblock private data.
	printk("\nFile Info %zu", filp_out->f_inode->i_ino);
	TRFS_SB(sb)->out_fd = filp_out;
	TRFS_SB(sb)->bitmap = 0xffffffff;          // set default bitmap value 
      	mutex_init(&TRFS_SB(sb)->trfs_record_lock); 
        write_file_buff = (char*)kmalloc(sizeof(char)*TRFS_PAGE_SIZE,GFP_KERNEL);
	if(write_file_buff == NULL){
		err = -ENOMEM;
		goto out_memfail;
	}
	write_file_buff[0] = '\0';
	vacant = TRFS_PAGE_SIZE;
	//write_file_buff = NULL;
	task = kthread_run(&thread_function,TRFS_SB(sb)->out_fd,"TRFS Writer thread");
/*	trfs_record_struct = (struct trfs_record_struct*)kmalloc(sizeof(struct trfs_record_struct), GFP_KERNEL);
	if(trfs_record_struct == NULL){
		err = -ENOMEM;
		goto out_memfail;
	}
	vacant = TRFS_PAGE_SIZE; 
	trfs_record_struct->type = -1;
	trfs_record_struct->trfs_list_inode = NULL;
	trfs_record_struct->trfs_list_dentry = NULL;
	trfs_record_struct->data = NULL; AAAAAAAAAAAAAAA*/
	goto out; /* all is well */

	/* no longer needed: free_dentry_private_data(sb->s_root); */
out_freeroot:
	dput(sb->s_root);
out_iput:
	iput(inode);
out_sput:
	/* drop refs we took earlier */
	atomic_dec(&lower_sb->s_active);
	kfree(TRFS_SB(sb));
	sb->s_fs_info = NULL;
out_free:
	path_put(&lower_path);

out_memfail:
	if(write_file_buff){
		kfree(write_file_buff);
		write_file_buff = NULL;
	}
/*	if(trfs_record_struct){
		kfree(trfs_record_struct);
		trfs_record_struct = NULL;
	}  AAAAAAAAA*/

out:
	if(parsed_file_path){
		kfree(parsed_file_path);
		parsed_file_path = NULL;
	}
	return err;
}

struct dentry *trfs_mount(struct file_system_type *fs_type, int flags,
		const char *dev_name, void *raw_data)
{
	void *lower_path_name = (void *) dev_name;

	if(raw_data == NULL)
        {
                printk("TRACE FILE PATH IS EXPECTED");
                return ERR_PTR(-EINVAL);
        }
	
	parse_raw_data((char *)raw_data);
	
	printk("\nMount is called %s", parsed_file_path);

	return mount_nodev(fs_type, flags, lower_path_name,
			trfs_read_super);
}

int parse_raw_data(char *raw_data)
{
	int err = 0;
	strsep(&raw_data,"=");
	parsed_file_path = (char *)kmalloc(strlen(raw_data),GFP_KERNEL);
	if(parsed_file_path == NULL)
		return -ENOMEM;
	strcpy(parsed_file_path, raw_data);
	//printk("\n PARSED PATH IS %s", parsed_file_path);
	return err;
}


u32 trfs_checksum(char *check_buff)
{
	return crc32(0^0xffffffff, (const void*)check_buff, strlen(check_buff))^0xffffffff;

}


static struct file_system_type trfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= TRFS_NAME,
	.mount		= trfs_mount,
	.kill_sb	= generic_shutdown_super,
	.fs_flags	= 0,
};

MODULE_ALIAS_FS(TRFS_NAME);

static int __init init_trfs_fs(void)
{
	int err;

	pr_info("Registering TRFS " TRFS_VERSION "\n");

	err = trfs_init_inode_cache();
	if (err)
		goto out;
	err = trfs_init_dentry_cache();
	if (err)
		goto out;
	err = register_filesystem(&trfs_fs_type);
out:
	if (err) {
		trfs_destroy_inode_cache();
		trfs_destroy_dentry_cache();
	}
	return err;
}

static void __exit exit_trfs_fs(void)
{
	trfs_destroy_inode_cache();
	trfs_destroy_dentry_cache();
	unregister_filesystem(&trfs_fs_type);
	pr_info("Completed trfs module unload\n");
}

MODULE_AUTHOR("Erez Zadok, Filesystems and Storage Lab, Stony Brook University"
		" (http://www.fsl.cs.sunysb.edu/)");
MODULE_DESCRIPTION("trfs"TRFS_VERSION
		" (http://trfs.filesystems.org/)");
MODULE_LICENSE("GPL");

module_init(init_trfs_fs);
module_exit(exit_trfs_fs);
