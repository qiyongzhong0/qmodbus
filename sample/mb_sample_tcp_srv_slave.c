/*
 * mb_sample_tcp_slave.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "rtthread.h"
#include "modbus.h"

#ifdef MB_USING_SAMPLE_TCP_SRV_SLAVE

#define DBG_TAG "mb.tcp.srv.slave"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define TCP_LISTEN_PORT         6000    //侦听端口号

#define MB_REG_ADDR_BEGIN       4000    //寄存器起始地址

static u16 regs[] = {//寄存器数据
    2210,
    2220,
    2230,
    111,
    112,
    113,
    3000,
    1111,
    1112,
    1113,
    600,
    201,
    202,
    203,
    4000,
    8008,
    9009,
    2001,
    2002,
    2003,
    2004,
    101,
    102,
    103,
    104,
    1111,
    1112,
    1113,
    1114
};

//重新实现 modbus_port_slave.c 中的回调函数
int mb_port_read_hold(u16 addr, u16 *preg)//读保持寄存器, 返回 : 0-成功, -2-地址错误
{
    MB_ASSERT(preg != NULL);
    
    if (addr < MB_REG_ADDR_BEGIN)
    {
        return(-2);
    }
    if (addr >= (MB_REG_ADDR_BEGIN + sizeof(regs)/sizeof(regs[0])))
    {
        return(-2);
    }

    *preg = regs[addr - MB_REG_ADDR_BEGIN];
    
    return(0);
}

//重新实现 modbus_port_slave.c 中的回调函数
int mb_port_write_hold(u16 addr, u16 reg)//写保持寄存器, 返回 : 0-成功, -2-地址错误, -3-值非法, -4-设备故障
{
    if (addr < MB_REG_ADDR_BEGIN)
    {
        return(-2);
    }
    if (addr >= (MB_REG_ADDR_BEGIN + sizeof(regs)/sizeof(regs[0])))
    {
        return(-2);
    }

    regs[addr - MB_REG_ADDR_BEGIN] = reg;
    
    return(0);
}

static void mb_slave_sock_srv(void *args)//从机socket连接线程服务函数
{
    int sock_fd = (int)args;
    mb_backend_param_t param;
    param.sock.fd = sock_fd;
    mb_inst_t *hinst = mb_create(MB_BACKEND_TYPE_SOCK, (void *)&param);
    MB_ASSERT(hinst != NULL);
    
    //mb_set_slave(hinst, 1);//修改从机地址, 默认地址为1, 可根据实际情况修改
    //mb_set_prot(hinst, MB_PROT_RTU);//修改通信协议类型, TCP后端默认使用MODBUS-TCP通信协议
    //mb_set_tmo(hinst, 500, 15);//修改超时时间, 应答超时500ms(默认300ms), 字节超时15ms(默认32ms)
    
    LOG_I("slave-server begining, sock-fd = %d", sock_fd);
    
    while(1)
    {
        mb_slave_fsm(hinst);
        
        if (hinst->backend->hinst == NULL)//socket已关闭
        {
            break;
        }
    }

    mb_destory(hinst);
    
    LOG_I("slave-server-thread end, sock-fd = %d", sock_fd);
}

static int mb_slave_sock_srv_create(int sock_fd)//创建socket连接服务线程
{
    char name[32];
    sprintf(name, "mb-slv-skt-%02d", sock_fd);
    rt_thread_t tid = rt_thread_create(name, mb_slave_sock_srv, (void *)sock_fd, 2048, 6, 20);
    if (tid == NULL)
    {
        return(-1);
    }
    
    rt_thread_startup(tid);
    return(0);
}

static void mb_tcp_listen_srv(void *args)//侦听服务线程
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//创建socket，指定SOCK_STREAM为TCP的socket
    if (sock == -1)
    {
        LOG_E("create socket fail");
        return;
    }
    
    //初始化服务端地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_LISTEN_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    
    //绑定socket到服务端地址
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        closesocket(sock);
        LOG_E("bind to socket fail");
        return;
    }
    
    //启动在socket上进行监听
    if (listen(sock, 10) == -1)
    {
        closesocket(sock);
        LOG_E("startup listen fail");
        return;
    }

    LOG_I("TCP server waiting for client on port %d...", TCP_LISTEN_PORT);
    
    while (1)
    {
        rt_thread_mdelay(5);
        
        struct sockaddr_in clt_addr;
        socklen_t sin_size = sizeof(clt_addr);
        //接受一个客户端连接socket的请求，返回的是连接成功的socket，这个函数调用是阻塞式的
        int conn = accept(sock, (struct sockaddr *)&clt_addr, &sin_size);
        if (conn < 0)
        {
            LOG_E("accept connection fail");
            continue;
        }
        
        //接受返回的client_addr指向了客户端的地址信息 */
        LOG_I("I got a connection from (%s , %d)", inet_ntoa(clt_addr.sin_addr), ntohs(clt_addr.sin_port));

        int rst = mb_slave_sock_srv_create(conn);
        if (rst < 0)
        {
            LOG_E("create slave-server-thread fail");
            continue;
        }

        LOG_I("create slave-server-thread success, sock-fd = %d", conn);
   }
}

static int mb_tcp_listen_srv_startup(void)//创建socket连接服务线程
{
    rt_thread_t tid = rt_thread_create("mb-tcp-listen", mb_tcp_listen_srv, NULL, 2048, 5, 20);
    RT_ASSERT(tid != NULL);
    rt_thread_startup(tid);
    return(0);
}
INIT_APP_EXPORT(mb_tcp_listen_srv_startup);

#endif
