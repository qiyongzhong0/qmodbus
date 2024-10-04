/*
 * modbus_slave.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include <string.h>
#include "modbus.h"
#include "modbus_cvt.h"
#include "modbus_port.h"

#ifdef MB_USING_SLAVE

static void mb_slave_pdu_deal_read_coils(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->read_coil == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->rd_req.addr;
    int nb = pdu->rd_req.nb;
    memset(hinst->datas, 0, sizeof(hinst->datas));
    for (int i=0; i<nb; i++)
    {
        u8 bit;
        int rst = hinst->cb->read_coil(addr + i, &bit);
        if (rst < 0)
        {
            pdu->exc.ec = -rst;
            pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
            return;
        }
        mb_bitmap_set(hinst->datas, i, bit);
    }
    
    pdu->rd_rsp.dlen = (nb + 7) / 8;
    pdu->rd_rsp.pdata = hinst->datas;
}

static void mb_slave_pdu_deal_read_discs(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->read_disc == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->rd_req.addr;
    int nb = pdu->rd_req.nb;
    memset(hinst->datas, 0, sizeof(hinst->datas));
    for (int i=0; i<nb; i++)
    {
        u8 bit;
        int rst = hinst->cb->read_disc(addr + i, &bit);
        if (rst < 0)
        {
            pdu->exc.ec = -rst;
            pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
            return;
        }
        mb_bitmap_set(hinst->datas, i, bit);
    }
    
    pdu->rd_rsp.dlen = (nb + 7) / 8;
    pdu->rd_rsp.pdata = hinst->datas;
}

static void mb_slave_pdu_deal_read_holds(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->read_hold == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->rd_req.addr;
    int nb = pdu->rd_req.nb;
    u8 *p = hinst->datas;
    for (int i=0; i<nb; i++)
    {
        u16 val;
        int rst = hinst->cb->read_hold(addr + i, &val);
        if (rst < 0)
        {
            pdu->exc.ec = -rst;
            pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
            return;
        }
        p += mb_cvt_u16_put(p, val);
    }
    
    pdu->rd_rsp.dlen = 2 * nb;
    pdu->rd_rsp.pdata = hinst->datas;
}

static void mb_slave_pdu_deal_read_inputs(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->read_input == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->rd_req.addr;
    int nb = pdu->rd_req.nb;
    u8 *p = hinst->datas;
    for (int i=0; i<nb; i++)
    {
        u16 val;
        int rst = hinst->cb->read_input(addr + i, &val);
        if (rst < 0)
        {
            pdu->exc.ec = -rst;
            pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
            return;
        }
        p += mb_cvt_u16_put(p, val);
    }
    
    pdu->rd_rsp.dlen = 2 * nb;
    pdu->rd_rsp.pdata = hinst->datas;
}

static void mb_slave_pdu_deal_write_coil(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->write_coil == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->wr_single.addr;
    u16 val = pdu->wr_single.val;
    if ((val != 0xFF00) && (val != 0x0000))
    {
        pdu->exc.ec = MODBUS_EC_ILLEGAL_DATA_VALUE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }
    int rst = hinst->cb->write_coil(addr, (val ? 1 : 0));
    if (rst < 0)
    {
        pdu->exc.ec = -rst;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }
}

static void mb_slave_pdu_deal_write_reg(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->write_hold == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->wr_single.addr;
    u16 val = pdu->wr_single.val;
    int rst = hinst->cb->write_hold(addr, val);
    if (rst < 0)
    {
        pdu->exc.ec = -rst;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }
}

static void mb_slave_pdu_deal_write_coils(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->write_coil == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->wr_req.addr;
    int nb = pdu->wr_req.nb;
    u8 *pbits = pdu->wr_req.pdata;
    for (int i=0; i<nb; i++)
    {
        u8 bit = mb_bitmap_get(pbits, i);
        int rst = hinst->cb->write_coil(addr + i, bit);
        if (rst < 0)
        {
            pdu->exc.ec = -rst;
            pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
            return;
        }
    }
}

static void mb_slave_pdu_deal_write_regs(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->write_hold == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->wr_req.addr;
    int nb = pdu->wr_req.nb;
    u8 *p = pdu->wr_req.pdata;
    for (int i=0; i<nb; i++)
    {
        u16 val;
        p += mb_cvt_u16_get(p, &val);
        int rst = hinst->cb->write_hold(addr + i, val);
        if (rst < 0)
        {
            pdu->exc.ec = -rst;
            pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
            return;
        }
    }
}

static void mb_slave_pdu_deal_mask_write_reg(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->read_hold == NULL) || (hinst->cb->write_hold == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 addr = pdu->mask_wr.addr;
    u16 val_and = pdu->mask_wr.val_and;
    u16 val_or = pdu->mask_wr.val_or;
    u16 val;
    int rst = hinst->cb->read_hold(addr, &val);
    if (rst < 0)
    {
        pdu->exc.ec = -rst;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }
    val = ((val & val_and) | (val_or & ~val_and));
    rst = hinst->cb->write_hold(addr, val);
    if (rst < 0)
    {
        pdu->exc.ec = -rst;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }
}

static void mb_slave_pdu_deal_write_and_read_regs(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    if ((hinst->cb == NULL) || (hinst->cb->read_hold == NULL) || (hinst->cb->write_hold == NULL))
    {
        pdu->exc.ec = MODBUS_EC_SLAVE_OR_SERVER_FAILURE;
        pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
        return;
    }

    u16 rd_addr = pdu->wr_rd_req.rd_addr;
    int rd_nb = pdu->wr_rd_req.rd_nb;
    u16 wr_addr = pdu->wr_rd_req.wr_addr;
    int wr_nb = pdu->wr_rd_req.wr_nb;
    u8 *p = pdu->wr_rd_req.pdata;
    for (int i=0; i<wr_nb; i++)
    {
        u16 val;
        p += mb_cvt_u16_get(p, &val);
        int rst = hinst->cb->write_hold(wr_addr + i, val);
        if (rst < 0)
        {
            pdu->exc.ec = -rst;
            pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
            return;
        }
    }

    p = hinst->datas;
    for (int i=0; i<rd_nb; i++)
    {
        u16 val;
        int rst = hinst->cb->read_hold(rd_addr + i, &val);
        if (rst < 0)
        {
            pdu->exc.ec = -rst;
            pdu->exc.fc = MODBUS_FC_EXCEPT_MAKE(pdu->exc.fc);
            return;
        }
        p += mb_cvt_u16_put(p, val);
    }
    
    pdu->rd_rsp.dlen = rd_nb * 2;
    pdu->rd_rsp.pdata = hinst->datas;
}

static void mb_slave_pdu_deal(mb_inst_t *hinst, mb_pdu_t *pdu)
{
    switch(pdu->fc)
    {
    case MODBUS_FC_READ_COILS :
        mb_slave_pdu_deal_read_coils(hinst, pdu);
        break;
    case MODBUS_FC_READ_DISCRETE_INPUTS :
        mb_slave_pdu_deal_read_discs(hinst, pdu);
        break;
    case MODBUS_FC_READ_HOLDING_REGISTERS :
        mb_slave_pdu_deal_read_holds(hinst, pdu);
        break;
    case MODBUS_FC_READ_INPUT_REGISTERS :
        mb_slave_pdu_deal_read_inputs(hinst, pdu);
        break;
    case MODBUS_FC_WRITE_SINGLE_COIL :
        mb_slave_pdu_deal_write_coil(hinst, pdu);
        break;
    case MODBUS_FC_WRITE_SINGLE_REGISTER :
        mb_slave_pdu_deal_write_reg(hinst, pdu);
        break;
    case MODBUS_FC_READ_EXCEPTION_STATUS :
        break;
    case MODBUS_FC_WRITE_MULTIPLE_COILS :
        mb_slave_pdu_deal_write_coils(hinst, pdu);
        break;
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS :
        mb_slave_pdu_deal_write_regs(hinst, pdu);
        break;
    case MODBUS_FC_REPORT_SLAVE_ID :
        break;
    case MODBUS_FC_MASK_WRITE_REGISTER :
        mb_slave_pdu_deal_mask_write_reg(hinst, pdu);
        break;
    case MODBUS_FC_WRITE_AND_READ_REGISTERS :
        mb_slave_pdu_deal_write_and_read_regs(hinst, pdu);
        break;
    default:
        break;
    }
}

#ifdef MB_USING_RTU_PROTOCOL
static void mb_slave_recv_deal_rtu(mb_inst_t *hinst, u8 *buf, int len)
{
    mb_rtu_frm_t frm;
    int pdu_len = mb_rtu_frm_parse(buf, len, &frm, MB_PDU_TYPE_REQ);
    if (pdu_len == 0)//帧错误, 不处理
    {
        return;
    }
    
    #ifdef MB_USING_ADDR_CHK
    if (frm.saddr != hinst->saddr)//地址不同, 不处理
    {
        return;
    }
    #endif
    
    if (pdu_len < 0)//功能码不支持, 响应异常帧
    {
        frm.pdu.exc.ec = MODBUS_EC_ILLEGAL_FUNCTION;
        frm.pdu.exc.fc = MODBUS_FC_EXCEPT_MAKE(frm.pdu.exc.fc);
    }
    else
    {
        mb_slave_pdu_deal(hinst, &(frm.pdu));
    }

    int flen = mb_rtu_frm_make(hinst->buf, &frm, MB_PDU_TYPE_RSP);
    mb_send(hinst, hinst->buf, flen);
}
#endif

#ifdef MB_USING_TCP_PROTOCOL
static void mb_slave_recv_deal_tcp(mb_inst_t *hinst, u8 *buf, int len)
{
    mb_tcp_frm_t frm;
    int pdu_len = mb_tcp_frm_parse(buf, len, &frm, MB_PDU_TYPE_REQ);
    if (pdu_len == 0)//帧错误, 不处理
    {
        return;
    }
    
    #ifdef MB_USING_ADDR_CHK
    if ((frm.mbap.did != 0xFF) && (frm.mbap.did != hinst->saddr))//使用地址且地址不同, 不处理
    {
        return;
    }
    #endif
    
    #ifdef MB_USING_MBAP_CHK
    if (frm.mbap.pid != MB_TCP_MBAP_PID)//协议标识错误, 不处理
    {
        return;
    }
    #endif
    
    if (pdu_len < 0)//功能码不支持, 响应异常帧
    {
        frm.pdu.exc.ec = MODBUS_EC_ILLEGAL_FUNCTION;
        frm.pdu.exc.fc = MODBUS_FC_EXCEPT_MAKE(frm.pdu.exc.fc);
    }
    else
    {
        mb_slave_pdu_deal(hinst, &(frm.pdu));
    }

    int flen = mb_tcp_frm_make(hinst->buf, &frm, MB_PDU_TYPE_RSP);
    mb_send(hinst, hinst->buf, flen);
}
#endif

static void mb_slave_recv_deal(mb_inst_t *hinst, u8 *buf, int len)
{
    switch(hinst->prot)
    {
    #ifdef MB_USING_RTU_PROTOCOL
    case MB_PROT_RTU :
        mb_slave_recv_deal_rtu(hinst, buf, len);
        break;
    #endif
    #ifdef MB_USING_TCP_PROTOCOL
    case MB_PROT_TCP :
        mb_slave_recv_deal_tcp(hinst, buf, len);
        break;
    #endif
    default:
        break;
    }
}

const mb_cb_table_t mb_cb_table = {
    .read_disc = mb_port_read_disc,      //读离散量输入
    .read_coil = mb_port_read_coil,      //读线圈
    .write_coil = mb_port_write_coil,    //写线圈
    .read_input = mb_port_read_input,    //读输入寄存器
    .read_hold = mb_port_read_hold,      //读保持寄存器
    .write_hold = mb_port_write_hold,    //写保持寄存器
};
    
//修改从机回调函数表, 默认使用modbus_port中接口函数做回调函数
void mb_set_cb_table(mb_inst_t *hinst, const mb_cb_table_t *cb)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(cb != NULL);

    hinst->cb = (mb_cb_table_t *)cb;
}

//从机状态机处理, 在线程中循环调用即可
void mb_slave_fsm(mb_inst_t *hinst)
{
    MB_ASSERT(hinst != NULL);

    if (mb_connect(hinst) < 0)//连接失败, 延时返回
    {
        mb_port_delay_ms(1000);
        return;
    }

    int rlen = mb_recv(hinst, hinst->buf, sizeof(hinst->buf));
    if (rlen <= 0)
    {
        return;
    }
    
    mb_slave_recv_deal(hinst, hinst->buf, rlen);
}

#endif
