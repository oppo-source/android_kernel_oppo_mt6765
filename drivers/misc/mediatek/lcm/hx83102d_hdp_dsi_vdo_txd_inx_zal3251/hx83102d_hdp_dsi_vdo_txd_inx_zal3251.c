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

#define LOG_TAG "LCM_HX83102D_TXD_INX"

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
//extern int hx_tp_oppo6765;


#define HX83102D_TXD_INX_LCM_ID (0x15)
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

extern void lcd_queue_load_tp_fw(void);
extern int ili_tp;

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
    {0xB9, 3, {0x83,0x10,0x2D}},
    {0xC0, 11, {0x30,0x30,0x00,0x00,0x19,0x21,0x00,0x08,0x00,0x1A,0x1B}},
    {0xB1, 11, {0x22,0x44,0x2F,0x2F,0x22,0x44,0x43,0x57,0x0D,0x0D,0x0D}},
    {0xB2, 14, {0x00,0x00,0x06,0x40,0x00,0x08,0xEE,0x35,0x00,0x00,0x00,0x00,0x14,0xA0}},
    {0xB4, 14, {0x0C,0x54,0x0C,0x54,0x54,0x57,0x54,0x57,0x08,0xFF,0x03,0x00,0x00,0xFF}},
    {0xCC, 1, {0x02}},
    {0xD3, 25, {0x06,0x02,0x3C,0x01,0x00,0x0C,0x00,0x3B,0x37,0x34,0x37,0x0A,0x00,0x0A,0x00,0x32,0x10,0x08,0x00,0x00,0x54,0x16,0x4E,0x00,0x00}},
    {REGFLAG_DELAY, 5, {}},
    {0xD5, 44, {0x18,0x18,0x18,0x18,0x18,0x18,0x24,0x25,0x1A,0x1A,0x1B,0x1B,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x20,0x21,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18}},
    {REGFLAG_DELAY, 5, {}},
    {0xE7, 3, {0xFF,0x0D,0x01}},
    {0xBD, 1, {0x01}},
    {0xE7, 1, {0x01}},
    {0xBD, 1, {0x00}},
    {0xE0, 46, {0x00,0x02,0x07,0x0C,0x10,0x13,0x26,0x2A,0x30,0x2D,0x49,0x51,0x5A,0x6D,0x6F,0x7D,0x89,0xA3,0xA9,0x57,0x62,0x71,0x7F,0x00,0x02,0x07,0x0C,0x10,0x13,0x26,0x2A,0x30,0x2D,0x49,0x51,0x5A,0x6D,0x6F,0x7D,0x89,0xA3,0xA9,0x57,0x62,0x71,0x7F}},
    {REGFLAG_DELAY, 5, {}},
    {0xBA, 19, {0x70,0x23,0xA8,0x83,0xB2,0xC0,0xC0,0x01,0x10,0x00,0x00,0x00,0x0C,0x3D,0x82,0x77,0x04,0x01,0x00}},
    {0xC7, 6, {0x00,0xC2,0x00,0x10,0x42,0xC0}},
    {0xBF, 7, {0xFC,0x00,0x04,0x9E,0xF6,0x00,0x4D}},
    {0xCB, 5, {0x00,0x13,0x00,0x02,0x49}},
    {0xBD, 1, {0x01}},
    {0xCB, 1, {0x01}},
    {0xBD, 1, {0x02}},
    {0xB4, 8, {0x42,0x00,0x33,0x00,0x33,0x88,0xB3,0x00}},
    {0xB1, 3, {0x7F,0x03,0xFF}},
    {0xBD, 1, {0x00}},
    {0x35, 1, {0x00}},
    {0x11, 0, {0x11}},
    {REGFLAG_DELAY, 40, {}},
    {0x29, 0, {0x29}},
    {REGFLAG_DELAY, 50, {}},
    {0x51, 2, {0x00,0x00}},
    {0xC9, 4, {0x04,0x08,0x76,0x01}},
    {0x55, 1, {0x00}},
    {REGFLAG_DELAY, 10, {}},
    {0x53, 1, {0x24}},
    {0x35, 1,{0x00}},
};

static struct LCM_setting_table bl_level[] = {
    {0x51, 2, {0x0F, 0xFF} },
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table bl_level_dimming_exit[] = {
	{0x53, 1, {0x24}},
    {0x51, 2, {0x0F, 0xFF} },
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table lcm_cabc_enter_setting_ui[] = {
    {0x55, 1, {0x01}},
    {REGFLAG_DELAY, 10, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table lcm_cabc_enter_setting_still[] = {
    {0x55, 1, {0x02}},
    {REGFLAG_DELAY, 10, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table lcm_cabc_enter_setting_moving[] = {
    {0x55, 1, {0x03}},
    {REGFLAG_DELAY, 10, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
static struct LCM_setting_table lcm_cabc_exit_setting[] = {
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
    params->dsi.vertical_backporch = 10;
    params->dsi.vertical_frontporch = 240;
    //params->dsi.vertical_frontporch_for_low_power = 540;
    params->dsi.vertical_active_line = FRAME_HEIGHT;

    params->dsi.horizontal_sync_active = 19;
    params->dsi.horizontal_backporch = 19;
    params->dsi.horizontal_frontporch = 20;
    params->dsi.horizontal_active_pixel = FRAME_WIDTH;
    params->dsi.ssc_disable = 1;
    /* this value must be in MTK suggested table */
    params->dsi.PLL_CLOCK = 276;
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
        params->dsi.esd_check_enable = 1;
        params->dsi.customization_esd_check_enable = 1;
        params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
        params->dsi.lcm_esd_check_table[0].count = 1;
        params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9D;
    }
	params->blmap = blmap_table;
	params->blmap_size = sizeof(blmap_table)/sizeof(blmap_table[0]);
	params->brightness_max = 4095;
	params->brightness_min = 10;
	register_device_proc("lcd", "hx83102d", "TXD_INX_THREE");

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
		disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT0);
		MDELAY(2);
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

static void lcm_init(void)
{
    int size;
	LCM_LOGI("%s: enter\n", __func__);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(5);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT0);
	MDELAY(2);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(50);
    //if(hx_tp_oppo6765)
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
	MDELAY(5);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT0);
	MDELAY(2);
	disp_dts_gpio_select_state(DTS_GPIO_STATE_LCM_RST_OUT1);
	MDELAY(50);
	//if(hx_tp_oppo6765)
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
	LCM_LOGI("%s: exit\n", __func__);
}

#ifdef BUILD_LK
static unsigned int lcm_compare_id(void)
{
}
#endif

static void lcm_setbacklight_cmdq(void *handle, unsigned int level)
{

    LCM_LOGI("%s,hx83102d_txd_inx backlight: level = %d\n", __func__, level);
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
    printk("%s [lcd] cabc_mode is %d \n", __func__, level);
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
LCM_DRIVER hx83102d_hdp_dsi_vdo_txd_inx_zal3251_lcm_drv = {
    .name = "hx83102d_txd_inx",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params = lcm_get_params,
    .init = lcm_init,
    .suspend = lcm_suspend,
    .resume = lcm_resume,
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
