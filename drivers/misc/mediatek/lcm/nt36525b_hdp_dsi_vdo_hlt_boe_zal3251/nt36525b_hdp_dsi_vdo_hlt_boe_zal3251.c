/********************************************
 ** Copyright (C) 2019 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: ili9882n_hdp_dsi_vdo_ls_panda_zal3251.c
 ** Description: Source file for LCD driver
 **          To Control LCD driver
 ** Version :1.0
 ** Date : 2020/08/31
 ** Author: wangcheng@ODM_HQ.Multimedia.LCD
 ** ---------------- Revision History: --------------------------
 ** <version>    <date>          < author >              <desc>
 **  1.0           2020/08/31   wangcheng@ODM_HQ   Source file for LCD driver
 ********************************************/

#define LOG_TAG "LCM_NT36525B_HLT_BOE"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#include <mt-plat/mtk_boot_common.h>
#endif


#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#include <platform/boot_mode.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include "disp_dts_gpio.h"
#endif

#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_info("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_info("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

//extern int gesture_flag;

#define NT36525B_HLT_BOE_LCM_ID (0x80)
#include "disp_dts_gpio.h"
static const unsigned int BL_MIN_LEVEL = 20;
#ifndef BUILD_LK
typedef struct LCM_UTIL_FUNCS LCM_UTIL_FUNCS;
typedef struct LCM_PARAMS LCM_PARAMS;
typedef struct LCM_DRIVER LCM_DRIVER;
#endif
static LCM_UTIL_FUNCS lcm_util;

struct NT5038_SETTING_TABLE {
	unsigned char cmd;
	unsigned char data;
};

static struct NT5038_SETTING_TABLE nt5038_cmd_data[3] = {
	{ 0x00, 0x14 },
	{ 0x01, 0x14 },
	{ 0x03, 0x73 }
};

#define LCM_RESET_PIN                                       (GPIO45|0x80000000)
#ifndef GPIO_LCD_BIAS_ENP_PIN
#define GPIO_LCD_BIAS_ENP_PIN                               (GPIO166|0x80000000)
#endif
#ifndef GPIO_LCD_BIAS_ENN_PIN
#define GPIO_LCD_BIAS_ENN_PIN                               (GPIO165|0x80000000)
#endif

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <soc/oppo/device_info.h>

#define I2C_I2C_LCD_BIAS_CHANNEL 3
#define DCDC_I2C_BUSNUM  I2C_I2C_LCD_BIAS_CHANNEL
#define DCDC_I2C_ID_NAME "nt5038"
#define DCDC_I2C_ADDR 0x3E

#if defined(CONFIG_MTK_LEGACY)
static struct i2c_board_info __initdata nt5038_board_info = {I2C_BOARD_INFO(DCDC_I2C_ID_NAME, DCDC_I2C_ADDR)};
#else
static const struct of_device_id lcm_of_match[] = {
	{.compatible = "mediatek,I2C_LCD_BIAS"},
	{},
};
#endif
extern struct i2c_client *nt5038_i2c_client;

static int nt5038_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int nt5038_remove(struct i2c_client *client);

struct nt5038_dev	{
	struct i2c_client	*client;
};
static const struct i2c_device_id nt5038_id[] = {
	{ DCDC_I2C_ID_NAME, 0 },
	{ }
};

static struct i2c_driver nt5038_iic_driver = {
	.id_table	= nt5038_id,
	.probe		= nt5038_probe,
	.remove		= nt5038_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "nt5038",
#if !defined(CONFIG_MTK_LEGACY)
		.of_match_table = lcm_of_match,
#endif
	},
};

static int nt5038_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	nt5038_i2c_client  = client;
	return 0;
}
static int nt5038_remove(struct i2c_client *client)
{
	nt5038_i2c_client = NULL;
	i2c_unregister_device(client);
	return 0;
}
static int nt5038_i2c_write_byte(unsigned char addr, unsigned char value)
{
	int ret = 0;
	struct i2c_client *client = nt5038_i2c_client;
	char write_data[2]={0};
	if(client == NULL)
	{
		LCM_LOGI("ERROR!!nt5038_i2c_client is null\n");
		return 0;
	}
	write_data[0]= addr;
	write_data[1] = value;
	ret=i2c_master_send(client, write_data, 2);
	if(ret<0)
		LCM_LOGI("nt5038 write data fail !!\n");
	return ret ;
}
static int __init nt5038_iic_init(void)
{
#if defined(CONFIG_MTK_LEGACY)
	i2c_register_board_info(DCDC_I2C_BUSNUM, &nt5038_board_info, 1);
#endif
	i2c_add_driver(&nt5038_iic_driver);
	return 0;
}
static void __exit nt5038_iic_exit(void)
{
	i2c_del_driver(&nt5038_iic_driver);
}
module_init(nt5038_iic_init);
module_exit(nt5038_iic_exit);
MODULE_AUTHOR("cheng.wang <wangcheng7@huaqin.com>");
MODULE_DESCRIPTION("MTK LCD BIAS I2C Driver");
MODULE_LICENSE("GPL");
#else
#define NT5038_SLAVE_ADDR_WRITE  0x7C
#define I2C_I2C_LCD_BIAS_CHANNEL 3
static struct mt_i2c_t NT5038_i2c;
static int nt5038_i2c_write_byte(kal_uint8 addr, kal_uint8 value)
{
	kal_uint32 ret_code = I2C_OK;
	kal_uint8 write_data[2];
	kal_uint16 len;
	write_data[0]= addr;
	write_data[1] = value;
	NT5038_i2c.id = I2C_I2C_LCD_BIAS_CHANNEL;
	NT5038_i2c.addr = (NT5038_SLAVE_ADDR_WRITE >> 1);
	NT5038_i2c.mode = ST_MODE;
	NT5038_i2c.speed = 100;
	len = 2;
	ret_code = i2c_write(&NT5038_i2c, write_data, len);
	LCM_LOGI("%s: i2c_write: addr:0x%x, value:0x%x ret_code: %d\n", __func__, addr, value, ret_code);
	return ret_code;
}
#endif
#define MDELAY(n)        (lcm_util.mdelay(n))
#define UDELAY(n)        (lcm_util.udelay(n))


#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
    lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
        lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
        lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
      lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
        lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#ifdef OPLUS_BUG_STABILITY
#define SET_LCD_BIAS_EN(en, seq, value)                           lcm_util.set_lcd_bias_en(en, seq, value)
#endif

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <soc/oppo/device_info.h>
#endif



/* static unsigned char lcd_id_pins_value = 0xFF; */
static const unsigned char LCD_MODULE_ID = 0x01;
static int cabc_lastlevel = 1;
//extern void lcd_queue_load_tp_fw(void);

#define LCM_DSI_CMD_MODE    0
#define FRAME_WIDTH        (720)
#define FRAME_HEIGHT    (1600)
#define LCM_DENSITY        (320)

#define LCM_PHYSICAL_WIDTH        (67932)
#define LCM_PHYSICAL_HEIGHT        (150960)

#define REGFLAG_DELAY        0xFFFC
#define REGFLAG_UDELAY    0xFFFB
#define REGFLAG_END_OF_TABLE    0xFFFD
#define REGFLAG_RESET_LOW    0xFFFE
#define REGFLAG_RESET_HIGH    0xFFFF

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//extern int nvt_tp_oppo6765;
struct LCM_setting_table {
    unsigned int cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static int blmap_table[] = {
	48,17,
	29,23,
	25,26,
	30,23,
	35,24,
	34,28,
	39,26,
	40,25,
	42,21,
	41,23,
	44,19,
	44,19,
	50,3,
	51,7,
	52,9,
	58,34,
	58,33,
	64,65,
	61,48,
	71,106,
	68,87,
	70,100,
	77,146,
	78,146,
	87,211,
	88,219,
	87,210,
	88,213,
	125,542,
	96,272,
	91,221,
	145,757,
	122,522,
	183,1166,
	122,502,
	155,872,
	161,943,
	151,823,
	190,1300,
	164,975,
	264,2255,
	152,785,
	277,2467,
	187,1228,
	239,1961,
	203,1440,
	242,2015,
	271,2446,
	284,2645,
	310,3045,
	303,2936,
	338,3510,
	329,3357,
	374,4123,
	371,4074,
	387,4357,
	352,3720,
	493,6294,
	445,5407,
	477,6015,
	490,6255,
	516,6760,
	584,8110,
	326,2906,
};

static struct LCM_setting_table lcm_suspend_setting[] = {
    {0x28, 0, {} },
    {REGFLAG_DELAY, 10, {} },
    {0x10, 0, {} },
    {REGFLAG_DELAY, 60, {} },
};

static struct LCM_setting_table init_setting_vdo[] = {
	{0xFF,1,{0x20}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0x01,1,{0x55}},
	{0x03,1,{0x55}},
	{0x05,1,{0x99}},
	{0x07,1,{0x4B}},
	{0x08,1,{0xDF}},
	{0x0E,1,{0x69}},
	{0x0F,1,{0x7D}},
	{0x94,1,{0x40}},
	{0x95,1,{0xF5}},
	{0x96,1,{0x1D}},
	{0x69,1,{0x91}},
	{0x1F,1,{0x02}},
	//CMD2_Page0
	{0xFF,1,{0x20}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0xB0,16,{0x00,0x0A,0x00,0x0A,0x00,0x2D,0x00,0x42,0x00,0x55,0x00,0x68,0x00,0x78,0x00,0x87}},
	{0xB1,16,{0x00,0x95,0x00,0xC7,0x00,0xEE,0x01,0x30,0x01,0x60,0x01,0xB3,0x01,0xF2,0x01,0xF4}},
	{0xB2,16,{0x02,0x34,0x02,0x78,0x02,0xA8,0x02,0xE0,0x03,0x09,0x03,0x36,0x03,0x47,0x03,0x57}},
	{0xB3,12,{0x03,0x6A,0x03,0x81,0x03,0x9E,0x03,0xC0,0x03,0xDA,0x03,0xDE}},
	{0xB4,16,{0x00,0x0A,0x00,0x0A,0x00,0x2D,0x00,0x42,0x00,0x55,0x00,0x68,0x00,0x78,0x00,0x87}},
	{0xB5,16,{0x00,0x95,0x00,0xC7,0x00,0xEE,0x01,0x30,0x01,0x60,0x01,0xB3,0x01,0xF2,0x01,0xF4}},
	{0xB6,16,{0x02,0x34,0x02,0x78,0x02,0xA8,0x02,0xE0,0x03,0x09,0x03,0x36,0x03,0x47,0x03,0x57}},
	{0xB7,12,{0x03,0x6A,0x03,0x81,0x03,0x9E,0x03,0xC0,0x03,0xDA,0x03,0xDE}},
	{0xB8,16,{0x00,0x0A,0x00,0x0A,0x00,0x2D,0x00,0x42,0x00,0x55,0x00,0x68,0x00,0x78,0x00,0x87}},
	{0xB9,16,{0x00,0x95,0x00,0xC7,0x00,0xEE,0x01,0x30,0x01,0x60,0x01,0xB3,0x01,0xF2,0x01,0xF4}},
	{0xBA,16,{0x02,0x34,0x02,0x78,0x02,0xA8,0x02,0xE0,0x03,0x09,0x03,0x36,0x03,0x47,0x03,0x57}},
	{0xBB,12,{0x03,0x6A,0x03,0x81,0x03,0x9E,0x03,0xC0,0x03,0xDA,0x03,0xDE}},
    //CMD2_Page1
	{0xFF,1,{0x21}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0xB0,16,{0x00,0x02,0x00,0x02,0x00,0x25,0x00,0x3A,0x00,0x4D,0x00,0x60,0x00,0x70,0x00,0x7F}},
	{0xB1,16,{0x00,0x8D,0x00,0xBF,0x00,0xE6,0x01,0x28,0x01,0x58,0x01,0xAB,0x01,0xEA,0x01,0xEC}},
	{0xB2,16,{0x02,0x2C,0x02,0x70,0x02,0xA0,0x02,0xD8,0x03,0x01,0x03,0x2E,0x03,0x3F,0x03,0x4F}},
	{0xB3,12,{0x03,0x62,0x03,0x79,0x03,0x96,0x03,0xB8,0x03,0xD2,0x03,0xD6}},
	{0xB4,16,{0x00,0x02,0x00,0x02,0x00,0x25,0x00,0x3A,0x00,0x4D,0x00,0x60,0x00,0x70,0x00,0x7F}},
	{0xB5,16,{0x00,0x8D,0x00,0xBF,0x00,0xE6,0x01,0x28,0x01,0x58,0x01,0xAB,0x01,0xEA,0x01,0xEC}},
	{0xB6,16,{0x02,0x2C,0x02,0x70,0x02,0xA0,0x02,0xD8,0x03,0x01,0x03,0x2E,0x03,0x3F,0x03,0x4F}},
	{0xB7,12,{0x03,0x62,0x03,0x79,0x03,0x96,0x03,0xB8,0x03,0xD2,0x03,0xD6}},
	{0xB8,16,{0x00,0x02,0x00,0x02,0x00,0x25,0x00,0x3A,0x00,0x4D,0x00,0x60,0x00,0x70,0x00,0x7F}},
	{0xB9,16,{0x00,0x8D,0x00,0xBF,0x00,0xE6,0x01,0x28,0x01,0x58,0x01,0xAB,0x01,0xEA,0x01,0xEC}},
	{0xBA,16,{0x02,0x2C,0x02,0x70,0x02,0xA0,0x02,0xD8,0x03,0x01,0x03,0x2E,0x03,0x3F,0x03,0x4F}},
	{0xBB,12,{0x03,0x62,0x03,0x79,0x03,0x96,0x03,0xB8,0x03,0xD2,0x03,0xD6}},
	{0xFF,1,{0x23}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0x12,1,{0xB4}},
	{0x15,1,{0xE9}},
	{0x16,1,{0x0B}},
	{0x00,1,{0x80}},
	{0x07,1,{0x00}},
	{0x08,1,{0x01}},
// CABC Setting
// APL_WT
	{ 0x10,1,{0x80}},
// APL_THD
	{ 0x11,1,{0x00}},
	{ 0x12,1,{0xB4}},
// APL_COMP
	{ 0x15,1,{0xE9}},
	{ 0x16,1,{0x0B}},
// GAMMACMP
	{ 0x29,1,{0x10}},
	{ 0x2A,1,{0x20}},
	{ 0x2B,1,{0x30}},
// CABC_PWM_UI
	{ 0x30,1,{0xFA}},
	{0x31, 1, {0xF2}},
	{0x32, 1, {0xE7}},
	{0x33, 1, {0xE7}},
	{0x34, 1, {0xE7}},
	{0x35, 1, {0xE7}},
	{0x36, 1, {0xE7}},
	{0x37, 1, {0xE7}},
	{0x38, 1, {0xE7}},
	{ 0x39,1,{0xE0}},
	{ 0x3A,1,{0xDC}},
	{ 0x3B,1,{0xD8}},
	{ 0x3D,1,{0xD5}},
	{ 0x3F,1,{0xD2}},
	{0x40, 1, {0xD2}},
	{0x41, 1, {0xD2}},
// CABC_STILL_PWM
	{0x45, 1, {0xE4}},
	{0x46, 1, {0xD4}},
	{0x47, 1, {0xB8}},
	{0x48, 1, {0xB8}},
	{0x49, 1, {0xB8}},
	{0x4A, 1, {0xB8}},
	{0x4B, 1, {0xB8}},
	{0x4C, 1, {0xB6}},
	{0x4D, 1, {0xB6}},
	{0x4E, 1, {0xB4}},
	{0x4F, 1, {0xAD}},
	{ 0x50,1,{0xA4}},
	{0x51, 1, {0xA3}},
	{0x52, 1, {0xA3}},
	{0x53, 1, {0xA2}},
	{0x54, 1, {0xA2}},
// CABC_PWM_MOV
	{0x58, 1, {0xD8}},
	{0x59, 1, {0xC8}},
	{0x5A, 1, {0xA4}},
	{0x5B, 1, {0xA4}},
	{0x5C, 1, {0xA4}},
	{0x5D, 1, {0xA4}},
	{0x5E, 1, {0xA4}},
	{0x5F, 1, {0xA4}},
	{0x60, 1, {0xA4}},
	{0x61, 1, {0xA3}},
	{0x62, 1, {0xA3}},
	{0x63, 1, {0x90}},
	{0x64, 1, {0x84}},
	{0x65, 1, {0x84}},
	{0x66, 1, {0x78}},
	{0x67, 1, {0x76}},

	{0xFF,1,{0x24}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0x93,1,{0x0A}},
	{0x94,1,{0x0E}},
	{0x00,1,{0x00}},
	{0x01,1,{0x1E}},
	{0x02,1,{0x1D}},
	{0x03,1,{0x21}},
	{0x04,1,{0x04}},
	{0x05,1,{0x05}},
	{0x06,1,{0x1F}},
	{0x07,1,{0x20}},
	{0x08,1,{0x0C}},
	{0x09,1,{0x0D}},
	{0x0A,1,{0x0E}},
	{0x0B,1,{0x0F}},
	{0x0C,1,{0x00}},
	{0x0D,1,{0x00}},
	{0x0E,1,{0x00}},
	{0x0F,1,{0x00}},
	{0x10,1,{0x00}},
	{0x11,1,{0x00}},
	{0x12,1,{0x00}},
	{0x13,1,{0x00}},
	{0x14,1,{0x00}},
	{0x15,1,{0x00}},
	{0x16,1,{0x00}},
	{0x17,1,{0x1E}},
	{0x18,1,{0x1D}},
	{0x19,1,{0x21}},
	{0x1A,1,{0x04}},
	{0x1B,1,{0x05}},
	{0x1C,1,{0x1F}},
	{0x1D,1,{0x20}},
	{0x1E,1,{0x0C}},
	{0x1F,1,{0x0D}},
	{0x20,1,{0x0E}},
	{0x21,1,{0x0F}},
	{0x22,1,{0x00}},
	{0x23,1,{0x00}},
	{0x24,1,{0x00}},
	{0x25,1,{0x00}},
	{0x26,1,{0x00}},
	{0x27,1,{0x00}},
	{0x28,1,{0x00}},
	{0x29,1,{0x00}},
	{0x2A,1,{0x00}},
	{0x2B,1,{0x00}},
	{0x2F,1,{0x08}},
	{0x30,1,{0x06}},
	{0x31,1,{0x00}},
	{0x32,1,{0x00}},
	{0x33,1,{0x06}},
	{0x34,1,{0x08}},
	{0x35,1,{0x00}},
	{0x36,1,{0x00}},
	{0x37,1,{0x44}},
	{0x38,1,{0x44}},
	{0x39,1,{0x00}},
	{0x3A,1,{0x24}},
	{0x3B,1,{0x90}},
	{0x3D,1,{0x52}},
	{0x4D,1,{0x34}},
	{0x4E,1,{0x12}},
	{0x4F,1,{0x00}},
	{0x50,1,{0x00}},
	{0x51,1,{0x21}},
	{0x52,1,{0x43}},
	{0x53,1,{0x00}},
	{0x54,1,{0x00}},
	{0x55,1,{0x47}},
	{0x56,1,{0x44}},
	{0x58,1,{0x21}},
	{0x59,1,{0x00}},
	{0x5A,1,{0x24}},
	{0x5B,1,{0x90}},
	{0x5C,1,{0x88}},
	{0x5D,1,{0x08}},
	{0x5E,1,{0x04}},
	{0x5F,1,{0x00}},
	{0x88,1,{0x2E}},
	{0x89,1,{0xC8}},
	{0x8A,1,{0x88}},
	{0x8D,1,{0x88}},
	{0x8E,1,{0x08}},
	{0x60,1,{0x80}},
	{0x61,1,{0x90}},
	{0x64,1,{0x10}},
	{0x80,1,{0x0A}},
	{0x81,1,{0x06}},
	{0x82,1,{0x0A}},
	{0x83,1,{0x06}},
	{0x84,1,{0x80}},
	{0x85,1,{0x01}},
	{0x86,1,{0x04}},
	{0x91,1,{0x40}},
	{0x92,1,{0xB3}},
	{0x98,1,{0x80}},
	{0xAB,1,{0x00}},
	{0xAD,1,{0x00}},
	{0xAF,1,{0x04}},
	{0xB0,1,{0x05}},
	{0xB1,1,{0xAF}},
	{0xFF,1,{0x25}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0x17,1,{0x82}},
	{0x18,1,{0x06}},
	{0x19,1,{0x0F}},
	{0xC0,1,{0x07}},
	{0xC2,1,{0x84}},
	{0xC6,1,{0x92}},
	{0xCA,1,{0x1C}},
	{0xCC,1,{0x1C}},
	{0xCD,1,{0x00}},
	{0xCE,1,{0x00}},
	{0xD3,1,{0x11}},
	{0xD4,1,{0xCC}},
	{0xD5,1,{0x11}},
	{0xD6,1,{0x1C}},
	{0xD7,1,{0x11}},
	{0x0A,1,{0x82}},
	{0x0B,1,{0x25}},
	{0x0C,1,{0x01}},
	{0xFF,1,{0x26}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0x00,1,{0xA0}},
	{0xFF,1,{0x27}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0x13,1,{0x00}},
	{0x15,1,{0xB4}},
	{0x1F,1,{0x55}},
	{0x26,1,{0x0F}},
	{0xC0,1,{0x18}},
	{0xC1,1,{0xF2}},
	{0xC7,1,{0x03}},
	{0xC2,1,{0x00}},
	{0xC3,1,{0x00}},
	{0xC4,1,{0xF2}},
	{0xC5,1,{0x00}},
	{0xC6,1,{0x00}},
	{0xFF,1,{0x10}},
	{REGFLAG_DELAY,1,{}},
	{0xFB,1,{0x01}},
	{0xBA,1,{0x03}},
	{0x53,1,{0x24}},
	{0x51,2,{0x00,0x00}},
	{0x68,2,{0x02,0x01}},
	{0x35,1,{0x00}},
	{0x29,1,{0x00}},
	{REGFLAG_DELAY,20,{}},
	{0x11,1,{0x00}},
	{REGFLAG_DELAY,100,{}},

};

static struct LCM_setting_table bl_level[] = {
    {0x51, 2, {0x0F, 0xFF} },
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table bl_level_dimming_exit[] = {
	{0x53, 1, {0x24}},
	{0x51, 3, {0x0F, 0xFF, 0x00} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table lcm_cabc_enter_setting_ui[] = {
	{0xFF, 1, {0x10}},
	{0xFB, 1, {0x01}},
	{0x55, 1, {0x01}},
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table lcm_cabc_enter_setting_still[] = {
	{0xFF, 1, {0x10}},
	{0xFB, 1, {0x01}},
	{0x55, 1, {0x02}},
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table lcm_cabc_enter_setting_moving[] = {
	{0xFF, 1, {0x10}},
	{0xFB, 1, {0x01}},
	{0x55, 1, {0x03}},
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table lcm_cabc_exit_setting[] = {
	{0xFF, 1, {0x10}},
	{0xFB, 1, {0x01}},
	{0x55, 1, {0x00}},
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};

static void push_table(void *cmdq, struct LCM_setting_table *table,
    unsigned int count, unsigned char force_update)
{
    unsigned int i;
    unsigned int cmd;

    for (i = 0; i < count; i++) {
        cmd = table[i].cmd;

        switch (cmd) {

        case REGFLAG_DELAY:
            if (table[i].count <= 10)
                MDELAY(table[i].count);
            else
                MDELAY(table[i].count);
            break;

        case REGFLAG_UDELAY:
            UDELAY(table[i].count);
            break;

        case REGFLAG_END_OF_TABLE:
            break;

        default:
            dsi_set_cmdq_V22(cmdq, cmd,
                table[i].count,
                table[i].para_list,
                force_update);
        }
    }
}


static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}



static void lcm_get_params(LCM_PARAMS *params)
{
    int boot_mode = 0;
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type = LCM_TYPE_DSI;

    params->width = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    params->physical_width = LCM_PHYSICAL_WIDTH/1000;
    params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
    params->physical_width_um = LCM_PHYSICAL_WIDTH;
    params->physical_height_um = LCM_PHYSICAL_HEIGHT;
//    params->density = LCM_DENSITY;

    params->dsi.mode = SYNC_PULSE_VDO_MODE;
    params->dsi.switch_mode_enable = 0;


    params->dsi.LANE_NUM = LCM_FOUR_LANE;
    /* The following defined the fomat for data coming from LCD engine. */
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

    /* Highly depends on LCD driver capability. */
    params->dsi.packet_size = 256;
    /* video mode timing */
    params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active = 2;
    params->dsi.vertical_backporch = 254;
    params->dsi.vertical_frontporch = 10;
    params->dsi.vertical_active_line = FRAME_HEIGHT;

    params->dsi.horizontal_sync_active = 4;
    params->dsi.horizontal_backporch = 100;
    params->dsi.horizontal_frontporch = 101;
    params->dsi.horizontal_active_pixel = FRAME_WIDTH;
    params->dsi.ssc_disable = 1;
    /* this value must be in MTK suggested table */
    params->dsi.PLL_CLOCK = 328;
    /* clk continuous video mode */
    params->dsi.cont_clock = 0;

    params->dsi.clk_lp_per_line_enable = 0;
    if (get_boot_mode() == META_BOOT) {
        boot_mode++;
        LCM_LOGI("META_BOOT\n");
    }
    if (get_boot_mode() == ADVMETA_BOOT) {
        boot_mode++;
        LCM_LOGI("ADVMETA_BOOT\n");
    }
    if (get_boot_mode() == ATE_FACTORY_BOOT) {
        boot_mode++;
        LCM_LOGI("ATE_FACTORY_BOOT\n");
    }
    if (get_boot_mode() == FACTORY_BOOT)     {
        boot_mode++;
        LCM_LOGI("FACTORY_BOOT\n");
    }
    if (boot_mode == 0) {
        LCM_LOGI("neither META_BOOT or FACTORY_BOOT\n");
        params->dsi.esd_check_enable = 0;
        params->dsi.customization_esd_check_enable = 0;
        params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
        params->dsi.lcm_esd_check_table[0].count = 1;
        params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
    }
	
	params->blmap = blmap_table;
	params->blmap_size = sizeof(blmap_table)/sizeof(blmap_table[0]);
	params->brightness_max = 4095;
	params->brightness_min = 10;
	register_device_proc("lcd", "nt36525b", "HLT_BOE");

}

static void lcm_init_power(void)
{
	LCM_LOGI("%s: enter\n", __func__);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCD_BIAS_ENP1);
	MDELAY(5);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCD_BIAS_ENN1);
	MDELAY(5);
	nt5038_i2c_write_byte(nt5038_cmd_data[0].cmd, nt5038_cmd_data[0].data);
	MDELAY(1);
	nt5038_i2c_write_byte(nt5038_cmd_data[1].cmd, nt5038_cmd_data[1].data);
	MDELAY(1);
	nt5038_i2c_write_byte(nt5038_cmd_data[2].cmd, nt5038_cmd_data[2].data);
	MDELAY(1);

	LCM_LOGI("%s: exit\n", __func__);
}

static void lcm_suspend_power(void)
{
	LCM_LOGI("%s: enter\n", __func__);
	//if(!gesture_flag) 
	{
		disp_dts_gpio_select_state(DTS_GPIO_STATE_LCD_BIAS_ENN0);
		MDELAY(5);
		disp_dts_gpio_select_state(DTS_GPIO_STATE_LCD_BIAS_ENP0);
		MDELAY(5);
	}
	LCM_LOGI("%s: exit\n", __func__);
}
#ifdef OPLUS_BUG_STABILITY
static void lcm_shudown_power(void)
{
    LCM_LOGI("%s: enter\n", __func__);
    disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT0);
    MDELAY(2);
    disp_dts_gpio_select_state(DTS_GPIO_STATE_LCD_BIAS_ENN0);
    MDELAY(2);
    disp_dts_gpio_select_state(DTS_GPIO_STATE_LCD_BIAS_ENP0);
    LCM_LOGI("%s: exit\n", __func__);
}
#endif

static void lcm_resume_power(void)
{
	LCM_LOGI("%s: enter\n", __func__);
	//if(!gesture_flag) 
	{
		disp_dts_gpio_select_state(DTS_GPIO_STATE_LCD_BIAS_ENP1);
		MDELAY(5);
		disp_dts_gpio_select_state(DTS_GPIO_STATE_LCD_BIAS_ENN1);
		MDELAY(5);
		nt5038_i2c_write_byte(nt5038_cmd_data[0].cmd, nt5038_cmd_data[0].data);
		MDELAY(1);
		nt5038_i2c_write_byte(nt5038_cmd_data[1].cmd, nt5038_cmd_data[1].data);
		MDELAY(1);
		nt5038_i2c_write_byte(nt5038_cmd_data[2].cmd, nt5038_cmd_data[2].data);
		MDELAY(1);
	}
	LCM_LOGI("%s: exit\n", __func__);
}
/*
static void lcm_hw_reset(void)
{
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(2);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT0);
	MDELAY(10);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(2);
}
*/
static void lcm_init(void)
{
    int size;
	LCM_LOGI("%s: enter\n", __func__);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(5);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT0);
	MDELAY(10);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(10);
	//if(nvt_tp_oppo6765)
	//lcd_queue_load_tp_fw();
    size = sizeof(init_setting_vdo) /
        sizeof(struct LCM_setting_table);
    push_table(NULL, init_setting_vdo, size, 1);

	LCM_LOGI("%s: exit\n", __func__);
}

static void lcm_suspend(void)
{
    LCM_LOGI("%s: enter\n", __func__);
    push_table(NULL, lcm_suspend_setting,
        sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table),
        1);
    MDELAY(10);

    LCM_LOGI("%s: exit\n", __func__);
}

static void lcm_resume(void)
{
	int size;
	LCM_LOGI("%s: enter\n", __func__);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(2);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT0);
	MDELAY(10);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(10);
	//if(nvt_tp_oppo6765)
	//lcd_queue_load_tp_fw();
	size = sizeof(init_setting_vdo) /
        sizeof(struct LCM_setting_table);
    push_table(NULL, init_setting_vdo, size, 1);
	switch (cabc_lastlevel) {
		case 1 :
			push_table(NULL, lcm_cabc_enter_setting_ui, sizeof(lcm_cabc_enter_setting_ui) / sizeof(struct LCM_setting_table), 1);
	        break;
		case 2 :
			push_table(NULL, lcm_cabc_enter_setting_still, sizeof(lcm_cabc_enter_setting_still) / sizeof(struct LCM_setting_table), 1);
	        break;
		case 3 :
			push_table(NULL, lcm_cabc_enter_setting_moving, sizeof(lcm_cabc_enter_setting_moving) / sizeof(struct LCM_setting_table), 1);
		    break;
	}
}

#ifdef BUILD_LK
static unsigned int lcm_compare_id(void)
{
}
#endif

static void lcm_setbacklight_cmdq(void *handle, unsigned int level)
{

    LCM_LOGI("%s,nt36525b_hlt_boe backlight: level = %d\n", __func__, level);
	if(level == 3768) {
		level = 3767;
	}
	if (level == 0){
		bl_level_dimming_exit[1].para_list[0] = (level >> 8) & 0x0F;
		bl_level_dimming_exit[1].para_list[1] = level & 0xFF;
        push_table(handle,
			bl_level_dimming_exit,
			sizeof(bl_level_dimming_exit) / sizeof(struct LCM_setting_table),
			1);
	}
	else{
		if (level > 4095){
			level = 4095;
		}else if(level > 0 && level < 10){
			level = 10;
		}

		bl_level[0].para_list[0] = (level >> 8) & 0x0F;
		bl_level[0].para_list[1] = level & 0xFF;
    	push_table(handle,
        	bl_level,
        	sizeof(bl_level) / sizeof(struct LCM_setting_table),
        	1);
		}
}

static void lcm_set_cabc_mode_cmdq(void *handle, unsigned int level)
{
    LCM_LOGI("%s [lcd] nt36525b_hlt_boe cabc_mode is %d \n", __func__, level);

	if (level == 1) {
        push_table(handle,lcm_cabc_enter_setting_ui, sizeof(lcm_cabc_enter_setting_ui) / sizeof(struct LCM_setting_table), 1);
    } else if (level == 2) {
        push_table(handle,lcm_cabc_enter_setting_still, sizeof(lcm_cabc_enter_setting_still) / sizeof(struct LCM_setting_table), 1);
    } else if (level == 3) {
        push_table(handle,lcm_cabc_enter_setting_moving, sizeof(lcm_cabc_enter_setting_moving) / sizeof(struct LCM_setting_table), 1);
    } else {
        push_table(handle,lcm_cabc_exit_setting, sizeof(lcm_cabc_exit_setting) / sizeof(struct LCM_setting_table), 1);
    }
    if (level > 0) {
        cabc_lastlevel = level;
    }
}
LCM_DRIVER nt36525b_hdp_dsi_vdo_hlt_boe_zal3251_lcm_drv = {
    .name = "nt36525b_hlt_boe",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params = lcm_get_params,
    .init = lcm_init,
    .suspend = lcm_suspend,
    .resume = lcm_resume,
    //.hw_reset_before_lp11 = lcm_hw_reset, // for lcm bias_on&hw_reset before mipi dsi goes to LP11
#ifdef BUILD_LK
    .compare_id = lcm_compare_id,
#endif
    .init_power = lcm_init_power,
    .suspend_power = lcm_suspend_power,
#ifdef OPLUS_BUG_STABILITY
    .shutdown_power = lcm_shudown_power,
#endif
    .resume_power = lcm_resume_power,
    .set_backlight_cmdq = lcm_setbacklight_cmdq,
    .set_cabc_mode_cmdq = lcm_set_cabc_mode_cmdq,
};
