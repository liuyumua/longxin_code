#include "gps.h"

int gps_analyse (char *buff,GNGGA *gps_data)
{
    char *ptr = NULL;

    if(gps_data == NULL)
    {
        return -1;
    }

    if(strlen(buff)<10)
    {
        return -1;
    }
	
	
	/*
    //如果buff字符串中包含字符"$GNRMC"则将$GNRMC的地址赋值给ptr
    if(NULL==(ptr=strstr(buff,"$GNRMC")))
    {
        return -1;
    }
    sscanf(ptr,"$GNRMC,%d.000,%c,%f,N,%f,E,%f,%f,%d,,,%c*",&(gps_data->time),&(gps_data->pos_state),&(gps_data->latitude),&(gps_data->longitude),&(gps_data->speed),&(gps_data->direction),&(gps_data->date),&(gps_data->mode));
//sscanf函数为从字符串输入，上面这句话的意思是将ptr内存单元的值作为输入分别输入到后面的结构体成员
    return 0;*/
	
	
	//如果buff字符串中包含字符"$GNGGA"则将$GNGGA的地址赋值给ptr
	if(NULL==(ptr=strstr(buff,"$GNGGA")))
    {
        return -1;
    }
    sscanf(ptr,"$GNGGA,%d.000,%f,N,%f,E,%d,%d,%f,%f,M,%f,M,,%d*",
	 &(gps_data->time), &(gps_data->latitude),
	 &(gps_data->longitude), &(gps_data->pos_state),
	 &(gps_data->weixing_number), &(gps_data->hdop),&(gps_data->altitude),
	 &(gps_data->dadi_height),&(gps_data->chafen_jizhan));
//sscanf函数为从字符串输入，上面这句话的意思是将ptr内存单元的值作为输入分别输入到后面的结构体成员
    return 0;
} 
/*
int print_gps (GNRMC *gps_data)
{
    printf("===========================================================\n");
    printf("==   GPS状态位 : %c  [A:有效状态 V:无效状态]              \n",gps_data->pos_state);
    printf("==   GPS模式位 : %c  [A:自主定位 D:差分定位]               \n", gps_data->mode);
    printf("==   日期 : 20%02d-%02d-%02d                                  \n",gps_data->date%100,(gps_data->date%10000)/100,gps_data->date/10000);
    printf("==   时间 : %02d:%02d:%02d                                   \n",(gps_data->time/10000+8)%24,(gps_data->time%10000)/100,gps_data->time%100);
    printf("==   纬度 : 北纬:%d度%d分%d秒                              \n", ((int)gps_data->latitude) / 100, (int)(gps_data->latitude - ((int)gps_data->latitude / 100 * 100)), (int)(((gps_data->latitude - ((int)gps_data->latitude / 100 * 100)) - ((int)gps_data->latitude - ((int)gps_data->latitude / 100 * 100))) * 60.0));
    printf("==   经度 : 东经:%d度%d分%d秒                              \n", ((int)gps_data->longitude) / 100, (int)(gps_data->longitude - ((int)gps_data->longitude / 100 * 100)), (int)(((gps_data->longitude - ((int)gps_data->longitude / 100 * 100)) - ((int)gps_data->longitude - ((int)gps_data->longitude / 100 * 100))) * 60.0));
    printf("==   速度 : %.3f  m/s                                      \n",gps_data->speed);
    printf("============================================================\n");

    return 0;
} */

int print_gps (GNGGA *gps_data)
{
    printf("===========================================================\n");
    printf("==   GPS状态位 : %d  [0:未定位 1:非差分定位 2：差分定位]  \n",gps_data->pos_state);
    printf("==   时间 : %02d:%02d:%02d                                   \n",(gps_data->time/10000+8)%24,(gps_data->time%10000)/100,gps_data->time%100);
    printf("==   纬度 : 北纬:%d度%d分%d秒                              \n", ((int)gps_data->latitude) / 100, (int)(gps_data->latitude - ((int)gps_data->latitude / 100 * 100)), (int)(((gps_data->latitude - ((int)gps_data->latitude / 100 * 100)) - ((int)gps_data->latitude - ((int)gps_data->latitude / 100 * 100))) * 60.0));
    printf("==   经度 : 东经:%d度%d分%d秒                              \n", ((int)gps_data->longitude) / 100, (int)(gps_data->longitude - ((int)gps_data->longitude / 100 * 100)), (int)(((gps_data->longitude - ((int)gps_data->longitude / 100 * 100)) - ((int)gps_data->longitude - ((int)gps_data->longitude / 100 * 100))) * 60.0));
    printf("==   卫星数量 : %d个                                    \n",gps_data->weixing_number);
	printf("==   海拔高度 : %fm                                    \n",gps_data->altitude);
    printf("============================================================\n");

    return 0;
} 
