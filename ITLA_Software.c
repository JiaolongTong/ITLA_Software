
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <linux/input.h>
#include "ITLA_Applation.h"
#include "ITLA_Physical.h"
#define  MAX_CHANEL  80

const float sin_test_array[MAX_CHANEL] ={
1,	
1.07945016697171,	1.15839802440722,	1.23634443853291,	1.31279660702223,	1.38727117465064,	1.45929728922298,	1.52841957845262,	1.59420102897171,	1.65622574927038,	1.71410159909675,	1.76746266869391,	1.81597159219916,	1.85932168057966,	1.89723886061921,	1.92948340769739,	1.95585146140576,	1.97617631441905,	1.99032946647250,	1.99822143678193,	1.99980232977007,	1.99506215052243,	1.98403086797842,	1.96677822545804,	1.94341329972213,	1.91408381135403,	1.87897519082243,	1.83830940613028,	1.79234355946157,	1.74136826169862,	1.68570579508641,	1.62570807566056,	1.56175442832077,	1.49424918861672,	1.42361914640853,	1.35031084756382,	1.27478777075103,	1.19752739717795,	1.11901819180190,	1.03975651509693,	0.960243484903075,	0.880981808198095,	0.802472602822048,	0.725212229248975,	0.649689152436176,	0.576380853591472,	0.505750811383277,	0.438245571679229,	0.374291924339441,	0.314294204913587,	0.258631738301382,	0.207656440538426,	0.161690593869724,	0.121024809177566,	0.0859161886459653,	0.0565867002778656,	0.0332217745419630,	0.0159691320215795,	0.00493784947757259,	0.000197670229934444,	0.00177856321806702,	0.00967053352749803,	0.0238236855809491,	0.0441485385942392,	0.0705165923026127,	0.102761139380788,	0.140678319420339,	0.184028407800839,	0.232537331306092,	0.285898400903250,	0.343774250729624,	0.405798971028289,	0.471580421547380,	0.540702710777022,	0.612728825349359,	0.687203392977772,	0.763655561467095,	0.841601975592785,	0.920549833028285,	1.00000000000000,
};

typedef struct laser_data_t {
        float  d_ad_power;
        float  laser_adpower;
        float  laser_outpower;
        float  laser_frequency;
        float  laser_wavelength;
        int    laser_chanel;          
}laser_data;

int Partion(laser_data array[],int begin,int end){

  int i= begin;
  int j= end;
  laser_data tmp,look= array[begin];
  while(i!=j ){
        while( array[j].d_ad_power>= look.d_ad_power && i<j) j--;             //Min  
        while( array[i].d_ad_power<= look.d_ad_power && i<j) i++;
        if(i<j){
		tmp= array[j];
		array[j] =array[i];
		array[i] =tmp;
        }
  }
  
  array[begin]=array[i];
  array[i] = look;

  return i; 
}
int findMax(laser_data array[],int len){
     int k=1;
     int index=0;
     int begin = 0;
     int end = len-1;
     index=Partion(array,begin,end);
     while(index != k-1){
          index=Partion(array,begin,end);
          if(index>k-1)end   = index-1;
          if(index<k-1)begin = index+1; 
     }
     
     return index;
}


int read_ADvalue(char * device_name, float *value){
    int count,ret,fd,tmp;
    char buf[20];
    char filename[80];
    sprintf(filename,"/sys/bus/iio/devices/iio:device0/%s",device_name);    

    
    fd =open(filename,O_RDONLY);
    count = read(fd, buf, sizeof(buf));	
    close(fd);
    if(count > 0 ){
         sscanf( buf, "%d", &tmp );
         *value = ((float)tmp/4096.0)*1.8;
         return 1;
    }else{
         printf("get AD error\n");
         return -1;
    }

}

int main(int argc ,char ** argv){

        if(argc != 3){
           printf("Plase input chanel range.Usage: ./main [begin chanel]  [end chanel] \n");
           return -1;
        }
	itla_t*  ctx=NULL;
	struct input_event ev_key;
        int  begin_chanel   = atoi(argv[1]);
        int  end_chanel     = atoi(argv[2]);
        int  length = end_chanel-begin_chanel +1;
        laser_data  data[length];
        int   key_fd=open("/dev/input/event0",O_RDWR);       
	ctx = (itla_t*)itla_new_device("/dev/ttyO2",9600,'N',1,1);
        if(ctx==NULL) return -1;
	itla_set_debug(ctx,FALSE);
	if(-1==itla_connect(ctx)){
            return -1;
        }
//************Init Processing*************/
        disalbeOutput(ctx);
        setOpticalPower(ctx,10);
        setLaserWavelength(ctx,1560.606236);
        setGradSpacing(ctx,50);
        setOpticalChanel(ctx,1);
        enalbeOutput(ctx);

/*************Init information*************/
        printf("Chanel Range :[NO.%d] ---->[NO.%d] \n",begin_chanel,end_chanel); 
        float CTemp    = getCurrentTemperature(ctx);
        printf("Current Temperature :%f \n",CTemp);

        float PWR_min = getOpticalPowerMin(ctx);
        printf("Optical Power Min :%f dBm\n",PWR_min);

        float PWR_max = getOpticalPowerMax(ctx);
        printf("Optical Power Max :%f dBm\n",PWR_max);

        float Freq_min =getFrequencyMin(ctx);
        printf("Frequency Min:  %fTHz  %fnm\n",Frequency_GtoT(Freq_min),   FrequencyG_to_Wavelength(Freq_min)  );

        float Freq_max =getFrequencyMax(ctx);
        printf("Frequency Max:  %fTHz  %fnm\n",Frequency_GtoT(Freq_max),   FrequencyG_to_Wavelength(Freq_max)  );

        float GRID_min= getGridSpacingMin(ctx);
        printf("Grid Spacing Min :%f GHz \n",GRID_min);

//************Chanel Scaning*************/
        int   chanel;
        int   count,index=0;
        float ad_value;
        float Freq_G;
        float PWR;
        while(1){
            count = read(key_fd, &ev_key, sizeof(struct input_event));		
              if (count > 0){
                   if (EV_KEY == ev_key.type){
                        switch(ev_key.code){
                            case 102 :
                                if(ev_key.value == 0){
                                        printf("------------Start Scaning ------------\n");
					for(chanel = begin_chanel,index=0 ;chanel <= end_chanel ;chanel++,index++){
					     disalbeOutput(ctx);
					     setOpticalChanel(ctx,chanel);
					     enalbeOutput(ctx);
					     Freq_G = getLaserFrequency(ctx);
                                             PWR = getOpticalOutputPower(ctx);
                                             read_ADvalue("in_voltage4_raw",&ad_value);
                                             data[index].laser_adpower       = sin_test_array[chanel-1];//ad_value;
                                             data[index].laser_outpower      = PWR;
                                             data[index].laser_chanel        = chanel;
                                             data[index].laser_wavelength    = FrequencyG_to_Wavelength(Freq_G);
                                             data[index].laser_frequency     = Frequency_GtoT(Freq_G);
                                             if(index==0)data[index].d_ad_power = 0;
                                             else data[index].d_ad_power     = data[index].laser_adpower -  data[index-1].laser_adpower;
                                             //data[index].d_ad_power = data[index].d_ad_power < 0 ? -data[index].d_ad_power:data[index].d_ad_power;
                                             printf("Chanel[%d] Frequency:%fTHz  %fnm<--->AD value :%fv<---->Df:%fv/nm<---->Out power %2.4fdBm\n",
                                                     data[index].laser_chanel,
                                                     data[index].laser_frequency,
                                                     data[index].laser_wavelength,
                                                     data[index].laser_adpower,
                                                     data[index].d_ad_power,
                                                     data[index].laser_outpower
                                             );  
					} 
                               		printf("------------Searching Best Chanel------------\n");
		                        index=findMax(data,length);
		                        printf("The Best Chanel :%d<--->Df:%fv/nm<--->Frequency:%fTHz  %fnm \n",
                                               data[index].laser_chanel,
                                               data[index].d_ad_power,
                                               data[index].laser_frequency,
                                               data[index].laser_wavelength);  
                               		printf("------------Setting to best Chanel:%d------------\n",data[index].laser_chanel);
                                        disalbeOutput(ctx);
					setOpticalChanel(ctx,data[index].laser_chanel);
					enalbeOutput(ctx); 
					Freq_G   = getLaserFrequency(ctx);
					CTemp    = getCurrentTemperature(ctx);
					printf("Current Frequency:%fTHz  %fnm<--->Temperature:%f\n",Frequency_GtoT(Freq_G),FrequencyG_to_Wavelength(Freq_G),CTemp); 
                                }
                                break;
                            default:printf("Don't have function !\n");
                        }
                  }
              }
        }
        return 0;
}
