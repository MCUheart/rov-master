#ifndef __ANO_LINK_H_
#define __ANO_LINK_H_

#include "../user/DataType.h"

//extern Vector3f_pid PID_Parameter[PID_USE_NUM];

void ANO_SEND_StateMachine(void); //各组数据循环发送

void ANO_Data_Send_Version(int hardwareType, int hardwareVER, int softwareVER, int protocolVER, int bootloaderVER); //发送版本信息

void ANO_Data_Send_Status(int roll, int pitch, int yaw, int depth); //发送基本信息（姿态、锁定状态）

void ANO_Data_Send_Voltage_Current(float volatge, float current); //发送电压电流

void ANO_DT_Data_Receive_Prepare(uint8_t data); //ANO地面站数据解析

void ANO_DT_Data_Receive_Anl(uint8_t *data_buf, uint8_t num); //ANO数据解析

void Save_Or_Reset_PID_Parameter(void);

int ano_udp_server_init(void);

#endif
