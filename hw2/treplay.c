#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "../fs/trfs/record.h"
//#include "record.h"
#include <zlib.h>
//#define ABS_PATH_LEN 9
#define ABS_PATH "/usr/src/hw2-cse506g15/dir_replica"
#define ABS_PATH_LEN strlen(ABS_PATH)
#define MAX_PATH_SIZE 4096
int f_ptr = -1;
struct list_file* my_file_list = NULL;
char* first_file_path = NULL; 
char* second_file_path = NULL;
bool strict_mode = false;
int cleanup_list();
int compute_trfs_checksum(char *check_buff)
{
	return crc32(0,(const void*)check_buff,strlen(check_buff));

}
void add_node(struct file* k_fd,int fd){
	struct list_file* node = malloc(sizeof(struct list_file));
	struct list_file* curr = NULL;
	node->k_fd = k_fd;
	node->fd = fd;			
	node->next = NULL;
	if(my_file_list == NULL){
		my_file_list = node;
		//printf("\nFIRST fd =  %d  Kernel fd = %llu ",  fd, k_fd);
		return;
	}	
	curr = my_file_list;
	while(curr->next != NULL){
		curr = curr->next;	
	}
	//printf("\nAdded fd =  %d  Kernel fd =  %llu ", fd, k_fd);
	//printf("\nParent fd = %d  Kernel fd =  %llu ",  curr->fd, curr->k_fd);
	curr->next = node;
	return;
};

void delete_node(int fd){
	struct list_file* curr = NULL;
	struct list_file* prev = my_file_list;
	curr = prev->next;
	while(prev != NULL && curr != NULL){
		if(curr->fd == fd){
			prev->next = curr->next;
			free(curr);
			//printf("Deleted node fd = %d", fd);
			return;
		}
		prev = prev->next;
		curr = curr->next;
	}

	//printf("\nNODE NOT FOUND");
	return;

}

int get_fd(struct file* k_fd){
	struct list_file* curr = my_file_list;
	if(curr == NULL){
		printf("\nNO ELEMENT IN LIST ");
	}
	//printf("\nParent fd = %d  Kernel fd =  %llu", curr->fd, curr->k_fd);
	//printf("\nLooking for Kernel fd = %llu,", k_fd);
	while(curr != NULL){
		if(curr->k_fd == k_fd){
			return curr->fd;
		}
		curr = curr->next;
	}
	//printf("\nFailed for fd: %llu", k_fd);
	return -1;
}

void set_first_file_path(struct trfs_record_struct_file* trfs_record_struct_file){

	if(first_file_path == NULL){
		printf("\nMEMORY ERROR");	
		return;
	}
	//memset(, '\0', sizeof(char)*MAX_PATH_SIZE );
	strncpy(first_file_path, ABS_PATH, ABS_PATH_LEN);
	strncpy(first_file_path + ABS_PATH_LEN , trfs_record_struct_file->path_name, trfs_record_struct_file->path_len);
	first_file_path[ABS_PATH_LEN + trfs_record_struct_file->path_len] = '\0';
	//printf("\nFirst path = %s", first_file_path);
}

void set_second_file_path(struct trfs_record_struct_file* trfs_record_struct_file){
	if(second_file_path == NULL){
		printf("\n MEMORY ERROR");
		return;
	}
	int first_path_len = trfs_record_struct_file->path_len;

	strncpy(second_file_path, ABS_PATH, ABS_PATH_LEN);
	//memset( buffer, '\0', sizeof(char)*MAX_PATH_SIZE );
	//printf("\nFirst construct: %s ABS_PATH_LEN: %zu", second_file_path, ABS_PATH_LEN);
	if(first_path_len == strlen(trfs_record_struct_file->path_name)){
		printf("\nOnly one file in path name");
		return; 
	}
	strncpy(second_file_path + ABS_PATH_LEN, 
			trfs_record_struct_file->path_name + first_path_len + 1,
			strlen(trfs_record_struct_file->path_name) - first_path_len - 1);
	//printf("\nSecond construct: %s", second_file_path);
	second_file_path[ABS_PATH_LEN + strlen(trfs_record_struct_file->path_name) - (first_path_len + 1)] = '\0';	
	//printf("\nSecond path = %s", second_file_path);
}

char* get_extra_info(struct trfs_record_struct_file* trfs_record_struct_file, trfs_operation type){
	int len = 0, buff_len = 0;
	if(type == TRFS_OP_SYMLINK){
		len = 1;
		buff_len = ABS_PATH_LEN + strlen(trfs_record_struct_file->path_name) - trfs_record_struct_file->path_len - 1 + len + 1;	

	}
	else
		buff_len = strlen(trfs_record_struct_file->path_name) - trfs_record_struct_file->path_len - 1 + len + 1;

	char* buff = NULL;
	//printf("\nBuff len: %d",buff_len);
	int first_path_len = trfs_record_struct_file->path_len;
	buff = (char*) malloc(sizeof(char)*buff_len); 
	if(type == TRFS_OP_SYMLINK){
		strncpy(buff, ABS_PATH, ABS_PATH_LEN);
		//printf("\nFirst extra info: %s", buff);
		strncpy(buff + ABS_PATH_LEN, "/",len);
		//printf("\nSecond extra info: %s", buff);
		strncpy(buff + ABS_PATH_LEN + len, 
				trfs_record_struct_file->path_name + first_path_len+1, 
				strlen(trfs_record_struct_file->path_name) - first_path_len - 1);
		buff[ABS_PATH_LEN + strlen(trfs_record_struct_file->path_name) - (first_path_len + 1) + 1] = '\0';
		//printf("\nLast extra info Symlink: %s buff_len = %d, strlen(buff) = %zu", buff, buff_len, strlen(buff));

		return buff;
	}
	//printf("\n BUFFER AFTER IT %s",buff );

	strncpy(buff, trfs_record_struct_file->path_name + first_path_len+1, buff_len);
	buff[buff_len] = '\0';
	//printf("\nEXTRA INFO = %s", buff);
	return buff;
	//TODO
	//FREE LOGIC
}


int main (int argc, char *argv[]){
	int c;
	bool opt_flag = true; 
	bool replay_mode = true, display_mode = false;
	FILE *file_pointer;
	long  file_size;
	int ret = 0, checksum = -1, test_checksum = -1;
	int record_size = 0 , bytes_read = 0 ;
	struct trfs_record_struct_file *trfs_record_struct_file = NULL;
	printf("argc = %d", argc);
	if (argc < 2)
	{

		printf("\nTrac file needed. Incorrect format");
		exit(-1);

	}
	else 
	{
		while((c = getopt(argc,argv,"ns"))!=-1){
			switch(c){
				case 'n':
					if(opt_flag){
						replay_mode = false;
						display_mode = true;
					}
					else{
						printf("\n1Invalid options/arguments\n");
						exit(-1);
					}
					opt_flag = false;
					//         strict_mode = false;
					break;
				case 's':
					if(opt_flag)
						strict_mode = true;
					else{
						printf("\n2Invalid options/arguments\n");
						exit(-1);
					}
					opt_flag = false;

					//       replay_mode = true;
					break;
				case '?': 
					printf("\n3Invalid options/arguments\n");
					exit(-1); 
			}

		} 

	}//           printf("Opt index value: %d\n",optind);

	if(argc-1 != optind){
		printf("\nInvalid Arguments\n");
		exit(-1);
	}
	file_pointer = fopen( argv[argc - 1] , "r" );
	printf("\nFile to trace replay -> %s",argv[argc - 1]);
	ret = fseek (file_pointer , 0 , SEEK_END);
	file_size = ftell (file_pointer); //GIVES TOTAL SIZE OF FILE IN BYTES
	ret = fseek(file_pointer, 0, SEEK_SET );
	//TODO ---- ret == 0 
	//rewind (file_pointer);

	//	printf("\nFile Size%d",file_size);

	char *file_buf = (char*) malloc (sizeof(char)*file_size);
	fread(file_buf,file_size,1,file_pointer); //TODO change nmemb
	printf("File:%s\n",file_buf);
	printf("File size:%ld\n",file_size);
	printf("\nstrict_mode = %d, display_mode = %d, replay_mode = %d", strict_mode, display_mode, replay_mode);
	if (file_buf == NULL) {
		fputs("Memory error",stderr); 
		exit(2);
	}
	if(first_file_path == NULL)
		first_file_path = (char*)malloc(sizeof(char)*MAX_PATH_SIZE);

	if(first_file_path == NULL)
		return -1;

	if(second_file_path == NULL)
		second_file_path = (char*)malloc(sizeof(char)*MAX_PATH_SIZE);

	if(second_file_path == NULL){
		free(first_file_path);
		return -1;
	}

	while(bytes_read < file_size-1){
		//printf("\nRECORD_NUMBER_SIZE: %d, \nRECORD_CHECKSUM_SIZE: %d", RECORD_NUMBER_SIZE, RECORD_CHECKSUM_SIZE);
		memcpy(&record_size, file_buf + RECORD_NUMBER_SIZE + RECORD_CHECKSUM_SIZE + bytes_read, RECORD_BUF_SIZE);
		//printf("\n record_size ----> %d", record_size);
		if(RECORD_CHECKSUM_SIZE){
			//printf("\n RECORD_CHECKSUM_SIZE = %d", RECORD_CHECKSUM_SIZE);
			memcpy(&checksum, file_buf + bytes_read, RECORD_CHECKSUM_SIZE); 
		}
		//printf("\nBYTES READ ----> %d  record size ---> %d  FILE SIZE ------>  %d", bytes_read, record_size, file_size);
		trfs_record_struct_file = (struct trfs_record_struct_file *)malloc(record_size);
		memcpy(trfs_record_struct_file, file_buf + bytes_read + RECORD_CHECKSUM_SIZE , record_size);
		
		if(RECORD_CHECKSUM_SIZE){
			test_checksum = compute_trfs_checksum((char*)trfs_record_struct_file);
                        //printf("\nChecksum ( %d, %d ) ",checksum, test_checksum);
			if(checksum != test_checksum){
				bytes_read = bytes_read + record_size + RECORD_CHECKSUM_SIZE;
				free(trfs_record_struct_file);
				trfs_record_struct_file = NULL;	
				//printf("\nCORRUPTED RECORD !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Number %d  type --> %d", trfs_record_struct_file->record_no, trfs_record_struct_file->type );
				if(strict_mode == true){
					printf("\nAborting Corrupted record");
				}
					
				continue;
			}
		}
		set_first_file_path(trfs_record_struct_file);
		//printf("\nPATH1 ----> %s 	%s", first_file_path, trfs_record_struct_file->path_name);
		set_second_file_path(trfs_record_struct_file);
		//printf("\n SECOND PATH: %s", second_file_path);
		//printf("\n RECORD NUM --------------------------> %d", trfs_record_struct_file->record_no);

		switch(trfs_record_struct_file->type)
		{
			case TRFS_OP_OPEN:
				printf("\nTRFS_OP_OPEN");
				if(display_mode)
					show_open(trfs_record_struct_file);
				else
					opt_flag = replay_open(trfs_record_struct_file);
				break;

			case TRFS_OP_CLOSE:
				printf("\nTRFS_OP_CLOSE");
				if(display_mode)
					show_close(trfs_record_struct_file);
				else
					opt_flag = replay_close(trfs_record_struct_file);
				break;
			case TRFS_OP_READ:
				printf("\nTRFS_OP_READ");
				if(display_mode)
					show_read(trfs_record_struct_file);
				else
					opt_flag = replay_read(trfs_record_struct_file);
				break;
			case TRFS_OP_MKDIR:
				printf("\nTRFS_OP_MKDIR");
				if(display_mode)
					show_mkdir(trfs_record_struct_file);
				else
					opt_flag = replay_mkdir(trfs_record_struct_file);
				break;

			case TRFS_OP_RMDIR:
				printf("\nTRFS_OP_RMDIR");
				if(display_mode)
					show_rmdir(trfs_record_struct_file);
				else
					opt_flag = replay_rmdir(trfs_record_struct_file);
				break;

			case TRFS_OP_UNLINK:
				printf("\nTRFS_OP_UNLINK");
				if(display_mode)
					show_unlink(trfs_record_struct_file);
				else
					opt_flag = replay_unlink(trfs_record_struct_file);
				break;

			case TRFS_OP_WRITE:
				printf("\nTRFS_OP_WRITE");
				if(display_mode)
					show_write(trfs_record_struct_file);
				else
					opt_flag = replay_write(trfs_record_struct_file);
				break;

			case TRFS_OP_SYMLINK:
				printf("\nTRFS_OP_SYMLINK");
				if(display_mode)
					show_symlink(trfs_record_struct_file);
				else
					opt_flag = replay_symlink(trfs_record_struct_file);
				break;

			case TRFS_OP_HARDLINK:
				printf("\nTRFS_OP_HARDLINK");
				if(display_mode)
					show_hardlink(trfs_record_struct_file);
				else
					opt_flag = replay_hardlink(trfs_record_struct_file);
				break;

			case TRFS_OP_RENAME:
				printf("\nTRFS_OP_RENAME");
				if(display_mode)
					show_rename(trfs_record_struct_file);
				else
					opt_flag = replay_rename(trfs_record_struct_file);
				break;

			case TRFS_OP_CREATE:
				printf("\nTRFS_OP_CREATE");
				if(display_mode)
					show_create(trfs_record_struct_file);
				else
					opt_flag = replay_create(trfs_record_struct_file);
				break;

				/*default:
				  printf("\nFILE DATA ---->%d %d %d %d %d %d %d %s"
				  ,trfs_record_struct_file->record_no
				  ,trfs_record_struct_file->buf_size
				  ,trfs_record_struct_file->type
				  ,trfs_record_struct_file->flags
				  ,trfs_record_struct_file->permission_mode
				  ,trfs_record_struct_file->path_len
				  ,trfs_record_struct_file->success_flag
				  ,trfs_record_struct_file->path_name);*/	
		}
		if(display_mode){
			if(RECORD_CHECKSUM_SIZE == 4){
				printf("\n Checksum(%d , %d) ", checksum, test_checksum);
			}
			printf("\nFILE DATA: \nRecord no: %d \nRecord size: %d \nType:%d \nFlags: %d \nPermissions: %d\nPath len: %d\nSuccess flag: %d \nPath name: %s"
				,trfs_record_struct_file->record_no
				,trfs_record_struct_file->buf_size
				,trfs_record_struct_file->type
				,trfs_record_struct_file->flags
				,trfs_record_struct_file->permission_mode
				,trfs_record_struct_file->path_len
				,trfs_record_struct_file->success_flag
				,trfs_record_struct_file->path_name);
		}
		if(strict_mode && opt_flag == false){
			printf("\nABORTING AS FAILURE IN STRICT MODE");
			goto out;
		}
		bytes_read = bytes_read + record_size + RECORD_CHECKSUM_SIZE;
		printf("\n\n-------------------------------------\n\n");
		if(trfs_record_struct_file)
			free(trfs_record_struct_file);
		trfs_record_struct_file = NULL;	
	}
out:
	if(trfs_record_struct_file)
		free(trfs_record_struct_file);
	if(first_file_path)
		free(first_file_path);
	if(second_file_path)
		free(second_file_path);
	printf("\n");
	ret = cleanup_list();
	fclose(file_pointer);
	return ret;
}
int cleanup_list(){
	struct list_file* curr = NULL;
	while(my_file_list){
		curr = my_file_list->next;
		free(my_file_list);
		my_file_list = curr;
	}	
	my_file_list = NULL;	
	return 0;
}
void show(struct trfs_record_struct_file * trfs_record_struct_file){
/*	printf("\nFILE DATA: \nRecord no: %d \nRecord size: %d \nType:%d \nFlags: %d \nPermissions: %d\nPath len: %d\nSuccess flag: %d \nPath name: %s"
                                ,trfs_record_struct_file->record_no
                                ,trfs_record_struct_file->buf_size
                                ,trfs_record_struct_file->type
                                ,trfs_record_struct_file->flags
                                ,trfs_record_struct_file->permission_mode
                                ,trfs_record_struct_file->path_len
                                ,trfs_record_struct_file->success_flag
                                ,trfs_record_struct_file->path_name);
                //bytes_read = bytes_read + record_size + RECORD_CHECKSUM_SIZE;
                printf("\n\n-------------------------------------\n\n");
                free(trfs_record_struct_file);
		
*/
}

bool replay_read(struct trfs_record_struct_file * trfs_record_struct_file){

	char *file_buf = NULL;
	int bytes_read = -1;
	char *second_ptr = NULL; 
	set_first_file_path(trfs_record_struct_file);
	second_ptr = get_extra_info(trfs_record_struct_file, TRFS_OP_READ);
	f_ptr = get_fd(trfs_record_struct_file->k_fd);
	if (f_ptr<0) {
		printf("\nReplay read operation failed, Perhaps Open not traced %d ", errno);	
		return false;
	}

	printf("\nOriginal Data read from file -> %s",second_ptr);           
	file_buf = (char *)malloc(strlen(second_ptr)+1);
	bytes_read = read(f_ptr,file_buf,strlen(second_ptr));
	
	file_buf[strlen(second_ptr)] = '\0';

	printf("\nData Read in treplay -> %s", file_buf);
	if (strcmp(file_buf,second_ptr)==0)
	{  
		printf("\nSame data is read in treplay operation");
		return trfs_record_struct_file->success_flag == bytes_read ? true : false;
		free(file_buf);
        	free(second_ptr);
		return true;
	}	
	free(file_buf);
	free(second_ptr); 
	return false;
	//return trfs_record_struct_file->success_flag == bytes_read ? true : false;
}

bool replay_write(struct trfs_record_struct_file * trfs_record_struct_file){

	int bytes_written = -1;   
	char *second_ptr = NULL;
	f_ptr = get_fd(trfs_record_struct_file->k_fd);
	if (f_ptr < 0) {
		printf("\nReplay write operation failed, Perhaps open not traced");
		return false;
	}  
	set_first_file_path(trfs_record_struct_file);
	second_ptr = get_extra_info(trfs_record_struct_file, TRFS_OP_WRITE);      
	
	printf("\nData Written to file -> %s", second_ptr);
/*	file_buf = (char *)malloc(strlen(ptr)+1);
	strcpy(file_buf,ptr);
	file_buf[strlen(ptr)] = '\0';
*/
	bytes_written = write(f_ptr,second_ptr,strlen(second_ptr));
	free(second_ptr);
	return trfs_record_struct_file->success_flag == bytes_written ? true : false;
}

bool replay_rmdir(struct trfs_record_struct_file * trfs_record_struct_file){
	set_first_file_path(trfs_record_struct_file);        
	int err =0;
	printf("\nRun rm dir replayer on file: %s",first_file_path);
	err = rmdir(first_file_path);         
	return trfs_record_struct_file->success_flag == err ? true : false;
}

bool replay_mkdir(struct trfs_record_struct_file * trfs_record_struct_file){
	set_first_file_path(trfs_record_struct_file);
	int err=0;
	printf("\nRun Make dir replayer on file: %s",first_file_path);
	err = mkdir(first_file_path, trfs_record_struct_file->permission_mode);         //giving default permissions for now
	return trfs_record_struct_file->success_flag == err ? true : false;
}

bool replay_unlink(struct  trfs_record_struct_file  * trfs_record_struct_file){
	set_first_file_path(trfs_record_struct_file);
	int err = 0;
	printf("\nRun unlink replayer on file: %s",first_file_path);
	err = unlink(first_file_path);
	return trfs_record_struct_file->success_flag == err ? true : false;
}

bool replay_hardlink(struct trfs_record_struct_file  * trfs_record_struct_file){

	set_first_file_path(trfs_record_struct_file);
	set_second_file_path(trfs_record_struct_file);

	printf("\nFirst Pathname:%s",first_file_path); 
	printf("\nSecond pathname:%s",second_file_path);    
	printf("\nRun hardlink replayer on files: %s, %s\n",first_file_path,second_file_path);
	int err = link(first_file_path, second_file_path);
	return trfs_record_struct_file->success_flag == err ? true : false;       
}

bool replay_symlink(struct trfs_record_struct_file  * trfs_record_struct_file){
	char *second_ptr = NULL;
	set_first_file_path(trfs_record_struct_file);
	second_ptr = get_extra_info(trfs_record_struct_file, TRFS_OP_SYMLINK);
	printf("\nFirst Pathname:%s",first_file_path);
	printf("\nSecond Pathname: %s",second_ptr);
	printf("\nRun symlink replayer on files: %s, %s",first_file_path,second_ptr);       // reverse the order of filenames after the correction.
	int err = symlink(second_ptr, first_file_path);
	free(second_ptr);
	return trfs_record_struct_file->success_flag == err ? true : false;
}

bool replay_rename(struct trfs_record_struct_file  * trfs_record_struct_file){

	set_first_file_path(trfs_record_struct_file);
	set_second_file_path(trfs_record_struct_file); 
	printf("\nRun rename replayer on files: %s  %s",first_file_path , second_file_path);     
	int err = rename(first_file_path, second_file_path);
	
	printf("Rename err %d", err);
	return trfs_record_struct_file->success_flag == err ? true : false;		
	
}

bool replay_create(struct trfs_record_struct_file  * trfs_record_struct_file){
	set_first_file_path(trfs_record_struct_file);
	printf("\nReplay create operaion on file: %s",first_file_path);
	f_ptr = creat(first_file_path,trfs_record_struct_file->permission_mode);//TODO: FILE PERMISSION figure out in what mode to open the file
	printf("\nPermission mode = %d", trfs_record_struct_file->permission_mode);
	if((f_ptr >= 0 && trfs_record_struct_file->success_flag == 0) || (f_ptr == trfs_record_struct_file->success_flag) ) {
		printf("Create Reply Successful\n");
                return true;
	}
	printf("\nCREATE FAILED");	
	return false;	
}


bool replay_open(struct trfs_record_struct_file  * trfs_record_struct_file){
	set_first_file_path(trfs_record_struct_file);	

	f_ptr = open (first_file_path,trfs_record_struct_file->flags,trfs_record_struct_file->permission_mode);//TODO: FILE PERMISSION figure out in what mode to open the file
	add_node(trfs_record_struct_file->k_fd , f_ptr);
	
	if((f_ptr >= 0 && trfs_record_struct_file->success_flag == 0) || (f_ptr == trfs_record_struct_file->success_flag) ) {
			//TODO clean_up
			
		printf("\nReplay open operation successful: %d", errno);
		return true;
	}
	return false;
}

bool replay_close(struct trfs_record_struct_file  * trfs_record_struct_file){
	int err = 0;
	f_ptr = -1;
	f_ptr = get_fd(trfs_record_struct_file->k_fd); 
	if(f_ptr < 0){
		printf("\nReplay close operation failed, No fd, perhaps open not traced");
		return true;
	}	
	err = close(f_ptr);
	delete_node(f_ptr);	
	
	f_ptr = -1;
	return trfs_record_struct_file->success_flag == err ? true : false;
}

void show_read(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nREAD------->  %s", trfs_record_struct_file->path_name);
	char * ex = NULL;
	ex = get_extra_info(trfs_record_struct_file, trfs_record_struct_file->type);
	printf("\nExtra info: %s",ex);
	free(ex);
}

void show_write(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nWRITE------>  %s", trfs_record_struct_file->path_name);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);
	char * ex = NULL;
	ex = get_extra_info(trfs_record_struct_file, trfs_record_struct_file->type);
	printf("\nExtra info: %s",ex);
	free(ex);
}

void show_rmdir(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nRMDIR------> %s", trfs_record_struct_file->path_name);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);
}

void show_mkdir(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nMKDIR------> %s", trfs_record_struct_file->path_name);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);
}

void show_unlink(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nUNLINK-----> %s", trfs_record_struct_file->path_name);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);
}

void show_hardlink(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nHARDLINK----> %d %d %s ", trfs_record_struct_file->path_len, trfs_record_struct_file->buf_size,trfs_record_struct_file->path_name);
	//	printf("\nHARDLINK----> %s %s", trfs_record_struct_file->path_name,trfs_record_struct_file->path_name+strlen(trfs_record_struct_file->path_name)+1);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);

}

void show_symlink(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nSymlink-----> %s %s", trfs_record_struct_file->path_name ,trfs_record_struct_file->path_name+strlen(trfs_record_struct_file->path_name)+1);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);
	char * ex = NULL;
	ex = get_extra_info(trfs_record_struct_file, trfs_record_struct_file->type);
	printf("\nExtra info: %s",ex);
	free(ex);
}

void show_rename(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nRENAME %s %s", trfs_record_struct_file->path_name, trfs_record_struct_file->path_name+strlen(trfs_record_struct_file->path_name)+1);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);

}

void show_create(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nCREATE %s",trfs_record_struct_file->path_name);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);
}

void show_open(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nOPEN %s",trfs_record_struct_file->path_name);
	//printf("\nReturn value %d", trfs_record_struct_file->success_flag);
}

void show_close(struct trfs_record_struct_file * trfs_record_struct_file){
	//	printf("\nClose %s",trfs_record_struct_file->path_name);
}

