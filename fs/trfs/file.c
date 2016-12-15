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
static ssize_t trfs_read(struct file *file, char __user *buf,
			   size_t count, loff_t *ppos)
{
	int err;
	struct file *lower_file;
	struct dentry *dentry = file->f_path.dentry;
	struct trfs_record_struct *trfs_record_struct = NULL;
	//trfs_add_record();
	//printk("\nTRFS READ  Reading Inode number %zu and file name %s ",dentry->d_inode->i_ino, dentry->d_iname );
	lower_file = trfs_lower_file(file);
	err = vfs_read(lower_file, buf, count, ppos);
	/* update our inode atime upon a successful lower read */
	if (err >= 0)
		fsstack_copy_attr_atime(d_inode(dentry),
					file_inode(lower_file));
	/***********************************************************************************************************/
	trfs_record_struct = get_trfs_record_struct(TRFS_OP_READ);//AAAAAAAAAAAAAAAA
	if(trfs_record_struct != NULL && is_enabled(TRFS_SB(dentry->d_sb)->bitmap, TRFS_OP_READ)){
                printk("\nTRFS_READ ");
                trfs_record_struct->type = TRFS_OP_READ;
                trfs_append_to_list(dentry->d_inode, 0, trfs_record_struct);
                trfs_append_to_list(dentry, 1, trfs_record_struct);
		trfs_record_struct->k_fd = file;
		if(trfs_record_struct->data == NULL)
                        trfs_record_struct->data = kmalloc(count*sizeof(char) +1, GFP_KERNEL);
                if(trfs_record_struct->data == NULL){
                        err = -ENOMEM;
                        return err;
                }
                strncpy((char*)trfs_record_struct->data, buf, count);
                ((char*)(trfs_record_struct->data))[count] = '\0';
		trfs_record_struct->success_flag = err;
                trfs_add_record(trfs_record_struct);
		//printk("\nINSIDE RECORD ADD FUNCTION, TYPE %d", trfs_record_struct->type);
        }	

	/***********************************************************************************************************/
	 if(trfs_record_struct);//AAAAAAAAAAAAAA
                kfree(trfs_record_struct);
	return err;
}

static ssize_t trfs_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	int err;
	//trfs_add_record();
	struct file *lower_file;
	struct dentry *dentry = file->f_path.dentry;
	struct trfs_record_struct *trfs_record_struct = NULL;
	lower_file = trfs_lower_file(file);
	err = vfs_write(lower_file, buf, count, ppos);
	/* update our inode times+sizes upon a successful lower write */
	if (err >= 0) {
		fsstack_copy_inode_size(d_inode(dentry),
					file_inode(lower_file));
		fsstack_copy_attr_times(d_inode(dentry),
					file_inode(lower_file));
	}
	
	/***********************************************************************************************************/
	trfs_record_struct = get_trfs_record_struct(TRFS_OP_WRITE);
	if(trfs_record_struct != NULL && is_enabled(TRFS_SB(dentry->d_sb)->bitmap, TRFS_OP_WRITE)){

                printk("\nTRFS_WRITE ");
                trfs_record_struct->type = TRFS_OP_WRITE;
                trfs_append_to_list(dentry->d_inode, 0, trfs_record_struct);
                trfs_append_to_list(dentry, 1, trfs_record_struct);
		trfs_record_struct->k_fd = file;
		
		if(trfs_record_struct->data == NULL){
			trfs_record_struct->data = kmalloc(count*sizeof(char) +1, GFP_KERNEL);
		}

		if(trfs_record_struct->data == NULL){
			printk("NOMEMORY---------!!!!");
			err = -ENOMEM;
			return err;
		}
		strncpy((char*)trfs_record_struct->data, buf, count);
		((char*)(trfs_record_struct->data))[count] = '\0';
		trfs_record_struct->success_flag = err;	
		trfs_record_struct->flags = file->f_flags;
                trfs_add_record(trfs_record_struct);
        }
	
	/***********************************************************************************************************/
	if(trfs_record_struct);//AAAAAAAAAAAAAA
                kfree(trfs_record_struct);
	return err;
}

static int trfs_readdir(struct file *file, struct dir_context *ctx)
{

	int err;
	struct file *lower_file = NULL;
	struct dentry *dentry = file->f_path.dentry;

	lower_file = trfs_lower_file(file);
	err = iterate_dir(lower_file, ctx);
	file->f_pos = lower_file->f_pos;
	if (err >= 0)		/* copy the atime */
		fsstack_copy_attr_atime(d_inode(dentry),
					file_inode(lower_file));
	return err;
}

static long trfs_unlocked_ioctl(struct file *file, unsigned int cmd,
				  unsigned long arg)
{
	long err = 0;
	struct file *lower_file;
        int bitmap;
        int *ptr = NULL;
        
        #ifdef EXTRA_CREDIT
        struct toggle_ops *toggle_ops;
        toggle_ops = (struct toggle_ops *)kmalloc(sizeof(struct toggle_ops),GFP_KERNEL);
        #endif    
    
        printk("Called unlocked_ioctl\n");
        ptr = &bitmap;

        switch(cmd)
        { 
  	   case TRFS_IOCTL_SET_MSG:           
	    printk("Called case 1\n");	
            
            #ifdef EXTRA_CREDIT
                err = copy_from_user((void *)toggle_ops,(void *)arg,sizeof(toggle_ops));
            #else
          	err = copy_from_user((void *)ptr,(void *)arg,sizeof(int));
            #endif 
             
            if(err)
            {
                printk("KERNEL ERROR: User to kernel data mapping failed\n");
                err = -EINVAL;
                goto out;
            }
           
           #ifdef EXTRA_CREDIT
            
            TRFS_SB(file->f_path.dentry->d_sb)->bitmap = ((TRFS_SB(file->f_path.dentry->d_sb)->bitmap)&(toggle_ops->rem_bits));
            TRFS_SB(file->f_path.dentry->d_sb)->bitmap = ((TRFS_SB(file->f_path.dentry->d_sb)->bitmap)|(toggle_ops->add_bits));
            kfree(toggle_ops);
           #else
            TRFS_SB(file->f_path.dentry->d_sb)->bitmap = bitmap;
           #endif
         
           printk("New Set Bitmap value:%x\n",TRFS_SB(file->f_path.dentry->d_sb)->bitmap);
           break;
          case TRFS_IOCTL_GET_MSG:
             printk("Called case 2\n");
             bitmap = TRFS_SB(file->f_path.dentry->d_sb)->bitmap;
             err = copy_to_user((void *)arg,ptr,sizeof(int));
           break;
         default:
             err = -ENOTTY;
             lower_file = trfs_lower_file(file);
 //          XXX: use vfs_ioctl if/when VFS exports it 
             if (!lower_file || !lower_file->f_op)
                goto out;
             if (lower_file->f_op->unlocked_ioctl)
                err = lower_file->f_op->unlocked_ioctl(lower_file, cmd, arg);
//           some ioctls can change inode attributes (EXT2_IOC_SETFLAGS) 
             if (!err)
                fsstack_copy_attr_all(file_inode(file),
                                      file_inode(lower_file));
            break;
       }
out:
        return err;
}

/*
static long trfs_unlocked_ioctl(struct file *file, unsigned int cmd,
				  unsigned long arg)
{
	long err = 0;
	struct file *lower_file;
        int bitmap;
        int *ptr = NULL;
        printk("Called unlocked_ioctl\n");
        ptr = &bitmap;
      
        switch(cmd)
        { 
  	   case TRFS_IOCTL_SET_MSG:           
	    printk("Called case 1\n");	
	    err = copy_from_user(ptr,(void *)arg,sizeof(int));                
            if(err)
            {
                printk("KERNEL ERROR: User to kernel data mapping failed\n");
                err = -EINVAL;
                goto out;
            }
           TRFS_SB(file->f_path.dentry->d_sb)->bitmap = bitmap;
           printk("New Set Bitmap value:%x\n",TRFS_SB(file->f_path.dentry->d_sb)->bitmap);
           break;
          case TRFS_IOCTL_GET_MSG:
             printk("Called case 2\n");
             bitmap = TRFS_SB(file->f_path.dentry->d_sb)->bitmap;
             err = copy_to_user((void *)arg,ptr,sizeof(int));
           break;
          default:
             err = -ENOTTY;
             lower_file = trfs_lower_file(file);     
 //	     XXX: use vfs_ioctl if/when VFS exports it 
	     if (!lower_file || !lower_file->f_op)
		goto out;
	     if (lower_file->f_op->unlocked_ioctl)
		err = lower_file->f_op->unlocked_ioctl(lower_file, cmd, arg);
//           some ioctls can change inode attributes (EXT2_IOC_SETFLAGS) 
	     if (!err)
		fsstack_copy_attr_all(file_inode(file),
				      file_inode(lower_file));
            break;
       }   
out:
	return err;
}
*/
#ifdef CONFIG_COMPAT
static long trfs_compat_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	long err = -ENOTTY;
	struct file *lower_file;
        printk("Called compat_ioctl\n");
	lower_file = trfs_lower_file(file);

	/* XXX: use vfs_ioctl if/when VFS exports it */
	if (!lower_file || !lower_file->f_op)
		goto out;
	if (lower_file->f_op->compat_ioctl)
		err = lower_file->f_op->compat_ioctl(lower_file, cmd, arg);

out:
	return err;
}
#endif

static int trfs_mmap(struct file *file, struct vm_area_struct *vma)
{
	int err = 0;
	bool willwrite;
	struct file *lower_file;
	const struct vm_operations_struct *saved_vm_ops = NULL;

	/* this might be deferred to mmap's writepage */
	willwrite = ((vma->vm_flags | VM_SHARED | VM_WRITE) == vma->vm_flags);

	/*
	 * File systems which do not implement ->writepage may use
	 * generic_file_readonly_mmap as their ->mmap op.  If you call
	 * generic_file_readonly_mmap with VM_WRITE, you'd get an -EINVAL.
	 * But we cannot call the lower ->mmap op, so we can't tell that
	 * writeable mappings won't work.  Therefore, our only choice is to
	 * check if the lower file system supports the ->writepage, and if
	 * not, return EINVAL (the same error that
	 * generic_file_readonly_mmap returns in that case).
	 */
	lower_file = trfs_lower_file(file);
	if (willwrite && !lower_file->f_mapping->a_ops->writepage) {
		err = -EINVAL;
		printk(KERN_ERR "trfs: lower file system does not "
		       "support writeable mmap\n");
		goto out;
	}

	/*
	 * find and save lower vm_ops.
	 *
	 * XXX: the VFS should have a cleaner way of finding the lower vm_ops
	 */
	if (!TRFS_F(file)->lower_vm_ops) {
		err = lower_file->f_op->mmap(lower_file, vma);
		if (err) {
			printk(KERN_ERR "trfs: lower mmap failed %d\n", err);
			goto out;
		}
		saved_vm_ops = vma->vm_ops; /* save: came from lower ->mmap */
	}

	/*
	 * Next 3 lines are all I need from generic_file_mmap.  I definitely
	 * don't want its test for ->readpage which returns -ENOEXEC.
	 */
	file_accessed(file);
	vma->vm_ops = &trfs_vm_ops;

	file->f_mapping->a_ops = &trfs_aops; /* set our aops */
	if (!TRFS_F(file)->lower_vm_ops) /* save for our ->fault */
		TRFS_F(file)->lower_vm_ops = saved_vm_ops;

out:
	return err;
}

static int trfs_open(struct inode *inode, struct file *file)
{
	int err = 0;
	struct file *lower_file = NULL;
	struct path lower_path;
	struct trfs_record_struct *trfs_record_struct = NULL;

	/* don't open unhashed/deleted files */
	if (d_unhashed(file->f_path.dentry)) {
		err = -ENOENT;
		goto out_err;
	}

	file->private_data =
		kzalloc(sizeof(struct trfs_file_info), GFP_KERNEL);
	if (!TRFS_F(file)) {
		err = -ENOMEM;
		goto out_err;
	}
	
	/* open lower object and link trfs's file struct to lower's */
	trfs_get_lower_path(file->f_path.dentry, &lower_path);
	lower_file = dentry_open(&lower_path, file->f_flags, current_cred());
	path_put(&lower_path);
	if (IS_ERR(lower_file)) {
		err = PTR_ERR(lower_file);
		lower_file = trfs_lower_file(file);
		if (lower_file) {
			trfs_set_lower_file(file, NULL);
			fput(lower_file); /* fput calls dput for lower_dentry */
		}
	} 
	else {
		trfs_set_lower_file(file, lower_file);
	}
	/***********************************************************************************************************/
	 trfs_record_struct = get_trfs_record_struct(TRFS_OP_OPEN);
		
	if(trfs_record_struct != NULL && is_enabled(TRFS_SB(inode->i_sb)->bitmap, TRFS_OP_OPEN)){
                trfs_record_struct->type = TRFS_OP_OPEN;
                trfs_append_to_list(inode, 0,trfs_record_struct);
                trfs_append_to_list(file->f_path.dentry, 1,trfs_record_struct);
		trfs_record_struct->k_fd = file;
		trfs_record_struct->success_flag = err;
		trfs_record_struct->flags = file->f_flags;
		trfs_add_record(trfs_record_struct);
		
        }

	/***********************************************************************************************************/
	
	if (err)
		kfree(TRFS_F(file));
	else
		fsstack_copy_attr_all(inode, trfs_lower_inode(inode));
out_err:
	if(trfs_record_struct);//AAAAAAAAAAAAAA
                kfree(trfs_record_struct);
	return err;
}

static int trfs_flush(struct file *file, fl_owner_t id)
{
	int err = 0;
	struct file *lower_file = NULL;

	lower_file = trfs_lower_file(file);
	if (lower_file && lower_file->f_op && lower_file->f_op->flush) {
		filemap_write_and_wait(file->f_mapping);
		err = lower_file->f_op->flush(lower_file, id);
	}

	return err;
}

/* release all lower object references & free the file info structure */
static int trfs_file_release(struct inode *inode, struct file *file)
{

	struct file *lower_file;
	struct trfs_record_struct *trfs_record_struct = NULL;
	
	lower_file = trfs_lower_file(file);
	if (lower_file) {
		trfs_set_lower_file(file, NULL);
		fput(lower_file);
	}
	
	/***********************************************************************************************************/
	 trfs_record_struct = get_trfs_record_struct(TRFS_OP_CLOSE);
	if(trfs_record_struct != NULL && is_enabled(TRFS_SB(inode->i_sb)->bitmap, TRFS_OP_CLOSE)){
		printk("\nTRFS CLOSE");
                trfs_record_struct->type = TRFS_OP_CLOSE;
                trfs_append_to_list(inode, 0,trfs_record_struct);
                trfs_append_to_list(file->f_path.dentry, 1, trfs_record_struct);
		trfs_record_struct->k_fd = file;
		trfs_record_struct->success_flag = 0;
		trfs_record_struct->flags = file->f_flags;
                trfs_add_record(trfs_record_struct);
                //printk("\nINSIDE RECORD ADD FUNCTION, TYPE %d", trfs_record_struct->type);
        }

	/***********************************************************************************************************/

	kfree(TRFS_F(file));
	if(trfs_record_struct);//AAAAAAAAAAAAAA
		kfree(trfs_record_struct);
	return 0;
}

static int trfs_fsync(struct file *file, loff_t start, loff_t end,
			int datasync)
{
	int err;
	struct file *lower_file;
	struct path lower_path;
	struct dentry *dentry = file->f_path.dentry;

	err = __generic_file_fsync(file, start, end, datasync);
	if (err)
		goto out;
	lower_file = trfs_lower_file(file);
	trfs_get_lower_path(dentry, &lower_path);
	err = vfs_fsync_range(lower_file, start, end, datasync);
	trfs_put_lower_path(dentry, &lower_path);
out:
	return err;
}

static int trfs_fasync(int fd, struct file *file, int flag)
{
	int err = 0;
	struct file *lower_file = NULL;

	lower_file = trfs_lower_file(file);
	if (lower_file->f_op && lower_file->f_op->fasync)
		err = lower_file->f_op->fasync(fd, lower_file, flag);

	return err;
}

/*
 * Trfs cannot use generic_file_llseek as ->llseek, because it would
 * only set the offset of the upper file.  So we have to implement our
 * own method to set both the upper and lower file offsets
 * consistently.
 */
static loff_t trfs_file_llseek(struct file *file, loff_t offset, int whence)
{
	int err;
	struct file *lower_file;

	err = generic_file_llseek(file, offset, whence);
	if (err < 0)
		goto out;

	lower_file = trfs_lower_file(file);
	err = generic_file_llseek(lower_file, offset, whence);

out:
	return err;
}

/*
 * Trfs read_iter, redirect modified iocb to lower read_iter
 */
ssize_t
trfs_read_iter(struct kiocb *iocb, struct iov_iter *iter)
{
	int err;
	struct file *file = iocb->ki_filp, *lower_file;

	lower_file = trfs_lower_file(file);
	if (!lower_file->f_op->read_iter) {
		err = -EINVAL;
		goto out;
	}

	get_file(lower_file); /* prevent lower_file from being released */
	iocb->ki_filp = lower_file;
	err = lower_file->f_op->read_iter(iocb, iter);
	iocb->ki_filp = file;
	fput(lower_file);
	/* update upper inode atime as needed */
	if (err >= 0 || err == -EIOCBQUEUED)
		fsstack_copy_attr_atime(d_inode(file->f_path.dentry),
					file_inode(lower_file));
out:
	return err;
}

/*
 * Trfs write_iter, redirect modified iocb to lower write_iter
 */
ssize_t
trfs_write_iter(struct kiocb *iocb, struct iov_iter *iter)
{
	int err;
	struct file *file = iocb->ki_filp, *lower_file;

	lower_file = trfs_lower_file(file);
	if (!lower_file->f_op->write_iter) {
		err = -EINVAL;
		goto out;
	}

	get_file(lower_file); /* prevent lower_file from being released */
	iocb->ki_filp = lower_file;
	err = lower_file->f_op->write_iter(iocb, iter);
	iocb->ki_filp = file;
	fput(lower_file);
	/* update upper inode times/sizes as needed */
	if (err >= 0 || err == -EIOCBQUEUED) {
		fsstack_copy_inode_size(d_inode(file->f_path.dentry),
					file_inode(lower_file));
		fsstack_copy_attr_times(d_inode(file->f_path.dentry),
					file_inode(lower_file));
	}
out:
	return err;
}

const struct file_operations trfs_main_fops = {
	.llseek		= generic_file_llseek,
	.read		= trfs_read,
	.write		= trfs_write,
	.unlocked_ioctl	= trfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= trfs_compat_ioctl,
#endif
	.mmap		= trfs_mmap,
	.open		= trfs_open,
	.flush		= trfs_flush,
	.release	= trfs_file_release,
	.fsync		= trfs_fsync,
	.fasync		= trfs_fasync,
	.read_iter	= trfs_read_iter,
	.write_iter	= trfs_write_iter,
};

/* trimmed directory options */
const struct file_operations trfs_dir_fops = {
	.llseek		= trfs_file_llseek,
	.read		= generic_read_dir,
	.iterate	= trfs_readdir,
	.unlocked_ioctl	= trfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= trfs_compat_ioctl,
#endif
	.open		= trfs_open,
	.release	= trfs_file_release,
	.flush		= trfs_flush,
	.fsync		= trfs_fsync,
	.fasync		= trfs_fasync,
};
