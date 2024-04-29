/*
 * modbus_cvt.c
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */

#include "modbus_cvt.h"

int mb_cvt_u8_put(u8 *buf, u8 val)
{
    *buf = val;
    return(1);
}

int mb_cvt_u8_get(const u8 *buf, u8 *pval)
{
    *pval = *buf;
    return(1);
}

int mb_cvt_u16_put(u8 *buf, u16 val)
{
    u8 *p = buf;
    *p++ = (val >> 8);
    *p++ = val;
    return(2);
}

int mb_cvt_u16_get(const u8 *buf, u16 *pval)
{
    u8 *p = (u8 *)buf;
    u16 uval = *p++;
    uval <<= 8;
    uval += *p++;
    *pval = uval;
    return(2);
}

int mb_cvt_u32_put(u8 *buf, u32 val)
{
    u8 *p = buf;
    *p++ = (val >> 24);
    *p++ = (val >> 16);
    *p++ = (val >> 8);
    *p++ = val;
    return(4);
}

int mb_cvt_u32_get(const u8 *buf, u32 *pval)
{
    u8 *p = (u8 *)buf;
    u32 uval = *p++;
    uval <<= 8;
    uval += *p++;
    uval <<= 8;
    uval += *p++;
    uval <<= 8;
    uval += *p++;
    *pval = uval;
    return(4);
}

int mb_cvt_f32_put(u8 *buf, f32 val)
{
    float fval = val;
    u32 *puv = (u32 *)&fval;
    return(mb_cvt_u32_put(buf, *puv));
}

int mb_cvt_f32_get(const u8 *buf, f32 *pval)
{
    return(mb_cvt_u32_get(buf, (u32 *)pval));
}

u8 mb_bitmap_get(const u8 *pbits, int idx)//从位表中读指定索引的位
{
    return(((pbits[idx/8] & (1 << (idx % 8))) != 0) ? 1 : 0);
}

void mb_bitmap_set(u8 *pbits, int idx, u8 bit)//向位表中写指定索引的位
{
    if (bit)
    {
        pbits[idx/8] |= (1 << (idx % 8));
    }
    else
    {
        pbits[idx/8] &= ~(1 << (idx % 8));
    }
}

