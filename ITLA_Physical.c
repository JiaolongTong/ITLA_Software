/*
 * Copyright © 2017-now Jiaolong Tong <tongjiaolong@yeah.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <assert.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <string.h>

#include "ITLA_Physical.h"


static char calcBIP4( unsigned char* itla_request_data ) 
{ 
	unsigned char bip8=(itla_request_data[0]& 0x0f) ^ itla_request_data[1] ^ itla_request_data[2] ^ itla_request_data[3]; 
	unsigned char bip4=((bip8 & 0xf0) >>4) ^ (bip8 & 0x0f); 
	return bip4; 
}

static int _itla_farme_en_packet(int RW,int function,int data,unsigned char* send_buf)
{
	unsigned char *buf = send_buf,i; 
	unsigned char bip4;
	buf[0]=RW;
	buf[1]=function;
	buf[2]= (data >> 8) & 0xff;
	buf[3]=data & 0xff;
	bip4=calcBIP4(buf);
	buf[0]|= (bip4<<4); 
       return 4;
}
static int _itla_farme_de_packet(unsigned char* recv_buf){



}
ssize_t _itla_send(itla_t *ctx, const uint8_t *req, int req_length)
{

    return write(ctx->fd, req, req_length);

}

ssize_t _itla_recv(itla_t *ctx, uint8_t *rsp, int rsp_length)
{
    return read(ctx->fd, rsp, rsp_length);

}

int _itla_check_integrity( uint8_t *itla_response_data){
	unsigned char bip4=0,checksum = (itla_response_data[0]&0xf0);
	bip4=calcBIP4(itla_response_data);
	if(checksum == (bip4<<4)){   
		 return 1;
	}else{
		 return 0;
	}
}


/* Sets up a serial port for RTU communications */
static int _itla_phy_connect(itla_t *ctx)
{

    struct termios tios;
    speed_t speed;
    itla_phy_t *ctx_phy = ctx->backend_data;
    if (ctx->debug) {
        printf("Opening %s at %d bauds (%c, %d, %d)\n",
               ctx_phy->device, ctx_phy->baud, ctx_phy->parity,
               ctx_phy->data_bit, ctx_phy->stop_bit);
    }
    ctx->fd = open(ctx_phy->device, O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL);
    if (ctx->fd == -1) {
        fprintf(stderr, "ERROR Can't open the device %s (%s)\n",
                ctx_phy->device, strerror(errno));
        return -1;
    }

    /* Save */
    tcgetattr(ctx->fd, &(ctx_phy->old_tios));

    memset(&tios, 0, sizeof(struct termios));
    switch (ctx_phy->baud) {
	    case 110:
		speed = B110;
		break;
	    case 300:
		speed = B300;
		break;
	    case 600:
		speed = B600;
		break;
	    case 1200:
		speed = B1200;
		break;
	    case 2400:
		speed = B2400;
		break;
	    case 4800:
		speed = B4800;
		break;
	    case 9600:
		speed = B9600;
		break;
	    case 19200:
		speed = B19200;
		break;
	    case 38400:
		speed = B38400;
		break;
	    case 57600:
		speed = B57600;
		break;
	    case 115200:
		speed = B115200;
		break;
	    default:
		speed = B9600;
		if (ctx->debug) {
		    fprintf(stderr,
		            "WARNING Unknown baud rate %d for %s (B9600 used)\n",
		            ctx_phy->baud, ctx_phy->device);
		}
    }

    /* Set the baud rate */
    if ((cfsetispeed(&tios, speed) < 0) ||
        (cfsetospeed(&tios, speed) < 0)) {
        close(ctx->fd);
        ctx->fd = -1;
        return -1;
    }
    tios.c_cflag |= (CREAD | CLOCAL);
    /* CSIZE, HUPCL, CRTSCTS (hardware flow control) */
    tios.c_cflag &= ~CSIZE;
    switch (ctx_phy->data_bit) {
	    case 5:
		tios.c_cflag |= CS5;
		break;
	    case 6:
		tios.c_cflag |= CS6;
		break;
	    case 7:
		tios.c_cflag |= CS7;
		break;
	    case 8:
	    default:
		tios.c_cflag |= CS8;
		break;
    }

    /* Stop bit (1 or 2) */
    if (ctx_phy->stop_bit == 1)
        tios.c_cflag &=~ CSTOPB;
    else /* 2 */
        tios.c_cflag |= CSTOPB;

    /* PARENB       Enable parity bit
       PARODD       Use odd parity instead of even */
    if (ctx_phy->parity == 'N') {
        /* None */
        tios.c_cflag &=~ PARENB;
    } else if (ctx_phy->parity == 'E') {
        /* Even */
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
    } else {
        /* Odd */
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
    }
    /* Raw input */
    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    if (ctx_phy->parity == 'N') {
        /* None */
        tios.c_iflag &= ~INPCK;
    } else {
        tios.c_iflag |= INPCK;
    }

    /* Software flow control is disabled */
    tios.c_iflag &= ~(IXON | IXOFF | IXANY);
    tios.c_oflag &=~ OPOST;
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(ctx->fd, TCSANOW, &tios) < 0) {
        close(ctx->fd);
        ctx->fd = -1;
        return -1;
    }

    return 0;
}


void _itla_phy_close(itla_t *ctx)
{
    itla_phy_t *ctx_phy = ctx->backend_data;
    tcsetattr(ctx->fd, TCSANOW, &(ctx_phy->old_tios));
    close(ctx->fd);
}

int _itla_phy_flush(itla_t *ctx)
{
    return tcflush(ctx->fd, TCIOFLUSH);
}

int _itla_phy_select(itla_t *ctx, fd_set *rfds,
                       struct timeval *tv, int length_to_read)
{
    int s_rc;
    while ((s_rc = select(ctx->fd+1, rfds, NULL, NULL, tv)) == -1) {
        if (errno == EINTR) {
            if (ctx->debug) {
                fprintf(stderr, "A non blocked signal was caught\n");
            }
            /* Necessary after an error */
            FD_ZERO(rfds);
            FD_SET(ctx->fd, rfds);
        } else {
            return -1;
        }
    }
    if (s_rc == 0) {
        /* Timeout */
        errno = ETIMEDOUT;
        return -1;
    }
    return s_rc;
}

const itla_backend_t _itla_phy_backend = {
    _ITLA_BACKEND_TYPE_TWO,
    _ITLA_FRAME_LENGTH,
    _ITLA_CHECK_LENGTH,
    _itla_farme_en_packet,
    _itla_farme_de_packet,
    _itla_send,
    _itla_recv,
    _itla_check_integrity,
    _itla_phy_connect,
    _itla_phy_close,
    _itla_phy_flush,
    _itla_phy_select
};

itla_t* itla_new_device(const char *device,int baud, char parity, int data_bit,int stop_bit)
{
    itla_t *ctx=NULL;
    itla_phy_t *ctx_phy;
    size_t dest_size;
    size_t ret_size;

    ctx = (itla_t *) malloc(sizeof(itla_t));
    _rtla_init_common(ctx);

    ctx->backend = &_itla_phy_backend;
    ctx->backend_data = (itla_phy_t *) malloc(sizeof(itla_phy_t));
    ctx_phy = (itla_phy_t *)ctx->backend_data;

    dest_size = strlen(device)+1;
    
    if (NULL == strncpy(ctx_phy->device, device,dest_size)) {
        fprintf(stderr, "The device string is empty\n");
        itla_free(ctx);
        errno = EINVAL;
        return NULL;
    }

    ctx_phy->baud = baud;
    if (parity == 'N' || parity == 'E' || parity == 'O') {
        ctx_phy->parity = parity;
    } else {
        itla_free(ctx);
        errno = EINVAL;
        return NULL;
    }
    ctx_phy->data_bit = data_bit;
    ctx_phy->stop_bit = stop_bit;

    return ctx;
}
