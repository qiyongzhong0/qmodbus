/*
 * modbus_port.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#ifndef __MODBUS_PORT_H__
#define __MODBUS_PORT_H__

#include "modbus_backend.h"

#define MB_ASSERT(x)        //ASSERT(x)
#define MB_WEAK             __attribute__((weak))

#if defined(MB_USING_PORT_RTT)
#include "rtthread.h"
#define MB_PRINTF           rt_kprintf
#elif defined(MB_USING_PORT_LINUX)
#include <stdio.h>
#define MB_PRINTF           printf
#else
#define MB_PRINTF
#endif

long long mb_port_get_ms(void);//获取毫秒时间
void mb_port_delay_ms(int tmo_ms);//系统延时

#ifdef MB_USING_RTU_BACKEND
void * mb_port_rtu_open(const mb_backend_param_t *param);//打开, 成功返回实例指针或文件标识, 错误返回NULL
int mb_port_rtu_close(void *hinst);//关闭, 成功返回0, 错误返回-1
int mb_port_rtu_read(void *hinst, u8 *buf, int bufsize);//接收数据, 返回接收到的数据长度, 0表示超时, 错误返回-1
int mb_port_rtu_write(void *hinst, u8 *buf, int size);//发送数据, , 返回成功发送的数据长度, 错误返回-1
int mb_port_rtu_flush(void *hinst);//清空接收缓存, 成功返回0, 错误返回-1
#endif

#ifdef MB_USING_TCP_BACKEND
void * mb_port_tcp_open(const mb_backend_param_t *param);//打开, 成功返回实例指针或文件标识, 错误返回NULL
int mb_port_tcp_close(void *hinst);//关闭, 成功返回0, 错误返回-1
int mb_port_tcp_read(void *hinst, u8 *buf, int bufsize);//接收数据, 返回接收到的数据长度, 0表示超时, 错误返回-1
int mb_port_tcp_write(void *hinst, u8 *buf, int size);//发送数据, , 返回成功发送的数据长度, 错误返回-1
int mb_port_tcp_flush(void *hinst);//清空接收缓存, 成功返回0, 错误返回-1
#endif

#ifdef MB_USING_SLAVE
int mb_port_read_disc(u16 addr, u8 *pbit);//读离散量输入, 返回 : 0-成功, -2-地址错误
int mb_port_read_coil(u16 addr, u8 *pbit);//读线圈, 返回 : 0-成功, -2-地址错误
int mb_port_write_coil(u16 addr, u8 bit);//写线圈, 返回 : 0-成功, -2-地址错误, -4-设备故障
int mb_port_read_input(u16 addr, u16 *preg);//读输入寄存器, 返回 : 0-成功, -2-地址错误
int mb_port_read_hold(u16 addr, u16 *preg);//读保持寄存器, 返回 : 0-成功, -2-地址错误
int mb_port_write_hold(u16 addr, u16 reg);//写保持寄存器, 返回 : 0-成功, -2-地址错误, -3-值非法, -4-设备故障
#endif

#endif
