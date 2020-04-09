/*
 * @Description: 简单PWM设备线程(推进器、云台、机械臂) 
 *               推进器初始化，推进器PWM线程
 */

#define LOG_TAG "pwm"

#include "../drivers/pca9685.h"

#include "data.h"
#include "pwmDevices.h"

#include <elog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>

static propellerPower_t propellerPower = {0}; // 推进器推力控制器

static propellerParam_t propellerParam = {
	.pMax     = 1750, // 正向最大值 单位us
	.med      = 1500, // 中值
	.nMax     = 1250, // 反向最大值
	.deadband = 1,    // 死区值
};

static easyPWM_dev_t light = { // 探照灯
    .pMax    = 20*1000, // 单位us
    .nMax    = 0,
    .speed   = 1000, 
    .channel = YUNTAI_CHANNEL,
};    

static easyPWM_dev_t yuntai = { // 云台 
    .pMax    = 2000, 
    .med     = 1500,
    .nMax    = 1000,
    .speed   = 50, 
    .channel = LIGHT_CHANNEL,
};   

static easyPWM_dev_t robot_arm = { // 机械臂
    .pMax    = 2000,
    .med     = 1500,
    .nMax    = 1000,
    .speed   = 50, 
    .channel = ROBOT_ARM_CHANNEL,
};


/**
 * @brief  计算 PWM 高电平所占的周期
 * @param  高电平持续的时间，单位us
 * eg. duty = 1500us = 1.5ms
 *  高电平所占的周期 = 1.5 / 20 * 4096
 */
int calcTicks(int16_t duty)
{
    float impulseMs;
	static float cycleMs = 1000.0f / HERTZ; // 总周期20ms
    impulseMs = duty / 1000.0f; // 单位转换为ms

	return (int)(impulseMs / cycleMs * MAX_PWM + 0.5f);
}

void propellerPwm_output_limit(int16_t *val)
{
    int i;
    for(i = 0;i < 6; i++)
    {
        if(val[i] > propellerParam.pMax) // 正向限幅
            val[i] = propellerParam.pMax;

        if(val[i] < propellerParam.nMax) // 反向限幅
            val[i] = propellerParam.nMax;    
    }
}

void easyPwm_output_limit(easyPWM_dev_t *easyPWM)
{
    if(easyPWM->cur > easyPWM->pMax) // 正向限幅
        easyPWM->cur = easyPWM->pMax;

    if(easyPWM->cur < easyPWM->nMax) // 反向限幅
        easyPWM->cur = easyPWM->nMax;    
}

/**
 * @brief  推进器初始化
 * @notice 
 *  初始化流程：
 *  1.接线,上电，哔-哔-哔三声,表示开机正常
 *  2.给电调2ms或1ms最高转速信号,哔一声
 *  3.给电调1.5ms停转信号,哔一声
 *  4.初始化完成，可以开始控制
 */
void propeller_init(void) //这边都需要经过限幅在给定  原先为2000->1500
{
    int i;
	// 给定最高转速信号
    for (i = 0; i < 6; i++)
	    pwmWrite(PCA9685_PIN_BASE + i, PROPELLER_POWER_P_MAX); 

	sleep(2); // 2s

	// 给定停转信号
    for (i = 0; i < 6; i++)
	    pwmWrite(PCA9685_PIN_BASE + i, PROPELLER_POWER_STOP); 

	sleep(1); // 1s
}

/**
 * @brief  推进器 PWM 更新
 * @param  propellerPower_t 推进器动力结构体
 */
void propellerPwm_update(propellerPower_t *propeller)
{
    int16_t power[6];

	power[0] = PROPELLER_POWER_STOP + propeller->leftUp;     // 水平推进器
	power[1] = PROPELLER_POWER_STOP + propeller->leftDown; 
	power[2] = PROPELLER_POWER_STOP + propeller->rightUp;
	power[3] = PROPELLER_POWER_STOP + propeller->rightDown;

	power[4] = PROPELLER_POWER_STOP + propeller->leftMiddle; // 垂直推进器
	power[5] = PROPELLER_POWER_STOP + propeller->rightMiddle;

    // PWM限幅
    propellerPwm_output_limit(power);

    for (int i = 0; i < 6; i++)
    {   
	    pwmWrite(PCA9685_PIN_BASE + i, calcTicks(power[i])); 
    }
}


/**
 * @brief  简单 PWM 设备处理函数
 * @param  *easyPWM 简单PWM设备描述符，*action控制指令
 */
void easyPWM_devices_handle(easyPWM_dev_t *easyPWM, uint8_t *action)
{
    switch (*action)
    {
        case 0x01:
            easyPWM->cur -= easyPWM->speed; // 正向
            break;
        case 0x02:
            easyPWM->cur += easyPWM->speed; // 反向
            break;
        case 0x03:
            easyPWM->cur = easyPWM->med; // 归中
            break; 
        default:
            break;
    }
    *action = 0x00; // 清除控制字

    easyPwm_output_limit(easyPWM); // 限幅

    pwmWrite(PCA9685_PIN_BASE + easyPWM->channel, calcTicks(easyPWM->cur));
}


/*******************************************************************************************************************/
//
// 线程
//
/*******************************************************************************************************************/

// TODO 收到 server 数据后，再唤醒pwm线程，pwm线程合并
void *propeller_thread(void *arg)
{
    uint8_t action;
    
    propeller_init();
	while (1)
	{
        sleep(1);
        propellerPwm_update(&propellerPower);

        easyPWM_devices_handle(&yuntai   , &action);
        easyPWM_devices_handle(&light    , &action);
        easyPWM_devices_handle(&robot_arm, &action);
    }
}



int pwmDevs_thread_init(void)
{
    int fd;
    pthread_t propeller_tid;

    fd = pca9685Setup(PCA9685_PIN_BASE, HERTZ);
    // 判断对应 i2c接口、pca9685器件 是否存在，不存在直接返回，不创建对应线程
    if(fd < 0)
    {
        // 错误日志打印
        ERROR_LOG(fd, "pca9685");
        return -1;
    }

    log_i("pca9685 init");
    pthread_create(&propeller_tid, NULL, &propeller_thread, NULL);
    pthread_detach(propeller_tid);

    return 0;
}