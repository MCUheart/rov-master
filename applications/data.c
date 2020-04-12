/*
 * @Description: ROV状态数据回传与控制命令接收解析
 */

#define LOG_TAG "data"

#include "data.h"
#include "sensor.h"
#include "server.h"

#include <elog.h>
#include <stdio.h>

/**
  * @brief  获取浮点型数据 头两位小数的100倍
  * @param  float data
  * @retval 头两位小数的100倍
  */
uint8_t get_decimal(float data)
{
    return (uint8_t)((data - (int)data) * 100);
}

/**
  * @brief  计算校验和
  * @param  数据包 *buff、数据包长度len
  * @retval 累加和 SUM
  */
uint8_t calculate_check_sum(uint8_t *buff, uint8_t len)
{
    uint8_t sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += buff[i];
    }
    return sum;
}

/**
  * @brief  上位机控制数据解析
  * @param  控制数据包 *buff
  * @notice 从第四个字节开始为控制字符
  */
void remote_control_data_analysis(uint8_t *buff, cmd_t *cmd) //控制数据解析
{
    uint8_t rxCheck = 0; //尾校验字

    if (buff[0] == 0xAA && buff[1] == 0x55) // 检测包头
    {
        if (buff[2] == RECV_DATA_LEN) // 检测数据包长度(此判断暂无作用，用于后续 可变长度数据包)
        {
            // 获取校验位
            rxCheck = calculate_check_sum(buff, RECV_DATA_LEN - 1);

            if (rxCheck == buff[RECV_DATA_LEN]) // 校验位核对
            {
                /* 开关类 */
                cmd->depth_lock = buff[3]; // 深度锁定
                cmd->sport_lock = buff[4]; // 方向锁定

                cmd->all_lock = buff[18]; // 运动控制总开关

                /* 姿态类 */
                cmd->move_back = buff[5];  // 前后
                cmd->left_right = buff[6]; // 左右平移
                cmd->up_down = buff[7];    // 垂直
                cmd->rotate = buff[8];     // 旋转

                /* 设备类 */
                cmd->power = buff[9];   // 动力控制  推进器动力系数
                cmd->light = buff[10];  // 灯光控制
                cmd->camera = buff[11]; // 变焦摄像头控制
                cmd->yuntai = buff[12]; // 云台控制
                cmd->arm = buff[13];    // 机械臂控制
            }
        }
    }
}

/* 
 * TODO 控制命令清零 【注意】这里仅清空控制数据指令，不能清除控制状态指令，因此，不能采用 meset 直接填充结构体为 0 
 */

/**
  * @brief  calculate_check_sum(计算校验和)
  * @param  数据包*buff、数据包长度len
  */
void convert_rov_status_data(uint8_t *buff) // 转换需要返回上位机数据
{
    uint16_t troll; //暂存数据
    uint16_t tpitch;
    uint16_t tyaw;
    static uint8_t speed_test;

    troll = (short)((rovInfo.jy901.roll + 180) * 100); // 数据转换:将角度数据转为正值并放大100倍
    tpitch = (short)((rovInfo.jy901.pitch + 180) * 100);
    tyaw = (short)((rovInfo.jy901.yaw + 180) * 100);

    buff[3] = (int)rovInfo.powerSource.voltage;         //整数倍
    buff[4] = get_decimal(rovInfo.powerSource.voltage); //小数的100倍

    buff[5] = (int)rovInfo.system.cpu.temperature;         //整数倍
    buff[6] = get_decimal(rovInfo.system.cpu.temperature); //小数的100倍

    buff[7] = (int)rovInfo.depthSensor.temperature;         //整数倍
    buff[8] = get_decimal(rovInfo.depthSensor.temperature); //小数的100倍

    buff[9] = (int)(rovInfo.depthSensor.depth) >> 16; //高8位
    buff[10] = (int)(rovInfo.depthSensor.depth) >> 8; //中8位
    buff[11] = (int)(rovInfo.depthSensor.depth);      //低8位

    buff[12] = tyaw >> 8;     // Yaw 高8位
    buff[13] = (uint8_t)tyaw; //低8位

    buff[14] = tpitch >> 8;     // Pitch 高8位
    buff[15] = (uint8_t)tpitch; //低8位

    buff[16] = troll >> 8;     // Roll 高8位
    buff[17] = (uint8_t)troll; //低8位

    buff[18] = (uint8_t)speed_test++; //x轴航速
    buff[19] = 0;                     //设备提示字符

    buff[20] = 0x01; // cmd->All_Lock;

    buff[21] = (int)rovInfo.powerSource.current;
    buff[22] = get_decimal(rovInfo.powerSource.current); //小数的100倍;

    buff[23] = 0x0; // 保留
    buff[24] = 0x0; // 保留

    buff[25] = calculate_check_sum(buff, RETURN_DATA_LEN - 1); //获取校验和
}
