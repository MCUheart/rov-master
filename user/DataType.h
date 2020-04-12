/*
 * @Description: 各类数据结构定义文件
 */

#ifndef __DATA_TYPE_H_
#define __DATA_TYPE_H_

#include <stdint.h>

#define my_abs(x) (((x) > 0) ? (x) : -(x))

/* ------------------------【数据结构体定义】---------------------------------*/

typedef struct
{
    float p;
    float i;
    float d;
} Vector3f_pid;

typedef enum
{
    System_NORMAL_STATUS = 1, //正常模式
    System_DEBUG_STATUS = 2,  //调试模式
    System_ERROR_STATUS,
} VehicleStatus_Enum; //枚举系统状态

/* ------------------------【重要定义】---------------------------------*/

#define LED_EVENT (1 << 0)    //LED事件标志位
#define KEY_EVENT (1 << 1)    //KEY事件标志位
#define BUZZ_EVENT (1 << 2)   //BUZZER事件标志位
#define OLED_EVENT (1 << 3)   //OLED事件标志位
#define GYRO_EVENT (1 << 4)   //Gyroscope事件标志位
#define ADC_EVENT (1 << 5)    //ADC事件标志位
#define PWM_EVENT (1 << 6)    //PWM事件标志位
#define CAM_EVENT (1 << 7)    //Camera事件标志位
#define MS5837_EVENT (1 << 8) //Sensor事件标志位

#define PI 3.141592f                     //大写标明其为常量
#define Rad2Deg(rad) (rad * 180.0f / PI) //弧度制转角度值
#define Deg2Rad(deg) (deg * PI / 180.0f) //角度值转弧度制

/* --------------【电池 参数】-----------------*/

#define STANDARD_VOLTAGE 3.7f //锂电池标准电压
#define FULL_VOLTAGE 4.2f     //锂电池满电压

/* ----------【航行器 总推进器数量】-----------*/

#define FOUR_AXIS 0 // ROV标志
#define SIX_AXIS 1  // AUV标志

/* ---------【工作模式 工作、调试】------------*/

#define WORK 0  // 工作模式
#define DEBUG 1 // 调试模式

/* -----------【解锁、锁定 标志】--------------*/

#define UNLOCK 1 //全局解锁【启动】  宏定义
#define LOCK 2   //全局锁  【停止】

/* -----------【深度传感器类型 标志】-----------*/

#define MS5837 0  //深度传感器：MS5837
#define SPL1301 1 //深度传感器：SPL1301
#define DS_NULL 2 //无深度传感器 Depth_Sensor:null

/* ---------------【推进器 参数】--------------*/

#define PropellerPower_Med 287 // 停止 284-291
#define PropellerPower_Min 172 // 反向最小值【反向推力最大】
#define PropellerPower_Max 429 // 正向最大值

#endif
