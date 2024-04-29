/*
 * modbus_tcp.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */
 
#include "modbus_tcp.h"
#include "modbus_cvt.h"

#ifdef MB_USING_TCP_PROTOCOL

int mb_tcp_frm_make(u8 *buf, const mb_tcp_frm_t *frm, mb_pdu_type_t type)//生成tcp帧, 返回帧长度
{
    int pdu_len = mb_pdu_make(buf + MB_TCP_MBAP_SIZE, &(frm->pdu), type);
    
    u8 *p = buf;
    p += mb_cvt_u16_put(p, frm->mbap.tid);
    p += mb_cvt_u16_put(p, frm->mbap.pid);
    p += mb_cvt_u16_put(p, pdu_len + 1);
    p += mb_cvt_u8_put(p, frm->mbap.did);
    p += pdu_len;
    
    return((int)(p - buf));
}

int mb_tcp_frm_parse(const u8 *buf, int len, mb_tcp_frm_t *frm, mb_pdu_type_t type)//解析tcp帧, 返回pdu数据长度, 解析失败返回0, 功能码不支持返回-1
{
    if (len < MB_TCP_FRM_MIN)
    {
        return(0);
    }

    u8 *p = (u8 *)buf;
    p += mb_cvt_u16_get(p, &(frm->mbap.tid));
    p += mb_cvt_u16_get(p, &(frm->mbap.pid));
    p += mb_cvt_u16_get(p, &(frm->mbap.dlen));
    p += mb_cvt_u8_get(p, &(frm->mbap.did));

    int remain = len - (int)(p - buf);
    int pdu_len = mb_pdu_parse(p, remain, &(frm->pdu), type);
    if (pdu_len <= 0)
    {
        return(pdu_len);
    }
    
    if (remain < pdu_len)
    {
        return(0);
    }
    
    return(pdu_len);
}

#endif
