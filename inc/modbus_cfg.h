/*
 * modbus_cfg.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#ifndef __MODBUS_CFG_H__
#define __MODBUS_CFG_H__

//#define MB_USING_RAW_PRT        //使用原始通信数据打印

//#define MB_USING_ADDR_CHK       //使用从机地址检查
//#define MB_USING_MBAP_CHK       //使用MBAP头检查

//#define MB_USING_PORT_RTT       //使用rt-thread系统接口
//#define MB_USING_PORT_LINUX     //使用linux系统接口
#if (defined(MB_USING_PORT_RTT) && defined(MB_USING_PORT_LINUX))
#error Only one of MB_USING_PORT_RTT and MB_USING_PORT_LINUX can be defined!
#endif

//#define MB_USING_RTU_BACKEND    //使用RTU后端
//#define MB_USING_TCP_BACKEND    //使用TCP后端
#if (!defined(MB_USING_RTU_BACKEND) && !defined(MB_USING_TCP_BACKEND))
#error MB_USING_RTU_BACKEND or MB_USING_TCP_BACKEND must being defined!
#endif

//#define MB_USING_RTU_PROTOCOL   //使用RTU协议
//#define MB_USING_TCP_PROTOCOL   //使用TCP协议
#if (!defined(MB_USING_RTU_PROTOCOL) && !defined(MB_USING_TCP_PROTOCOL))
#error MB_USING_RTU_PROTOCOL or MB_USING_TCP_PROTOCOL must being defined!
#endif

//#define MB_USING_MASTER         //使用主机功能
//#define MB_USING_SLAVE          //使用从机功能
#if (!defined(MB_USING_MASTER) && !defined(MB_USING_SLAVE))
#error MB_USING_MASTER or MB_USING_SLAVE must being defined!
#endif

//#define MB_USING_SAMPLE         //使用示例
#ifdef MB_USING_SAMPLE
//#define MB_USING_SAMPLE_RTU_MASTER  //使用基于RTU后端的主机示例
//#define MB_USING_SAMPLE_RTU_SLAVE   //使用基于RTU后端的从机示例
//#define MB_USING_SAMPLE_TCP_MASTER  //使用基于TCP后端的主机示例
//#define MB_USING_SAMPLE_TCP_SLAVE   //使用基于TCP后端的从机示例
#endif

#endif
