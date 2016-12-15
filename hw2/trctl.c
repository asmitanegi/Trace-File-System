#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>  
#include <sys/ioctl.h>  
#include <stdlib.h>
//#include "trctl.h"
#include <string.h>
#include <stdbool.h>
#include "../fs/trfs/record.h"

bool e_ioctl = false;


struct toggle_ops
{
	int add_bits;
	int rem_bits;
};
//#endif


int main(int argc,char *argv[])
{
	int fd,ret_value=0;
	int bitmap = 0x00000000;
	int *ret_arg = NULL;
	int *arg = NULL;
	char *mnt_path = NULL;
	char *cmd = NULL;
        int i;
        
	#ifdef EXTRA_CREDIT
	e_ioctl = true;
	#endif

//#ifdef EXTRA_CREDIT
	struct toggle_ops *toggle_ops;
        toggle_ops = (struct toggle_ops *)malloc(sizeof(struct toggle_ops));
        toggle_ops->add_bits = 0x00000000;
        toggle_ops->rem_bits = 0xffffffff;
//#endif       

	arg = &bitmap;
	ret_arg = &bitmap;

	if(e_ioctl == true){
               
                if (argc>=3)
                {
                        mnt_path = argv[argc-1];
                        i =1;  
			while(i<= (argc-2))
			{    
       //                         printf("Entered while loop\n");
				cmd = argv[i];
				/* if (err_flag == 1)
				   { 
				   free(toggle_ops);
				   exit(-1);                     
				   }
				   */		        
				if (strcasecmp(cmd,"all") == 0)                  
				{
					toggle_ops->add_bits = 0xffffffff;
					toggle_ops->rem_bits = 0xffffffff;
					break;
				} 
				else if (strcasecmp(cmd,"none")==0)
				{
					toggle_ops->add_bits = 0x00000000;
					toggle_ops->rem_bits = 0x00000000;
					break;
				}
				else if ((cmd[0]=='-') || (cmd[0]=='+'))
				{   

					if(strcmp("create", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000001;
						}
						else if(cmd[0] == '-'){
                                            //            printf("Entered -create call\n");
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xfffffffe;
						}
						else{
							goto out;
						}
					}		
					else if(strcmp("open", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000002;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xfffffffd;
						}
						else{
							goto out;
						}
					}
					else if(strcmp("close", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000004;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xfffffffb;
						}
						else{
							goto out;
						}
					}
					else if(strcmp("mkdir", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000008;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xfffffff7;
						}
						else{

							goto out;
						}
					}
					else if(strcmp("rmdir", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000010;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xffffffef;
						}
						else{
							goto out;
						}
					}
					else if(strcmp("unlink", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000020;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xffffffdf;
						}
						else{
							goto out;
						}
					}
					else if(strcmp("read", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000040;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xffffffbf;
						}
						else{
							goto out;
						}
					}
					else if(strcmp("write", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000080;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xffffff7f;
						}
						else{
							goto out;
						}
					}
					else if(strcmp("symlink", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000100;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xfffffeff;
						}
						else{
							goto out;
						}
					}
					else if(strcmp("hardlink", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000200;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xfffffdff;
						}
						else{
							goto out;
						}
					}
					else if(strcmp("rename", cmd+1) == 0){
						if(cmd[0] == '+'){
							toggle_ops->add_bits = toggle_ops->add_bits | 0x00000400;
						}
						else if(cmd[0] == '-'){
							toggle_ops->rem_bits = toggle_ops->rem_bits & 0xfffffbff;
						}
						else{
							goto out;
						}
					}
					else{
						goto out;
					}

				} 

				else
				{

					bitmap = (int)strtol(cmd, NULL, 16);
					printf("bitmap value being set: %x\n",bitmap);
					toggle_ops->add_bits = bitmap;
					toggle_ops->rem_bits = bitmap;
					break;
				}

				i++;
			}
                    
			fd = open(mnt_path,O_DIRECTORY);
			if (fd<0)
			{
				printf ("Invalid mount point\n");
                                if (toggle_ops)
                                {
	                            free(toggle_ops);
                                    toggle_ops = NULL;
                                }
                                exit(-1);
			}
			/*set bitmap*/
                  //      printf("run the ioctl call\n");
			ret_value = ioctl(fd, TRFS_IOCTL_SET_MSG,(void *)toggle_ops);
			if (toggle_ops)
                        {
                           free(toggle_ops);       
                           toggle_ops = NULL;
			}
                        if (ret_value < 0) {
				printf ("trfs_ioctl_set_msg failed:%d\n", ret_value);
				close(fd);
				exit(-1);
			}
			close(fd);

                }
                else if (argc == 2)
		{
        	 	mnt_path = argv[1];
        		fd = open(mnt_path,O_DIRECTORY);
        		if (fd<0)
       		 	{
                		printf ("Invalid mount point\n");
               			 exit(-1);
        		}
        		/*retrieve bitmap*/
       			 ret_value = ioctl(fd, TRFS_IOCTL_GET_MSG,ret_arg);
        		  if (ret_value < 0) {
                		printf ("trfs_ioctl_get_msg failed:%d\n", ret_value);
                		close(fd);
                		exit(-1);
      		  	  }
			 printf("Retrieved bitmap value:%x\n", bitmap);
        		 close(fd);
		}
		else
		{

        		printf("invalid arguments\n");

		}   	
                        
	}
	else
        {
           	cmd = argv[1];
                mnt_path = argv[2];
               
                if(argc==3)
                {
                   
		        if (strcasecmp(cmd,"all") == 0) 
                        {         
	                   bitmap = 0xffffffff;

                        }              
		       else if (strcasecmp(cmd,"none") == 0)
		        { 	
	                   bitmap = 0x00000000;
                        }
                        else 
                        {                        
	                   bitmap = (int)strtol(cmd, NULL, 16);
  	                   printf("bitmap value being set: %x\n",bitmap);
                        }
                       fd = open(mnt_path,O_DIRECTORY);
		       if (fd<0)
		       {
                          printf ("Invalid mount point\n");
        		  exit(-1);
                       }
		       /*set bitmap*/
			ret_value = ioctl(fd, TRFS_IOCTL_SET_MSG,(void *)arg);
			if (ret_value < 0) {
        			printf ("trfs_ioctl_set_msg failed:%d\n", ret_value);
        			close(fd);
       				 exit(-1);
			}
			close(fd);
               }
               else if (argc == 2)
               {
                        mnt_path = argv[1];
                        fd = open(mnt_path,O_DIRECTORY);
                        if (fd<0)
                        {
                                printf ("Invalid mount point\n");
                                 exit(-1);
                        }
                        /*retrieve bitmap*/
                         ret_value = ioctl(fd, TRFS_IOCTL_GET_MSG,ret_arg);
                          if (ret_value < 0) {
                                printf ("trfs_ioctl_get_msg failed:%d\n", ret_value);
                                close(fd);
                                exit(-1);
                          }
                         printf("Retrieved bitmap value:%x\n", bitmap);
                         close(fd);
                }
                else
                {

                        printf("invalid arguments\n");

                }

	 }



out:
if (toggle_ops)
   {
      free(toggle_ops);
      toggle_ops = NULL;
   }

return 0;

}

