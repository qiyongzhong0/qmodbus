/*
 * modbus_port_rtt.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include "modbus_port.h"

#ifdef MB_USING_PORT_RTT

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include "rtdevice.h"

#define DBG_TAG "modbus.port.rtt"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

MB_WEAK long long mb_port_get_ms(void)//获取毫秒时间
{
    return(rt_tick_get_millisecond());
}
 
MB_WEAK void mb_port_delay_ms(int tmo_ms)
{
    rt_thread_mdelay(tmo_ms);
}

#ifdef MB_USING_RTU_BACKEND
MB_WEAK void * mb_port_rtu_open(const mb_backend_param_t *param)//打开, 成功返回实例指针或文件标识, 错误返回NULL
{
    MB_ASSERT(param != NULL);
    MB_ASSERT(param->rtu.dev != NULL);
        
    char *name = param->rtu.dev;
    rt_device_t dev = rt_device_find(name);
    if (dev == RT_NULL)
    {
        LOG_E("device (%s) not found.", name);
        return(NULL);
    }

    struct serial_configure cfg = RT_SERIAL_CONFIG_DEFAULT;
    cfg.baud_rate = param->rtu.baudrate;
    cfg.parity = param->rtu.parity;
    if (rt_device_control(dev, RT_DEVICE_CTRL_CONFIG, &cfg) < 0)
    {
        LOG_E("device (%s) config fail.", name);
        return(NULL);
    }
    
    if ( rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) < 0)
    {
        LOG_E("device (%s)  open fail.", name);
        return(NULL);
    }
    
    int pin = param->rtu.pin;
    int lvl = param->rtu.lvl;
    if (pin >= 0)
    {
        u32 ud = 0xABCD0000 + ((pin << 1) & 0xFFFF)  + (lvl ? 1 : 0);
        dev->user_data = (void *)ud;
        rt_pin_mode(pin, PIN_MODE_OUTPUT);
        rt_pin_write(pin, ! lvl);
    }

    LOG_D("device (%s) open suceess.", name);

    return((void *)dev);
}

MB_WEAK int mb_port_rtu_close(void *hinst)//关闭, 成功返回0, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    
    rt_device_t dev = (rt_device_t)hinst;
    return(rt_device_close(dev));
}

MB_WEAK int mb_port_rtu_read(void *hinst, u8 *buf, int bufsize)//接收数据, 返回接收到的数据长度, 0表示超时, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(buf != NULL);

    rt_device_t dev = (rt_device_t)hinst;
    int len = rt_device_read(dev, -1, buf, bufsize);
    if (len < 0)
    {
        LOG_E("device read error.");
        return(-1);
    }

    return(len);
}

MB_WEAK int mb_port_rtu_write(void *hinst, u8 *buf, int size)//发送数据, , 返回成功发送的数据长度, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(buf != NULL);
    
    rt_device_t dev = (rt_device_t)hinst;
    u32 ud = (u32)(dev->user_data);
    int pin = ((ud & 0xFFFF0000) == 0xABCD0000) ? ((ud & 0xFFFF) >> 1) : -1;
    int lvl = (ud & 0x01);
    if (pin >= 0) rt_pin_write(pin, lvl);
    int len = rt_device_write(dev, -1, buf, size);
    if (pin >= 0) rt_pin_write(pin, ! lvl);
    if (len < 0)
    {
        LOG_E("device read error.");
        return(-1);
    }

    return(len);
}

MB_WEAK int mb_port_rtu_flush(void *hinst)//清空接收缓存, 成功返回0, 错误返回-1
{
    MB_ASSERT(hinst != NULL);

    u8 c;
    rt_device_t dev = (rt_device_t)hinst;
    while(1)
    {
        int len = rt_device_read(dev, -1, &c, 1);
        if (len < 0)
        {
            return(-1);
        }
        if (len == 0)
        {
            break;
        }
    }

    return(0);
}
#endif

#ifdef MB_USING_TCP_BACKEND    
MB_WEAK void * mb_port_tcp_open(const mb_backend_param_t *param)//打开, 成功返回实例指针或文件标识, 错误返回NULL
{
    MB_ASSERT(param != NULL);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LOG_E("socket create fail.");
        return(NULL);
    }
    LOG_D("socket create success, fd = %d.", sock);
    
    struct hostent *host = gethostbyname(param->tcp.host);
    if (host == NULL)
    {
        close(sock);
        LOG_E("host get error.");
        return(NULL);
    }

    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(param->tcp.port);
    srv_addr.sin_addr = *((struct in_addr *)host->h_addr);
    if (connect(sock, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr)) < 0)
    {
        close(sock);
        LOG_E("socket connect fail.");
        return(NULL);
    }
    
    return((void *)sock);
}

MB_WEAK int mb_port_tcp_close(void *hinst)//关闭, 成功返回0, 错误返回-1
{
    MB_ASSERT(hinst != NULL);

    int sock = (int)hinst;
    return((close(sock) == 0) ? 0 : -1);
}

MB_WEAK int mb_port_tcp_read(void *hinst, u8 *buf, int bufsize)//接收数据, 返回接收到的数据长度, 0表示超时, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(buf != NULL);

    int sock = (int)hinst;
    int len = recv(sock, buf, bufsize, MSG_DONTWAIT);
    if (len == 0)//socket已关闭
    {
        LOG_E("TCP read error.");
        return(-1);
    }
    if (len < 0)//超时或其它错误
    {
        return(0);
    }
    
    return(len);
}

MB_WEAK int mb_port_tcp_write(void *hinst, u8 *buf, int size)//发送数据, , 返回成功发送的数据长度, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(buf != NULL);

    int sock = (int)hinst;
    int len = send(sock, buf, size, 0);
    if (len <= 0)//socket已关闭
    {
        LOG_E("TCP write error.");
        return(-1);
    }
    
    return(len);
}

MB_WEAK int mb_port_tcp_flush(void *hinst)//清空接收缓存, 成功返回0, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    
    u8 c;
    int sock = (int)hinst;
    while(1)
    {
        int len = recv(sock, &c, 1, MSG_DONTWAIT);
        if (len == 0)//socket已关闭
        {
            return(-1);
        }
        if (len < 0)//超时或其它错误
        {
            break;
        }
    }

    return(0);
}
#endif

#endif
