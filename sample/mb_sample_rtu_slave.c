/*
 * mb_sample_rtu_slave.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include "rtthread.h"
#include "modbus.h"

#ifdef MB_USING_SAMPLE_RTU_SLAVE

#define DBG_TAG "mb.rtu.slave"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static const mb_backend_param_t mb_bkd_prm = {
    .rtu.dev = "uart1",		//设备名
    .rtu.baudrate = 9600,   //波特率
    .rtu.parity = 0,    	//校验位, 0-无, 1-奇, 2-偶
    .rtu.pin = -1,        	//控制引脚, <0 表示不使用
    .rtu.lvl = 0        	//控制发送电平
};

#define MB_REG_ADDR_BEGIN   4000    //寄存器起始地址

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

static void mb_sample_thread(void *args)//线程服务函数
{
    mb_inst_t *hinst = mb_create(MB_BACKEND_TYPE_RTU, &mb_bkd_prm);
    RT_ASSERT(hinst != NULL);
    
    //mb_set_slave(hinst, 1);//修改从机地址, 默认地址为1, 可根据实际情况修改
    //mb_set_prot(hinst, MB_PROT_TCP);//修改通信协议类型, RTU后端默认使用MODBUS-RTU通信协议
    //mb_set_tmo(hinst, 500, 15);//修改超时时间, 应答超时500ms(默认300ms), 字节超时15ms(默认32ms)
    
    while(1)
    {
        mb_slave_fsm(hinst);//循环调用从机状态机
    }
}

static int mb_sample_rtu_slave_startup(void)
{
    rt_thread_t tid = rt_thread_create("mb-rtu-slave", mb_sample_thread, NULL, 2048, 5, 20);
    RT_ASSERT(tid != NULL);
    rt_thread_startup(tid);
    return(0);
}
INIT_APP_EXPORT(mb_sample_rtu_slave_startup);

#endif
