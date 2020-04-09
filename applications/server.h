

#ifndef __SERVER_H_
#define __SERVER_H_

#include "../user/DataType.h"


#define LISTEN_PORT 8899 // 监听端口号
#define BACKLOG     10   // 最大连接数

// 服务器线程初始化
int server_thread_init(void);
// 获取对应网卡IP地址
void get_localip(const char *eth_name, char *ip);


#endif
