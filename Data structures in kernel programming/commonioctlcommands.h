//These are our ioctl definition
#define MAGIC 'M'
#define IOC_MAXNR 3
#define IOCTL_SFIFO_RESET _IOR(MAGIC, 0, char)
#define IOCTL_SSTACK_RESET _IOR(MAGIC, 1, char)
#define IOCTL_ALL_RESET _IOR(MAGIC, 2, char)

