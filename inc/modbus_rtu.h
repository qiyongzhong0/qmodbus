/*
 * modbus_rtu.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */

#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

#include "modbus_cfg.h"
#include "modbus_pdu.h"

#ifdef MB_USING_RTU_PROTOCOL

#define MB_RTU_SADDR_SIZE       1   //RTU从机地址尺寸
#define MB_RTU_CRC_SIZE         2
#define MB_RTU_FRM_MIN          (MB_RTU_SADDR_SIZE + MB_RTU_CRC_SIZE + MB_PDU_SIZE_MIN)
#define MB_RTU_FRM_MAX          (MB_RTU_SADDR_SIZE + MB_RTU_CRC_SIZE + MB_PDU_SIZE_MAX)

#define MB_RTU_ADDR_BROADCAST   0   //广播地址
#define MB_RTU_ADDR_MIN         1   //最小地址
#define MB_RTU_ADDR_MAX         247 //最大地址
#define MB_RTU_ADDR_DEF         1   //默认地址

typedef struct{
    u8 saddr;       //从机地址
    mb_pdu_t pdu;   //pdu
}mb_rtu_frm_t;

int mb_rtu_frm_make(u8 *buf, const mb_rtu_frm_t *frm, mb_pdu_type_t type);//生成rtu帧, 返回帧长度
int mb_rtu_frm_parse(const u8 *buf, int len, mb_rtu_frm_t *frm, mb_pdu_type_t type);//解析rtu帧, 返回pdu数据长度, 帧错误返回0, 功能码不支持返回-1

#endif
#endif