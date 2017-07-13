#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>


int read_ADvalue(char * device_name, float *value){
    int count,ret,fd,tmp;
    char buf[20];
    char filename[80];
    sprintf(filename,"/sys/bus/iio/devices/iio:device0/%s",device_name);    
    
    memset(buf,0,sizeof(buf));
    
    fd =open(filename,O_RDONLY);
    count = read(fd, buf, sizeof(buf));	
    close(fd);
    if(count > 0 ){
         sscanf( buf, "%d", &tmp );
         tmp = tmp & 0x0FFF;
         //printf("AD  : %s \n", buf);
         *value = ((float)tmp/4096.0)*1.8;
         return 1;
    }else{
         printf("get AD error\n");
         return -1;
    }

}


int main(int argc ,char ** argv){

        float ad_value;
        int fd = open("./dat.txt",O_RDWR|O_CREAT|O_TRUNC);
	char buf[20]; 																											
        if(fd<0)return -1;
        else{
		lseek(fd,0,SEEK_SET);
	}
        while(1){
           /**/          
           //printf("<-----Start Scaning AD chanel------>\n");
           read_ADvalue("in_voltage0_raw",&ad_value);
           sprintf(buf,"%f\n",ad_value);
	   write(fd,buf,strlen(buf));
           //usleep(2);
        }
        return 0;
}
