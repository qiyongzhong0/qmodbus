/*
 * modbus_pdu.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */

#ifndef __MODBUS_PDU_H__
#define __MODBUS_PDU_H__

#include "typedef.h"

#define MB_PDU_SIZE_MIN                     2       //最小pdu尺寸
#define MB_PDU_SIZE_MAX                     253     //最大pdu尺寸

#define MODBUS_READ_BITS_MAX                2000    //最大可读位总数
#define MODBUS_WRITE_BITS_MAX               1968    //最大可写位总数

#define MODBUS_READ_REG_MAX                 125     //最大可读寄存器总数
#define MODBUS_WRITE_REG_MAX                123     //最大可写寄存器总数
#define MODBUS_WR_READ_REG_MAX              125     //写然后读时最大可读寄存器总数
#define MODBUS_WR_WRITE_REG_MAX             121     //写然后读时最大可写寄存器总数

#define MODBUS_FC_READ_COILS                0x01    //读线圈
#define MODBUS_FC_READ_DISCRETE_INPUTS      0x02    //读离散输入
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03    //读保持寄存器
#define MODBUS_FC_READ_INPUT_REGISTERS      0x04    //读输入寄存器
#define MODBUS_FC_WRITE_SINGLE_COIL         0x05    //写单个线圈
#define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06    //写单个寄存器
#define MODBUS_FC_READ_EXCEPTION_STATUS     0x07    //读异常状态
#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F    //写多个线圈
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10    //写多个寄存器
#define MODBUS_FC_REPORT_SLAVE_ID           0x11    //上报从机标识
#define MODBUS_FC_MASK_WRITE_REGISTER       0x16    //屏蔽写寄存器
#define MODBUS_FC_WRITE_AND_READ_REGISTERS  0x17    //写然后读寄存器

#define MODBUS_FC_EXCEPT_MAKE(x)            (x | 0x80)    
#define MODBUS_FC_EXCEPT_PARSE(x)           (x & ~0x80)
#define MODBUS_FC_EXCEPT_CHK(x)             ((x & 0x80) != 0)

typedef enum {
    MODBUS_EC_ILLEGAL_FUNCTION = 0x01,
    MODBUS_EC_ILLEGAL_DATA_ADDRESS,
    MODBUS_EC_ILLEGAL_DATA_VALUE,
    MODBUS_EC_SLAVE_OR_SERVER_FAILURE,
    MODBUS_EC_ACKNOWLEDGE,
    MODBUS_EC_SLAVE_OR_SERVER_BUSY,
    MODBUS_EC_NEGATIVE_ACKNOWLEDGE,
    MODBUS_EC_MEMORY_PARITY,
    MODBUS_EC_NOT_DEFINED,
    MODBUS_EC_GATEWAY_PATH,
    MODBUS_EC_GATEWAY_TARGET,
    MODBUS_EC_MAX
}modbus_except_code_t;

typedef enum{
    MB_PDU_TYPE_REQ = 0,
    MB_PDU_TYPE_RSP
}mb_pdu_type_t;

typedef struct{
    u8 fc;          //功能码
    u8 ec;          //异常码
}mb_pdu_except_t;//异常pdu定义

typedef struct{
    u8 fc;          //功能码
    u16 addr;       //寄存器地址
    u16 nb;         //数量
}mb_pdu_rd_req_t;//读请求pdu定义

typedef struct{
    u8 fc;          //功能码
    u8 dlen;        //数据长度
    u8 *pdata;      //数据指针
}mb_pdu_rd_rsp_t;//读响应pdu定义

typedef struct{
    u8 fc;          //功能码
    u16 addr;       //寄存器地址
    u16 val;        //数据值
}mb_pdu_wr_single_t;//写单个寄存器请求/响应pdu定义

typedef struct{
    u8 fc;          //功能码
    u16 addr;       //寄存器地址
    u16 nb;         //数量
    u8 dlen;        //数据长度
    u8 *pdata;      //数据指针
}mb_pdu_wr_req_t;//写多个寄存器请求pdu定义

typedef struct{
    u8 fc;          //功能码
    u16 addr;       //寄存器地址
    u16 nb;         //输出数量
}mb_pdu_wr_rsp_t;//写多个寄存器响应pdu定义

typedef struct{
    u8 fc;          //功能码
    u16 addr;       //寄存器地址
    u16 val_and;    //与值
    u16 val_or;     //或值
}mb_pdu_mask_wr_t;//写多个寄存器请求/响应pdu定义

typedef struct{
    u8 fc;          //功能码
    u16 rd_addr;    //读取寄存器地址
    u16 rd_nb;      //读取数量
    u16 wr_addr;    //写寄存器地址
    u16 wr_nb;      //写入数量
    u8 dlen;        //数据长度
    u8 *pdata;      //数据指针
}mb_pdu_wr_rd_req_t;//写然后读请求pdu定义

typedef union{
    u8 fc;
    mb_pdu_except_t     exc;
    mb_pdu_rd_req_t     rd_req;
    mb_pdu_rd_rsp_t     rd_rsp;
    mb_pdu_wr_single_t  wr_single;
    mb_pdu_wr_req_t     wr_req;
    mb_pdu_wr_rsp_t     wr_rsp;
    mb_pdu_mask_wr_t    mask_wr;
    mb_pdu_wr_rd_req_t  wr_rd_req;
}mb_pdu_t;

int mb_pdu_make(u8 *buf, const mb_pdu_t *pdu, mb_pdu_type_t type);//生成pdu帧, 返回帧长度, 失败返回0
int mb_pdu_parse(const u8 *buf, int len, mb_pdu_t *pdu, mb_pdu_type_t type);//解析pdu帧, 成功返回帧长度, 帧错误返回0, 功能码不支持返回-1

#endif
