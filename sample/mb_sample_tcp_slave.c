/*
 * mb_sample_tcp_slave.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include "rtthread.h"
#include "modbus.h"

#ifdef MB_USING_SAMPLE_TCP_SLAVE

#define DBG_TAG "mb.tcp.slave"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static const mb_backend_param_t mb_bkd_prm = {
    .tcp.host = "192.168.43.62",    //主机地址
    .tcp.port = 60000               //端口号
};

#define MB_REG_ADDR_BEGIN       4000    //寄存器起始地址
#define MB_INPUT_REG_TOTAL      10      //输入寄存器的数量, 前10个寄存器是只读的输入寄存器, 后面的都是可读写的保持寄存器

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
    
//自定义读输入寄存器回调函数
int myself_read_input(u16 addr, u16 *preg)//读输入寄存器, 返回 : 0-成功, -2-地址错误
{
    MB_ASSERT(preg != NULL);
    
    if (addr < MB_REG_ADDR_BEGIN)
    {
        return(-2);
    }
    if (addr >= (MB_REG_ADDR_BEGIN + MB_INPUT_REG_TOTAL))
    {
        return(-2);
    }

    *preg = regs[addr - MB_REG_ADDR_BEGIN];
    
    return(0);
}

//自定义读保持寄存器回调函数
int myself_read_hold(u16 addr, u16 *preg)//读保持寄存器, 返回 : 0-成功, -2-地址错误
{
    MB_ASSERT(preg != NULL);
    
    if (addr < MB_REG_ADDR_BEGIN + MB_INPUT_REG_TOTAL)
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

//自定义写保持寄存器回调函数
int myself_write_hold(u16 addr, u16 reg)//写保持寄存器, 返回 : 0-成功, -2-地址错误, -3-值非法, -4-设备故障
{
    if (addr < MB_REG_ADDR_BEGIN + MB_INPUT_REG_TOTAL)
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

const mb_cb_table_t myself_cb_table = {
    .read_disc  = NULL,                 //读离散量输入
    .read_coil  = NULL,                 //读线圈
    .write_coil = NULL,                 //写线圈
    .read_input = myself_read_input,    //读输入寄存器
    .read_hold  = myself_read_hold,     //读保持寄存器
    .write_hold = myself_write_hold     //写保持寄存器.
};

static void mb_sample_thread(void *args)//线程服务函数
{
    mb_inst_t *hinst = mb_create(MB_BACKEND_TYPE_TCP, &mb_bkd_prm);
    RT_ASSERT(hinst != NULL);
    
    //mb_set_slave(hinst, 1);//修改从机地址, 默认地址为1, 可根据实际情况修改
    //mb_set_prot(hinst, MB_PROT_RTU);//修改通信协议类型, TCP后端默认使用MODBUS-TCP通信协议
    //mb_set_tmo(hinst, 500, 15);//修改超时时间, 应答超时500ms(默认300ms), 字节超时15ms(默认32ms)
    mb_set_cb_table(hinst, &myself_cb_table);//挂载自定义回调函数表
    
    while(1)
    {
        mb_slave_fsm(hinst);
    }
}

static int mb_sample_tcp_slave_startup(void)
{
    rt_thread_t tid = rt_thread_create("mb-tcp-slave", mb_sample_thread, NULL, 2048, 5, 20);
    RT_ASSERT(tid != NULL);
    rt_thread_startup(tid);
    return(0);
}
INIT_APP_EXPORT(mb_sample_tcp_slave_startup);

#endif
