#include "../tools/ano_link.h"
#include <arpa/inet.h>
#include <elog.h>
#include <errno.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <wiringPi.h>
#define ANO_SERVER_PORT 8000

static int sockfd;
static struct sockaddr_in serverAddr;
static struct sockaddr_in clientAddr;
static socklen_t addrLen = sizeof(clientAddr);

void ano_udp_send(uint8_t *data_to_send, uint8_t len)
{
    if ((sendto(sockfd, data_to_send, len, 0, (struct sockaddr *)&clientAddr, addrLen)) < 0)
        perror("sendto error");
}

void *ano_recv_thread(void *arg)
{
    uint8_t len;
    uint8_t buff[50];

    while (1)
    {
        len = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&clientAddr, &addrLen);
        ANO_DT_Data_Receive_Anl(buff, len); //数据解析
    }
    return NULL;
}

void *ano_send_thread(void *arg)
{
    while (1)
    {
        ANO_SEND_StateMachine();
        delay(20);
    }
    return NULL;
}

int ano_udp_server_init(void)
{
    char mesg[50];
    pthread_t send_tid;
    pthread_t recv_tid;

    // 设置端口及IP
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET; // 使用IPv4协议
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(ANO_SERVER_PORT);

    // 创建UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) // SOCK_DGRAM:数据报协议
        perror("socket error");

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)))
        perror("bind error");

    // 等待客户端发送数据，获取客户端IP clientAddr
    if (recvfrom(sockfd, mesg, sizeof(mesg), 0, (struct sockaddr *)&clientAddr, &addrLen) < 0)
        perror("recvfrom error");

    log_i("ano link [%s:%d]", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); //打印消息发送方的IP与PORT

    pthread_create(&send_tid, NULL, ano_send_thread, NULL);
    pthread_detach(send_tid);

    pthread_create(&recv_tid, NULL, ano_recv_thread, NULL);
    pthread_detach(recv_tid);
    return 0;
}