#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
 

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define SERADDR         "192.168.188.100"                // 服务器开放给我们的IP地
#define SERPORT         6003

char sendbuf[100];

#define     path1  "/sys/bus/spi/drivers/TM7705_1/spi0.1/ch1"
#define     path2  "/sys/bus/spi/drivers/TM7705_1/spi0.1/ch2"
#define     path3  "/sys/bus/spi/drivers/TM7705_3/spi0.3/ch1"
#define     path4  "/sys/bus/spi/drivers/TM7705_3/spi0.3/ch2"
int main (void)
{
	/*****************************************************/
    int fd1 = 0;
	int fd2 = 0;
	int fd3 = 0;
	int fd4 = 0;
   // int ret = 0;
    unsigned char buff1[128] = {0};
	unsigned char buff2[128] = {0};
	unsigned char buff3[128] = {0};
	unsigned char buff4[128] = {0};
  /*************************************************************/

 // 第1步：先socket打开文件描述符
        int sockfd = -1, ret = -1;
        struct sockaddr_in seraddr = {0};
        struct sockaddr_in cliaddr = {0};

        // 第1步：socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == sockfd)
        {
                perror("socket");
                return -1;
        }
        printf("socketfd = %d.\n", sockfd);

        // 第2步：connect链接服务器
        seraddr.sin_family = AF_INET;           // 设置地址族为IPv4
        seraddr.sin_port = htons(SERPORT);      // 设置地址的端口号信息
        seraddr.sin_addr.s_addr = inet_addr(SERADDR);   //　设置IP地址
        ret = connect(sockfd, (const struct sockaddr *)&seraddr, sizeof(seraddr));
        if (ret < 0)
        {
                perror("listen");
                return -1;
        }
        printf("成功建立连接\n");



/****************************************************************/
    fd1 = open(path1, O_RDONLY);
    if (-1 == fd1)
    {
        printf("[%s] open device file fail.\n", __FUNCTION__);
        return -1 ;
    }
    
    memset(buff1, 0, 128);
    ret = read(fd1, buff1, 128);
    if (0 > ret)
    {
        printf("[%s] not read data. ret=%d\n", __FUNCTION__, ret);
    }
    printf("[%s] buff1=%s\n", __FUNCTION__, buff1);
	//sleep(10);
 /*****************************************************/
 
 fd2 = open(path2, O_RDONLY);
    if (-1 == fd2)
    {
        printf("[%s] open device file fail.\n", __FUNCTION__);
        return -1 ;
    }
    
    memset(buff2, 0, 128);
    ret = read(fd2, buff2, 128);
    if (0 > ret)
    {
        printf("[%s] not read data. ret=%d\n", __FUNCTION__, ret);
    }
    printf("[%s] buff2=%s\n", __FUNCTION__, buff2);

 /***************************************************/
fd3 = open(path1, O_RDONLY);
    if (-1 == fd3)
    {
        printf("[%s] open device file fail.\n", __FUNCTION__);
        return -1 ;
    }
    
    memset(buff3, 0, 128);
    ret = read(fd3, buff3, 128);
    if (0 > ret)
    {
        printf("[%s] not read data. ret=%d\n", __FUNCTION__, ret);
    }
    printf("[%s] buff3=%s\n", __FUNCTION__, buff3);
	

 /**************************************************************/

 
  fd4 = open(path4, O_RDONLY);
    if (-1 == fd4)
    {
        printf("[%s] open device file fail.\n", __FUNCTION__);
        return -1 ;
    }
    
    memset(buff4, 0, 128);
    ret = read(fd4, buff4, 128);
    if (0 > ret)
    {
        printf("[%s] not read data. ret=%d\n", __FUNCTION__, ret);
    }
    printf("[%s] buff4=%s\n", __FUNCTION__, buff4);
  
	sleep(10);
	/**********************************************/
while(1)
{
   ret = read(fd1, buff1, 128); 
   ret = read(fd2, buff2, 128);
   ret = read(fd1, buff3, 128);
   ret = read(fd1, buff4, 128);

ret = send(sockfd, buff1, strlen(buff1), 0);
ret = send(sockfd, alcohol, strlen(alcohol), 0);
ret = send(sockfd1, buff3, strlen(buff3), 0);
ret = send(sockfd1, buff4, strlen(buff4), 0);
sleep(1);
} 
 close(fd1);
 close(fd2);
 close(fd3);
 close(fd4);
 
         return 0 ;
}
 
 


