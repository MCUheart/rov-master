
#ifndef __RC_DATA_H_
#define __RC_DATA_H_

#include "../user/DataType.h"

/*
 * 数据包长度【包含包头、长度位、校验位】
 * 包头：3 包体：16 校验字：1
*/
#define RECV_DATA_LEN  20 
            
/*
 * 数据包长度【包含包头、长度位、校验位】
 * 包头：3 包体：22 校验字：1
*/
#define RETURN_DATA_LEN 26


typedef struct
{
    int16 X; //摇杆 X、Y轴模拟量 （X正方向为机头方向）
    int16 Y;
    int16 Z;     //Z为垂直方向
    int16 Fx;    //转换的 X轴分力
    int16 Fy;    //转换的 Y轴分力
    int16 Yaw;   //偏航
    int16 Force; //合力
    float Angle; //合力的角度
} Rocker_Type;

typedef struct
{
    uint16 THR;
    uint16 YAW;
    uint16 ROL;
    uint16 PIT;
} ReceiveData_Type;

typedef struct
{
    uint8 All_Lock;       //总开关
    uint8 Depth_Lock;     //【深度】	 0x01 深度锁定、0x02 人工控制
    uint8 Direction_Lock; //【方向】	 0x01 方向锁定、0x02 随水动
    uint8 Move;           //【运动】	 0x01 前进、    0x02 后退、  0x00 不动作（默认）
    uint8 Translation;    //【平移】	 0x01 左平移、  0x02 右平移、0x00 不动作（默认）
    uint8 Vertical;       //【垂直】	 0x01 向上、    0x02 向下、  0x00 不动作（默认）
    uint8 Rotate;         //【旋转】	 0x01 左旋、    0x02 右旋、  0x00表示不动作（默认）
    uint16 Power;         //【油门】	 0x00~0xff (0~255) 表示的油门大小：4档位可调，LB加档，LT减档，可分别设置4个档位油门大小
    uint8 Light;          //【灯光】	 0x01 打开、    0x00 关闭（默认）
    uint8 Focus;          //【聚焦】  0x01 聚焦、		0x02 放焦、  0x11 放大、0x12 缩小、0x00表示不动作（默认）
    uint8 Yuntai;         //【云台】	 0x01 向上、    0x02 向下、  0x00表示不动作（默认）
    uint8 Arm;            //【机械臂】0x01 张开、    0x02 关闭、  0x00表示不动作（默认）
    uint8 Raspi;          //【树莓派】0x01 代表树莓派开机，0x00为无开机响应（默认）
    uint8 Reserve;        //【机械臂】0x01 张开、    0x02 关闭、  0x00表示不动作（默认）
} ControlCmd_Type;

void remote_control_data_analysis(uint8 *recv_buff); //控制数据解析
void Control_Cmd_Clear(ControlCmd_Type *cmd);        //控制命令清零
uint8 calculate_check_sum(uint8 *buff, uint8 len);
void convert_rov_status_data(uint8 *buff); // 转换需要返回上位机数据

extern uint8 device_hint_flag; //设备提示字符

extern Rocker_Type Rocker;
extern ControlCmd_Type ControlCmd;
extern ReceiveData_Type ReceiveData;
extern uint8 recv_buff[RECV_DATA_LEN];
extern uint8 Frame_EndFlag;

#endif
