/**
 * @desc: 初始化程序
 */
#define LOG_TAG "init"

#include "init.h"
#include "../applications/PID.h"
#include "../applications/data.h"
#include "../applications/display.h"
#include "../applications/ioDevices.h"
#include "../applications/pwmDevices.h"
#include "../applications/sensor.h"
#include "../applications/server.h"
#include "../applications/system.h"
#include "../tools/ano_link.h"
#include "debug.h"

#include <elog.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>

void rov_params_init(void)
{
    /* 判断config参数配置文件是否存在
	 * 1.存在，  则读取config中的参数，进行初始化
     * 2.不存在，则创建config文件，将PID参数写入
     * 
    */
    if (0 == access(ROV_CONFIG_FILE_PATH, F_OK)) // 0:exist
    {
        read_pid_params();
        log_i("read rov params");
    }
    else
    {
        Total_PID_Init();   // 初始化PID参数
        write_pid_params(); // 将初始化的参数写入文件
        log_i("create config file & write pid params");
    }
}

int system_init(void)
{
    // easylogger日志系统 初始化
    easylogger_init();

    if (wiringPiSetup() < 0)
    {
        log_e("Unable to start wiringPi: %s", strerror(errno));
        return -1;
    }
    // TODO ROV模式获取 4推 还是 6推
    log_i("Welcome to ROV Master V%s\n", ROV_MASTER_VERSION);

    rov_params_init(); // ROV参数初始化

    control_server_thread_init(); // 上位机服务器线程 初始化

    anoUdp_server_thread_init(); // ANO服务器线程 初始化

    //sensor_thread_init(); // 传感器线程 初始化

    //pwmDevs_thread_init(); // PWM设备线程 初始化

    ioDevs_thread_init(); // IO设备线程 初始化

    system_status_thread_init(); // 获取系统状态线程 初始化

    display_thread_init(); // 显示模块线程 初始化

    return 0;
}