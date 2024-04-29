/*
 * modbus.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2024-04-02     qiyongzhong       first version
 */

#include <stdio.h>
#include <stdlib.h>
#include "modbus.h"
#include "modbus_cvt.h"
#include "modbus_port.h"

#ifdef MB_USING_RAW_PRT
static void mb_raw_prt(bool is_send, const u8 *pdata, int dlen)//原始数据打印
{
    MB_PRINTF("%s", is_send ? ">>" : "<<");
    for (int i=0; i<dlen; i++)
    {
        MB_PRINTF("%02X ", pdata[i]);
    }
    MB_PRINTF("\n");
}
#endif

//创建modbus实例, 成功返回实例指针, 失败返回NULL
mb_inst_t * mb_create(mb_backend_type_t type, const mb_backend_param_t *param)
{
    MB_ASSERT(param != NULL);
    
    mb_backend_t *backend = mb_backend_create(type, param);
    if (backend == NULL)
    {
        return(NULL);
    }
    
    mb_inst_t *hinst = malloc(sizeof(mb_inst_t));
    if (hinst == NULL)
    {
        mb_backend_destory(backend);
        return(NULL);
    }


    hinst->saddr = MB_RTU_ADDR_DEF;
    hinst->prot = type;
    hinst->tid = 0;
    hinst->backend = backend;
    #ifdef MB_USING_SLAVE
    extern const mb_cb_table_t mb_cb_table;
    hinst->cb = (mb_cb_table_t *)&mb_cb_table;
    #else
    hinst->cb = NULL;
    #endif
    
    return(hinst);
}

//销毁modbus实例
void mb_destory(mb_inst_t *hinst)
{
    MB_ASSERT(hinst != NULL);
    
    if (hinst->backend != NULL)
    {
        mb_backend_destory(hinst->backend);
        hinst->backend = NULL;
    }
    
    free(hinst);
}

//修改从机地址, 默认地址为1
void mb_set_slave(mb_inst_t *hinst, u8 saddr)
{
    MB_ASSERT(hinst != NULL);

    hinst->saddr = saddr;
}
//修改协议, 默认使用与后端类型一致的协议类型
void mb_set_prot(mb_inst_t *hinst, mb_prot_t prot)
{
    MB_ASSERT(hinst != NULL);

    hinst->prot = prot;
}

//修改超时时间, 默认应答超时300ms, 字节超时32ms
void mb_set_tmo(mb_inst_t *hinst, int ack_tmo_ms, int byte_tmo_ms)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(hinst->backend != NULL);
    
    mb_backend_config(hinst->backend, ack_tmo_ms, byte_tmo_ms);
}

//建立连接, 成功返回0, 失败返回-1
int mb_connect(mb_inst_t *hinst)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(hinst->backend != NULL);
    
    return(mb_backend_open(hinst->backend));
}

//断开连接, 成功返回0, 失败返回-1
int mb_disconn(mb_inst_t *hinst)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(hinst->backend != NULL);
    
    return(mb_backend_close(hinst->backend));
}

int mb_recv(mb_inst_t *hinst, u8 *buf, int bufsize)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(hinst->backend != NULL);
    MB_ASSERT(buf != NULL);
    MB_ASSERT(bufsize > 0);
    
    int len = mb_backend_read(hinst->backend, buf, bufsize);
    if (len < 0)//发生错误, 关闭后端
    {
        mb_backend_close(hinst->backend);
    }

    #ifdef MB_USING_RAW_PRT
    if (len > 0)
    {
        mb_raw_prt(false, buf, len);
    }
    #endif
    
    return(len);
}

int mb_send(mb_inst_t *hinst, u8 *buf, int size)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(hinst->backend != NULL);
    MB_ASSERT(buf != NULL);
    MB_ASSERT(size > 0);
    
    int len = mb_backend_write(hinst->backend, buf, size);
    if (len < 0)//发生错误, 关闭后端
    {
        mb_backend_close(hinst->backend);
    }
    
    #ifdef MB_USING_RAW_PRT
    if (len > 0)
    {
        mb_raw_prt(true, buf, len);
    }
    #endif
    
    return(len);
}

//清空接收缓存, 成功返回0, 失败返回-1
int mb_flush(mb_inst_t *hinst)
{
    MB_ASSERT(hinst != NULL);
    MB_ASSERT(hinst->backend != NULL);
    
    return(mb_backend_flush(hinst->backend));
}

