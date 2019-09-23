#ifndef __GPS_H__
#define __GPS_H__

#include "usart.h"

#define GPS_LEN 512

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;


/*
typedef struct __gnrmc__
{
    u32 time;
    char pos_state;
    float latitude;
    float longitude;
    float speed;
    float direction;
    u32 date;
    float declination;
    char dd;
    char mode;
}GNRMC;
*/


typedef struct __gngga__
{
    u32 time;
    float latitude;      //纬度
    float longitude;     //经度
	int pos_state;      //GPS状态
	int weixing_number;
	float hdop;
	float altitude;       //海拔高度
	float dadi_height;    //大地水准面高度
    //u32 chafen_time;   //差分时间
    u32 chafen_jizhan; //差分参考基站标号
}GNGGA;


int gps_analysis(char *buff, GNGGA *gps_date);
int print_gps(GNGGA *gps_date);

#endif



