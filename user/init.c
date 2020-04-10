/*
 * @Description: 初始化程序
 */
#define LOG_TAG "init"

#include "../applications/data.h"
#include "../applications/sensor.h"
#include "../applications/server.h"
#include "../applications/display.h"
#include "../applications/ioDevices.h"
#include "../applications/pwmDevices.h"

#include "init.h"

#include <elog.h>
#include <stdio.h>
#include <string.h>

#include <wiringPi.h>



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

	//server_thread_init(); // 服务器线程 初始化

	sensor_thread_init(); // 传感器线程 初始化

	//pwmDevs_thread_init(); // PWM设备线程 初始化


	//ioDevs_thread_init(); // IO设备线程 初始化

	//system_status_thread_init(); // 获取系统状态线程 初始化

	//display_thread_init(); // 显示模块线程 初始化



	return 0;
}