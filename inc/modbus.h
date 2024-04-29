/*
 * modbus.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "modbus_cvt.h"
#include "modbus_rtu.h"
#include "modbus_tcp.h"
#include "modbus_port.h"
#include "modbus_backend.h"

#ifdef MB_TCP_FRM_MAX 
#define MB_BUF_SIZE MB_TCP_FRM_MAX
#else
#define MB_BUF_SIZE MB_RTU_FRM_MAX
#endif

typedef enum{
    MB_PROT_RTU = 0,
    MB_PROT_TCP
}mb_prot_t;

typedef int (*mb_read_bit_t)(u16 addr, u8 *pbit);//读bit位, 返回: 0-成功, -2-地址错误
typedef int (*mb_write_bit_t)(u16 addr, u8 bit);//写bit位, 返回: 0-成功, -2-地址错误, -4-设备故障
typedef int (*mb_read_reg_t)(u16 addr, u16 *pval);//读16位寄存器, 返回 : 0-成功, -2-地址错误
typedef int (*mb_write_reg_t)(u16 addr, u16 val);//写16位寄存器, 返回 : 0-成功, -2-地址错误, -3-值非法, -4-设备故障
typedef int (*mb_mask_write_t)(u16 addr, u16 mask_and, u16 mask_or);//屏蔽写寄存器, 返回 : 0-成功, -2-地址错误, -3-值非法, -4-设备故障

typedef struct{
    mb_read_bit_t   read_disc;  //读离散量输入
    mb_read_bit_t   read_coil;  //读线圈
    mb_write_bit_t  write_coil; //写线圈
    mb_read_reg_t   read_input; //读输入寄存器
    mb_read_reg_t   read_hold;  //读保持寄存器
    mb_write_reg_t  write_hold; //写保持寄存器
}mb_cb_table_t;

typedef struct{
    u8 saddr;               //从机地址
    mb_prot_t prot;         //通信协议
    u16 tid;                //TCP传输标识计数
    mb_backend_t *backend;  //后端指针
    mb_cb_table_t *cb;      //从机回调函数表
    u8 datas[256];          //读写数据缓冲区
    u8 buf[MB_BUF_SIZE];    //收发缓冲区
}mb_inst_t;

//创建modbus实例, 成功返回实例指针, 失败返回NULL
mb_inst_t * mb_create(mb_backend_type_t type, const mb_backend_param_t *param);
//销毁modbus实例
void mb_destory(mb_inst_t *hinst);
//修改从机地址, 默认地址为1
void mb_set_slave(mb_inst_t *hinst, u8 saddr);
//修改协议, 默认使用与后端类型一致的协议类型
void mb_set_prot(mb_inst_t *hinst, mb_prot_t prot);
//修改超时时间, 默认应答超时300ms, 字节超时32ms
void mb_set_tmo(mb_inst_t *hinst, int ack_tmo_ms, int byte_tmo_ms);
//建立连接, 成功返回0, 失败返回-1, 如已建立连接则立即返回
int mb_connect(mb_inst_t *hinst);
//断开连接, 成功返回0, 失败返回-1, 如已断开连接则立即返回
int mb_disconn(mb_inst_t *hinst);
//接收数据, 返回收到数据长度, 超时返回0, 错误返回-1, 发生错误时会自动关闭后端
int mb_recv(mb_inst_t *hinst, u8 *buf, int bufsize);
//发送数据, 返回发送数据长度, 错误返回-1, 发生错误时会自动关闭后端
int mb_send(mb_inst_t *hinst, u8 *buf, int size);
//清空接收缓存, 成功返回0, 失败返回-1
int mb_flush(mb_inst_t *hinst);

#ifdef MB_USING_MASTER
//读请求, 功能码和数据由用户确定, 成功返回应答数据长度, 异常应答返回负值错误码, 其它错误返回0
int mb_read_req(mb_inst_t *hinst, u8 func, u16 addr, int nb, u8 *pdata);
//写请求, 功能码和数据由用户确定, 成功返回请求数量, 异常应答返回负值错误码, 其它错误返回0
int mb_write_req(mb_inst_t *hinst, u8 func, u16 addr, int nb, const u8 *pdata, int dlen);
//读多个线圈, 功能码-0x01, 成功返回读取位数量, 异常应答返回负值错误码, 其它错误返回0
int mb_read_bits(mb_inst_t *hinst, u16 addr, int nb, u8 *pbits);
//读多个离散量输入, 功能码-0x02, 成功返回读取位数量, 异常应答返回负值错误码, 其它错误返回0
int mb_read_input_bits(mb_inst_t *hinst, u16 addr, int nb, u8 *pbits);
//读多个保持寄存器, 功能码-0x03, 成功返回读取寄存器数量, 异常应答返回负值错误码, 其它错误返回0
int mb_read_regs(mb_inst_t *hinst, u16 addr, int nb, u16 *pregs);
//读多个输入寄存器, 功能码-0x04, 成功返回读取寄存器数量, 异常应答返回负值错误码, 其它错误返回0
int mb_read_input_regs(mb_inst_t *hinst, u16 addr, int nb, u16 *pregs);
//写单个线圈, 功能码-0x05, 成功返回1, 异常应答返回负值错误码, 其它错误返回0
int mb_write_bit(mb_inst_t *hinst, u16 addr, u8 bit);
//写单个保持寄存器, 功能码-0x06, 成功返回1, 异常应答返回负值错误码, 其它错误返回0
int mb_write_reg(mb_inst_t *hinst, u16 addr, u16 val);
//写多个线圈, 功能码-0x0F, 成功返回写位数量, 异常应答返回负值错误码, 其它错误返回0
int mb_write_bits(mb_inst_t *hinst, u16 addr, int nb, const u8 *pbits);
//写多个保持寄存器, 功能码-0x10, 成功返回写寄存器数量, 异常应答返回负值错误码, 其它错误返回0
int mb_write_regs(mb_inst_t *hinst, u16 addr, int nb, const u16 *pregs);
//屏蔽写保持寄存器, 功能码-0x16, 成功返回1, 异常应答返回负值错误码, 其它错误返回0
int mb_mask_write_reg(mb_inst_t *hinst, u16 addr, u16 mask_and, u16 mask_or);
//读/写多个保持寄存器, 功能码-0x17, 成功返回读取寄存器数量, 异常应答返回负值错误码, 其它错误返回0
int mb_write_and_read_regs(mb_inst_t *hinst, u16 wr_addr, int wr_nb, const u16 *p_wr_regs, 
                                        u16 rd_addr, int rd_nb, u16 *p_rd_regs);
#endif

#ifdef MB_USING_SLAVE
//修改从机回调函数表, 默认使用modbus_port中接口函数做回调函数
void mb_set_cb_table(mb_inst_t *hinst, const mb_cb_table_t *cb);
//从机状态机处理, 在线程中循环调用即可
void mb_slave_fsm(mb_inst_t *hinst);
#endif

#endif
