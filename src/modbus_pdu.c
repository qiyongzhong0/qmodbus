/*
 * modbus_pdu.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */
    
#include <string.h>
#include "modbus_pdu.h"
#include "modbus_cvt.h"

static int mb_pdu_except_make(u8 *buf, const mb_pdu_except_t *exc)
{
    u8 *p = buf;
    
    p += mb_cvt_u8_put(p, exc->fc);
    p += mb_cvt_u8_put(p, exc->ec);
    
    return((int)(p - buf));
}

static int mb_pdu_except_parse(const u8 *buf, int len, mb_pdu_except_t *exc)
{
    if (len < 2)
    {
        return(0);
    }
    
    u8 *p = (u8 *)buf;
    
    p += mb_cvt_u8_get(p, &(exc->fc));
    p += mb_cvt_u8_get(p, &(exc->ec));
    
    return((int)(p - buf));
}

static int mb_pdu_rd_req_make(u8 *buf, const mb_pdu_rd_req_t *rd_req)
{
    u8 *p = buf;
    
    p += mb_cvt_u8_put(p, rd_req->fc);
    p += mb_cvt_u16_put(p, rd_req->addr);
    p += mb_cvt_u16_put(p, rd_req->nb);
    
    return((int)(p - buf));
}

static int mb_pdu_rd_req_parse(const u8 *buf, int len, mb_pdu_rd_req_t *rd_req)
{
    if (len < 5)
    {
        return(0);
    }
    
    u8 *p = (u8 *)buf;
    
    p += mb_cvt_u8_get(p, &(rd_req->fc));
    p += mb_cvt_u16_get(p, &(rd_req->addr));
    p += mb_cvt_u16_get(p, &(rd_req->nb));
    
    return((int)(p - buf));
}

static int mb_pdu_rd_rsp_make(u8 *buf, const mb_pdu_rd_rsp_t *rd_rsp)
{
    u8 *p = buf;
    
    p += mb_cvt_u8_put(p, rd_rsp->fc);
    p += mb_cvt_u8_put(p, rd_rsp->dlen);
    memcpy(p, rd_rsp->pdata, rd_rsp->dlen);
    p += rd_rsp->dlen;
    
    return((int)(p - buf));
}

static int mb_pdu_rd_rsp_parse(const u8 *buf, int len, mb_pdu_rd_rsp_t *rd_rsp)
{
    if (len < 3)
    {
        return(0);
    }
    
    u8 *p = (u8 *)buf;
    
    p += mb_cvt_u8_get(p, &(rd_rsp->fc));
    p += mb_cvt_u8_get(p, &(rd_rsp->dlen));
    rd_rsp->pdata = p;
    p += rd_rsp->dlen;
    
    return((int)(p - buf));
}

static int mb_pdu_wr_single_make(u8 *buf, const mb_pdu_wr_single_t *wr_single)
{
    u8 *p = buf;
    
    p += mb_cvt_u8_put(p, wr_single->fc);
    p += mb_cvt_u16_put(p, wr_single->addr);
    p += mb_cvt_u16_put(p, wr_single->val);
    
    return((int)(p - buf));
}

static int mb_pdu_wr_single_parse(const u8 *buf, int len, mb_pdu_wr_single_t *wr_single)
{
    if (len < 5)
    {
        return(0);
    }
    
    u8 *p = (u8 *)buf;
    
    p += mb_cvt_u8_get(p, &(wr_single->fc));
    p += mb_cvt_u16_get(p, &(wr_single->addr));
    p += mb_cvt_u16_get(p, &(wr_single->val));
    
    return((int)(p - buf));
}

static int mb_pdu_wr_req_make(u8 *buf, const mb_pdu_wr_req_t *wr_req)
{
    u8 *p = buf;
    
    p += mb_cvt_u8_put(p, wr_req->fc);
    p += mb_cvt_u16_put(p, wr_req->addr);
    p += mb_cvt_u16_put(p, wr_req->nb);
    p += mb_cvt_u8_put(p, wr_req->dlen);
    memcpy(p, wr_req->pdata, wr_req->dlen);
    p += wr_req->dlen;
    
    return((int)(p - buf));
}

static int mb_pdu_wr_req_parse(const u8 *buf, int len, mb_pdu_wr_req_t *wr_req)
{
    if (len < 7)
    {
        return(0);
    }
    
    u8 *p = (u8 *)buf;
    
    p += mb_cvt_u8_get(p, &(wr_req->fc));
    p += mb_cvt_u16_get(p, &(wr_req->addr));
    p += mb_cvt_u16_get(p, &(wr_req->nb));
    p += mb_cvt_u8_get(p, &(wr_req->dlen));
    wr_req->pdata = p;
    p += wr_req->dlen;
    
    return((int)(p - buf));
}

static int mb_pdu_wr_rsp_make(u8 *buf, const mb_pdu_wr_rsp_t *wr_rsp)
{
    u8 *p = buf;
    
    p += mb_cvt_u8_put(p, wr_rsp->fc);
    p += mb_cvt_u16_put(p, wr_rsp->addr);
    p += mb_cvt_u16_put(p, wr_rsp->nb);
    
    return((int)(p - buf));
}

static int mb_pdu_wr_rsp_parse(const u8 *buf, int len, mb_pdu_wr_rsp_t *wr_rsp)
{
    if (len < 5)
    {
        return(0);
    }
    
    u8 *p = (u8 *)buf;
    
    p += mb_cvt_u8_get(p, &(wr_rsp->fc));
    p += mb_cvt_u16_get(p, &(wr_rsp->addr));
    p += mb_cvt_u16_get(p, &(wr_rsp->nb));
    
    return((int)(p - buf));
}

static int mb_pdu_mask_wr_make(u8 *buf, const mb_pdu_mask_wr_t *mask_wr)
{
    u8 *p = buf;
    
    p += mb_cvt_u8_put(p, mask_wr->fc);
    p += mb_cvt_u16_put(p, mask_wr->addr);
    p += mb_cvt_u16_put(p, mask_wr->val_and);
    p += mb_cvt_u16_put(p, mask_wr->val_or);
    
    return((int)(p - buf));
}

static int mb_pdu_mask_wr_parse(const u8 *buf, int len, mb_pdu_mask_wr_t *mask_wr)
{
    if (len < 7)
    {
        return(0);
    }
    
    u8 *p = (u8 *)buf;
    
    p += mb_cvt_u8_get(p, &(mask_wr->fc));
    p += mb_cvt_u16_get(p, &(mask_wr->addr));
    p += mb_cvt_u16_get(p, &(mask_wr->val_and));
    p += mb_cvt_u16_get(p, &(mask_wr->val_or));
    
    return((int)(p - buf));
}

static int mb_pdu_wr_rd_req_make(u8 *buf, const mb_pdu_wr_rd_req_t *wr_rd_req)
{
    u8 *p = buf;
    
    p += mb_cvt_u8_put(p, wr_rd_req->fc);
    p += mb_cvt_u16_put(p, wr_rd_req->rd_addr);
    p += mb_cvt_u16_put(p, wr_rd_req->rd_nb);
    p += mb_cvt_u16_put(p, wr_rd_req->wr_addr);
    p += mb_cvt_u16_put(p, wr_rd_req->wr_nb);
    p += mb_cvt_u8_put(p, wr_rd_req->dlen);
    memcpy(p, wr_rd_req->pdata, wr_rd_req->dlen);
    p += wr_rd_req->dlen;
    
    return((int)(p - buf));
}

static int mb_pdu_wr_rd_req_parse(const u8 *buf, int len, mb_pdu_wr_rd_req_t *wr_rd_req)
{
    if (len < 11)
    {
        return(0);
    }
    
    u8 *p = (u8 *)buf;
    
    p += mb_cvt_u8_get(p, &(wr_rd_req->fc));
    p += mb_cvt_u16_get(p, &(wr_rd_req->rd_addr));
    p += mb_cvt_u16_get(p, &(wr_rd_req->rd_nb));
    p += mb_cvt_u16_get(p, &(wr_rd_req->wr_addr));
    p += mb_cvt_u16_get(p, &(wr_rd_req->wr_nb));
    p += mb_cvt_u8_get(p, &(wr_rd_req->dlen));
    wr_rd_req->pdata = p;
    p += wr_rd_req->dlen;
    
    return((int)(p - buf));
}

static int mb_pdu_req_make(u8 *buf, const mb_pdu_t *pdu)//生成pdu请求帧, 返回帧长度, 失败返回0
{
    switch(pdu->fc)
    {
    case MODBUS_FC_READ_COILS :
        return(mb_pdu_rd_req_make(buf, (mb_pdu_rd_req_t *)pdu));
    case MODBUS_FC_READ_DISCRETE_INPUTS :
        return(mb_pdu_rd_req_make(buf, (mb_pdu_rd_req_t *)pdu));
    case MODBUS_FC_READ_HOLDING_REGISTERS :
        return(mb_pdu_rd_req_make(buf, (mb_pdu_rd_req_t *)pdu));
    case MODBUS_FC_READ_INPUT_REGISTERS :
        return(mb_pdu_rd_req_make(buf, (mb_pdu_rd_req_t *)pdu));
    case MODBUS_FC_WRITE_SINGLE_COIL :
        return(mb_pdu_wr_single_make(buf, (mb_pdu_wr_single_t *)pdu));
    case MODBUS_FC_WRITE_SINGLE_REGISTER :
        return(mb_pdu_wr_single_make(buf, (mb_pdu_wr_single_t *)pdu));
    case MODBUS_FC_READ_EXCEPTION_STATUS :
        break;
    case MODBUS_FC_WRITE_MULTIPLE_COILS :
        return(mb_pdu_wr_req_make(buf, (mb_pdu_wr_req_t *)pdu));
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS :
        return(mb_pdu_wr_req_make(buf, (mb_pdu_wr_req_t *)pdu));
    case MODBUS_FC_REPORT_SLAVE_ID :
        break;
    case MODBUS_FC_MASK_WRITE_REGISTER :
        return(mb_pdu_mask_wr_make(buf, (mb_pdu_mask_wr_t *)pdu));
    case MODBUS_FC_WRITE_AND_READ_REGISTERS :
        return(mb_pdu_wr_rd_req_make(buf, (mb_pdu_wr_rd_req_t *)pdu));
    default:
        break;
    }

    return(0);
}

static int mb_pdu_req_parse(const u8 *buf, int len, mb_pdu_t *pdu)//解析pdu请求帧, 成功返回帧长度, 失败返回-1
{
    u8 fc = *buf;
    switch(fc)
    {
    case MODBUS_FC_READ_COILS :
        return(mb_pdu_rd_req_parse(buf, len, (mb_pdu_rd_req_t *)pdu));
    case MODBUS_FC_READ_DISCRETE_INPUTS :
        return(mb_pdu_rd_req_parse(buf, len, (mb_pdu_rd_req_t *)pdu));
    case MODBUS_FC_READ_HOLDING_REGISTERS :
        return(mb_pdu_rd_req_parse(buf, len, (mb_pdu_rd_req_t *)pdu));
    case MODBUS_FC_READ_INPUT_REGISTERS :
        return(mb_pdu_rd_req_parse(buf, len, (mb_pdu_rd_req_t *)pdu));
    case MODBUS_FC_WRITE_SINGLE_COIL :
        return(mb_pdu_wr_single_parse(buf, len, (mb_pdu_wr_single_t *)pdu));
    case MODBUS_FC_WRITE_SINGLE_REGISTER :
        return(mb_pdu_wr_single_parse(buf, len, (mb_pdu_wr_single_t *)pdu));
    case MODBUS_FC_READ_EXCEPTION_STATUS :
        break;
    case MODBUS_FC_WRITE_MULTIPLE_COILS :
        return(mb_pdu_wr_req_parse(buf, len, (mb_pdu_wr_req_t *)pdu));
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS :
        return(mb_pdu_wr_req_parse(buf, len, (mb_pdu_wr_req_t *)pdu));
    case MODBUS_FC_REPORT_SLAVE_ID :
        break;
    case MODBUS_FC_MASK_WRITE_REGISTER :
        return(mb_pdu_mask_wr_parse(buf, len, (mb_pdu_mask_wr_t *)pdu));
    case MODBUS_FC_WRITE_AND_READ_REGISTERS :
        return(mb_pdu_wr_rd_req_parse(buf, len, (mb_pdu_wr_rd_req_t *)pdu));
    default:
        break;
    }

    return(-1);
}

static int mb_pdu_rsp_make(u8 *buf, const mb_pdu_t *pdu)//生成pdu响应帧, 返回帧长度, 失败返回0
{
    if (MODBUS_FC_EXCEPT_CHK(pdu->fc))
    {
        return(mb_pdu_except_make(buf, (mb_pdu_except_t *)pdu));
    }
    
    switch(pdu->fc)
    {
    case MODBUS_FC_READ_COILS :
        return(mb_pdu_rd_rsp_make(buf, (mb_pdu_rd_rsp_t *)pdu));
    case MODBUS_FC_READ_DISCRETE_INPUTS :
        return(mb_pdu_rd_rsp_make(buf, (mb_pdu_rd_rsp_t *)pdu));
    case MODBUS_FC_READ_HOLDING_REGISTERS :
        return(mb_pdu_rd_rsp_make(buf, (mb_pdu_rd_rsp_t *)pdu));
    case MODBUS_FC_READ_INPUT_REGISTERS :
        return(mb_pdu_rd_rsp_make(buf, (mb_pdu_rd_rsp_t *)pdu));
    case MODBUS_FC_WRITE_SINGLE_COIL :
        return(mb_pdu_wr_single_make(buf, (mb_pdu_wr_single_t *)pdu));
    case MODBUS_FC_WRITE_SINGLE_REGISTER :
        return(mb_pdu_wr_single_make(buf, (mb_pdu_wr_single_t *)pdu));
    case MODBUS_FC_READ_EXCEPTION_STATUS :
        break;
    case MODBUS_FC_WRITE_MULTIPLE_COILS :
        return(mb_pdu_wr_rsp_make(buf, (mb_pdu_wr_rsp_t *)pdu));
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS :
        return(mb_pdu_wr_rsp_make(buf, (mb_pdu_wr_rsp_t *)pdu));
    case MODBUS_FC_REPORT_SLAVE_ID :
        break;
    case MODBUS_FC_MASK_WRITE_REGISTER :
        return(mb_pdu_mask_wr_make(buf, (mb_pdu_mask_wr_t *)pdu));
    case MODBUS_FC_WRITE_AND_READ_REGISTERS :
        return(mb_pdu_rd_rsp_make(buf, (mb_pdu_rd_rsp_t *)pdu));
    default:
        break;
    }

    return(0);
}

static int mb_pdu_rsp_parse(const u8 *buf, int len, mb_pdu_t *pdu)//解析pdu响应帧, 成功返回帧长度, 失败返回-1
{
    u8 fc = *buf;
    if (MODBUS_FC_EXCEPT_CHK(fc))
    {
        return(mb_pdu_except_parse(buf, len, (mb_pdu_except_t *)pdu));
    }
    
    switch(fc)
    {
    case MODBUS_FC_READ_COILS :
        return(mb_pdu_rd_rsp_parse(buf, len, (mb_pdu_rd_rsp_t *)pdu));
    case MODBUS_FC_READ_DISCRETE_INPUTS :
        return(mb_pdu_rd_rsp_parse(buf, len, (mb_pdu_rd_rsp_t *)pdu));
    case MODBUS_FC_READ_HOLDING_REGISTERS :
        return(mb_pdu_rd_rsp_parse(buf, len, (mb_pdu_rd_rsp_t *)pdu));
    case MODBUS_FC_READ_INPUT_REGISTERS :
        return(mb_pdu_rd_rsp_parse(buf, len, (mb_pdu_rd_rsp_t *)pdu));
    case MODBUS_FC_WRITE_SINGLE_COIL :
        return(mb_pdu_wr_single_parse(buf, len, (mb_pdu_wr_single_t *)pdu));
    case MODBUS_FC_WRITE_SINGLE_REGISTER :
        return(mb_pdu_wr_single_parse(buf, len, (mb_pdu_wr_single_t *)pdu));
    case MODBUS_FC_READ_EXCEPTION_STATUS :
        break;
    case MODBUS_FC_WRITE_MULTIPLE_COILS :
        return(mb_pdu_wr_rsp_parse(buf, len, (mb_pdu_wr_rsp_t *)pdu));
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS :
        return(mb_pdu_wr_rsp_parse(buf, len, (mb_pdu_wr_rsp_t *)pdu));
    case MODBUS_FC_REPORT_SLAVE_ID :
        break;
    case MODBUS_FC_MASK_WRITE_REGISTER :
        return(mb_pdu_mask_wr_parse(buf, len, (mb_pdu_mask_wr_t *)pdu));
    case MODBUS_FC_WRITE_AND_READ_REGISTERS :
        return(mb_pdu_rd_rsp_parse(buf, len, (mb_pdu_rd_rsp_t *)pdu));
    default:
        break;
    }

    return(-1);
}

int mb_pdu_make(u8 *buf, const mb_pdu_t *pdu, mb_pdu_type_t type)//生成pdu帧, 返回帧长度, 错误返回0
{
    switch(type)
    {
    case MB_PDU_TYPE_REQ:
        return(mb_pdu_req_make(buf, pdu));
    case MB_PDU_TYPE_RSP:
        return(mb_pdu_rsp_make(buf, pdu));
    default:
        break;
    }
    return(0);
}

int mb_pdu_parse(const u8 *buf, int len, mb_pdu_t *pdu, mb_pdu_type_t type)//解析pdu帧, 成功返回帧长度, 帧错误返回0, 功能码不支持返回-1
{
    switch(type)
    {
    case MB_PDU_TYPE_REQ:
        return(mb_pdu_req_parse(buf, len, pdu));
    case MB_PDU_TYPE_RSP:
        return(mb_pdu_rsp_parse(buf, len, pdu));
    default:
        break;
    }
    return(0);
}


