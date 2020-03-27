/*
 * @Description: ��ȴ�������غ���
 * @Author: chenxi
 * @Date: 2020-01-14 16:52:11
 * @LastEditTime : 2020-02-10 16:57:25
 * @LastEditors  : chenxi
 */

#ifndef SPL1301_H
#define SPL1301_H

#include "../user/DataType.h"

#define SPL1301_I2C_DEV  "/dev/i2c-1"  // SPL1301 ʹ�õ� I2C�豸
#define SPL1301_I2C_ADDR 0x77          // Ĭ�� SPL1301 I2C ��ַ

/* �Ĵ�����ַ */
#define REG_ID 0x0D

/* ���� */
#define CONTINUOUS_PRESSURE 1
#define CONTINUOUS_TEMPERATURE 2
#define CONTINUOUS_P_AND_T 3

/* ��������� */
#define PRESSURE_SENSOR 0
#define TEMPERATURE_SENSOR 1

typedef struct
{
    int16 c0;
    int16 c1;
    int32 c00;
    int32 c10;
    int16 c01;
    int16 c11;
    int16 c20;
    int16 c21;
    int16 c30;
}spl1301_calib_param_t; // spl1301 ����У׼����

typedef struct
{
    spl1301_calib_param_t calib_param; /* calibration data */
    uint8_t product_id;  // ��ƷID     
    uint8_t revision_id; // �޶�ID   

    int32 i32rawPressure;    // ԭʼѹ��ֵ
    int32 i32rawTemperature; // ԭʼ�¶�ֵ
    int32 i32kP; // ѹ��ϵ��
    int32 i32kT; // �¶�ϵ��

    float pressure;    // ʵ��ѹ��ֵ
    float temperature; // ʵ���¶�ֵ
}spl1301_t; 

//��ʼ������
int spl1301Setup(const int pinBase);
//�����ض��������Ĳ����ʺ�ÿ���������
void spl1301_rateset(int fd, uint8_t iSensor, uint8_t u8SmplRate, uint8_t u8OverSmpl);
//��ʼһ���¶Ȳ���
void spl1301_start_temperature(int fd);
//��ʼһ��ѹ������
void spl1301_start_pressure(int fd);
//ѡ����������ģʽ
void spl1301_start_continuous(int fd, uint8_t mode);
// ��ȡԭʼ�¶�ֵ������ת��Ϊ32λ����
void spl1301_get_raw_temp(int fd);
//���øþ�ת������,��ȡԭʼѹ��ֵ������ת��Ϊ32λ����
void spl1301_get_raw_pressure(int fd);
//����ԭʼֵ����У׼�¶�ֵ��
float get_spl1301_temperature(void);
//����ԭֵ����У׼ѹ��ֵ��
float get_spl1301_pressure(void);

#endif
