#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#define A1 0x11
#define A2 0x22
#define A3 0x33
#define A4 0x44


int main (int argc, char *argv[])
{
    //printf ("%s\n",argv[2]);
	char writebuf[100];
	char readbuf[100];
	int fd = 0;
    int ret = 0;
	char *filename;
	filename = argv[1];
    static char usrdata[] = {"usr data!"};
    //int ioct (int fd, int cmd, ...int args);

    if (argc !=3 ){
        printf("Error usage! 18 \n");
        return -1;
    }

/* open */
	fd = open(filename,O_RDWR);
	if(fd<0){
	printf("Can't open file %s\n, filename");
	return -1;
	}

/* ioctl */ 
ret = ioctl(fd,100);
printf("fd= %d\n",fd);
printf("A1= %d\n",A1);
if(ret = 0){
    printf ("ioctl succeed\n");
}
else{
    printf ("ioctl failed\n");
}

/* read */
    if (atoi(argv[2])==1){
	    ret = read(fd,readbuf,50);
	    if (ret<0){
	    printf("read file %s failed , filename");
	    }
	    else{
        printf("App read data:%s\n",readbuf);
	    }

    }

    
/* write */
if (atoi(argv[2])==2){
        memcpy(writebuf,usrdata,sizeof(usrdata));
	    ret = write(fd,writebuf,50);
	    if (ret<0){
	    printf("write file %s failed , filename");
	    }
	    else{
	    }
        //printf("okkk\n");
    }
 
/* close */
	ret = close(fd);
	if(ret<0){
	printf("close file %s falied\n,filename");
	}

	return 0;
}
