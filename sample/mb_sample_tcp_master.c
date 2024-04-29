/*
 * mb_sample_tcp_master.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include "rtthread.h"
#include "modbus.h"

#ifdef MB_USING_SAMPLE_TCP_MASTER

#define DBG_TAG "mb.tcp.master"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static const mb_backend_param_t mb_bkd_prm = {
    .tcp.host = "192.168.43.62",    //主机地址
    .tcp.port = 60000               //端口号
};

static void mb_sample_read_regs(mb_inst_t *hinst)
{
    if (mb_connect(hinst) < 0)//连接失败, 延时返回
    {
        LOG_E("modbus connect fail.");
        return;
    }
    
    u16 regs[64];
    int addr = 4000;
    int nb = 29;
    int total = mb_read_regs(hinst, addr, nb, regs);
    if (total <= 0)
    {
        LOG_E("modbus read register fail.");
        return;
    }
    
    LOG_D("modbus read register success.");
    
    for (int i=0; i<total; i++)
    {
        LOG_D("addr : %d, value : %d", addr + i, regs[i]);
    }
}

static void mb_sample_thread(void *args)//线程服务函数
{
    mb_inst_t *hinst = mb_create(MB_BACKEND_TYPE_TCP, &mb_bkd_prm);
    RT_ASSERT(hinst != NULL);
    
    //mb_set_slave(hinst, 1);//修改从机地址, 默认地址为1, 可根据实际情况修改
    //mb_set_prot(hinst, MB_PROT_RTU);//修改通信协议类型, TCP后端默认使用MODBUS-TCP通信协议
    //mb_set_tmo(hinst, 500, 15);//修改超时时间, 应答超时500ms(默认300ms), 字节超时15ms(默认32ms)
    
    while(1)
    {
        mb_sample_read_regs(hinst);
        rt_thread_mdelay(1000);
    }
}

static int mb_sample_tcp_master_startup(void)
{
    rt_thread_t tid = rt_thread_create("mb-tcp-master", mb_sample_thread, NULL, 2048, 5, 20);
    RT_ASSERT(tid != NULL);
    rt_thread_startup(tid);
    return(0);
}
INIT_APP_EXPORT(mb_sample_tcp_master_startup);

#endif
