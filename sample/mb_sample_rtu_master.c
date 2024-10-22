/*
 * mb_sample_rtu_master.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include "rtthread.h"
#include "modbus.h"

#ifdef MB_USING_SAMPLE_RTU_MASTER

#define DBG_TAG "mb.rtu.master"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static const mb_backend_param_t mb_bkd_prm = {
    .rtu.dev = "uart1",		//设备名
    .rtu.baudrate = 9600,   //波特率
    .rtu.parity = 0,    	//校验位, 0-无, 1-奇, 2-偶
    .rtu.pin = -1,        	//控制引脚, <0 表示不使用
    .rtu.lvl = 0        	//控制发送电平
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
    mb_inst_t *hinst = mb_create(MB_BACKEND_TYPE_RTU, &mb_bkd_prm);
    RT_ASSERT(hinst != NULL);
    
	//mb_set_slave(hinst, 1);//修改从机地址, 默认地址为1, 可根据实际情况修改
    //mb_set_prot(hinst, MB_PROT_TCP);//修改通信协议类型, RTU后端默认使用MODBUS-RTU通信协议
	//mb_set_tmo(hinst, 500, 15);//修改超时时间, 应答超时500ms(默认300ms), 字节超时15ms(默认32ms)
    
    while(1)
    {
        mb_sample_read_regs(hinst);
        rt_thread_mdelay(1000);
    }
}

static int mb_sample_rtu_master_startup(void)
{
    rt_thread_t tid = rt_thread_create("mb-rtu-master", mb_sample_thread, NULL, 2048, 5, 20);
    RT_ASSERT(tid != NULL);
    rt_thread_startup(tid);
    return(0);
}
INIT_APP_EXPORT(mb_sample_rtu_master_startup);

#endif
