#include "usart.h"
#include "gps.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
//#define SERADDR		"221.231.138.62"		// 服务器开放给我们的IP地址和端口号
//#define SERADDR "192.168.1.10"                 //电脑的Ip地址
//#define SERPORT   16663
#define SERADDR "192.168.188.101"                 //电脑的Ip地址
#define SERPORT   6003

int main(void)    
{
	/*************网络描述符**************/
	
	int socketfd = -1;
	int ret = -1;
	struct sockaddr_in seraddr = {0};
	struct sockaddr_in cliaddr = {0};
	/*********************************/
	
	u32 fd = 0;           //文件描述符，先定义一个与程序无关的值，防止fd为任意值导致程序出bug  
    u32 nread = 0;

  //  GNRMC gnrmc;
	GNGGA gngga;
    u8 gps_buff[GPS_LEN];

    fd = open(SERIAL1, O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd<0)
    {
        perror("open serial1 failed");
        return -1;
    }

    UART_Set(fd, 38400, 0, 8, 1, 'N');
/**************************************************************/	
	socketfd = socket(AF_INET,SOCK_STREAM,0);
	if( -1 == socketfd)
	{
		perror("socket");
		return -1;
	}
//	printf("socketfd 为:%d.\n",socketfd);
	
	
	seraddr.sin_family = AF_INET;		// 设置地址族为IPv4
	seraddr.sin_port = htons(SERPORT);	// 设置地址的端口号信息
	seraddr.sin_addr.s_addr = inet_addr(SERADDR);	//　设置IP地址
	ret = connect(socketfd,(const struct sockaddr *)&seraddr,sizeof(seraddr));
	if(ret < 0)
	{
		perror("listen");
		return -1;
		
	}
		//printf("connect succees ,ret = %d\n",ret);	
	
	
	
/****************************************************************/	
    while(1)
    {
        sleep(1);         //此处数据一定要根据GPS模块测试频率设置正确，否则数据采样会出现错误
        nread = read(fd, gps_buff, sizeof(gps_buff));
        if(nread<0)
        {
            perror("read GPS date error");
            return -2;
        }
		
       // printf("gps_buff: %s\n", gps_buff);
		ret = send(socketfd,gps_buff,  sizeof(gps_buff),0);	
		

	}
    close(fd);
	
    return 0;
}


