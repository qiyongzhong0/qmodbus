/*
 * modbus_rtu.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */

#include "modbus_rtu.h"
#include "modbus_crc.h"
#include "modbus_cvt.h"

int mb_rtu_frm_make(u8 *buf, const mb_rtu_frm_t *frm, mb_pdu_type_t type)//生成rtu帧, 返回帧长度
{
    u8 *p = buf;
    p += mb_cvt_u8_put(p, frm->saddr);
    p += mb_pdu_make(p, &(frm->pdu), type);
    u16 crc = mb_crc_cal(buf, (int)(p - buf));
    p += mb_cvt_u16_put(p, crc);
    return((int)(p - buf));
}

int mb_rtu_frm_parse(const u8 *buf, int len, mb_rtu_frm_t *frm, mb_pdu_type_t type)//解析rtu帧, 返回pdu数据长度, 解析失败返回0, 功能码不支持返回-1
{
    if (len < MB_RTU_FRM_MIN)
    {
        return(0);
    }

    frm->saddr = *buf;
    int remain = len - (MB_RTU_SADDR_SIZE + MB_RTU_CRC_SIZE);
    int pdu_len = mb_pdu_parse(buf + 1, remain, &(frm->pdu), type);
    if (pdu_len <= 0)
    {
        return(pdu_len);
    }
    
    if (remain < pdu_len)
    {
        return(0);
    }
    
    int flen = pdu_len + (MB_RTU_SADDR_SIZE + MB_RTU_CRC_SIZE);
    u16 cal_crc = mb_crc_cal(buf, flen);
    if (cal_crc != 0)
    {
        return(0);
    }

    return(pdu_len);
}

