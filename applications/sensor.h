#ifndef __SENSOR_H_
#define __SENSOR_H_

#include "../user/config.h"

#include "../drivers/ads1118.h"
#include "../drivers/jy901.h"
#include "../drivers/ms5837.h"
#include "../drivers/spl1301.h"
#include "../drivers/sys_status.h"

/* 深度传感器设备描述符 */
typedef struct
{
    int pin;               // wiringpi node的编号
    char *name;            // 深度传感器名称
    float depth;           // 实际深度 (单位 cm)
    float temperature;     // 水温     (单位 ℃)
    int32_t pressure;      // 压力值
    int32_t last_pressure; // 上次压力值
    int32_t init_pressure; // 初始化压力值

} depthSensor_t;

/* 电源 设备描述符 */
typedef struct
{
    float percent;  // 电量百分比
    float quantity; // 电池容量
    float voltage;  // 电压(A)
    float current;  // 电流(V)

} powerSource_t;

/* 系统状态 */
typedef struct
{
    net_t net;       // 网卡
    cpu_t cpu;       // cpu
    disk_t disk;     // 硬盘
    memory_t memory; // 内存

} system_status_t;
/* --------------------------------------------------------------------------------------------------- */

/* ROV状态信息结构体 */
typedef struct
{

    uint8_t type;              // 系统状态
    char name[20];             // 航行器名称
    jy901_t jy901;             // 九轴
    powerSource_t powerSource; // 电源
    depthSensor_t depthSensor; // 深度传感器
    system_status_t system;    // 系统状态

} rovInfo_t;

// 外部调用变量
extern rovInfo_t rovInfo;

// 传感器线程初始化
int sensor_thread_init(void);

#endif
