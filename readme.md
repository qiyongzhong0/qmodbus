# QModbus组件包


## 1. 简介

**QModbus** 是一款快捷易用的modbus通信协议栈，支持MODBUS-RTU、MODBUS-TCP通信协议，支持使用RTU、TCP通信链路，支持主机、从机功能，支持多实例应用。采用通信链路与通信协议分离的设计架构，用户可灵活地交叉搭配使用。。


### 1.1 目录结构

`QModbus` 软件包目录结构如下所示：

``` 
modbus
├───inc                         // 头文件目录
│   |   modbus.h                // API接口头文件
│   |   modbus_backend.h        // 后端模块头文件
│   |   modbus_cfg.h            // 配置头文件
│   |   modbus_crc.h            // CRC校验模块头文件
│   |   modbus_cvt.h            // 数据转换模块头文件
│   |   modbus_pdu.h            // PDU模块头文件
│   |   modbus_port.h           // 移植接口头文件
│   |   modbus_rtu.h            // RTU通信协议模块头文件
│   |   modbus_tcp.h            // TCP通信协议模块头文件
│   └───typedef.h               // 数据类型定义头文件
├───src                         // 源码目录
│   |   modbus.c                // 主模块
│   |   modbus_backend.c        // 后端模块
│   |   modbus_crc.c            // CRC校验模块
│   |   modbus_cvt.c            // 数据转换模块
│   |   modbus_master.c         // 主机功能模块
│   |   modbus_pdu.c            // PDU模块
│   |   modbus_port_linux.c     // linux移植接口模块
│   |   modbus_port_rtt.c       // RT-Thread移植接口模块
│   |   modbus_port_slave.c     // 从机移植接口模块
│   |   modbus_rtu.c            // RTU通信协议模块
│   |   modbus_slave.c          // 从机功能模块
│   └───modbus_tcp.c            // TCP通信协议模块
├───sample                      // 使用示例目录
│   |   mb_sample_rtu_master.c  // RTU主机使用示例
│   |   mb_sample_rtu_slave.c   // RTU从机使用示例
│   |   mb_sample_tcp_master.c  // TCP主机使用示例
│   └───mb_sample_rtu_slave.c   // TCP从机使用示例
│   license                     // 软件包许可证
│   readme.md                   // 软件包使用说明
└───SConscript                  // RT-Thread 默认的构建脚本
```


### 1.2 许可证

QModbus package 遵循 LGPLv2.1 许可，详见 `LICENSE` 文件。


### 1.3 依赖

- RT_Thread 4.0
- serial
- pin
- SAL


## 2. 使用


### 2.1 常用API函数说明

#### mb_inst_t * mb_create(mb_backend_type_t type, const mb_backend_param_t *param);
- 功能 ：动态创建modbus应用实例
- 参数 ：type--通信链路后端类型，MB_BACKEND_TYPE_RTU 或 MB_BACKEND_TYPE_TCP 
- 参数 ：param--后端参数指针，当type为MB_BACKEND_TYPE_RTU时须填写后端参数的rtu域，当type为MB_BACKEND_TYPE_TCP时须填写后端参数的tcp域
- 返回 ：成功返回实例指针，失败返回NULL

#### void mb_destory(mb_inst_t *hinst);
- 功能 ：销毁modbus应用实例
- 参数 ：hinst--modbus应用实例指针
- 返回 ：无

#### void mb_set_slave(mb_inst_t *hinst, u8 saddr);
- 功能 ：修改从机地址，默认地址为1
- 参数 ：hinst--modbus应用实例指针
- 参数 ：saddr--从机地址
- 返回 ：无

#### void mb_set_prot(mb_inst_t *hinst, mb_prot_t prot);
- 功能 ：修改通信协议，默认使用与通信链路后端类型一致的通信协议类型
- 参数 ：hinst--modbus应用实例指针
- 参数 ：prot--通信协议类型，MB_PROT_RTU 或 MB_PROT_TCP
- 返回 ：无

#### void mb_set_tmo(mb_inst_t *hinst, int ack_tmo_ms, int byte_tmo_ms);
- 功能 ：修改超时时间，默认应答超时300ms，字节超时32ms
- 参数 ：hinst--modbus应用实例指针
- 参数 ：ack_tmo_ms--应答超时时间，单位ms
- 参数 ：byte_tmo_ms--字节超时时间，单位ms
- 返回 ：无

#### int mb_read_req(mb_inst_t *hinst, u8 func, u16 addr, int nb, u8 *pdata);
- 功能 ：主机功能函数，读请求，功能码和数据由用户确定，本函数可用于功能扩展使用。
- 参数 ：hinst--modbus应用实例指针
- 参数 ：func--功能码
- 参数 ：addr--寄存器地址
- 参数 ：nb--读取寄存器数量
- 参数 ：pdata--保存应答数据的缓冲区指针
- 返回 ：>0 为读取到的数据长度；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_write_req(mb_inst_t *hinst, u8 func, u16 addr, int nb, const u8 *pdata, int dlen);
- 功能 ：主机功能函数，写请求，功能码和数据由用户确定, 本函数可用于功能扩展使用。
- 参数 ：hinst--modbus应用实例指针
- 参数 ：func--功能码
- 参数 ：addr--寄存器地址
- 参数 ：nb--写入寄存器数量
- 参数 ：pdata--写入数据的缓冲区指针
- 参数 ：dlen--写入数据的长度
- 返回 ：>0 为写入的数据长度；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_read_bits(mb_inst_t *hinst, u16 addr, int nb, u8 *pbits);
- 功能 ：主机功能函数，读取多个线圈，功能码-0x01
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：nb--读取线圈数量
- 参数 ：pbits--保存应答数据的缓冲区指针
- 返回 ：>0 为读取到的线圈数量；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_read_input_bits(mb_inst_t *hinst, u16 addr, int nb, u8 *pbits);
- 功能 ：主机功能函数，读取多个离散量输入, 功能码-0x02
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：nb--读取离散量数量
- 参数 ：pbits--保存应答数据的缓冲区指针
- 返回 ：>0 为读取到的离散量数量；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_read_regs(mb_inst_t *hinst, u16 addr, int nb, u16 *pregs);
- 功能 ：主机功能函数，读取多个保持寄存器, 功能码-0x03
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：nb--读取寄存器数量
- 参数 ：pregs--保存应答寄存器数据的缓冲区指针
- 返回 ：>0 为读取到寄存器数量；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_read_input_regs(mb_inst_t *hinst, u16 addr, int nb, u16 *pregs);
- 功能 ：主机功能函数，读取多个输入寄存器, 功能码-0x04
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：nb--读取寄存器数量
- 参数 ：pregs--保存应答寄存器数据的缓冲区指针
- 返回 ：>0 为读取到寄存器数量；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_write_bit(mb_inst_t *hinst, u16 addr, u8 bit);
- 功能 ：主机功能函数，写入单个线圈, 功能码-0x05
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：bit--写入的线圈值
- 返回 ：=1 写入成功；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_write_reg(mb_inst_t *hinst, u16 addr, u16 val);
- 功能 ：主机功能函数，写入单个保持寄存器, 功能码-0x06
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：val--写入的寄存器值
- 返回 ：=1 写入成功；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_write_bits(mb_inst_t *hinst, u16 addr, int nb, const u8 *pbits);
- 功能 ：主机功能函数，写入多个线圈, 功能码-0x0F
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：nb--写入线圈数量
- 参数 ：pbits--写入线圈数据的缓冲区指针
- 返回 ：>0 为写入线圈数量；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_write_regs(mb_inst_t *hinst, u16 addr, int nb, const u16 *pregs);
- 功能 ：主机功能函数，写入多个保持寄存器, 功能码-0x10
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：nb--写入寄存器数量
- 参数 ：pregs--写入寄存器数据的缓冲区指针
- 返回 ：>0 为写入寄存器数量；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_mask_write_reg(mb_inst_t *hinst, u16 addr, u16 mask_and, u16 mask_or);
- 功能 ：主机功能函数，屏蔽写保持寄存器, 功能码-0x16
- 参数 ：hinst--modbus应用实例指针
- 参数 ：addr--寄存器地址
- 参数 ：mask_and--屏蔽与值
- 参数 ：mask_or--屏蔽或值
- 返回 ：=1 写入成功；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### int mb_write_and_read_regs(mb_inst_t *hinst, u16 wr_addr, int wr_nb, const u16 *p_wr_regs, u16 rd_addr, int rd_nb, u16 *p_rd_regs);
- 功能 ：主机功能函数，读写入多个保持寄存器, 功能码-0x17
- 参数 ：hinst--modbus应用实例指针
- 参数 ：wr_addr--写入寄存器地址
- 参数 ：wr_nb--写入寄存器数量
- 参数 ：p_wr_regs--写入寄存器数据的缓冲区指针
- 参数 ：rd_addr--读取寄存器地址
- 参数 ：rd_nb--读取寄存器数量
- 参数 ：p_rd_regs--保存读取应答寄存器数据的缓冲区指针
- 返回 ：>0 为读取到寄存器数量；=0 表示发生错误；<0 表示从站异常应答，返回值为异常码的负值

#### void mb_set_cb_table(mb_inst_t *hinst, const mb_cb_table_t *cb);
- 功能 ：从机功能函数，修改从机回调函数表，默认使用modbus_port中的接口函数做为回调函数
- 参数 ：hinst--modbus应用实例指针
- 参数 ：cb--回调函数表指针
- 返回 ：无

#### void mb_slave_fsm(mb_inst_t *hinst);
- 功能 ：从机功能函数，从机状态机处理，在线程中循环调用即可
- 参数 ：hinst--modbus应用实例指针
- 返回 ：无


### 2.2 API函数使用说明

#### 2.2.1 MODBUS主机应用（可参考示例代码 mb_sample_rtu_master.c 或 mb_sample_tcp_master.c）
1. 定义并初始化后端参数 `const mb_backend_param_t bkd` ；
2. 使用已定义的后端参数，调用 `mb_create` 函数创建应用实例；
3. 如默认参数不满足需求，则修改从机地址、通信协议类型、超时时间等参数。
4. 调用`mb_read_bits`、`mb_read_input_bits`、`mb_read_regs`...等功能函数完成数据读写。

#### 2.2.2 MODBUS从机应用（可参考示例代码 mb_sample_rtu_slave.c 或 mb_sample_tcp_slave.c）
1. 定义和实现使用到的从机回调函数。
2. 定义并初始化后端参数 `const mb_backend_param_t bkd` ；
3. 使用已定义的后端参数，调用 `mb_create` 函数创建应用实例；
4. 如默认参数不满足需求，则修改从机地址、通信协议类型、超时时间等参数。
5. 如自定义了从机回调函数表，则调用函数 `mb_set_cb_table` 修改从机回调函数表。
6. 在线程中循环调用从机状态机函数 `mb_slave_fsm`。


### 2.3 获取组件

- **方式1：**
通过 *Env配置工具* 或 *RT-Thread studio* 开启软件包，根据需要配置各项参数；配置路径为 *RT-Thread online packages -> IoT - internet of things -> qmodbus* 


### 2.4 配置参数说明

| 参数宏 | 说明 |
| ---- | ---- |
| MB_USING_RAW_PRT		    | 使用原始通信数据打印
| MB_USING_ADDR_CHK         | 使用从机地址检查
| MB_USING_MBAP_CHK	        | 使用MBAP头检查
| MB_USING_PORT_RTT         | 使用rt-thread系统接口
| MB_USING_PORT_LINUX       | 使用linux系统接口
| MB_USING_RTU_BACKEND      | 使用RTU通信链路后端
| MB_USING_TCP_BACKEND      | 使用TCP通信链路后端
| MB_USING_RTU_PROTOCOL     | 使用MODBUS-RTU通信协议
| MB_USING_TCP_PROTOCOL     | 使用MODBUS-TCP通信协议
| MB_USING_MASTER           | 使用主机功能
| MB_USING_SLAVE            | 使用从机功能
| MB_USING_SAMPLE_RTU_MASTER| 使用基于RTU通信链路后端的主机示例
| MB_USING_SAMPLE_RTU_SLAVE | 使用基于RTU通信链路后端的从机示例
| MB_USING_SAMPLE_TCP_MASTER| 使用基于TCP通信链路后端的主机示例
| MB_USING_SAMPLE_TCP_SLAVE | 使用基于TCP通信链路后端的从机示例

> #### 配置特别说明：
> ##### - MB_USING_PORT_RTT 与 MB_USING_PORT_LINUX 仅可使能其一。
> ##### - MB_USING_RTU_BACKEND 与 MB_USING_TCP_BACKEND 至少使能其一。
> ##### - MB_USING_RTU_PROTOCOL 与 MB_USING_TCP_PROTOCOL 至少使能其一。
> ##### - MB_USING_MASTER 与 MB_USING_SLAVE 至少使能其一。

## 3. 联系方式

* 维护：qiyongzhong
* 主页：https://github.com/qiyongzhong0/qmodbus
* 主页：https://gitee.com/qiyongzhong0/qmodbus
* 邮箱：917768104@qq.com
