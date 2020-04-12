/*
 * @Description: 获取 系统状态(CPU、内存、硬盘、网卡网速)
 */

#define LOG_TAG "data"

#include "data.h"
#include "sensor.h"
#include "server.h"

#include <elog.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static system_status_t *psystem = &rovInfo.system;

/*******************************************************************************************************************/
//
// 线程
//
/*******************************************************************************************************************/

/**
 * @brief  获取CPU状态 线程
 *  get_cpu_usage函数内已经休眠1s，因此不再设置休眠  
 */
void *cpu_status_thread(void *arg)
{
    while (1)
    {
        psystem->cpu.temperature = get_cpu_temp();
        psystem->cpu.usage_rate = get_cpu_usage();
    }
}

/**
 * @brief  获取网速 线程
 *  get_net_speed函数内已经休眠1s，因此不再设置休眠  
 */
void *net_speed_thread(void *arg)
{
    psystem->net.name = "eth0"; // 指定 eht0 网卡
    // 获取ip地址
    get_localip(psystem->net.name, psystem->net.ip);
    while (1)
    {
        psystem->net.netspeed = get_net_speed(psystem->net.name);
    }
}

/**
 * @brief  获取内存、硬盘状态 线程
 */
void *mem_disk_status_thread(void *arg)
{
    while (1)
    {
        // 获取内存使用情况
        get_memory_status(&psystem->memory);
        // 获取硬盘使用情况
        get_disk_status(&psystem->disk);
        // 1s更新一次
        sleep(1);
    }
}

/**
  * @brief  系统状态获取 线程初始化
  *  CPU、网卡网速 状态都是通过休眠一段时间，两次数据对比进行测算的，因此都需要各开一个线程
  *  内存、硬盘 状态共用1个即可
  */

int system_status_thread_init(void)
{
    pthread_t cpu_tid;
    pthread_t net_tid;
    pthread_t mem_disk_tid;

    pthread_create(&cpu_tid, NULL, cpu_status_thread, NULL);
    pthread_detach(cpu_tid);

    pthread_create(&net_tid, NULL, net_speed_thread, NULL);
    pthread_detach(net_tid);

    // 内存、硬盘 状态获取共用1个线程
    pthread_create(&mem_disk_tid, NULL, mem_disk_status_thread, NULL);
    pthread_detach(mem_disk_tid);

    return 0;
}
