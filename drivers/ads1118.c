
/*
 * @Description: ads1118 ADC 驱动程序
 *
 * Copyright (c) 2019-2020, Ian, <zengwangfa@outlook.com>
 */

#define LOG_TAG "ads1118"

#include "ads1118.h"

#include <elog.h>
#include <stdio.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

static ads1118_t ads1118_dev;
static ads1118_t *ads1118 = &ads1118_dev;

// 存放增益放大参数
static float vref[6] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};

/**
  * @brief  ads1118 spi数据写入与读取
  */
static int16_t ads1118_transmit(int fd, uint16_t data)
{
    unsigned char buff[4] = {0};
    
    buff[0] = (data >> 8) & 0xff; // SPI传输高位在前 (datasheet P24)
    buff[1] = (data & 0xff);
    buff[2] = 0;
    buff[3] = 0;

    printf("%x %x %x %x\n",buff[0],buff[1],buff[2],buff[3]);
    wiringPiSPIDataRW(fd, buff, 4);

    printf("%x %x %x %x\n",buff[0],buff[1],buff[2],buff[3]);
    return (buff[0] << 8) | buff[1]; // SPI传输高位在前
}


/**
  * @brief  ads1118 根据通道获取数据
  */
static int16_t ads1118_convert(int fd, int channel)
{
    ads1118->config &= ~CONFIG_MUX_MASK; // 通道寄存器位 清0
    switch (channel) {
        case 0:
            ads1118->config |= (MUX_S0 << 12); break;
        case 1:
            ads1118->config |= (MUX_S1 << 12); break;
        case 2:
            ads1118->config |= (MUX_S2 << 12); break;
        case 3:
            ads1118->config |= (MUX_S3 << 12); break;
        default:
            log_e("ads1118 channel range in [0, 3]");
    }

    return ads1118_transmit(fd, ads1118->config);
}



//------------------------------------------------------------------------------------------------------------------
//
//	用于 WiringPi functions
//
//------------------------------------------------------------------------------------------------------------------

/**
  * @brief  ads1118 根据增益转换实际电压值
  */
static int myAnalogRead(struct wiringPiNodeStruct *node, int pin)
{
    int vol     = 0;
    int fd      = node->fd;
    // 获取是 ads1118 的第几通道
    int channel = pin - node->pinBase;

    ads1118->adcVal = ads1118_convert(fd, channel);

    // 注意FS为 正负2.048，所以计算时为2.048/32768. (满量程是65535)
    // verf[PGA_2_048] 即为 FSR增益值 ±2.048
    vol = ads1118->adcVal * (vref[PGA_2_048] / 32768);

    return vol;
}


/**
 * @brief  初始化并设置 ads1118
 * @param 
 *  int pinBase  pinBase > 64
 *
 *  可修改相关配置: 增益放大、采样率...
 */
int ads1118Setup(const int pinBase)
{
    static int fd;
	struct wiringPiNodeStruct *node;
	// 小于0代表无法找到该spi接口，输入命令 sudo npi-config 使能该spi接口
    fd = wiringPiSPISetup(1, ADS1118_OSC_CLK); // ads1118使用的 /dev/spidev1.0   1MHz
    if (fd < 0)
        return -1;

    ads1118->config = 0;
    /* 设置配置寄存器 448Bh */
    ads1118->config |= (SS_NONE        << 15); // 不启动单发转换
    ads1118->config |= (MUX_S0         << 12); // 通道 AIN0
    ads1118->config |= (PGA_2_048      << 9);  // 可编程增益放大 ±2.048v
    ads1118->config |= (MODE_CONTINOUS << 8);  // 连续转换模式
    ads1118->config |= (DR_128_SPS     << 5);  // 每秒采样率 128
    ads1118->config |= (TS_MODE_ADC    << 4);  // ADC 模式
    ads1118->config |= (PULL_UP_EN_ON  << 3);  // 数据引脚上拉使能
    ads1118->config |= (NOP_DATA_VALID << 1);  // 有效数据,更新配置寄存器
    ads1118->config |= (RESERVED_BIT);         // 保留位

    // 读写数据返回0，代表未接入 ads1118设备
    if(ads1118_transmit(fd, ads1118->config) == 0)     // 写入配置寄存器
        return -2;

	// 创建节点加入链表 4 pins 共4个通道
    node = wiringPiNewNode(pinBase, 4);
    if (!node)
    {
        log_e("ads1118 node create failed");
		return -1;
    }
    node->fd         = fd;
    node->analogRead = myAnalogRead;

    return fd;
}
