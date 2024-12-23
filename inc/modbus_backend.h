/*
 * modbus_backend.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-01     qiyongzhong       first version
 */

#ifndef __MODBUS_BACKEND_H__
#define __MODBUS_BACKEND_H__

#include "modbus_cfg.h"
#include "typedef.h"

#define MB_BKD_ACK_TMO_MS_DEF       300
#define MB_BKD_BYTE_TMO_MS_DEF      32

typedef enum{
    MB_BACKEND_TYPE_RTU = 0,//RTU后端
    MB_BACKEND_TYPE_TCP     //TCP后端
}mb_backend_type_t;//后端类型定义
    
typedef struct{
    char *dev;      //设备名
    int baudrate;   //波特率
    int parity;     //校验位
    int pin;        //收发控制引脚, <0 表示不使用
    int lvl;        //发送控制电平
}mb_backend_param_rtu_t;//RTU后端参数定义

typedef struct{
    char *host;     //ip地址或域名
    int port;       //端口
}mb_backend_param_tcp_t;//TCP后端参数定义

typedef union{
    mb_backend_param_rtu_t rtu;//RTU后端参数
    mb_backend_param_tcp_t tcp;//TCP后端参数
}mb_backend_param_t;//后端参数联合体定义

//打开, 成功返回实例指针或文件标识, 错误返回NULL
typedef void * (* mb_bkd_ops_open_t)(const mb_backend_param_t *param);
//关闭, 成功返回0, 错误返回-1
typedef int (* mb_bkd_ops_close_t)(void *hinst);
//接收数据, 返回接收到的数据长度, 0表示超时, 错误返回-1
typedef int (* mb_bkd_ops_read_t)(void *hinst, u8 *buf, int bufsize);
//发送数据, , 返回成功发送的数据长度, 错误返回-1
typedef int (* mb_bkd_ops_write_t)(void *hinst, u8 *buf, int size);
//清空接收缓存, 成功返回0, 错误返回-1
typedef int (* mb_bkd_ops_flush_t)(void *hinst);

typedef struct{
    mb_bkd_ops_open_t open;
    mb_bkd_ops_close_t close;
    mb_bkd_ops_read_t read;
    mb_bkd_ops_write_t write;
    mb_bkd_ops_flush_t flush;
}mb_backend_ops_t;

typedef struct{
    mb_backend_type_t type;//类型
    mb_backend_param_t param;//配置参数
    const mb_backend_ops_t *ops;//操作函数集
    int ack_tmo_ms;//应答超时
    int byte_tmo_ms;//字节间隔超时
    void *hinst;//实例指针或文件标识
}mb_backend_t;

mb_backend_t *mb_backend_create(mb_backend_type_t type, const mb_backend_param_t *param);//创建后端, 成功返回后端指针, 失败返回NULL
void mb_backend_destory(mb_backend_t *backend);//销毁后端
int mb_backend_open(mb_backend_t *backend);//打开后端, 成功返回0, 错误返回-1
int mb_backend_close(mb_backend_t *backend);//关闭后端, 成功返回0, 错误返回-1
int mb_backend_config(mb_backend_t *backend, int ack_tmo_ms, int byte_tmo_ms);//配置后端超时参数, 成功返回0, 错误返回-1
int mb_backend_read(mb_backend_t *backend, u8 *buf, int bufsize);//从后端读数据, 返回读取到数据长度, 0表示超时, 错误返回-1
int mb_backend_write(mb_backend_t *backend, u8 *buf, int size);//向后端写数据, 返回已发送数据长度, 错误返回-1
int mb_backend_flush(mb_backend_t *backend);//清空后端接收缓存, 成功返回0, 错误返回-1

#endif
