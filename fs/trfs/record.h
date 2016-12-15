#ifndef TREPLAY_H__
#define TREPLAY_H_

#include<linux/ioctl.h>

#define RECORD_NUMBER_SIZE 4
#define RECORD_BUF_SIZE 2

//#define EXTRA_CREDIT
#define APPTYPE 101

#define TRFS_IOCTL_SET_MSG _IOW(APPTYPE, 0, void *)
#define TRFS_IOCTL_GET_MSG _IOR(APPTYPE, 1, char *)

#ifdef EXTRA_CREDIT
	#define RECORD_CHECKSUM_SIZE 4
#else
	#define RECORD_CHECKSUM_SIZE 0
#endif



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
        TRFS_OP_RENAME } trfs_operation;

struct trfs_record_struct_file{ //NEW_TRFS
        int record_no;
        short int buf_size;
        trfs_operation type;
        unsigned int flags;
        unsigned int permission_mode;
        short int path_len;
        int success_flag;
	struct file* k_fd;
	
        char path_name[1];

};

struct list_file{
	struct file* k_fd;
	int fd;
	struct list_file* next; 	
};

int compute_trfs_checksum(char *check_buff);

bool replay_open(struct trfs_record_struct_file  * trfsrecord);
bool replay_close(struct trfs_record_struct_file  * trfsrecord);
bool replay_read(struct trfs_record_struct_file * trfsrecord);
bool replay_mkdir(struct trfs_record_struct_file * trfsrecord);
bool replay_rmdir(struct trfs_record_struct_file * trfsrecord);
bool replay_unlink(struct  trfs_record_struct_file  * trfsrecordd);
bool replay_write(struct trfs_record_struct_file * trfsrecord);
bool replay_hardlink(struct trfs_record_struct_file  * trfsrecord);
bool replay_symlink(struct trfs_record_struct_file  * trfsrecord);
bool replay_rename(struct trfs_record_struct_file  * trfsrecord);
bool replay_create(struct trfs_record_struct_file  * trfsrecord);

void show_read(struct trfs_record_struct_file * trfs_record_struct_file);
void show_open(struct trfs_record_struct_file * trfs_record_struct_file);
void show_close(struct trfs_record_struct_file * trfs_record_struct_file);
void show_mkdir(struct trfs_record_struct_file * trfs_record_struct_file);
void show_rmdir(struct trfs_record_struct_file * trfs_record_struct_file);
void show_unlink(struct trfs_record_struct_file * trfs_record_struct_file);
void show_write(struct trfs_record_struct_file * trfs_record_struct_file);
void show_hardlink(struct trfs_record_struct_file * trfs_record_struct_file);
void show_symlink(struct trfs_record_struct_file * trfs_record_struct_file);
void show_rename(struct trfs_record_struct_file * trfs_record_struct_file);
void show_create(struct trfs_record_struct_file * trfs_record_struct_file);
#endif
