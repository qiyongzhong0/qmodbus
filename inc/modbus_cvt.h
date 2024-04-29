/*
 * modbus_cvt.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */

#ifndef __MODBUS_CVT_H__
#define __MODBUS_CVT_H__

#include "typedef.h"

int mb_cvt_u8_put(u8 *buf, u8 val);
int mb_cvt_u8_get(const u8 *buf, u8 *pval);
int mb_cvt_u16_put(u8 *buf, u16 val);
int mb_cvt_u16_get(const u8 *buf, u16 *pval);
int mb_cvt_u32_put(u8 *buf, u32 val);
int mb_cvt_u32_get(const u8 *buf, u32 *pval);
int mb_cvt_f32_put(u8 *buf, f32 val);
int mb_cvt_f32_get(const u8 *buf, f32 *pval);
u8 mb_bitmap_get(const u8 *pbits, int idx);//从位表中读指定索引的位
void mb_bitmap_set(u8 *pbits, int idx, u8 bit);//向位表中写指定索引的位

#endif
