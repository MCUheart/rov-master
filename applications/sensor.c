/*
 * @Description: 各个传感器线程 (深度传感器、九轴、CPU 设备数据解析并获取)
 */

#define LOG_TAG "sensor"

#include "../drivers/sys_status.h"
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

#define ADS1118_PIN_BASE      500 
#define DEPTH_SENSOR_PIN_BASE 360

adc_t adc_dev;
adc_t *adc = &adc_dev;

depthSensor_t depthSensor_dev;
depthSensor_t *depthSensor = &depthSensor_dev;



Sensor_Type Sensor; //传感器参数

void *adc_thread(void *arg)
{
    // TODO 待测试
    // 存放增益放大参数
    static float vref[6] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};
   
    while(1)
    {
        sleep(1);
        
        // 注意FS为 正负2.048，所以计算时为2.048/32768. (满量程是65535)
        // verf[PGA_2_048] 即为 FSR增益值 ±2.048

        adc->voltage = (float)analogRead(ADS1118_PIN_BASE) * (vref[2] / 32768);
        adc->current = (float)analogRead(ADS1118_PIN_BASE + 1) * (vref[2] / 32768);

        //printf("voltage %f  current %f\n", adc->voltage, adc->current);
    }


    return NULL;
}



void *jy901_thread(void *arg)
{
    // 文件描述符由 创建线程参数 arg
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
 * @brief  ms5837深度传感器 处理函数
 */
void ms5837_handle(int pin)
{
    // TODO 待测试
    depthSensor->depth = 
    (depthSensor->pressure - depthSensor->init_pressure) / 1.37f;

    //printf("ms5837 depth: %.2f\n", depthSensor->depth);
    //printf("init_pressure: %.2f\n", depthSensor->init_pressure);
    //printf("pressure: %.2f\n", depthSensor->pressure);
}

/**
 * @brief  spl1301深度传感器 处理函数
 */
void spl1301_handle(int pin)
{
  
    depthSensor->depth = (depthSensor->pressure - depthSensor->init_pressure) / 93.0f;


    //printf("spl1301 depth: %.2f\n", depthSensor->depth);
    //printf("init_pressure: %.2f\n", depthSensor->init_pressure);
    //printf("pressure: %.2f\n", depthSensor->pressure);

}

/**
 * @brief  深度传感器处理线程
 */
void *depthSensor_thread(void *arg)
{
    // 获取初始压力值
    depthSensor->init_pressure = digitalRead(depthSensor->pin);
    while(1)
    {
        // TODO 如果压力值 由很大变为较小，判定为由水中拿至在岸上， init_pressure值重新获取，更新

        // 获取压力值 通道0
        depthSensor->pressure    = digitalRead(depthSensor->pin);
        // 获取温度值 通道1
        depthSensor->temperature = digitalRead(depthSensor->pin + 1);

        /* (预留，此写法过于冗长，后期可删除) 对应深度传感器数据处理函数 */
        depthSensor->handle(depthSensor->pin);

        sleep(1);
    }
}


int depthSensor_init(int pin)
{
    static int fd;
    // 对应pin
    depthSensor->pin = pin;
    /* 依次初始化 spl1301、ms5837，根据内部数据 来判断接入的是哪种传感器 */
    /* TODO 后期是否可以根据 i2c地址来确定是否接入 */
    if((fd = spl1301Setup(pin)) > 0)
    {
        depthSensor->name = "spl1301";
        depthSensor->handle = spl1301_handle;
        return fd;
    }
    else if((fd = ms5837Setup(pin)) > 0)
    {
        depthSensor->name = "ms5837";
        depthSensor->handle = ms5837_handle;
        return fd;
    }
    else 
    {
        depthSensor->name = "null";
        depthSensor->handle = NULL; // 防止产生野指针
        return -2;
    }
    
}


int sensor_thread_init(void)
{
    static int fd;

    pthread_t adc_tid;
    pthread_t jy901_tid;
    pthread_t depth_tid;


    // ADS1118 ADC 初始化
    fd = ads1118Setup(ADS1118_PIN_BASE); 
    if(fd < 0) // 先判断设备是否存在，不存在不创建对应线程
        ERROR_LOG(fd, "ads1118");
    else
    {
        log_i("ads1118 init");
        pthread_create(&adc_tid, NULL, adc_thread, NULL);
        pthread_detach(adc_tid);     
    }

    // JY901 九轴 初始化
    fd = jy901Setup();
    if(fd < 0)
        ERROR_LOG(fd, "jy901");
    else
    {
        log_i("jy901   init");
        pthread_create(&jy901_tid, NULL, jy901_thread, &fd);
        pthread_detach(jy901_tid);
    }

    // 深度传感器 初始化
    fd = depthSensor_init(DEPTH_SENSOR_PIN_BASE);
    if(fd < 0)
        ERROR_LOG(fd ,"depth sersor");
    else
    {
        log_i("%s init", depthSensor->name);
        pthread_create(&depth_tid, NULL, depthSensor_thread, NULL);
        pthread_detach(depth_tid);
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
