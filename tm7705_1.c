#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
 
 
 
 
// 通信寄存器bit定义
enum 
{
	// 寄存器选择  RS2 RS1 RS0
	TM7705_REG_COMM	        = (0 << 4), // 通信寄存器
	TM7705_REG_SETUP	    = (1 << 4), // 设置寄存器
	TM7705_REG_CLOCK	    = (2 << 4), // 时钟寄存器
	TM7705_REG_DATA	        = (3 << 4), // 数据寄存器
	TM7705_REG_TEST         = (4 << 4), // 测试寄存器
	TM7705_REG_OFFSET       = (6 << 4), // 偏移寄存器
	TM7705_REG_GAIN         = (7 << 4), // 增益寄存器
	
    // 读写操作
	TM7705_WRITE 		    = (0 << 3), // 写操作
	TM7705_READ 		    = (1 << 3), // 读操作
 
	// 通道
	TM7705_CH_1		        = 0,    // AIN1+  AIN1-
	TM7705_CH_2		        = 1,    // AIN2+  AIN2-
	TM7705_CH_3		        = 2,    // AIN1-  AIN1-
	TM7705_CH_4		        = 3     // AIN1-  AIN2-
};
 
 
/* 设置寄存器bit定义 */
enum
{
	TM7705_MD_NORMAL		= (0 << 6),	/* 正常模式 */
	TM7705_MD_CAL_SELF		= (1 << 6),	/* 自校准模式 */
	TM7705_MD_CAL_ZERO		= (2 << 6),	/* 校准0刻度模式 */
	TM7705_MD_CAL_FULL		= (3 << 6),	/* 校准满刻度模式 */
 
	TM7705_GAIN_1			= (0 << 3),	/* 增益 */
	TM7705_GAIN_2			= (1 << 3),	/* 增益 */
	TM7705_GAIN_4			= (2 << 3),	/* 增益 */
	TM7705_GAIN_8			= (3 << 3),	/* 增益 */
	TM7705_GAIN_16			= (4 << 3),	/* 增益 */
	TM7705_GAIN_32			= (5 << 3),	/* 增益 */
	TM7705_GAIN_64			= (6 << 3),	/* 增益 */
	TM7705_GAIN_128		    = (7 << 3),	/* 增益 */
 
	/* 无论双极性还是单极性都不改变任何输入信号的状态，它只改变输出数据的代码和转换函数上的校准点 */
	TM7705_BIPOLAR			= (0 << 2),	/* 双极性输入 */
	TM7705_UNIPOLAR		    = (1 << 2),	/* 单极性输入 */
 
	TM7705_BUF_NO			= (0 << 1),	/* 输入无缓冲（内部缓冲器不启用) */
	TM7705_BUF_EN			= (1 << 1),	/* 输入有缓冲 (启用内部缓冲器) */
 
	TM7705_FSYNC_0			= 0,    // 模拟调制器和滤波器正常处理数据
	TM7705_FSYNC_1			= 1		// 模拟调制器和滤波器不启用
};
 
 
 
/* 时钟寄存器bit定义 */
enum
{
	TM7705_CLKDIS_0	        = (0 << 4),		/* 时钟输出使能 （当外接晶振时，必须使能才能振荡） */
	TM7705_CLKDIS_1	        = (1 << 4),		/* 时钟禁止 （当外部提供时钟时，设置该位可以禁止MCK_OUT引脚输出时钟以省电 */
 
    TM7705_CLKDIV_0         = (0 << 3),     // 不分频
    TM7705_CLKDIV_1         = (1 << 3),     // 2分频，外部晶振为4.9152Mhz时，应2分频
 
    TM7705_CLK_0            = (0 << 2),     // 主时钟=1Mhz并且CLKDIV=0，主时钟=2Mhz并且CLKDIV=1
    TM7705_CLK_1            = (1 << 2),     // 主时钟=2.4576Mhz并且CLKDIV=0, 主时钟=4.9152Mhz并且CLKDIV=1
 
    // 注意输出更新率与clk位有关
    // 当TM7705_CLK_0时，输出更新率只能为20,25,100,200
    TM7705_UPDATE_20        = (0),
    TM7705_UPDATE_25        = (1),
    TM7705_UPDATE_100       = (2),
    TM7705_UPDATE_200       = (3),
    // 当TM7705_CLK_1时，输出更新率只能为50,60,250,500
    TM7705_UPDATE_50        = (0),
    TM7705_UPDATE_60        = (1),
    TM7705_UPDATE_250       = (2),
    TM7705_UPDATE_500       = (3)
};
 
 
 
#define TM7705_CHANNEL_NUM              (2)     // tm7705通道个数
#define TM7705_DRDY_PIN                 (87)    // GPIO87/I2S_DI   tm7705的引脚DRDY 
#define TM7705_RESET_PIN                (89)    // GPIO89/I2S_LRCK  tm7705的引脚RESET
 
 
struct tm7705 {
    struct device *hwmon_dev;
    struct mutex lock;
};
 
 
 
// 通过reset脚复位tm7705
static void tm7705_reset(void)
{
    gpio_direction_output(TM7705_RESET_PIN, 1);
    msleep(1);
    gpio_direction_output(TM7705_RESET_PIN, 0);
    msleep(2);
    gpio_direction_output(TM7705_RESET_PIN, 1);
    msleep(1);
 
    return ;
}
 
 
// 同步spi接口时序
static void tm7705_sync_spi(struct spi_device *spi)
{
    u8 tx_buf[4] = {0xFF};
    
    // 至少32个串行时钟内向TM7705的DIN线写入逻辑"1"
    spi_write(spi, tx_buf, sizeof(tx_buf));
 
    return ;
}
 
 
// 等待内部操作完成
static int tm7705_wait_DRDY(void)
{
    int i = 0;
    int time_cnt = 500*1000;
 
    for (i=0; i<time_cnt; i++)
    {
        if (0 == gpio_get_value(TM7705_DRDY_PIN))
        {
            break;
        }
        udelay(1);
    }
 
    if (i >= time_cnt)
    {
        return -1;
    }
 
    return 0;
}
 
 
// 自校准
static void tm7705_calib_self(struct spi_device *spi, u8 channel)
{
    u8 tx_buf[2] = {0};
 
    tx_buf[0] = TM7705_REG_SETUP | TM7705_WRITE | channel;
    tx_buf[1] = TM7705_MD_CAL_SELF | TM7705_GAIN_1 | TM7705_UNIPOLAR | TM7705_BUF_EN | TM7705_FSYNC_0;
    spi_write(spi, tx_buf, sizeof(tx_buf));
 
    tm7705_wait_DRDY();         /* 等待内部操作完成 --- 时间较长，约180ms */
 
    msleep(50);
    
    return ;
}
 
 
// 配置tm7705的指定通道
static void tm7705_config_channel(struct spi_device *spi, u8 channel)
{
    u8 tx_buf[2] = {0};
 
    tx_buf[0] = TM7705_REG_CLOCK | TM7705_WRITE | channel;
    tx_buf[1] = TM7705_CLKDIS_0 | TM7705_CLKDIV_1 | TM7705_CLK_1 | TM7705_UPDATE_50;
    spi_write(spi, tx_buf, sizeof(tx_buf));
 
    // 自校准
    tm7705_calib_self(spi, channel);
 
    return ;
}
 
 
// 复位tm7705并重新配置
static void tm7705_reset_and_reconfig(struct spi_device *spi)
{
    // 通过reset脚复位tm7705
    tm7705_reset();
 
    // 同步spi接口时序
    msleep(5);
    tm7705_sync_spi(spi);
    msleep(5);
 
    // 配置tm7705时钟寄存器
    tm7705_config_channel(spi, TM7705_CH_1);
//    tm7705_config_channel(spi, TM7705_CH_2);
    
    return ;
}
 
 
/*
 * 读取一个通道的值
 * @dev 设备描述符
 * @channel 通道
 * ad 读到的ad值
 */
static int tm7705_read_channel(struct device *dev, u8 channel, u16 *ad)
{
    struct spi_device *spi = to_spi_device(dev);
    struct tm7705 *adc = spi_get_drvdata(spi);
    int ret = 0;
    u16 value = 9;
    u8 tx_buf[1] = {0};
    u8 rx_buf[2] = {0};
 
    if (mutex_lock_interruptible(&adc->lock))
    {
        return -ERESTARTSYS;
    }
 
    // 等待转换完成
    ret = tm7705_wait_DRDY();
    if(ret)
    {
        printk(KERN_ERR "[%s] tm7705_wait_DRDY() time out.\n", __FUNCTION__);
        goto fail;
    }
    
    tx_buf[0] = TM7705_REG_DATA | TM7705_READ | channel;
    ret = spi_write_then_read(spi, tx_buf, sizeof(tx_buf), rx_buf, sizeof(rx_buf));
    value = (rx_buf[0]<<8) + rx_buf[1];
    if (0 > ret)    // spi通信失败
    {
        printk(KERN_ERR "[%s] tm7705_read_byte() fail. ret=%d\n", __FUNCTION__, ret);
        goto fail;
    }
    if (0xfff == value)  // tm7705上电一段时间后，可能会出现读到的值一直是0xfff的情况
    {
        printk(KERN_ERR "[%s] value=0xfff\n", __FUNCTION__);
        ret = -1;
        goto fail;
    }
 
    // 输出AD值
    *ad = value;
 
fail:
    mutex_unlock(&adc->lock);
    return ret;
}
 
 
/* sysfs hook function */
static ssize_t tm7705_get_sensor_value(struct device *dev,
                           struct device_attribute *devattr,
                           char *buf)
{
    struct spi_device *spi = to_spi_device(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int ret = 0;
    u16 ad = 0;
    int i = 0;
 
    /*
	 * 为了避免通道切换造成读数失效，读2次
	 * 实际上每次读到的是上一次采集的结果(可以两个通道交替采集就能看到效果)
	 */
    for (i=0; i<2; i++)
    {
        ret = tm7705_read_channel(dev, attr->index, &ad);
        if (ret)
        {
            // 失败，则重启tm7705并重新配置
            tm7705_reset_and_reconfig(spi);
            printk(KERN_ERR "[%s] tm7705 reset and reconfig.\n", __FUNCTION__);
            return ret;
        }
        printk(KERN_DEBUG "[%s] tm7705 ad=0x%x\n", __FUNCTION__, ad);
        
        // ls1c的速度相当TM7705太快，延时一下避免在一次读完后DRDY还未及时改变状态ls1c又开始了下一次读写
        msleep(1);
    }
 
    // 将ad值传递给用户程序
    ret = sprintf(buf, "%u\n", ad);
    
    return ret;
}
 
 
 
 
static struct sensor_device_attribute ad_input[] = {
    SENSOR_ATTR(ch1, S_IRUGO, tm7705_get_sensor_value, NULL, TM7705_CH_1),
    SENSOR_ATTR(ch2, S_IRUGO, tm7705_get_sensor_value, NULL, TM7705_CH_2),
};
 
 
// printk("<7>  tm7705_1 jion\n");
static int __devinit tm7705_probe(struct spi_device *spi)
{
    struct tm7705 *adc;
    int i;
    int status;
printk(KERN_ERR" tm7705_1 jion\n");



 printk(KERN_WARNING "start 7705_1 2 \n");
    adc = kzalloc(sizeof *adc, GFP_KERNEL);
    if (!adc)
    {
        return -ENOMEM;
    }
 printk(KERN_ERR  "start 7705_1 3\n");
    mutex_init(&adc->lock);
    mutex_lock(&adc->lock);
    printk(KERN_ERR "start 7705_1 4 \n");
    spi_set_drvdata(spi, adc);
    printk(KERN_ERR  "start 7705_1 5 \n");
    for (i=0; i<TM7705_CHANNEL_NUM; i++)
    {
        status = device_create_file(&spi->dev, &ad_input[i].dev_attr);
       printk(KERN_ERR  "start 7705_1 6 \n");
       	if (status)
        {
            dev_err(&spi->dev, "device_create_file() failed.\n");
            goto fail_crete_file;
        }
	printk(KERN_ERR  "start 7705_1 7 \n");
    }
 printk(KERN_WARNING  "start 7705_1 8 \n");
    adc->hwmon_dev = hwmon_device_register(&spi->dev);
    printk(KERN_ERR  "start 7705_1 9 \n");
    if (IS_ERR(adc->hwmon_dev))
    {
        dev_err(&spi->dev, "hwmon_device_register() fail.\n");
        status = PTR_ERR(adc->hwmon_dev);
        goto fail_crete_file;
	printk(KERN_ERR  "start 7705_1 10 \n");
    }
 
    // gpio初始化
    status = gpio_request(TM7705_DRDY_PIN, "TM7705");   // tm7705 DRDY pin
    if (status)
    {
        dev_err(&spi->dev, "gpio_request(TM7705_DRDY_PIN) fail.\n");
        goto fail_device_register;
    }
    gpio_direction_input(TM7705_DRDY_PIN);
    status = gpio_request(TM7705_RESET_PIN, "TM7705");  // tm7705 reset pin
    if (status)
    {
        dev_err(&spi->dev, "gpio_request(TM7705_RESET_PIN) fail.\n");
        goto fail_request_drdy_pin;
    }
    gpio_direction_output(TM7705_RESET_PIN, 1);
 
    // 复位tm7705并重新配置
    tm7705_reset_and_reconfig(spi);
 
    mutex_unlock(&adc->lock);
    return 0;
 
fail_request_drdy_pin:
    gpio_free(TM7705_DRDY_PIN);
fail_device_register:
    hwmon_device_unregister(adc->hwmon_dev);
fail_crete_file:
    for (i--; i>=0; i--)
    {
        device_remove_file(&spi->dev, &ad_input[i].dev_attr);
    }
    spi_set_drvdata(spi, NULL);
    mutex_unlock(&adc->lock);
    kfree(adc);
    
    return status;
}
 
 
static int __devexit tm7705_remove(struct spi_device *spi)
{
    struct tm7705 *adc = spi_get_drvdata(spi);
    int i;
 
    mutex_lock(&adc->lock);
 
    gpio_free(TM7705_DRDY_PIN);
    gpio_free(TM7705_DRDY_PIN);
    hwmon_device_unregister(adc->hwmon_dev);
    for (i=0; i<TM7705_CHANNEL_NUM; i++)
    {
        device_remove_file(&spi->dev, &ad_input[i].dev_attr);
    }
    spi_set_drvdata(spi, NULL);
 
    mutex_unlock(&adc->lock);
    
    kfree(adc);
    return 0;
}
 
 
static struct spi_driver tm7705_driver = {
    .driver = {
        .name = "TM7705_1",
        .owner = THIS_MODULE,
    },
    .probe = tm7705_probe,
    .remove = __devexit_p(tm7705_remove),
};
 
 
 
static int __init init_tm7705(void)
{
    return spi_register_driver(&tm7705_driver);
}
 
 
static void __exit exit_tm7705(void)
{
    spi_unregister_driver(&tm7705_driver);
}
 
 
module_init(init_tm7705);
module_exit(exit_tm7705);
 
 
MODULE_AUTHOR("勤为本 1207280597@qq.com");
MODULE_DESCRIPTION("TM7705 linux driver");
MODULE_LICENSE("GPL");

