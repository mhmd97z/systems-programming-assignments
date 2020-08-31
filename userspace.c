#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "commonioctlcommands.h"

int main(int argc, char *argv[]){

  	if (argc != 2){
  		printf("module3_ioctl: argc != 2, BAD INPUT\n");
  		return 1;
  	}
    
    char output[30];
  	int fd = open("/proc/module3_ioctl", O_RDONLY);
    
  	if(fd > 0 ){

  		if( !(strcmp(argv[1], "fifo")) ){
  			ioctl(fd, IOCTL_SFIFO_RESET, output);

	    }
	    else if( !(strcmp(argv[1], "stack")) ){
	   		ioctl(fd, IOCTL_SSTACK_RESET, output);

	    }
	    else if( !(strcmp(argv[1], "all")) ) {
			ioctl(fd, IOCTL_ALL_RESET, output);

	    }
	    else{
	    	printf("invalid input");

		}
	close(fd);

    }

	return 0;
}
