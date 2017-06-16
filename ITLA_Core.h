/*
 * Copyright © 2017- now Jiaolong Tong <tongjiaolong@yeah.net>
 */

#ifndef _ITLA_SOFTWARE_H_
#define _ITLA_SOFTWARE_H_



# include <time.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "ITLA_Physical.h"
/* Timeouts in microsecond (0.5 s) */
#define _RESPONSE_TIMEOUT    500000
#define _BYTE_TIMEOUT        500000
#define FALSE 0
#define TRUE  1

typedef int ssize_t;
/* */
#define _READ_ITLA  0
#define _WRITE_ITLA 1
#define _ITLA_FRAME_LENGTH    4
#define _ITLA_CHECK_LENGTH    4
#define _MIN_REQ_LENGTH       10
#define _MAX_RESP_LENGTH      10  
/* Protocol exceptions */
enum {
    ITLA_EXCE_SUCCEFUL = 0x00,    //无指令错误
    ITLA_EXCEPTION_RNI,           //寄存器不支持
    ITLA_EXCEPTION_RNW,           //只读不可写的寄存器，或具有锁保护不可写
    ITLA_EXCEPTION_RVE,           //写寄存器参数错误，原寄存器参数不变
    ITLA_EXCEPTION_CIP,           //由于挂起操作此条命令被忽略
    ITLA_EXCEPTION_CII,           //命令被忽略由于模块正在初始化，或包含无效配置
    ITLA_EXCEPTION_ERE,           //扩展地址错误
    ITLA_EXCEPTION_ERO,           //扩展地址只读
    ITLA_EXCEPTION_EXF,           //执行失败
    ITLA_EXCEPTION_CIE,           //由于承载业务命令被忽略    
    ITLA_EXCEPTION_IVC,           //无效配置
    ITLA_EXCEPTION_VSE = 0x0F     //厂家自定义
};


typedef struct _itla itla_t;

typedef enum {
    _ITLA_BACKEND_TYPE_TWO=0,
    _ITLA_BACKEND_TYPE_MULTI
} itla_bakend_type_t;



typedef struct _itla_bakend {
    unsigned int backend_type;    
    unsigned int frame_length;   
    unsigned int check_length;
    int (*frame_enpacket)(int RW,int function,int data,unsigned char* send_buf);  
    int (*frame_depacket)(unsigned char* recv_buf);
    ssize_t (*send) (itla_t *ctx, const uint8_t *req, int req_length);
    ssize_t (*recv) (itla_t *ctx, uint8_t *rsp, int rsp_length);
    int (*check_integrity) ( uint8_t *itla_response_data);
    int (*connect) (itla_t *ctx);
    void (*close) (itla_t *ctx);
    int (*flush) (itla_t *ctx);
    int (*select) (itla_t *ctx, fd_set *rfds, struct timeval *tv, int msg_length);
} itla_backend_t;

struct _itla {
    /* Socket or file descriptor */
    int fd;
    int debug;
    struct timeval response_timeout;
    struct timeval byte_timeout;
    const  itla_backend_t *backend;
    void   *backend_data;
};

/* Function codes */
#define _FC_GENERAL_MODULE_COMMANDs   0x00     //通用指令
#define _FC_MODULE_STATUS_COMMANDS    0x20     //模块状态指令
#define _FC_MODULE_OPTICAL_COMMANDS   0x30     //模块光信息配置命令
#define _FC_MODULE_CAPABILITILES_COMMANDS      0x50     //模块调谐能力命令
#define _FC_MSA_COMMANDS              0x57     //MSA命令
#define _FC_MANUFACTURER_SPECIFIC     0x80     //厂商扩展命令

#define NB_GENERAL_REG                23   
#define NB_STATUS_REG                 11
#define NB_OPTICAL_REG                32
#define NB_CAPABILITILES_REG          7
#define NB_MSA_REG                    11
#define NB_MANUFACTURER_REG           0 
typedef struct {
     uint16_t * general_registers;
     uint16_t * status_registers;
     uint16_t * optical_registers;
     uint16_t * capablities_registers;
     uint16_t * msa_registers;
     uint16_t * manufacturer_registers;
} itla_mapping_t;

void _itla_init_common(itla_t *ctx);
void _error_print(itla_t *ctx, const char *context);
int receive_msg(itla_t *ctx, uint8_t *msg);
int send_msg(itla_t *ctx, uint8_t *msg, int msg_length);


#endif  /* _MODBUS_PRIVATE_H_ */
