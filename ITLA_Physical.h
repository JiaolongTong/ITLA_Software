/*
 * Copyright © 2017-now Jiaolong Tong <tongjiaolong@yeah.net>
 */

#ifndef _ITLA_PHYSICAL_H_
#define _ITLA_PHYSICAL_H_

#include <stdint.h>
#include <termios.h>

#include "ITLA_Core.h"

typedef struct _itla_phy {
    /* Device: "/dev/ttyS0", "/dev/ttyUSB0" or "/dev/tty.USA19*"*/
    char device[16];
    /* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;
    /* Save old termios settings */
    struct termios old_tios;
} itla_phy_t;



#endif /* _ITLA_PHYSICAL_H_ */
