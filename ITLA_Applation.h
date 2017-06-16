/*
 * Copyright © 2017-now Jiaolong Tong <tongjiaolong@yeah.net>
 *
*/

#ifndef _ITLA_APPLATION_H_
#define _ITLA_APPLATION_H_

/* Add this for macros that defined unix flavor */
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif


#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ITLA_Core.h"
#define  SPEED_OF_LIGHT                   (299792458.0)
#define  FrequencyG_to_Wavelength(fg)    (SPEED_OF_LIGHT/fg)
#define  Wavelength_to_FrequencyG(wl)    (SPEED_OF_LIGHT/wl)
#define  Frequency_GtoT(fg)              (fg/1000)

void itla_get_response_timeout(itla_t *ctx, struct timeval *timeout);  
void itla_set_response_timeout(itla_t *ctx, const struct timeval *timeout);
void itla_get_byte_timeout(itla_t *ctx, struct timeval *timeout);
void itla_set_byte_timeout(itla_t *ctx, const struct timeval *timeout);
int  itla_connect(itla_t *ctx); 
void itla_close(itla_t *ctx);   
int  itla_flush(itla_t *ctx);  
void itla_free(itla_t *ctx);   
void itla_set_debug(itla_t *ctx, int boolean); 

itla_mapping_t* itla_mapping_new(); 
void itla_mapping_free(itla_mapping_t *mb_mapping); 
int itla_read_registers(itla_t *ctx, int function, int abs_addr,uint16_t *dest);
int itla_write_registers(itla_t *ctx, int function, int abs_addr,uint16_t reg_data,uint16_t *dest);

/*General Optical*/
void  wait_sucessful(itla_t *ctx);
void  resetModule(itla_t *ctx);
void  disalbeOutput(itla_t *ctx);
void  enalbeOutput(itla_t *ctx);

/*Get Optical Information*/
float getLaserFrequency (itla_t *ctx);
float getOpticalOutputPower(itla_t *ctx);
float getCurrentTemperature(itla_t *ctx);
float getOpticalPowerMin(itla_t *ctx);
float getOpticalPowerMax(itla_t *ctx);
float getFrequencyMin(itla_t *ctx);
float getFrequencyMax(itla_t *ctx);
float getGridSpacingMin(itla_t *ctx);
/*Set Optical Configure*/
int setLaserWavelength(itla_t *ctx,float wavelength);
int setOpticalPower(itla_t *ctx,float power);
int setOpticalChanel(itla_t * ctx,uint16_t chanel);
int setGradSpacing(itla_t * ctx,float Space);

#endif 
