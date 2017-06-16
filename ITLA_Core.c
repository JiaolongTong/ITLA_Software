/*
 * Copyright © 2017-now Jiaolong Tong <tongjiaolong@yeah.net>
 *
*/

#include "ITLA_Core.h"
void _rtla_init_common(itla_t *ctx)
{
    /* Slave and socket are initialized to -1 */
    ctx->fd = -1;
    ctx->debug = FALSE;
    ctx->response_timeout.tv_sec = 0;
    ctx->response_timeout.tv_usec = _RESPONSE_TIMEOUT;
    ctx->byte_timeout.tv_sec = 0;
    ctx->byte_timeout.tv_usec = _BYTE_TIMEOUT;
}

const char *itla_error(int errnum) {
   // printf("errnum :%x ",errnum);
    switch (errnum) {
    case ITLA_EXCEPTION_RNI:
        return "Register not implemented";
    case ITLA_EXCEPTION_RNW:
        return "Register not write-able";
    case ITLA_EXCEPTION_RVE:
        return "Register value range error";
    case ITLA_EXCEPTION_CIP:
        return "Command ignored due to pending operation";
    case ITLA_EXCEPTION_CII:
        return "Command ignored while module is initializing";
    case ITLA_EXCEPTION_ERE:
        return "Extended address range error";
    case ITLA_EXCEPTION_ERO:
        return "Extended address read only";
    case ITLA_EXCEPTION_EXF:
        return "Command ignored while module’s optical output is enabled";
    case ITLA_EXCEPTION_CIE:
        return "Invalid configuration, command ignored";
    case ITLA_EXCEPTION_IVC:
        return "Vendor specific error";
    default:
        return strerror(errnum);
    }
}
void _error_print(itla_t *ctx, const char *context)
{
        if(errno !=0){
		fprintf(stderr, "ERROR %s", itla_error(errno));
		if (context != NULL) {
		    fprintf(stderr, ": %s\n", context);
		} else {
		    fprintf(stderr, "\n");
                }
        }
}

static int check_confirmation(itla_t *ctx, uint8_t *req,
                              uint8_t *rsp, int rsp_length)
{
        
        return 4;
}


/* Sends a request/response */
 int send_msg(itla_t *ctx, uint8_t *msg, int msg_length)
{
    int rc;
    int i;
    if (ctx->debug) {
        printf("Send:");
        for (i = 0; i < msg_length; i++)
            printf("[%.2X]", msg[i]);
        printf("\n");
    }
    do {
        rc = ctx->backend->send(ctx, msg, msg_length);
        if (rc == -1) {
            _error_print(ctx, NULL);
                int saved_errno = errno;
                if ((errno == EBADF || errno == ECONNRESET || errno == EPIPE)) {
                    ctx->backend->close(ctx);
                    ctx->backend->connect(ctx);
                } else {
                    _sleep_and_flush(ctx);
                }
                errno = saved_errno;
        }
    } while ( rc == -1);

    if (rc > 0 && rc != msg_length) {
        errno = -1;
        return -1;
    }
    return rc;
}
/* Recvs a request/response */
int receive_msg(itla_t *ctx, uint8_t *msg)
{
    int rc;
    fd_set rfds;
    struct timeval tv;
    struct timeval *p_tv;
    int length_to_read;
    int msg_length = 0;

    if (ctx->debug) 
        printf("Recv:");
    FD_ZERO(&rfds);
    FD_SET(ctx->fd, &rfds);

    length_to_read = ctx->backend->frame_length;
    tv.tv_sec      = ctx->response_timeout.tv_sec;
    tv.tv_usec     = ctx->response_timeout.tv_usec;
    p_tv = &tv;
    while (length_to_read != 0) {
        rc = ctx->backend->select(ctx, &rfds, p_tv, length_to_read);
        if (rc == -1) {
            _error_print(ctx, "select");
            int saved_errno = errno;
            if (errno == ETIMEDOUT) {
                _sleep_and_flush(ctx);
            } else if (errno == EBADF) {
                ctx->backend->close(ctx);
                ctx->backend->connect(ctx);
            }
            errno = saved_errno;
            return -1;
        }
        rc = ctx->backend->recv(ctx, msg + msg_length, length_to_read);
        if (rc == 0) {
            errno = ECONNRESET;
            rc = -1;
        }
        if (rc == -1) {
            _error_print(ctx, "read");
            if ((errno == ECONNRESET || errno == ECONNREFUSED || errno == EBADF)) {
                int saved_errno = errno;
                ctx->backend->close(ctx);
                ctx->backend->connect(ctx);
                errno = saved_errno;
            }
            return -1;
        }
        if (ctx->debug) {
            int i;
            for (i=0; i < rc; i++)
                printf("<%.2X>", msg[msg_length + i]);
        }
        msg_length += rc;
        length_to_read -= rc;

        if (length_to_read > 0 && ctx->byte_timeout.tv_sec != -1) {
            tv.tv_sec = ctx->byte_timeout.tv_sec;
            tv.tv_usec = ctx->byte_timeout.tv_usec;
            p_tv = &tv;
        }
    }

    if (ctx->debug)
        printf("\n");
    return ctx->backend->check_integrity(msg);
}

