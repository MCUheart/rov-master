/*
 * @Description: MS5837 ��ȴ���������
 *
 *       Notes: ˮ������豸����
 *   Attention: SCL - E10 (��ɫ)   
 *				SDA - E12 (��ɫ)   
 */
#define LOG_TAG "ms5837"

#include "ms5837.h"

#include <elog.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

static ms5837_t ms5837_dev;
static ms5837_t *ms5837 = &ms5837_dev;

/**
  * @brief  crc4У�麯�� (datasheet P12)
  * @param  ����
  * @retval ����crc 4bitУ��
  */
uint8_t _crc4(uint16_t *n_prom)
{
	int32_t  cnt;
	uint32_t n_rem = 0; // crc ����
	uint8_t  n_bit;

	n_prom[0] = ((n_prom[0]) & 0x0FFF); // CRC byte is replaced by 0
	n_prom[7] = 0; // Subsidiary value, set to 0
	for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
	{ 	
		// choose LSB or MSB
		if (cnt%2==1) 
			n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
		else 
			n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
		for (n_bit = 8; n_bit > 0; n_bit--)
		{
			if (n_rem & (0x8000)) 
				n_rem = (n_rem << 1) ^ 0x3000;
			else 
				n_rem = (n_rem << 1);
		}
	}
	n_rem= ((n_rem >> 12) & 0x000f); // final 4-bit remainder is CRC code
	return (n_rem ^ 0x00);
}

/**
  * @brief  ms5837 ��λ
  */
void ms5837_reset(int fd)
{
	wiringPiI2CWrite(fd, MS583703BA_RESET);
}

/**
  * @brief  ms5837��ȡ�����궨����
  * @param  None
  * @retval ���س����궨���� crcУ�� �Ƿ�ɹ���־��1�ɹ���-1ʧ��
  *  ���ɹ���ʾΪms5837������
  *  ��ʧ�ܱ�ʾΪ�������ʹ���������
  */
int ms5837_get_calib_param(int fd)
{	
	static int i;
	for (i = 0; i <= 6; i++) 
	{
		// ��ȡprom�еĳ����궨����
    	ms5837->c[i] = \
		wiringPiI2CReadReg16(fd, MS583703BA_PROM_RD + (i * 2));
	}
	/* crcУ��Ϊ C[0]�� bit[15,12] */
	ms5837->crc = (uint8_t)(ms5837->c[0] >> 12);
	// �����������Ϊ c[0] ��bit[14,0]
	ms5837->factory_id = (uint8_t)(ms5837->c[0] & 0x0fff);

	/* 
	 * crcУ��Ϊ�����ж�ms5837�Ƿ��ʼ���ɹ� 
	 * ����˵�Ƿ��� ms5837������
	*/
	if(ms5837->crc == _crc4(ms5837->c))
	{
		return 1;
	}
	return -1;
}


 /**
 * @brief  ms5837 ��ȡת������
 * @param 
 *  uint8_t command  �������¶�����  �������¶�ѹ��(��ͷ�ļ�)
 * @retval
 *  uint32_t ���ݽ��
 */
uint32_t ms5837_get_conversion(int fd, uint8_t command)
{
 	uint8_t temp[3];
	uint32_t conversion;
	// 1.��д��ת������(��ָ��ת��������������) (datasheet P11)
	wiringPiI2CWrite(fd, command);

 	// 2.��ʱ�ȴ�ת�����  ��ȡ8196ת��ֵ�ùؼ���������� datasheet P2ҳ�е�18.08����
	delay(30);
	
	// 3.��д�� ADC read����
	wiringPiI2CWrite(fd, MS583703BA_ADC_RD);

	// 4.��ȡ 24bit��ת������ ��λ��ǰ
	temp[0] = wiringPiI2CRead(fd); // bit 23-16
	temp[1] = wiringPiI2CRead(fd); // bit 8-15
	temp[2] = wiringPiI2CRead(fd); // bit 0-7

	conversion = ((uint32_t)temp[0] <<16) | ((uint32_t)temp[1] <<8) | ((uint32_t)temp[2]);

	return conversion;
}

/**
 * @brief  ��ȡ�������¶�ֵ
 *  ��ʱ���¶�ֵ��û��������������׼ȷ
 */
void ms5837_cal_raw_temperature(int fd)
{
	// ��ȡԭʼ�¶�������
	ms5837->D2_Temp = ms5837_get_conversion(fd, MS583703BA_D2_OSR_2048);
	// ʵ���¶���ο��¶�֮�� (��ʽ��datasheet P7)
	ms5837->dT = ms5837->D2_Temp - (((uint32_t)ms5837->c[5]) * 256);
	// ʵ�ʵ��¶�
	ms5837->TEMP = 2000 + ms5837->dT * ((uint32_t)ms5837->c[6]) / 8388608; // 8388608 = 2^23,���ﲻ��������23λ����Ϊ������Ϊ�з��� 
}

/**
 * @brief  ��ȡ������ѹ��ֵ
 *  �������¶Ȳ���
 */
void ms5837_cal_pressure(int fd)
{
	int64_t  Ti, OFFi, SENSi;
	uint32_t dT_squ;
	uint32_t temp_minus_squ, temp_plus_squ;

	// ��ȡԭʼѹ��������
	ms5837->D1_Pres= ms5837_get_conversion(fd, MS583703BA_D1_OSR_8192);
	// ʵ���¶�ƫ��
	ms5837->OFF  = (int64_t)ms5837->c[2] * 65536 + ((int64_t)ms5837->c[4] * ms5837->dT) / 128;
	// ʵ���¶�������
	ms5837->SENS = (int64_t)ms5837->c[1] * 32768 + ((int64_t)ms5837->c[3] * ms5837->dT) / 256;

	dT_squ   = (ms5837->dT * ms5837->dT); // dT��2�η�
	temp_minus_squ = (2000 - ms5837->TEMP) * (2000 - ms5837->TEMP); // �¶Ȳ��2�η�

	/* ���¶Ⱥ�ѹ�����ж������� (datasheet P8) */
	if(ms5837->TEMP < 2000) // �������:����20��ʱ
	{
		Ti    = 3 * dT_squ / 0x200000000;
		OFFi  = 3 * temp_minus_squ / 2;
		SENSi = 5 * temp_minus_squ / 8;

		if(ms5837->TEMP < -1500) // ���������:����-15��ʱ
		{
			temp_plus_squ = (ms5837->TEMP + 1500) * (ms5837->TEMP + 1500); // �¶Ⱥ͵�2�η�
			OFFi  += 7 * temp_plus_squ;
			SENSi += 4 * temp_plus_squ;
		}
	}
	else // �������:����20��ʱ
	{
		Ti    = 2 * dT_squ / 0x2000000000;
		OFFi  = 1 * temp_minus_squ / 16;
		SENSi = 0; 
	}
	ms5837->OFF  -= OFFi;
	ms5837->SENS -= SENSi;	

	// �¶Ȳ������ѹ��ֵ
	ms5837->P = (ms5837->SENS / 0x200000 - ms5837->OFF) / 0x1000;

	// ʵ���¶�ֵ
	ms5837->temperature = (ms5837->TEMP - Ti) / 100;
	// ʵ��ѹ��ֵ
	ms5837->pressure = ms5837->P / 10;
}


//------------------------------------------------------------------------------------------------------------------
//
//	���� WiringPi functions
//
//------------------------------------------------------------------------------------------------------------------


/**
  * @brief  ms5837 ��������ת��Ϊͨ����ȡ��Ӧ��ֵ
  */
static int myDigitalRead(struct wiringPiNodeStruct *node, int pin)
{
    /* 0Ϊѹ��ͨ����1Ϊ�¶�ͨ�� */
    int channel = pin - node->pinBase;
    int fd      = node->fd;

    /* �Ȼ�ȡ�¶����ݣ���Ϊ��Ҫ�����¶Ȳ��� 
	 * �ڼ���ѹ�������У�������¶ȶ��ף�ʹ���¶ȸ���׼ȷ
	 * ��˲��ܻ�ȡѹ�������¶ȣ���Ӧ������������������
	*/
    ms5837_cal_raw_temperature(fd);
	ms5837_cal_pressure(fd);

    if(PRESSURE_SENSOR == channel)
    {
        return ms5837->pressure;
    }
    else if(TEMPERATURE_SENSOR == channel)
    {
        return ms5837->temperature;
    }

    log_e("ms5837 channel range in [0, 1]");
    return -1;
}


/**
 * @brief  ��ʼ�������� ms5837
 * @param 
 *  int pinBase  pinBase > 64
 */
int ms5837Setup(const int pinBase)
{
    static int fd;
	struct wiringPiNodeStruct *node;

    if ((fd = wiringPiI2CSetupInterface(MS5837_I2C_DEV, MS583703BA_I2C_ADDR)) < 0)
    {
        log_e("ms5837 i2c init failed");
        return -1;
    }

	/* �ȸ�λ�ٶ�ȡprom���� (datasheet P10) */
	ms5837_reset(fd);	     
	
	// ��ȡ�궨����
	if(ms5837_get_calib_param(fd) < 0) 
	{
		return -1;
	}
	
    // �����ڵ㣬2��ͨ����һ��Ϊѹ��ֵ��һ��Ϊ�¶�ֵ
    node = wiringPiNewNode(pinBase, 2);
	if (!node)
    {
        log_e("ms5837 node create failed");
        return -1;
    }

    // ע�᷽��
    node->fd         = fd;
    node->analogRead = myDigitalRead;

	return fd;
}

