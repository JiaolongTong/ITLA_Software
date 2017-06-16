/*
 * Copyright © 2017-now Jiaolong Tong <tongjiaolong@yeah.net>
 *
*/
#include "ITLA_Applation.h"
int _sleep_and_flush(itla_t *ctx)
{
    /* usleep source code */
    struct timespec request, remaining;
    request.tv_sec = ctx->response_timeout.tv_sec;
    request.tv_nsec = ((long int)ctx->response_timeout.tv_usec % 1000000)
        * 1000;
    while (nanosleep(&request, &remaining) == -1 && errno == EINTR)
        request = remaining;
    return rtla_flush(ctx);
}

int rtla_flush(itla_t *ctx)
{
    int rc = ctx->backend->flush(ctx);
    if (rc != -1 && ctx->debug) {
        printf("%d bytes flushed\n", rc);
    }
    return rc;
}


void itla_free(itla_t *ctx)
{
    if (ctx == NULL)
        return;
    free(ctx->backend_data);
    free(ctx);
}

void itla_set_debug(itla_t *ctx, int boolean)
{
    ctx->debug = boolean;
}

void itla_close(itla_t *ctx)
{
    if (ctx == NULL)
        return;
    ctx->backend->close(ctx);
}

int itla_connect(itla_t *ctx)
{
    return ctx->backend->connect(ctx);
}


void itla_get_response_timeout(itla_t *ctx, struct timeval *timeout)
{
    *timeout = ctx->response_timeout;
}

void itla_set_response_timeout(itla_t *ctx, const struct timeval *timeout)
{
    ctx->response_timeout = *timeout;
}


void itla_get_byte_timeout(itla_t *ctx, struct timeval *timeout)
{
    *timeout = ctx->byte_timeout;
}

void itla_set_byte_timeout(itla_t *ctx, const struct timeval *timeout)
{
    ctx->byte_timeout = *timeout;
}
itla_mapping_t* modbus_mapping_new()
{
    itla_mapping_t *mb_mapping;
    mb_mapping = (itla_mapping_t *)malloc(sizeof(itla_mapping_t));
    if (mb_mapping == NULL) {
        return NULL;
    }
    int nb_reg;
    /*********GENERAL_REG*************/
    nb_reg = NB_GENERAL_REG;
    if (nb_reg == 0) {
        mb_mapping->general_registers = NULL;
    } else {
        /* Negative number raises a POSIX error */
        mb_mapping->general_registers = (uint16_t *) malloc(nb_reg * sizeof(uint16_t));
        if (mb_mapping->general_registers == NULL) {
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->general_registers, 0, nb_reg * sizeof(uint16_t));
    }
    /*********STATUS_REG*************/
    nb_reg = NB_STATUS_REG;
    if (nb_reg == 0) {
        mb_mapping->status_registers = NULL;
    } else {
        /* Negative number raises a POSIX error */
        mb_mapping->status_registers = (uint16_t *) malloc(nb_reg * sizeof(uint16_t));
        if (mb_mapping->status_registers == NULL) {
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->status_registers, 0, nb_reg * sizeof(uint16_t));
    }
    /*********OPTICAL_REG*************/
    nb_reg = NB_OPTICAL_REG;
    if (nb_reg == 0) {
        mb_mapping->optical_registers = NULL;
    } else {
        /* Negative number raises a POSIX error */
        mb_mapping->optical_registers = (uint16_t *) malloc(nb_reg * sizeof(uint16_t));
        if (mb_mapping->optical_registers == NULL) {
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->optical_registers, 0, nb_reg * sizeof(uint16_t));
    }
    /*********CAPABILITILES_REG*************/
    nb_reg = NB_CAPABILITILES_REG;
    if (nb_reg == 0) {
        mb_mapping->capablities_registers = NULL;
    } else {
        /* Negative number raises a POSIX error */
        mb_mapping->capablities_registers = (uint16_t *) malloc(nb_reg * sizeof(uint16_t));
        if (mb_mapping->capablities_registers == NULL) {
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->capablities_registers, 0, nb_reg * sizeof(uint16_t));
    }
    /*********MSA_REG*************/
    nb_reg = NB_MSA_REG;
    if (nb_reg == 0) {
        mb_mapping->msa_registers = NULL;
    } else {
        /* Negative number raises a POSIX error */
        mb_mapping->msa_registers = (uint16_t *) malloc(nb_reg * sizeof(uint16_t));
        if (mb_mapping->msa_registers == NULL) {
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->msa_registers, 0, nb_reg * sizeof(uint16_t));
    }
    /*********NB_MANUFACTURER_REF*************/
    nb_reg = NB_MANUFACTURER_REG;
    if (nb_reg == 0) {
        mb_mapping->manufacturer_registers = NULL;
    } else {
        /* Negative number raises a POSIX error */
        mb_mapping->manufacturer_registers = (uint16_t *) malloc(nb_reg * sizeof(uint16_t));
        if (mb_mapping->manufacturer_registers == NULL) {
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->manufacturer_registers, 0, nb_reg * sizeof(uint16_t));
    }

    return mb_mapping;
}


int  error_msg_handle(itla_t *ctx,uint8_t status){
  
     status =status &0x03;
     uint16_t ret = 0;
     switch(status){

           case 0x00 :  break;
           case 0x01 :  {
		        itla_read_registers(ctx,_FC_GENERAL_MODULE_COMMANDs,_FC_GENERAL_MODULE_COMMANDs+0x00,&ret); 
		        ret = ret & 0x000F;
		        break;
           }
           case 0x02 :{
                        if(ctx->debug ==1)
                        printf("Waring :AEA flag, Automatic extended addressing result being returned or ready to write \n");
                         
		        break;
           }
           case 0x03 : {
                        if(ctx->debug ==1)
                        printf("Waring :CP flag, Command not complete, pending\n");
		        break;
           } 
           default: ret =-1;
 
      }

      return ret;
}

void itla_mapping_free(itla_mapping_t *mb_mapping)
{
    if (mb_mapping == NULL) {
        return;
    }
    free(mb_mapping->general_registers);
    free(mb_mapping->status_registers);
    free(mb_mapping->optical_registers);
    free(mb_mapping->capablities_registers);
    free(mb_mapping->msa_registers);
    free(mb_mapping->manufacturer_registers);
    free(mb_mapping);
}



int itla_read_registers(itla_t *ctx, int function, int abs_addr,uint16_t *dest)
{
    int rc;
    int req_length;
    uint8_t req[_MIN_REQ_LENGTH];
    uint8_t rsp[_MAX_RESP_LENGTH];

    req_length = ctx->backend->frame_enpacket(_READ_ITLA,abs_addr,0x0000,req);
    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        rc = receive_msg(ctx, rsp);
        if (rc == -1){
           if(ctx->debug <=0){
                 printf("recv msg error \n");
                 return -1;
           }
        }
        if(rsp[0] & 0x03 != 0){
           int eno=error_msg_handle(ctx,rsp[0]);
           if(ctx->debug <=0){
           	_error_print(ctx,(const char *) itla_error(eno));
           }
           return -eno;
        }else
           *dest  = (rsp[2] << 8) | rsp[3];
    }
    return rc;
}

void wait_sucessful(itla_t *ctx){
        uint16_t  reg =0xFFFF;
        int rc;
        while(reg!=0x10){
               rc=itla_read_registers(ctx, _FC_GENERAL_MODULE_COMMANDs,_FC_GENERAL_MODULE_COMMANDs+0,&reg);     
               //if(rc>0)printf("NOP:%x \n",reg);
        } 

}
int itla_write_registers(itla_t *ctx, int function, int abs_addr,uint16_t reg_data,uint16_t *dest)
{
    int rc;
    int req_length;
    uint8_t req[_MIN_REQ_LENGTH];
    uint8_t rsp[_MAX_RESP_LENGTH];

    req_length = ctx->backend->frame_enpacket(_WRITE_ITLA,abs_addr,reg_data,req);
    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        rc = receive_msg(ctx, rsp);
        if (rc == -1){
           if(ctx->debug <=0){
                 printf("recv msg error \n");
                 return -1;
           }
        }
        if(rsp[0] & 0x03 != 0){
            int eno=error_msg_handle(ctx,rsp[0]);
            if(ctx->debug <=0){
            	_error_print(ctx, (const char *)itla_error(eno));
            }
            return -eno;
        }
        else
           *dest  = (rsp[2] << 8) | rsp[3];
    }
    return rc;
}

/*return xxx GHz*/
float getLaserFrequency (itla_t *ctx){
 
    int rc;
    uint16_t reg_Int,reg_Flo;
    rc = itla_read_registers(ctx,_FC_MODULE_STATUS_COMMANDS,0x40,&reg_Int);  
    if(rc<0)return -1;

    rc = itla_read_registers(ctx,_FC_MODULE_STATUS_COMMANDS,0x41,&reg_Flo);  
    if(rc<0)return -1;      

    return (float)(reg_Int*1000)+ (float)reg_Flo/10.0;

}

float getOpticalOutputPower(itla_t *ctx){
        uint16_t reg;
        itla_read_registers(ctx, _FC_MODULE_OPTICAL_COMMANDS,0x42,&reg);
        return (float)reg/100.0;
}

float getCurrentTemperature(itla_t *ctx){
        uint16_t reg;
        itla_read_registers(ctx, _FC_MODULE_OPTICAL_COMMANDS,0x43,&reg);
        return (float)reg/100.0;
}

float getOpticalPowerMin(itla_t *ctx){

        uint16_t reg;
        itla_read_registers(ctx, _FC_MODULE_CAPABILITILES_COMMANDS,0x50,&reg);
        return (float)reg/100.0;

}

float getOpticalPowerMax(itla_t *ctx){

        uint16_t reg;
        itla_read_registers(ctx, _FC_MODULE_CAPABILITILES_COMMANDS,0x51,&reg);
        return (float)reg/100.0;

}
/*return xxx GHz*/
float getFrequencyMin(itla_t *ctx){

    int rc;
    uint16_t reg_Int,reg_Flo;
    rc = itla_read_registers(ctx,_FC_MODULE_CAPABILITILES_COMMANDS,0x52,&reg_Int);  
    if(rc<0)return -1;

    rc = itla_read_registers(ctx,_FC_MODULE_CAPABILITILES_COMMANDS,0x53,&reg_Flo);  
    if(rc<0)return -1;      

    return (float)(reg_Int*1000)+ (float)reg_Flo/10.0;

}
/*return xxx GHz*/
float getFrequencyMax(itla_t *ctx){

    int rc;
    uint16_t reg_Int,reg_Flo;
    rc = itla_read_registers(ctx,_FC_MODULE_CAPABILITILES_COMMANDS,0x54,&reg_Int);  
    if(rc<0)return -1;

    rc = itla_read_registers(ctx,_FC_MODULE_CAPABILITILES_COMMANDS,0x55,&reg_Flo);  
    if(rc<0)return -1;      

    return (float)(reg_Int*1000)+ (float)reg_Flo/10.0;

}
/*return xxx GHz*/
float getGridSpacingMin(itla_t *ctx){
        uint16_t reg;
        itla_read_registers(ctx, _FC_MODULE_CAPABILITILES_COMMANDS,0x56,&reg);
        return (float)reg/10.0;
}


void  resetModule(itla_t *ctx){
        uint16_t reg;
        itla_write_registers(ctx,_FC_MODULE_STATUS_COMMANDS,_FC_MODULE_STATUS_COMMANDS+2,0x0003,&reg);    // soft reset
        wait_sucessful(ctx);
}
void disalbeOutput(itla_t *ctx){
        uint16_t reg;
        itla_write_registers(ctx,_FC_MODULE_STATUS_COMMANDS,_FC_MODULE_STATUS_COMMANDS+9,0x0000,&reg);    // Fatal_Trigger
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_STATUS_COMMANDS,_FC_MODULE_STATUS_COMMANDS+0,0x0000,&reg);    // Fatal_Status
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+3,0x0000,&reg);  // SDF
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_STATUS_COMMANDS,_FC_MODULE_STATUS_COMMANDS+0,0x0000,&reg);    // DIS
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+2,0x0000,&reg);  // SENA 
        wait_sucessful(ctx);

}

void enalbeOutput(itla_t *ctx){
        uint16_t reg;
        itla_write_registers(ctx,_FC_MODULE_STATUS_COMMANDS,_FC_MODULE_STATUS_COMMANDS+9,0xFFFF,&reg);    // Fatal_Trigger
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_STATUS_COMMANDS,_FC_MODULE_STATUS_COMMANDS+0,0xFFFF,&reg);    // Fatal_Status
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+3,0x0002,&reg);  // SDF
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_STATUS_COMMANDS,_FC_MODULE_STATUS_COMMANDS+0,0x1000,&reg);    // DIS
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+2,0x0008,&reg);  // SENA  
        wait_sucessful(ctx);
}
/*input by xxx nm*/
int setLaserWavelength(itla_t *ctx,float wavelength){
        uint16_t reg;
        float freq = Wavelength_to_FrequencyG(wavelength);
        uint16_t FCF1 = (int32_t)freq/1000;
        uint16_t FCF2 = ((int32_t)freq%1000)*10; 
        itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+5,FCF1,&reg);    //Set FCF1  
        wait_sucessful(ctx);
        itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+6,FCF2,&reg);    //Set FCF2
        wait_sucessful(ctx);
        return 0;
}

/*input by xxx dBm*/
int setOpticalPower(itla_t *ctx,float power){

       uint16_t PWR = (uint16_t)(power*100);
       uint16_t reg;
       itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+1, PWR,&reg);       //Set PWR  10:0x3E8  12:4B0   
       wait_sucessful(ctx);
       return 0; 
}

/*input by 1-XXX*/
int setOpticalChanel(itla_t * ctx,uint16_t chanel){
       uint16_t reg;
       itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+0,chanel,&reg);     //Set Chanal
       wait_sucessful(ctx);
       return 0;
}
/*input by xxx GHz min = 50 */
int setGradSpacing(itla_t * ctx,float Space){

       uint16_t reg;
       uint16_t GRID = (uint16_t)(Space*10);
       itla_write_registers(ctx,_FC_MODULE_OPTICAL_COMMANDS,_FC_MODULE_OPTICAL_COMMANDS+0,GRID,&reg);       //Set Chanal
       wait_sucessful(ctx);
       return 0;

}
