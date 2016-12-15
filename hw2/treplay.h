#ifndef TREPLAY_H__
#define TREPLAY_H_

#define RECORD_NUMBER_SIZE 4
#define RECORD_BUF_SIZE 2
#define RECORD_CHECKSUM_SIZE 4
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

struct treplay_record_struct_file{ //NEW_TRFS
        int record_no;
        short int buf_size;
        trfs_operation type;
        unsigned int flags;
        unsigned short permission_mode;
        short int path_len;
        int success_flag;
	struct file* k_fd;
	
        char path_name[1];

};
typedef enum {
	FMODE_READ,
	FMODE_WRITE,
} file_mode;	

struct list_file{
	struct file* k_fd;
	int fd;
	struct list_file* next; 	
};

int trfs_checksum(char *check_buff);

int replay_open(struct treplay_record_struct_file  * trfsrecord);
int replay_close(struct treplay_record_struct_file  * trfsrecord);
int replay_read(struct treplay_record_struct_file * trfsrecord);
int replay_mkdir(struct treplay_record_struct_file * trfsrecord);
int replay_rmdir(struct treplay_record_struct_file * trfsrecord);
int replay_unlink(struct  treplay_record_struct_file  * trfsrecordd);
int replay_write(struct treplay_record_struct_file * trfsrecord);
int replay_hardlink(struct treplay_record_struct_file  * trfsrecord);
int replay_symlink(struct treplay_record_struct_file  * trfsrecord);
int replay_rename(struct treplay_record_struct_file  * trfsrecord);
int replay_create(struct treplay_record_struct_file  * trfsrecord);

void show_read(struct treplay_record_struct_file * treplay_record_struct_file);
void show_open(struct treplay_record_struct_file * treplay_record_struct_file);
void show_close(struct treplay_record_struct_file * treplay_record_struct_file);
void show_mkdir(struct treplay_record_struct_file * treplay_record_struct_file);
void show_rmdir(struct treplay_record_struct_file * treplay_record_struct_file);
void show_unlink(struct treplay_record_struct_file * treplay_record_struct_file);
void show_write(struct treplay_record_struct_file * treplay_record_struct_file);
void show_hardlink(struct treplay_record_struct_file * treplay_record_struct_file);
void show_symlink(struct treplay_record_struct_file * treplay_record_struct_file);
void show_rename(struct treplay_record_struct_file * treplay_record_struct_file);
void show_create(struct treplay_record_struct_file * treplay_record_struct_file);
void show_hardlink(struct treplay_record_struct_file * treplay_record_struct_file);
#endif
