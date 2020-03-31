

#ifndef __SENSOR_H_
#define __SENSOR_H_

#include "../user/DataType.h"

/* 深度传感器设备描述符 */
typedef struct
{
    int     pin;          // 引脚
    char    *name;        // 深度传感器名称
    float   depth;        // 实际深度
    float   pressure;     // 实时压力值s
    float   init_pressure;// 初始化压力值
    void  (*handle)(int); // 对应深度传感器处理函数 (参数为 pin)

}depthSensor_t;


int sensor_thread_init(void);
void print_sensor_info(void); // 打印传感器信息


extern Sensor_Type Sensor; //传感器参数

#endif
