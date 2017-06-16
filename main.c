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
        while(1){
           /**/
           
           printf("<-----Start Scaning AD chanel------>\n");
           read_ADvalue("in_voltage0_raw",&ad_value);
           usleep(1000);
           printf("AIN0 value: %f\n",ad_value);
	   read_ADvalue("in_voltage1_raw",&ad_value);
           usleep(1000);
           printf("AIN1 value: %f\n",ad_value);
	   read_ADvalue("in_voltage2_raw",&ad_value);
           usleep(1000);
           printf("AIN2 value: %f\n",ad_value);
	   read_ADvalue("in_voltage3_raw",&ad_value);
           usleep(1000);
           printf("AIN3 value: %f\n",ad_value);
	   read_ADvalue("in_voltage4_raw",&ad_value);
           usleep(1000);
           printf("AIN4 value: %f\n",ad_value);
	   read_ADvalue("in_voltage5_raw",&ad_value);
           usleep(1000);
           printf("AIN5 value: %f\n",ad_value);
	   read_ADvalue("in_voltage6_raw",&ad_value);
           usleep(1000);
           printf("AIN6 value: %f\n",ad_value);

/*
	   read_ADvalue("in_voltage7_raw",&ad_value);
           //usleep(1000);
           printf("AIN7 value: %f\n",ad_value);
*/
           sleep(1);
        }
        return 0;
}
