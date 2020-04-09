#ifndef __MS5837_H_
#define __MS5837_H_

#include "../user/DataType.h"

#define MS5837_I2C_DEV  "/dev/i2c-1"  // MS5837 ʹ�õ� I2C�豸
/* MS5837-30BA address is 1110110x (write: x=0, read: x=1). */
#define MS583703BA_I2C_ADDR         0x76  // MS5387 I2C ��ַ (datasheet P9)

#define MS583703BA_RESET            0x1E
#define MS583703BA_ADC_RD           0x00
#define	MS583703BA_PROM_RD          0xA0

/* ת�����ѹ�� (datasheet P9) */
#define MS583703BA_D1_OSR_256	    0x40
#define MS583703BA_D1_OSR_512		0x42
#define MS583703BA_D1_OSR_1024		0x44
#define MS583703BA_D1_OSR_2048		0x46
#define MS583703BA_D1_OSR_4096		0x48
#define	MS583703BA_D1_OSR_8192   	0x4A

/* ת������¶� (datasheet P9) */
#define MS583703BA_D2_OSR_256		0x50
#define MS583703BA_D2_OSR_512		0x52
#define MS583703BA_D2_OSR_1024		0x54
#define MS583703BA_D2_OSR_2048		0x56
#define MS583703BA_D2_OSR_4096		0x58
#define	MS583703BA_D2_OSR_8192   	0x5A


/* ��������� */
#define PRESSURE_SENSOR 0
#define TEMPERATURE_SENSOR 1


typedef struct
{
    /** ����У׼���� calib_param[7] (datasheet P12)
    *
    * C0    CRCУ�� bit[15,12]  ��������bit[11,0] 
    * C1    ѹ�������� SENS|T1
    * C2    ѹ������   OFF|T1
    * C3	�¶�ѹ��������ϵ�� TCS
    * C4	�¶�ϵ����ѹ������ TCO
    * C5	�ο��¶� T|REF
    * C6 	�¶�ϵ�����¶� TEMPSENS
    */
    uint16_t c[7];      // prom�еĳ���У׼���� calib_param[7]
    uint8_t crc;        // crcУ��
    uint8_t factory_id; // �������� (Ϊ c[0]��ǰ12bit) 

    uint32_t D1_Pres; // ԭʼѹ��������
    uint32_t D2_Temp; // ԭʼ�¶�������

    int32_t dT;   // ʵ���¶���ο��¶�֮��
    int32_t TEMP; // ʵ�ʵ��¶�

    int64_t OFF;  // ʵ���¶�ƫ��
    int64_t SENS; // ʵ���¶�������
    int32_t P;    // �¶Ȳ�����ѹ��

    float pressure;    // ʵ��ѹ��ֵ
    float temperature; // ʵ���¶�ֵ

}ms5837_t; /* ���ms5837��ز��� */


int ms5837Setup(const int pinBase);


#endif

