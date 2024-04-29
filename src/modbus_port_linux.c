/*
 * modbus_port.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include "modbus_port.h"

#ifdef MB_USING_PORT_LINUX

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define QLOG_TAG "modbus.port.linux"
#define QLOG_LVL QLOG_LVL_LOG //QLOG_LVL_INFO //
#include "qlog.h"

MB_WEAK long long mb_port_get_ms(void)//获取毫秒时间
{
    long long ms;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ms = tv.tv_sec;
    ms = ms * 1000;
    ms = ms + (tv.tv_usec / 1000);
    return(ms);
}
 
MB_WEAK void mb_port_delay_ms(int tmo_ms)
{
    usleep((long long)tmo_ms * 1000);
}

#ifdef MB_USING_RTU_BACKEND
static int mb_port_uart_config(int fd, int baudrate, int databit, int parity, int stopbit)//配置串口, 成功返回0, 失败返回-1
{
    struct termios options;  
     
    //获取串口相关参数，该函数还可以测试配置是否正确，该串口是否可用等。
    //若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    if( tcgetattr(fd, &options)  !=  0)
    {  
        //zlog_error(g_zc, "uart(fd=%d) get attr fail.", fd);      
        return(-1);   
    }  
    
    //设置串口输入波特率和输出波特率
    int speed = B115200;
    switch(baudrate)
    {
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
        break;
    }
    cfsetispeed(&options, speed);   
    cfsetospeed(&options, speed);       
     
    options.c_cflag |= CLOCAL;//修改控制模式，保证程序不会占用串口
    options.c_cflag |= CREAD;//修改控制模式，使得能够从串口中读取输入数据
    //options.c_cflag &= ~CRTSCTS;//不使用流控制
    options.c_cflag &= ~(IXON | IXOFF | IXANY);//不使用流控制
    //options.c_cflag |= IXON | IXOFF | IXANY;//使用软件流控制
     
    options.c_cflag &= ~CSIZE;//屏蔽其他标志位
    switch (databit)//设置数据位
    {    
    case 5:  
        options.c_cflag |= CS5;  
        break;  
    case 6:  
        options.c_cflag |= CS6;  
        break;  
    case 7:      
        options.c_cflag |= CS7;  
        break;  
    case 8: 
        options.c_cflag |= CS8;  
        break;
    default:
        options.c_cflag |= CS8;  
        break;
    }  
      
    switch (parity)//设置校验位
    {    
    case 0: //无奇偶校验位。
        options.c_cflag &= ~PARENB;   
        options.c_iflag &= ~INPCK;      
        break;   
    case 1://设置为奇校验
        options.c_cflag |= (PARODD | PARENB);   
        options.c_iflag |= INPCK;               
        break;   
    case 2://设置为偶校验
        options.c_cflag |= PARENB;         
        options.c_cflag &= ~PARODD;         
        options.c_iflag |= INPCK;        
        break;     
    default://无奇偶校验位。
        options.c_cflag &= ~PARENB;   
        options.c_iflag &= ~INPCK;  
        break;
    }   
      
    switch (stopbit)// 设置停止位
    {    
    case 1:     
        options.c_cflag &= ~CSTOPB; 
        break;   
    case 2:     
        options.c_cflag |= CSTOPB; 
        break;  
    default:
        options.c_cflag &= ~CSTOPB; 
        break;
    }  
    
    options.c_oflag &= ~OPOST;//修改输出模式，原始数据输出
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  
    //options.c_lflag &= ~(ISIG | ICANON);  
     
    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 0; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 0; /* 读取字符的最少个数为1 */
     
    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    //tcflush(fd, TCIFLUSH);  
     
    //激活配置 (将修改后的termios数据设置到串口中）
    //opt : TCSANOW--立即生效, TCSADRAIN--发送完成后生效, TCSAFLUSH--发送完成后生效并清除输入缓冲
    if (tcsetattr(fd, TCSADRAIN, &options) != 0)//发送完成后生效
    {  
        LOG_E("uart set attr error!");    
        return (-1);   
    } 
    
    return (0);  
}

MB_WEAK void * mb_port_rtu_open(const mb_backend_param_t *param)//打开, 成功返回实例指针或文件标识, 错误返回NULL
{
    MB_ASSERT(param != NULL);
    MB_ASSERT(param->rtu.dev != NULL);
        
    char *path = param->rtu.dev;
    int fd = open(path, O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (fd == -1)  
    {  
        LOG_E("device (%s) open fail.", path);
        return(NULL);  
    }  
    
    if(fcntl(fd, F_SETFL, 0) < 0)//恢复串口为阻塞状态
    {  
        close(fd);
        LOG_E("device (%s) fcntl fail.", path);
        return(NULL);  
    }
    if (mb_port_uart_config(fd, param->rtu.baudrate, 8, param->rtu.parity, 1) < 0)
    {  
        close(fd);
        LOG_E("device (%s) config fail.", path);
        return(NULL);  
    }  

    LOG_D("device (%s, fd=%d) open suceess.", path, fd);
    
    return((void *)fd);  
}

MB_WEAK int mb_port_rtu_close(void *hinst)//关闭, 成功返回0, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    
    int fd = (int)hinst;
    return(close(fd));
}

MB_WEAK int mb_port_rtu_read(void *hinst, u8 *buf, int bufsize)//接收数据, 返回接收到的数据长度, 0表示超时, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(buf != NULL);
    
    int fd = (int)hinst;
    int len = read(fd, buf, bufsize);
    if (len < 0)
    {
        LOG_E("device (fd = %d) read error!", fd);
        return(-1);
    }
    return(len);
}

MB_WEAK int mb_port_rtu_write(void *hinst, u8 *buf, int size)//发送数据, , 返回成功发送的数据长度, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(buf != NULL);
    
    int fd = (int)hinst;
    int len = write(fd, buf, size);
    if (len <= 0)
    {
        LOG_E("device (fd = %d) write error!", fd);
        return(-1);
    }
    return(len);
}

MB_WEAK int mb_port_rtu_flush(void *hinst)//清空接收缓存, 成功返回0, 错误返回-1
{
    MB_ASSERT(hinst != NULL);
    
    int fd = (int)hinst;
    return(tcflush(fd, TCIFLUSH));
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
