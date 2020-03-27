/*
 * @Description: jy901 �������ݶ�ȡ����
 */
 
#define LOG_TAG "jy901"

#include "jy901.h"

#include <elog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include <wiringPi.h>
#include <wiringSerial.h>

static jy901_t jy901_dev;
static jy901_t *jy901 = &jy901_dev;

short Compass_Offset_Angle = 0;//ָ���벹���Ƕ�   �����ܵ����Ӵų����ţ���Ҫ��һ�������Ƕ�  -360 ~ +360
extern Sensor_Type Sensor;

// ��������
void jy901_convert(JY901_Type *pArr);


/**
  * @brief  copeJY901_dataΪ�����жϵ��ú���������ÿ�յ�һ�����ݣ�����һ���������
  *         �ж��������������ݣ�Ȼ���俽������Ӧ�Ľṹ����
  */ 
void copeJY901_data(uint8_t Data)
{
	static uint8_t rxBuffer[20] = {0}; // ���ݰ�
	static uint8_t rxCheck = 0;		 // βУ����
	static uint8_t rxCount = 0;		 // ���ռ���
	static uint8_t i = 0;				 // ���ռ���

	rxBuffer[rxCount++] = Data; // ���յ������ݴ��뻺������

	if (rxBuffer[0] != 0x55)
	{
		// ����ͷ���ԣ������¿�ʼѰ��0x55����ͷ
		rxCount = 0; // ��ջ�����
		return;
	}
	if (rxCount < JY901_PACKET_LENGTH)
	{
		return;
	} // ���ݲ���11�����򷵻�

	/*********** ֻ�н�����11���ֽ����� �Ż�������³��� ************/
	for (i = 0; i < 10; i++)
	{
		rxCheck += rxBuffer[i]; //У��λ�ۼ�
	}

	if (rxCheck == rxBuffer[JY901_PACKET_LENGTH - 1]) // �ж����ݰ�У���Ƿ���ȷ
	{
		// �ж��������������ݣ�Ȼ���俽������Ӧ�Ľṹ���У���Щ���ݰ���Ҫͨ����λ���򿪶�Ӧ������󣬲��ܽ��յ�������ݰ�������
		switch (rxBuffer[1])
		{
			case 0x50:	memcpy(&jy901->stcTime,    &rxBuffer[2],8);break; // ������ȥ ��ͷ�����ݸ���λ
			case 0x51:	memcpy(&jy901->stcAcc,     &rxBuffer[2],8);break;
			case 0x52:	memcpy(&jy901->stcGyro,    &rxBuffer[2],8);break;
			case 0x53:	memcpy(&jy901->stcAngle,   &rxBuffer[2],8);break;
			case 0x54:	memcpy(&jy901->stcMag,     &rxBuffer[2],8);break;
			case 0x55:	memcpy(&jy901->stcDStatus, &rxBuffer[2],8);break;
			case 0x56:	memcpy(&jy901->stcPress,   &rxBuffer[2],8);break;
			case 0x57:	memcpy(&jy901->stcLonLat,  &rxBuffer[2],8);break;
			case 0x58:	memcpy(&jy901->stcGPSV,    &rxBuffer[2],8);break;
			case 0x59:	memcpy(&jy901->stcQ,       &rxBuffer[2],8);break;
		}
		rxCount = 0; // ��ջ�����
		rxCheck = 0; // У��λ����

		jy901_convert(&Sensor.JY901); // JY901����ת��
	}
	else // ��������
	{
		rxCount = 0; // ��ջ�����
		rxCheck = 0; // У��λ����
		return;
	}
}

// Sensor.JY901 ����ת��
void jy901_convert(JY901_Type *pArr)
{
	pArr->Acc.x = (float)jy901->stcAcc.a[0] / 2048; // 32768*16
	pArr->Acc.y = (float)jy901->stcAcc.a[1] / 2048;
	pArr->Acc.z = (float)jy901->stcAcc.a[2] / 2048;

	pArr->Gyro.x = (float)jy901->stcGyro.w[0] / 2048 * 125; // 32768*2000
	pArr->Gyro.y = (float)jy901->stcGyro.w[1] / 2048 * 125;
	pArr->Gyro.z = (float)jy901->stcGyro.w[2] / 2048 * 125;

	pArr->Euler.Roll = (float)jy901->stcAngle.angle[0] / 8192 * 45; // 32768*180;
	pArr->Euler.Pitch = (float)jy901->stcAngle.angle[1] / 8192 * 45;
	pArr->Euler.Yaw = (float)jy901->stcAngle.angle[2] / 8192 * 45;

	// ƫ�ƽǶ� ���ڵ�ָ�� ����ʱ�ĽǶ�(-360 ~ +360 )
	if (Compass_Offset_Angle != 0) // ���δ���ò����Ƕȣ��򲻽��нǶȲ���������Ϊ ���Ƕȡ�
	{
		pArr->Euler.Yaw -= Compass_Offset_Angle; // ��ȥ�����Ƕ�
		if (pArr->Euler.Yaw < -180){
			pArr->Euler.Yaw += 360; // �Ƕȷ��򲹳�
		}
		if (pArr->Euler.Yaw > 180){
			pArr->Euler.Yaw -= 360; // �Ƕȷ��򲹳�
		}
	}

	pArr->Mag.x = jy901->stcMag.h[0];
	pArr->Mag.y = jy901->stcMag.h[1];
	pArr->Mag.z = jy901->stcMag.h[2];

	pArr->Temperature = (float)jy901->stcAcc.T / 100;
}


/**
  * @brief  �򿪶�Ӧ JY901 �����豸
  */
int jy901Setup(void)
{
	static int fd = 0;

	if ((fd = serialOpen(JY901_UART_DEV, JY901_UART_BAUD)) < 0)
	{
		log_e("Unable to open serial device: %s", strerror(errno));
		log_e("JY901 init failed");
		return -1;
	}

	return fd;
}
