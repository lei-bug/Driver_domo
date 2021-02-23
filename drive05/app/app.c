#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main(int argc,char *argv[])
{
    int fd;
	/* 打开文件 */
    if ((fd=open("/dev/second_drv",O_RDONLY )) < 0)
    {
        printf("Open Device  failed.\n");
        exit(1);
    }
    else
        while(1)
            {				
            	printf("in the while(1)!\n");				
            }
	return 0;
}