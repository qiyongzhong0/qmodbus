/*
 * modbus_port_slave.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include "modbus_port.h"

#ifdef MB_USING_SLAVE
MB_WEAK int mb_port_read_disc(u16 addr, u8 *pbit)//读离散量输入, 返回 : 0-成功, -2-地址错误
{
    MB_ASSERT(pbit != NULL);
    
    return(-2);
}

MB_WEAK int mb_port_read_coil(u16 addr, u8 *pbit)//读线圈, 返回 : 0-成功, -2-地址错误
{
    MB_ASSERT(pbit != NULL);
    
    return(-2);
}

MB_WEAK int mb_port_write_coil(u16 addr, u8 bit)//写线圈, 返回 : 0-成功, -2-地址错误, -4-设备故障
{
    return(-2);
}

MB_WEAK int mb_port_read_input(u16 addr, u16 *preg)//读输入寄存器, 返回 : 0-成功, -2-地址错误
{
    MB_ASSERT(preg != NULL);
    
    return(-2);
}

MB_WEAK int mb_port_read_hold(u16 addr, u16 *preg)//读保持寄存器, 返回 : 0-成功, -2-地址错误
{
    MB_ASSERT(preg != NULL);
    
    return(-2);
}

MB_WEAK int mb_port_write_hold(u16 addr, u16 reg)//写保持寄存器, 返回 : 0-成功, -2-地址错误, -3-值非法, -4-设备故障
{
    return(-2);
}
#endif


