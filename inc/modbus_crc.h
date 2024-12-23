/*
 * modbus_crc.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#ifndef __MODBUS_CRC_H__
#define __MODBUS_CRC_H__

#include "typedef.h"

#define MB_CRC_INIT_VOL     0xFFFF

u16 mb_crc_cyc_cal(u16 init, const u8 *pdata, int len);
u16 mb_crc_cal(const u8 *pdata, int len);

#endif
