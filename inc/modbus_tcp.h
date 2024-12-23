/*
 * modbus_tcp.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */

#ifndef __MODBUS_TCP_H__
#define __MODBUS_TCP_H__

#include "modbus_cfg.h"
#include "modbus_pdu.h"

#ifdef MB_USING_TCP_PROTOCOL

#define MB_TCP_MBAP_SIZE    7   //MBAP头尺寸
#define MB_TCP_FRM_MIN      (MB_TCP_MBAP_SIZE + MB_PDU_SIZE_MIN)
#define MB_TCP_FRM_MAX      (MB_TCP_MBAP_SIZE + MB_PDU_SIZE_MAX)

#define MB_TCP_MBAP_PID     0x0000//使用的协议类型代码

typedef struct{
    u16 tid;    //传输标识符, 响应须与请求一致
    u16 pid;    //协议类型, 响应须与请求一致
    u16 dlen;   //等于pdu数据长度加1, 打包时会自动计算处理不需人工赋值
    u8  did;    //逻辑设备ID, 响应须与请求一致
}mb_tcp_mbap_t;//TCP协议MBAP头定义

typedef struct{
    mb_tcp_mbap_t mbap;//MBAP头
    mb_pdu_t pdu;//PDU数据
}mb_tcp_frm_t;//TCP帧定义

int mb_tcp_frm_make(u8 *buf, const mb_tcp_frm_t *frm, mb_pdu_type_t type);//生成tcp帧, 返回帧长度
int mb_tcp_frm_parse(const u8 *buf, int len, mb_tcp_frm_t *frm, mb_pdu_type_t type);//解析tcp帧, 返回pdu数据长度, 解析失败返回0, 功能码不支持返回-1

#endif
#endif
