#ifndef TRCTL_H
#define TRCLT_H

#include<linux/ioctl.h>

#define APPTYPE 101

#define TRFS_IOCTL_SET_MSG _IOW(APPTYPE, 0, void *)
#define TRFS_IOCTL_GET_MSG _IOR(APPTYPE, 1, char *)

#endif 
