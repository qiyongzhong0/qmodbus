/*
 * modbus_backend.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include <stdlib.h>
#include <string.h>
#include "modbus_backend.h"
#include "modbus_port.h"

extern char *strdup(const char *str);

#ifdef MB_USING_RTU_BACKEND
static const mb_backend_ops_t mb_port_rtu_ops = 
{
    mb_port_rtu_open,
    mb_port_rtu_close,
    mb_port_rtu_read,
    mb_port_rtu_write,
    mb_port_rtu_flush
};

static mb_backend_t * mb_backend_create_rtu(const mb_backend_param_rtu_t *rtu)//创建rtu后端
{
    mb_backend_t *backend = malloc(sizeof(mb_backend_t));
    if (backend)
    {
        backend->type = MB_BACKEND_TYPE_RTU;
        backend->param.rtu = *rtu;
        backend->param.rtu.dev = strdup(rtu->dev);
        backend->ops = &mb_port_rtu_ops;
        backend->ack_tmo_ms = MB_BKD_ACK_TMO_MS_DEF;
        backend->byte_tmo_ms = MB_BKD_BYTE_TMO_MS_DEF;
        backend->hinst = NULL;
    }
    
    return(backend);
}
#endif

#ifdef MB_USING_TCP_BACKEND
static const mb_backend_ops_t mb_port_tcp_ops = 
{
    mb_port_tcp_open,
    mb_port_tcp_close,
    mb_port_tcp_read,
    mb_port_tcp_write,
    mb_port_tcp_flush
};

static mb_backend_t *mb_backend_create_tcp(const mb_backend_param_tcp_t *tcp)//创建tcp后端
{
    mb_backend_t *backend = calloc(1, sizeof(mb_backend_t));
    if (backend)
    {
        backend->type = MB_BACKEND_TYPE_TCP;
        backend->param.tcp.host = strdup(tcp->host);
        backend->param.tcp.port = tcp->port;
        backend->ops = &mb_port_tcp_ops;
        backend->ack_tmo_ms = MB_BKD_ACK_TMO_MS_DEF;
        backend->byte_tmo_ms = MB_BKD_BYTE_TMO_MS_DEF;
        backend->hinst = NULL;
    }
    
    return(backend);
}
#endif

mb_backend_t *mb_backend_create(mb_backend_type_t type, const mb_backend_param_t *param)//创建后端, 成功返回后端指针, 失败返回NULL
{
    mb_backend_t *backend = NULL;
    switch(type)
    {
    #ifdef MB_USING_RTU_BACKEND
    case MB_BACKEND_TYPE_RTU :
        backend = mb_backend_create_rtu(&(param->rtu));
        break;
    #endif
    #ifdef MB_USING_TCP_BACKEND
    case MB_BACKEND_TYPE_TCP :
        backend = mb_backend_create_tcp(&(param->tcp));
        break;
    #endif
    default:
        break;
    }
    
    return(backend);
}

void mb_backend_destory(mb_backend_t *backend)//销毁后端
{
    if (backend == NULL)
    {
        return;
    }
    
    mb_backend_close(backend);
    
    switch(backend->type)
    {
    case MB_BACKEND_TYPE_RTU :
        if (backend->param.rtu.dev)
        {
            free(backend->param.rtu.dev);
            backend->param.rtu.dev = NULL;
        }
        break;
    case MB_BACKEND_TYPE_TCP :
        if (backend->param.tcp.host)
        {
            free(backend->param.tcp.host);
            backend->param.tcp.host = NULL;
        }
        break;
    default:
        break;
    }

    free(backend);
}

int mb_backend_open(mb_backend_t *backend)//打开后端, 成功返回0, 错误返回-1
{
    if (backend == NULL)
    {
        return(-1);
    }
    if (backend->hinst != NULL)//已打开
    {
        return(0);
    }
    if ((backend->ops == NULL) || (backend->ops->open == NULL))
    {
        return(-1);
    }
    backend->hinst = backend->ops->open((void *)&(backend->param));
    if (backend->hinst == NULL)//打开失败
    {
        return(-1);
    }
    return(0);
}

int mb_backend_close(mb_backend_t *backend)//关闭后端, 成功返回0, 错误返回-1
{
    if (backend == NULL)
    {
        return(-1);
    }
    if (backend->hinst == NULL)//已关闭
    {
        return(0);
    }
    if ((backend->ops == NULL) || (backend->ops->close == NULL))
    {
        return(-1);
    }
    if (backend->ops->close(backend->hinst) != 0)//关闭失败
    {
        return(-1);
    }
    backend->hinst = NULL;
    return(0);
}

int mb_backend_config(mb_backend_t *backend, int ack_tmo_ms, int byte_tmo_ms)//配置后端超时参数, 成功返回0, 错误返回-1
{
    if (backend == NULL)
    {
        return(-1);
    }
    backend->ack_tmo_ms = ack_tmo_ms;
    backend->byte_tmo_ms = byte_tmo_ms;
    return(0);
}

int mb_backend_read(mb_backend_t *backend, u8 *buf, int bufsize)//从后端读数据, 返回读取到数据长度, 0表示超时, 错误返回-1
{
    if ((backend == NULL) || (buf == NULL) || (bufsize <= 0))
    {
        return(-1);
    }
    if (backend->hinst == NULL)//未打开
    {
        return(-1);
    }
    if ((backend->ops == NULL) || (backend->ops->read == NULL))
    {
        return(-1);
    }
    int pos = 0;
    long long told_ms = mb_port_get_ms();
    while(pos < bufsize)
    {
        int len = backend->ops->read(backend->hinst, buf + pos, bufsize - pos);
        if (len < 0)//发生错误
        {
            return(-1);
        }
        if (len > 0)//读到数据
        {
            told_ms = mb_port_get_ms();
            pos += len;
            continue;
        }
        int tmo_ms = mb_port_get_ms() - told_ms;
        if (pos)//已有数据接收到, 则检查字节超时
        {
            if (tmo_ms > backend->byte_tmo_ms)//字节超时了
            {
                break;
            }
        }
        else//未收到过数据, 则检查应答超时
        {
            if (tmo_ms > backend->ack_tmo_ms)//应答超时了
            {
                break;
            }
        }
        mb_port_delay_ms(2);
    }
    return(pos);
}

int mb_backend_write(mb_backend_t *backend, u8 *buf, int size)//向后端写数据, 返回已发送数据长度, 错误返回-1
{
    if ((backend == NULL) || (buf == NULL) || (size <= 0))
    {
        return(-1);
    }
    if (backend->hinst == NULL)//未打开
    {
        return(-1);
    }
    if ((backend->ops == NULL) || (backend->ops->write == NULL))
    {
        return(-1);
    }
    return(backend->ops->write(backend->hinst, buf, size));
}

int mb_backend_flush(mb_backend_t *backend)//清空后端接收缓存, 成功返回0, 错误返回-1
{
    if (backend == NULL)
    {
        return(-1);
    }
    if (backend->hinst == NULL)//未打开
    {
        return(-1);
    }
    if ((backend->ops == NULL) || (backend->ops->flush == NULL))
    {
        return(-1);
    }
    return(backend->ops->flush(backend->hinst));
}

