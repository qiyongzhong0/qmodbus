/*
 * modbus_master.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include <string.h>
#include "modbus.h"
#include "modbus_cvt.h"
#include "modbus_port.h"

#ifdef MB_USING_MASTER

#ifdef MB_USING_RTU_PROTOCOL
static int mb_read_req_rtu(mb_inst_t *hinst, u8 func, u16 addr, int nb, u8 *pdata)
{
    mb_rtu_frm_t frm;
    frm.saddr = hinst->saddr;
    frm.pdu.rd_req.fc = func;
    frm.pdu.rd_req.addr = addr;
    frm.pdu.rd_req.nb = nb;
    
    int flen = mb_rtu_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_rtu_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len <= 0)
    {
        return(0);
    }
    
    #ifdef MB_USING_ADDR_CHK
    if (frm.saddr != hinst->saddr)
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    int dlen = frm.pdu.rd_rsp.dlen;
    memcpy(pdata, frm.pdu.rd_rsp.pdata, dlen);

    return(dlen);
}
#endif

#ifdef MB_USING_TCP_PROTOCOL
static int mb_read_req_tcp(mb_inst_t *hinst, u8 func, u16 addr, int nb, u8 *pdata)
{
    mb_tcp_frm_t frm;
    hinst->tid++;
    frm.mbap.tid = hinst->tid;
    frm.mbap.pid = MB_TCP_MBAP_PID;
    frm.mbap.did = hinst->saddr;
    frm.pdu.rd_req.fc = func;
    frm.pdu.rd_req.addr = addr;
    frm.pdu.rd_req.nb = nb;
    
    int flen = mb_tcp_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_tcp_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len < 0)
    {
        return(0);
    }

    #ifdef MB_USING_ADDR_CHK
    if (frm.mbap.did != hinst->saddr)
    {
        return(0);
    }
    #endif

    #ifdef MB_USING_MBAP_CHK
    if ((frm.mbap.tid != hinst->tid) || (frm.mbap.pid != MB_TCP_MBAP_PID) || (frm.mbap.dlen != (pdu_len + 1)))
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    int dlen = frm.pdu.rd_rsp.dlen;
    memcpy(pdata, frm.pdu.rd_rsp.pdata, dlen);

    return(dlen);
}
#endif

//读请求, 功能码和数据由用户确定, 成功返回应答数据长度, 异常应答返回负值错误码, 其它错误返回0
int mb_read_req(mb_inst_t *hinst, u8 func, u16 addr, int nb, u8 *pdata)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(pdata != NULL);
    MB_ASSERT(nb > 0);

    switch (hinst->prot)
    {
    #ifdef MB_USING_RTU_PROTOCOL
    case MB_PROT_RTU :
        return(mb_read_req_rtu(hinst, func, addr, nb, pdata));
    #endif
    #ifdef MB_USING_TCP_PROTOCOL
    case MB_PROT_TCP :
        return(mb_read_req_tcp(hinst, func, addr, nb, pdata));
    #endif
    default:
        break;
    }
    
    return(0);
}

#ifdef MB_USING_RTU_PROTOCOL
static int mb_write_req_rtu(mb_inst_t *hinst, u8 func, u16 addr, int nb, const u8 *pdata, int dlen)
{
    mb_rtu_frm_t frm;
    frm.saddr = hinst->saddr;
    frm.pdu.wr_req.fc = func;
    frm.pdu.wr_req.addr = addr;
    frm.pdu.wr_req.nb = nb;
    frm.pdu.wr_req.dlen = dlen;
    frm.pdu.wr_req.pdata = (u8 *)pdata;
    
    int flen = mb_rtu_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_rtu_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len <= 0)
    {
        return(0);
    }
    
    #ifdef MB_USING_ADDR_CHK
    if (frm.saddr != hinst->saddr)
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    return(frm.pdu.wr_rsp.nb);
}
#endif

#ifdef MB_USING_TCP_PROTOCOL
static int mb_write_req_tcp(mb_inst_t *hinst, u8 func, u16 addr, int nb, const u8 *pdata, int dlen)
{
    mb_tcp_frm_t frm;
    hinst->tid++;
    frm.mbap.tid = hinst->tid;
    frm.mbap.pid = MB_TCP_MBAP_PID;
    frm.mbap.did = hinst->saddr;
    frm.pdu.wr_req.fc = func;
    frm.pdu.wr_req.addr = addr;
    frm.pdu.wr_req.nb = nb;
    frm.pdu.wr_req.dlen = dlen;
    frm.pdu.wr_req.pdata = (u8 *)pdata;
    
    int flen = mb_tcp_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_tcp_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len < 0)
    {
        return(0);
    }

    #ifdef MB_USING_ADDR_CHK
    if (frm.mbap.did != hinst->saddr)
    {
        return(0);
    }
    #endif

    #ifdef MB_USING_MBAP_CHK
    if ((frm.mbap.tid != hinst->tid) || (frm.mbap.pid != MB_TCP_MBAP_PID) || (frm.mbap.dlen != (pdu_len + 1)))
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    return(frm.pdu.wr_rsp.nb);
}
#endif

//写请求, 功能码和数据由用户确定, 成功返回写入数量, 异常应答返回负值错误码, 其它错误返回0
int mb_write_req(mb_inst_t *hinst, u8 func, u16 addr, int nb, const u8 *pdata, int dlen)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(pdata != NULL);
    MB_ASSERT(nb > 0);
    MB_ASSERT(dlen > 0);

    switch (hinst->prot)
    {
    #ifdef MB_USING_RTU_PROTOCOL
    case MB_PROT_RTU :
        return(mb_write_req_rtu(hinst, func, addr, nb, pdata, dlen));
    #endif
    #ifdef MB_USING_TCP_PROTOCOL
    case MB_PROT_TCP :
        return(mb_write_req_tcp(hinst, func, addr, nb, pdata, dlen));
    #endif
    default:
        break;
    }
    
    return(0);
}

//读多个线圈, 功能码-0x01, 成功返回读取位数量, 异常应答返回负值错误码, 其它错误返回0
int mb_read_bits(mb_inst_t *hinst, u16 addr, int nb, u8 *pbits)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(pbits != NULL);
    MB_ASSERT(nb > 0);
    
    int dlen = mb_read_req(hinst, MODBUS_FC_READ_COILS, addr, nb, pbits);
    return((dlen != (nb + 7) / 8) ? dlen : nb);
}

//读多个离散量输入, 功能码-0x02, 成功返回读取位数量, 异常应答返回负值错误码, 其它错误返回0
int mb_read_input_bits(mb_inst_t *hinst, u16 addr, int nb, u8 *pbits)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(pbits != NULL);
    MB_ASSERT(nb > 0);
    
    int dlen = mb_read_req(hinst, MODBUS_FC_READ_DISCRETE_INPUTS, addr, nb, pbits);
    return((dlen != (nb + 7) / 8) ? dlen : nb);
}

//读多个保持寄存器, 功能码-0x03, 成功返回读取寄存器数量, 异常应答返回负值错误码, 其它错误返回0
int mb_read_regs(mb_inst_t *hinst, u16 addr, int nb, u16 *pregs)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(pregs != NULL);
    MB_ASSERT(nb > 0);
    
    int dlen = mb_read_req(hinst, MODBUS_FC_READ_HOLDING_REGISTERS, addr, nb, hinst->datas);
    if (dlen <= 0)
    {
        return(dlen);
    }
    
    if (dlen != (nb * 2))
    {
        return(0);
    }

    u8 *p = hinst->datas;
    for (int i=0; i<nb; i++)
    {
        p += mb_cvt_u16_get(p, pregs + i);
    }

    return(nb);
}

//读多个输入寄存器, 功能码-0x04, 成功返回读取寄存器数量, 异常应答返回负值错误码, 其它错误返回0
int mb_read_input_regs(mb_inst_t *hinst, u16 addr, int nb, u16 *pregs)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(pregs != NULL);
    MB_ASSERT(nb > 0);
    
    int dlen = mb_read_req(hinst, MODBUS_FC_READ_INPUT_REGISTERS, addr, nb, hinst->datas);
    if (dlen <= 0)
    {
        return(dlen);
    }
    
    if (dlen != (nb * 2))
    {
        return(0);
    }

    u8 *p = hinst->datas;
    for (int i=0; i<nb; i++)
    {
        p += mb_cvt_u16_get(p, pregs + i);
    }

    return(nb);
}

#ifdef MB_USING_RTU_PROTOCOL
static int mb_write_single_rtu(mb_inst_t *hinst, u8 func, u16 addr, u16 val)
{
    mb_rtu_frm_t frm;
    frm.saddr = hinst->saddr;
    frm.pdu.wr_single.fc = func;
    frm.pdu.wr_single.addr = addr;
    frm.pdu.wr_single.val = val;
    
    int flen = mb_rtu_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_rtu_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len <= 0)
    {
        return(0);
    }
    
    #ifdef MB_USING_ADDR_CHK
    if (frm.saddr != hinst->saddr)
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    return(1);
}
#endif

#ifdef MB_USING_TCP_PROTOCOL
static int mb_write_single_tcp(mb_inst_t *hinst, u8 func, u16 addr, u16 val)
{
    mb_tcp_frm_t frm;
    hinst->tid++;
    frm.mbap.tid = hinst->tid;
    frm.mbap.pid = MB_TCP_MBAP_PID;
    frm.mbap.did = hinst->saddr;
    frm.pdu.wr_single.fc = func;
    frm.pdu.wr_single.addr = addr;
    frm.pdu.wr_single.val = val;
    
    int flen = mb_tcp_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_tcp_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len < 0)
    {
        return(0);
    }

    #ifdef MB_USING_ADDR_CHK
    if (frm.mbap.did != hinst->saddr)
    {
        return(0);
    }
    #endif

    #ifdef MB_USING_MBAP_CHK
    if ((frm.mbap.tid != hinst->tid) || (frm.mbap.pid != MB_TCP_MBAP_PID) || (frm.mbap.dlen != (pdu_len + 1)))
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    return(1);
}
#endif

static int mb_write_single(mb_inst_t *hinst, u8 func, u16 addr, u16 val)
{
    switch (hinst->prot)
    {
    #ifdef MB_USING_RTU_PROTOCOL
    case MB_PROT_RTU :
        return(mb_write_single_rtu(hinst, func, addr, val));
    #endif
    #ifdef MB_USING_TCP_PROTOCOL
    case MB_PROT_TCP :
        return(mb_write_single_tcp(hinst, func, addr, val));
    #endif
    default:
        break;
    }
    
    return(0);
}

//写单个线圈, 功能码-0x05, 成功返回1, 异常应答返回负值错误码, 其它错误返回0
int mb_write_bit(mb_inst_t *hinst, u16 addr, u8 bit)
{
    MB_ASSERT(hinst != NULL);
    
    u16 val = bit ? 0xFF00 : 0x0000;
    return(mb_write_single(hinst, MODBUS_FC_WRITE_SINGLE_COIL, addr, val));
}

//写单个寄存器, 功能码-0x06, 成功返回1, 异常应答返回负值错误码, 其它错误返回0
int mb_write_reg(mb_inst_t *hinst, u16 addr, u16 val)
{
    MB_ASSERT(hinst != NULL);
    
    return(mb_write_single(hinst, MODBUS_FC_WRITE_SINGLE_REGISTER, addr, val));
}

//写多个线圈, 功能码-0x0F, 成功返回写位数量, 异常应答返回负值错误码, 其它错误返回0
int mb_write_bits(mb_inst_t *hinst, u16 addr, int nb, const u8 *pbits)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(pbits != NULL);
    MB_ASSERT(nb > 0);
    
    int dlen = (nb + 7) / 8;
    return(mb_write_req(hinst, MODBUS_FC_WRITE_MULTIPLE_COILS, addr, nb, pbits, dlen));
}

//写多个寄存器, 功能码-0x10, 成功返回写寄存器数量, 异常应答返回负值错误码, 其它错误返回0
int mb_write_regs(mb_inst_t *hinst, u16 addr, int nb, const u16 *pregs)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(pregs != NULL);
    MB_ASSERT(nb > 0);
    
    u8 *p = hinst->datas;
    for (int i=0; i<nb; i++)
    {
        p += mb_cvt_u16_put(p, pregs[i]);
    }
    int dlen = nb * 2;
    return(mb_write_req(hinst, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, addr, nb, hinst->datas, dlen));
}

#ifdef MB_USING_RTU_PROTOCOL
static int mb_mask_write_rtu(mb_inst_t *hinst, u16 addr, u16 val_and, u16 val_or)
{
    mb_rtu_frm_t frm;
    frm.saddr = hinst->saddr;
    frm.pdu.mask_wr.fc = MODBUS_FC_MASK_WRITE_REGISTER;
    frm.pdu.mask_wr.addr = addr;
    frm.pdu.mask_wr.val_and = val_and;
    frm.pdu.mask_wr.val_or = val_or;
    
    int flen = mb_rtu_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_rtu_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len <= 0)
    {
        return(0);
    }
    
    #ifdef MB_USING_ADDR_CHK
    if (frm.saddr != hinst->saddr)
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    return(1);
}
#endif

#ifdef MB_USING_TCP_PROTOCOL
static int mb_mask_write_tcp(mb_inst_t *hinst, u16 addr, u16 val_and, u16 val_or)
{
    mb_tcp_frm_t frm;
    hinst->tid++;
    frm.mbap.tid = hinst->tid;
    frm.mbap.pid = MB_TCP_MBAP_PID;
    frm.mbap.did = hinst->saddr;
    frm.pdu.mask_wr.fc = MODBUS_FC_MASK_WRITE_REGISTER;
    frm.pdu.mask_wr.addr = addr;
    frm.pdu.mask_wr.val_and = val_and;
    frm.pdu.mask_wr.val_or = val_or;
    
    int flen = mb_tcp_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_tcp_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len < 0)
    {
        return(0);
    }

    #ifdef MB_USING_ADDR_CHK
    if (frm.mbap.did != hinst->saddr)
    {
        return(0);
    }
    #endif

    #ifdef MB_USING_MBAP_CHK
    if ((frm.mbap.tid != hinst->tid) || (frm.mbap.pid != MB_TCP_MBAP_PID) || (frm.mbap.dlen != (pdu_len + 1)))
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    return(1);
}
#endif

//屏蔽写寄存器, 功能码-0x16, 成功返回1, 异常应答返回负值错误码, 其它错误返回0
int mb_mask_write_reg(mb_inst_t *hinst, u16 addr, u16 mask_and, u16 mask_or)
{
    MB_ASSERT(hinst != NULL);

    switch (hinst->prot)
    {
    #ifdef MB_USING_RTU_PROTOCOL
    case MB_PROT_RTU :
        return(mb_mask_write_rtu(hinst, addr, mask_and, mask_or));
    #endif
    #ifdef MB_USING_TCP_PROTOCOL
    case MB_PROT_TCP :
        return(mb_mask_write_tcp(hinst, addr, mask_and, mask_or));
    #endif
    default:
        break;
    }
    
    return(0);
}

#ifdef MB_USING_RTU_PROTOCOL
static int mb_write_and_read_regs_rtu(mb_inst_t *hinst, u16 wr_addr, int wr_nb, const u16 *p_wr_regs, 
                                                    u16 rd_addr, int rd_nb, u16 *p_rd_regs)
{
    u8 *p = hinst->datas;
    for (int i=0; i<wr_nb; i++)
    {
        p += mb_cvt_u16_put(p, p_wr_regs[i]);
    }
    
    mb_rtu_frm_t frm;
    frm.saddr = hinst->saddr;
    frm.pdu.wr_rd_req.fc = MODBUS_FC_WRITE_AND_READ_REGISTERS;
    frm.pdu.wr_rd_req.rd_addr = rd_addr;
    frm.pdu.wr_rd_req.rd_nb = rd_nb;
    frm.pdu.wr_rd_req.wr_addr = wr_addr;
    frm.pdu.wr_rd_req.wr_nb = wr_nb;
    frm.pdu.wr_rd_req.dlen = wr_nb * 2;
    frm.pdu.wr_rd_req.pdata = hinst->datas;
    
    int flen = mb_rtu_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_rtu_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len <= 0)
    {
        return(0);
    }
    
    #ifdef MB_USING_ADDR_CHK
    if (frm.saddr != hinst->saddr)
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    int dlen = frm.pdu.rd_rsp.dlen;
    if (dlen != (rd_nb * 2))
    {
        return(0);
    }

    p = frm.pdu.rd_rsp.pdata;
    for (int i=0; i<rd_nb; i++)
    {
        p += mb_cvt_u16_get(p, p_rd_regs + i);
    }

    return(rd_nb);
}
#endif

#ifdef MB_USING_TCP_PROTOCOL
static int mb_write_and_read_regs_tcp(mb_inst_t *hinst, u16 wr_addr, int wr_nb, const u16 *p_wr_regs, 
                                                    u16 rd_addr, int rd_nb, u16 *p_rd_regs)
{
    u8 *p = hinst->datas;
    for (int i=0; i<wr_nb; i++)
    {
        p += mb_cvt_u16_put(p, p_wr_regs[i]);
    }
    
    mb_tcp_frm_t frm;
    hinst->tid++;
    frm.mbap.tid = hinst->tid;
    frm.mbap.pid = MB_TCP_MBAP_PID;
    frm.mbap.did = hinst->saddr;
    frm.pdu.wr_rd_req.fc = MODBUS_FC_WRITE_AND_READ_REGISTERS;
    frm.pdu.wr_rd_req.rd_addr = rd_addr;
    frm.pdu.wr_rd_req.rd_nb = rd_nb;
    frm.pdu.wr_rd_req.wr_addr = wr_addr;
    frm.pdu.wr_rd_req.wr_nb = wr_nb;
    frm.pdu.wr_rd_req.dlen = wr_nb * 2;
    frm.pdu.wr_rd_req.pdata = hinst->datas;
    
    int flen = mb_tcp_frm_make(hinst->buf, (void *)&frm, MB_PDU_TYPE_REQ);
    int slen = mb_send(hinst, hinst->buf, flen);
    if (slen != flen)
    {
        return(0);
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return(0);
    }
    
    int pdu_len = mb_tcp_frm_parse(hinst->buf, rlen, &frm, MB_PDU_TYPE_RSP);
    if (pdu_len < 0)
    {
        return(0);
    }

    #ifdef MB_USING_ADDR_CHK
    if (frm.mbap.did != hinst->saddr)
    {
        return(0);
    }
    #endif

    #ifdef MB_USING_MBAP_CHK
    if ((frm.mbap.tid != hinst->tid) || (frm.mbap.pid != MB_TCP_MBAP_PID) || (frm.mbap.dlen != (pdu_len + 1)))
    {
        return(0);
    }
    #endif

    if (MODBUS_FC_EXCEPT_CHK(frm.pdu.fc))//是异常应答
    {
        return(-(int)frm.pdu.exc.ec);
    }

    int dlen = frm.pdu.rd_rsp.dlen;
    if (dlen != (rd_nb * 2))
    {
        return(0);
    }

    p = frm.pdu.rd_rsp.pdata;
    for (int i=0; i<rd_nb; i++)
    {
        p += mb_cvt_u16_get(p, p_rd_regs + i);
    }

    return(rd_nb);

}
#endif

//读/写多个寄存器, 功能码-0x17, 成功返回读取寄存器数量, 异常应答返回负值错误码, 其它错误返回0
int mb_write_and_read_regs(mb_inst_t *hinst, u16 wr_addr, int wr_nb, const u16 *p_wr_regs, 
                                     u16 rd_addr, int rd_nb, u16 *p_rd_regs)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(p_wr_regs != NULL);
    MB_ASSERT(p_rd_regs != NULL);
    MB_ASSERT(wr_nb > 0);
    MB_ASSERT(rd_nb > 0);

    switch (hinst->prot)
    {
    #ifdef MB_USING_RTU_PROTOCOL
    case MB_PROT_RTU :
        return(mb_write_and_read_regs_rtu(hinst, wr_addr, wr_nb, p_wr_regs, rd_addr, rd_nb, p_rd_regs));
    #endif
    #ifdef MB_USING_TCP_PROTOCOL
    case MB_PROT_TCP :
        return(mb_write_and_read_regs_tcp(hinst, wr_addr, wr_nb, p_wr_regs, rd_addr, rd_nb, p_rd_regs));
    #endif
    default:
        break;
    }
    
    return(0);
}
                                     
#endif
