/*
 * @Description: 各个传感器线程 (深度传感器、九轴、CPU 设备数据解析并获取)
 */

#define LOG_TAG "sensor"

#include "../drivers/cpu_status.h"
#include "../drivers/ads1118.h"
#include "../drivers/spl1301.h"
#include "../drivers/ms5837.h"
#include "../drivers/jy901.h"
#include "../tools/filter.h"
#include "sensor.h"

#include <elog.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#define ADS1118_PIN_BASE      350
#define DEPTH_SENSOR_PIN_BASE 360

depthSensor_t depthSensor_dev;
depthSensor_t *depthSensor = &depthSensor_dev;


Sensor_Type Sensor; //传感器参数

void *adc_thread(void *arg)
{

    // printf("adc: %d", analogRead(201));

    return NULL;
}



void *jy901_thread(void *arg)
{
    // 文件描述符由 arg传递进来
    int fd = *(int *)arg;

    while (1)
    {
        delay(10);
        while (serialDataAvail(fd))
        {
            copeJY901_data((uint8_t)serialGetchar(fd));
        }
    }
    return NULL;
}


/**
 * @brief  ms5837 处理函数
 */
void ms5837_handle(int pin)
{
    //TODO
    //printf("ms5837 handle\n ");

}

/**
 * @brief  spl1301深度传感器 处理函数
 */
void spl1301_handle(int pin)
{
    //TODO
    //printf("spl1301 handle\n ");
    //printf("P: %d, T: %d\n",digitalRead(DEPTH_SENSOR_PIN_BASE), digitalRead(DEPTH_SENSOR_PIN_BASE + 1));
    //get_spl1301_depthSensor();

}

/**
 * @brief  深度传感器处理线程
 */
void *depthSensor_thread(void *arg)
{
    while(1)
    {
        depthSensor->handle(depthSensor->pin);
        sleep(1);
    }
}




int depthSensor_init(int pin)
{
    static int fd;
    /* 依次初始化 spl1301、ms5837，根据内部数据 来判断接入的是哪种传感器 */

    if((fd = spl1301Setup(pin)) > 0)
    {
        depthSensor->pin = pin;
        depthSensor->name = "spl1301";
        depthSensor->handle = spl1301_handle;
        return fd;
    }
    else if((fd = ms5837Setup(pin)) > 0)
    {
        depthSensor->pin = pin;
        depthSensor->name = "ms5837";
        depthSensor->handle = ms5837_handle;
        return fd;
    }
    else
    {
        depthSensor->name = "No plugged a depthSensor";
        depthSensor->handle = NULL; // 防止产生野指针
        log_e("%s", depthSensor->name);
        return -1;
    }
}


int sensor_thread_init(void)
{
    static int fd;
    pthread_t adc_tid;
    pthread_t jy901_tid;
    pthread_t depthSensor_tid;


    // 先判断设备是否存在，不存在不创建对应线程
    if(ads1118Setup(ADS1118_PIN_BASE) > 0)
    {
        log_i("ads1118 init");
        pthread_create(&adc_tid, NULL, adc_thread, NULL);
        pthread_detach(adc_tid);
    }
        
    if((fd = jy901Setup()) > 0)
    {
        log_i("jy901 init");
        pthread_create(&jy901_tid, NULL, jy901_thread, &fd);
        pthread_detach(jy901_tid);
    }

    if(depthSensor_init(DEPTH_SENSOR_PIN_BASE) > 0)
    {
        log_i("%s init", depthSensor->name);
        pthread_create(&depthSensor_tid, NULL, depthSensor_thread, NULL);
        pthread_detach(depthSensor_tid);
    }


    return 0;
}

// 打印传感器信息
void print_sensor_info(void)
{
    log_d("      variable      |   value");
    log_d("--------------------|------------");

    log_d("        Roll        |  %+0.3f", Sensor.JY901.Euler.Roll);
    log_d("        Pitch       |  %+0.3f", Sensor.JY901.Euler.Pitch);
    log_d("        Yaw         |  %+0.3f", Sensor.JY901.Euler.Yaw);
    log_d("--------------------|------------");
    log_d("        Acc.x       |  %+0.3f", Sensor.JY901.Acc.x);
    log_d("        Acc.y       |  %+0.3f", Sensor.JY901.Acc.y);
    log_d("        Acc.z       |  %+0.3f", Sensor.JY901.Acc.z);
    log_d("--------------------|------------");
    log_d("       Gyro.x       |  %+0.3f", Sensor.JY901.Gyro.x);
    log_d("       Gyro.y       |  %+0.3f", Sensor.JY901.Gyro.y);
    log_d("       Gyro.z       |  %+0.3f", Sensor.JY901.Gyro.z);
    log_d("  JY901_Temperature |  %+0.3f", Sensor.JY901.Temperature);
    log_d("--------------------|------------");
    log_d("       Voltage      |  %0.3f",  Sensor.PowerSource.Voltage); // 电压
    log_d("       Current      |  %0.3f",  Sensor.PowerSource.Current); // 电流
    log_d("--------------------|------------");
    log_d(" Depth Sensor Type  |  %s",     "nop"); // 深度传感器类型
    log_d(" Water Temperature  |  %0.3f",  Sensor.DepthSensor.Temperature);          // 水温
    log_d("sensor_Init_Pressure|  %0.3f",  Sensor.DepthSensor.Init_PessureValue);    // 深度传感器初始压力值
    log_d("   sensor_Pressure  |  %0.3f",  Sensor.DepthSensor.PessureValue);         // 深度传感器当前压力值
    log_d("        Depth       |  %0.3f",  Sensor.DepthSensor.Depth);                // 深度值
    log_d("--------------------|------------");
    log_d("   CPU.Temperature  |  %0.3f",  Sensor.CPU.Temperature); // CPU温度
    log_d("      CPU.Usages    |  %0.3f",  Sensor.CPU.Usage);       // CPU使用率
}
