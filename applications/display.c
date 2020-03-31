
/*
 * @Description: ROV状态数据回传与控制命令接收解析，获取 系统状态(CPU、内存、硬盘、网卡网速)
 */

#define LOG_TAG "data"

#include "../drivers/oled.h"
#include "../drivers/cpu_status.h"

#include "data.h"
#include "display.h"

#include <elog.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/**
  * @brief  oled显示系统状态(IP、内存、硬盘、CPU、网速)
  */
void oled_show_status(void)
{
    char str[20];
	sprintf(str,"IP  %s", psystem->net.ip);
	OLED_ShowString(0, 0, (uint8_t *)str, 12);

	sprintf(str,"Mem: %0.1f%% of %d Mb", psystem->memory.usage_rate, psystem->memory.total / 1024);
	OLED_ShowString(0,  16, (uint8_t *)str, 12);

	sprintf(str,"Disk: %0.1f%% of %0.1f G", psystem->disk.usage_rate, (float)psystem->disk.total / 1024);
	OLED_ShowString(0,  32, (uint8_t *)str, 12);

	sprintf(str,"CPU: %0.1f%% ", psystem->cpu.usage_rate);
	OLED_ShowString(0,  48, (uint8_t *)str, 12);

    if(psystem->net.netspeed < 512) 
    {
        // 此时单位为 kbps
        sprintf(str,"%0.1f kb/s", psystem->net.netspeed);
        OLED_ShowString(70,  48, (uint8_t *)str, 12);
    }
    else 
    {
        // 转换单位为 Mbps
        sprintf(str,"%0.1f Mb/s", psystem->net.netspeed / 1024);
        OLED_ShowString(70,  48, (uint8_t *)str, 12);
    }

}

/**
  * @brief  oled显示线程
  */
void *oled_thread(void *arg)
{
    while(1)
    {
        oled_show_status();
        sleep(1);
    }
    return NULL;
}


/**
  * @brief  显示线程 初始化
  */
int display_thread_init(void)
{
    pthread_t oled_tid;

    // 先判断设备是否存在，不存在直接返回，不创建对应线程
    if(oledSetup() < 0)
        return -1;

    log_i("oled init");
    pthread_create(&oled_tid, NULL, &oled_thread, NULL);
    pthread_detach(oled_tid);

    return 0;
}
