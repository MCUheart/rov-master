/*
 * @Description: SPL1301 ��ȴ�������������
 */

#define LOG_TAG "spl1301"

#include "spl1301.h"

#include <elog.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>


static spl1301_t spl1301_dev;
static spl1301_t *spl1301 = &spl1301_dev;

/**
 * @brief  ��ȡ SPL1301 ����ID��汾ID
 * @retval ����ID
 */
int spl1301_get_id(int fd)
{
    int8_t reg;
    reg = wiringPiI2CReadReg8(fd, PRODUCT_REVISION_ID);

    // ���޷���ȡ���ݣ��ж�Ϊ ����Ĳ���SPL1301����δ����
    if(reg < 0)
        return -1;

    spl1301->revision_id = reg & 0x0F;
    spl1301->product_id  = (reg & 0xF0) >> 4;

    return spl1301->product_id;
}

/**
 * @brief ���� SPL1301 ѹ�����¶� �Ĳ����ʺ͹����� 
 * @param 
 *  uint8_t iSensor     0: Pressure;      1: Temperature 
 *  uint8_t u8SmplRate  sample rate(Hz)   Maximal = 128
 *  uint8_t u8OverSmpl  oversample rate   Maximal = 128
 */
void spl1301_rateset(int fd, uint8_t iSensor, uint8_t u8SmplRate, uint8_t u8OverSmpl)
{
    uint8_t reg = 0;
    int32_t i32kPkT = 0;
    switch (u8SmplRate)
    {
    case 2:
        reg |= (1 << 4);
        break;
    case 4:
        reg |= (2 << 4);
        break;
    case 8:
        reg |= (3 << 4);
        break;
    case 16:
        reg |= (4 << 4);
        break;
    case 32:
        reg |= (5 << 4);
        break;
    case 64:
        reg |= (6 << 4);
        break;
    case 128:
        reg |= (7 << 4);
        break;
    case 1:
    default:
        break;
    }
    switch (u8OverSmpl)
    {
    case 2:
        reg |= 1;
        i32kPkT = 1572864;
        break;
    case 4:
        reg |= 2;
        i32kPkT = 3670016;
        break;
    case 8:
        reg |= 3;
        i32kPkT = 7864320;
        break;
    case 16:
        i32kPkT = 253952;
        reg |= 4;
        break;
    case 32:
        i32kPkT = 516096;
        reg |= 5;
        break;
    case 64:
        i32kPkT = 1040384;
        reg |= 6;
        break;
    case 128:
        i32kPkT = 2088960;
        reg |= 7;
        break;
    case 1:
    default:
        i32kPkT = 524288;
        break;
    }

    if (iSensor == PRESSURE_SENSOR)
    {
        spl1301->i32kP = i32kPkT;
        wiringPiI2CWriteReg8(fd, 0x06, reg);
        if (u8OverSmpl > 8)
        {
            reg = wiringPiI2CReadReg8(fd, 0x09);
            wiringPiI2CWriteReg8(fd, 0x09, reg | 0x04);
        }
        else
        {
            reg = wiringPiI2CReadReg8(fd, 0x09);
            wiringPiI2CWriteReg8(fd, 0x09, reg & (~0x04));
        }
    }
    if (iSensor == TEMPERATURE_SENSOR)
    {
        spl1301->i32kT = i32kPkT;
        wiringPiI2CWriteReg8(fd, 0x07, reg | 0x80); //Using mems temperature
        if (u8OverSmpl > 8)
        {
            reg = wiringPiI2CReadReg8(fd, 0x09);
            wiringPiI2CWriteReg8(fd, 0x09, reg | 0x08);
        }
        else
        {
            reg = wiringPiI2CReadReg8(fd, 0x09);
            wiringPiI2CWriteReg8(fd, 0x09, reg & (~0x08));
        }
    }
}

/**
 * @brief ��ȡ spl1301 �ڲ�����У׼���ݣ����ں���������ת��
 */
void spl1301_get_calib_param(int fd)
{
    uint32_t h;
    uint32_t m;
    uint32_t l;

    h = wiringPiI2CReadReg8(fd, 0x10);
    l = wiringPiI2CReadReg8(fd, 0x11);
    spl1301->calib_param.c0 = (int16_t)h << 4 | l >> 4;
    spl1301->calib_param.c0 = (spl1301->calib_param.c0 & 0x0800) ? (0xF000 | spl1301->calib_param.c0) : spl1301->calib_param.c0;
    h = wiringPiI2CReadReg8(fd, 0x11);
    l = wiringPiI2CReadReg8(fd, 0x12);
    spl1301->calib_param.c1 = (int16_t)(h & 0x0F) << 8 | l;
    spl1301->calib_param.c1 = (spl1301->calib_param.c1 & 0x0800) ? (0xF000 | spl1301->calib_param.c1) : spl1301->calib_param.c1;
    h = wiringPiI2CReadReg8(fd, 0x13);
    m = wiringPiI2CReadReg8(fd, 0x14);
    l = wiringPiI2CReadReg8(fd, 0x15);
    spl1301->calib_param.c00 = (int32_t)h << 12 | (int32_t)m << 4 | (int32_t)l >> 4;
    spl1301->calib_param.c00 = (spl1301->calib_param.c00 & 0x080000) ? (0xFFF00000 | spl1301->calib_param.c00) : spl1301->calib_param.c00;
    h = wiringPiI2CReadReg8(fd, 0x15);
    m = wiringPiI2CReadReg8(fd, 0x16);
    l = wiringPiI2CReadReg8(fd, 0x17);
    spl1301->calib_param.c10 = (int32_t)(h & 0x0F) << 16 | (int32_t)m << 8 | l;
    spl1301->calib_param.c10 = (spl1301->calib_param.c10 & 0x080000) ? (0xFFF00000 | spl1301->calib_param.c10) : spl1301->calib_param.c10;
    h = wiringPiI2CReadReg8(fd, 0x18);
    l = wiringPiI2CReadReg8(fd, 0x19);
    spl1301->calib_param.c01 = (int16_t)h << 8 | l;
    h = wiringPiI2CReadReg8(fd, 0x1A);
    l = wiringPiI2CReadReg8(fd, 0x1B);
    spl1301->calib_param.c11 = (int16_t)h << 8 | l;
    h = wiringPiI2CReadReg8(fd, 0x1C);
    l = wiringPiI2CReadReg8(fd, 0x1D);
    spl1301->calib_param.c20 = (int16_t)h << 8 | l;
    h = wiringPiI2CReadReg8(fd, 0x1E);
    l = wiringPiI2CReadReg8(fd, 0x1F);
    spl1301->calib_param.c21 = (int16_t)h << 8 | l;
    h = wiringPiI2CReadReg8(fd, 0x20);
    l = wiringPiI2CReadReg8(fd, 0x21);
    spl1301->calib_param.c30 = (int16_t)h << 8 | l;
}


/**
 * @brief ���� spl1301 �¶Ȳ���
 * @notice ������� spl1301 ģʽλ������������ú�������Ҫʹ��
 */
void spl1301_start_temperature(int fd)
{
    wiringPiI2CWriteReg8(fd, 0x08, 0x02);
}

/**
 * @brief ���� SPL1301 ѹ������
 * @notice ������� SPL1301 ģʽλ������������ú�������Ҫʹ��
 */
void spl1301_start_pressure(int fd)
{
    wiringPiI2CWriteReg8(fd, 0x08, 0x01);
}

/**
 * @brief  ѡ�񴫸���Ϊ��������ģʽ
 * @param 
 *  uint8_t mode  1: pressure; 2: temperature; 3: pressure and temperature  
 */
void spl1301_start_continuous(int fd, uint8_t mode)
{
    wiringPiI2CWriteReg8(fd, 0x08, mode + 4);
}

/**
 * @brief  spl1301 ֹͣת��
 */
void spl1301_stop(int fd)
{
    wiringPiI2CWriteReg8(fd, 0x08, 0);
}

/**
 * @brief  ��ȡԭʼ�¶�ֵ��������ת��Ϊ32λ����
 */
void spl1301_get_raw_temp(int fd)
{
    uint8_t h, m, l;

    // ��λ��ǰ (datasheet P17)
    h = wiringPiI2CReadReg8(fd, 0x03);
    m = wiringPiI2CReadReg8(fd, 0x04);
    l = wiringPiI2CReadReg8(fd, 0x05);

    spl1301->i32rawTemperature = (int32_t)h << 16 | (int32_t)m << 8 | (int32_t)l;
    spl1301->i32rawTemperature = (spl1301->i32rawTemperature & 0x800000) ? (0xFF000000 | spl1301->i32rawTemperature) : spl1301->i32rawTemperature;
}

/**
 * @brief  ��ȡԭʼѹ��ֵ��������ת��Ϊ32λ����
 */
void spl1301_get_raw_pressure(int fd)
{
    uint8_t h, m, l;

    // ��λ��ǰ (datasheet P17)
    h = wiringPiI2CReadReg8(fd, 0x00);
    m = wiringPiI2CReadReg8(fd, 0x01);
    l = wiringPiI2CReadReg8(fd, 0x02);

    spl1301->i32rawPressure = (int32_t)h << 16 | (int32_t)m << 8 | (int32_t)l;
    spl1301->i32rawPressure = (spl1301->i32rawPressure & 0x800000) ? (0xFF000000 | spl1301->i32rawPressure) : spl1301->i32rawPressure;
}

/**
 * @brief  ����ԭʼ���ݷ���У׼����¶�ֵ
 * 
 *  ���ô˺���ǰ��Ҫ�ȵ��� spl1301_get_raw_temp,��ȡԭʼ����
 */
float get_spl1301_temperature(void)
{
    float fTsc;

    fTsc = spl1301->i32rawTemperature / (float)spl1301->i32kT;
    spl1301->temperature = spl1301->calib_param.c0 * 0.5 + spl1301->calib_param.c1 * fTsc;

    return spl1301->temperature;
}

/**
 * @brief  ����ԭʼ���ݷ���У׼�����ѹֵ
 * 
 *  ���ô˺���ǰ��Ҫ�ȵ��� spl1301_get_raw_pressure,��ȡԭʼ����
 */
float get_spl1301_pressure(void)
{
    float fTsc, fPsc;
    float qua2, qua3;

    fTsc = spl1301->i32rawTemperature / (float)spl1301->i32kT;
    fPsc = spl1301->i32rawPressure / (float)spl1301->i32kP;
    qua2 = spl1301->calib_param.c10 + fPsc * (spl1301->calib_param.c20 + fPsc * spl1301->calib_param.c30);
    qua3 = fTsc * fPsc * (spl1301->calib_param.c11 + fPsc * spl1301->calib_param.c21);

    spl1301->pressure = spl1301->calib_param.c00 + fPsc * qua2 + fTsc * spl1301->calib_param.c01 + qua3;

    return spl1301->pressure;
}

//------------------------------------------------------------------------------------------------------------------
//
//	���� WiringPi functions
//
//------------------------------------------------------------------------------------------------------------------

/**
  * @brief  spl1301 ��������ת��Ϊͨ����ȡ��Ӧ��ֵ
  */
static int myDigitalRead(struct wiringPiNodeStruct *node, int pin)
{
    /* 0Ϊѹ��ͨ����1Ϊ�¶�ͨ�� */
    int channel = pin - node->pinBase;
    int fd      = node->fd;

    /* �Ȼ�ȡ�¶����ݣ��¶Ȳ��� */
    spl1301_get_raw_temp(fd);

    if(PRESSURE_SENSOR == channel)
    {
        /* �Ȼ�ȡԭʼ���� */
        spl1301_get_raw_pressure(fd);
        // TODO �˴��Ƿ���Ҫ��ʱ��������
        /* �ٸ����ڲ�ram�������ݽ���ת�� */
        return get_spl1301_pressure();
    }
    else if(TEMPERATURE_SENSOR == channel)
    {
        return get_spl1301_temperature();
    }

    log_e("spl1301 channel range in [0, 1]");
    return -1;
}



/**
 * @brief  ��ʼ�������� spl1301
 * @param 
 *  int pinBase  pinBase > 64
 *
 * ע�⣬��ȡ����Ϊ int�ͣ�ʹ��ʱ��Ҫǿ��ת��Ϊfloat��
 */
int spl1301Setup(const int pinBase)
{
    static int fd;
	struct wiringPiNodeStruct *node;

    if ((fd = wiringPiI2CSetupInterface(SPL1301_I2C_DEV, SPL1301_I2C_ADDR)) < 0)
    {
        log_e("spl1301 i2c init failed");
        return -1;
    }
    // ���޷���ȡIDʱ���ж� ���벻��SPL1301����δ����SPL1301
    if(spl1301_get_id(fd) < 0)
        return -1;

    // ��ȡ�����궨����
    spl1301_get_calib_param(fd);
    // ������ = 32Hz; Pressure �������� = 8;
    spl1301_rateset(fd, PRESSURE_SENSOR, 32, 8);
    // ������ = 1Hz; Temperature �������� = 8;
    spl1301_rateset(fd, TEMPERATURE_SENSOR, 32, 8);
    /* ��̨ģʽ(�Զ�����ת�� ��ѹ �� �¶�)�����Զ���������ģʽ */
    spl1301_start_continuous(fd, CONTINUOUS_P_AND_T); 

    // �����ڵ��������2��ͨ����һ��Ϊѹ��ֵ��һ��Ϊ�¶�ֵ
    node = wiringPiNewNode(pinBase, 2);
	if (!node)
    {
        log_e("spl1301 node create failed");
        return -1;
    }

    // ע�᷽��
    node->fd          = fd;
    node->digitalRead = myDigitalRead;

    return fd;
}

