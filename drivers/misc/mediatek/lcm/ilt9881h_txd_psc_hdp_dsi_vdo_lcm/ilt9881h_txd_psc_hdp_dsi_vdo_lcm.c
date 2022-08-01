/***********************************************************
** Copyright (C), 2008-2016, OPPO Mobile Comm Corp., Ltd.
** ODM_WT_EDIT
** File: - ili9881h_txd_hdp_dsi_vdo_lcm.c
** Description: source file for lcm ili9881h+txd in kernel stage
**
** Version: 1.0
** Date : 2019/9/25
**
** ------------------------------- Revision History: -------------------------------
**  	<author>		<data> 	   <version >	       <desc>
**  lianghao       2019/9/25     1.0     source file for lcm ili9881h+txd in kernel stage
**
****************************************************************/

#define LOG_TAG "LCM"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include "disp_dts_gpio.h"
#endif

//#ifdef ODM_WT_EDIT
//#include <linux/update_tpfw_notifier.h>
//#endif
//#include "disp_cust.h"
//#include <soc/oppo/device_info.h>
static struct LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)	(lcm_util.set_reset_pin((v)))
#define SET_LCM_VDD18_PIN(v)	(lcm_util.set_gpio_lcm_vddio_ctl((v)))
#define SET_LCM_VSP_PIN(v)	(lcm_util.set_gpio_lcd_enp_bias((v)))
#define SET_LCM_VSN_PIN(v)	(lcm_util.set_gpio_lcd_enn_bias((v)))
#define MDELAY(n)		(lcm_util.mdelay(n))
#define UDELAY(n)		(lcm_util.udelay(n))

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

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
/* #include <linux/jiffies.h> */
/* #include <linux/delay.h> */
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif

#define LCM_DSI_CMD_MODE									0
#define FRAME_WIDTH										(720)
#define FRAME_HEIGHT									(1600)

#define LCM_PHYSICAL_WIDTH									(68332)
#define LCM_PHYSICAL_HEIGHT									(151300)

#define REGFLAG_DELAY		0xFFFC
#define REGFLAG_UDELAY	0xFFFB
#define REGFLAG_END_OF_TABLE	0xFFFD
#define REGFLAG_RESET_LOW	0xFFFE
#define REGFLAG_RESET_HIGH	0xFFFF

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
//extern unsigned int esd_recovery_backlight_level;
struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{REGFLAG_DELAY, 20, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 80, {} }
};

#if 0
static int blmap_table[] = {
					36, 4,
					16, 11,
					17, 12,
					19, 13,
					19, 15,
					20, 14,
					22, 14,
					22, 14,
					24, 10,
					24, 8 ,
					26, 4 ,
					27, 0 ,
					29, 9 ,
					29, 9 ,
					30, 14,
					33, 25,
					34, 30,
					36, 44,
					37, 49,
					40, 65,
					40, 69,
					43, 88,
					46, 109,
					47, 112,
					50, 135,
					53, 161,
					53, 163,
					60, 220,
					60, 223,
					64, 257,
					63, 255,
					71, 334,
					71, 331,
					75, 375,
					80, 422,
					84, 473,
					89, 529,
					88, 518,
					99, 653,
					98, 640,
					103, 707,
					117, 878,
					115, 862,
					122, 947,
					128, 1039,
					135, 1138,
					132, 1102,
					149, 1355,
					157, 1478,
					166, 1611,
					163, 1563,
					183, 1900,
					180, 1844,
					203, 2232,
					199, 2169,
					209, 2344,
					236, 2821,
					232, 2742,
					243, 2958,
					255, 3188,
					268, 3433,
					282, 3705,
					317, 4400,
					176, 1555
};
#endif

static struct LCM_setting_table init_setting_cmd[] = {
	{ 0xFF, 0x03, {0x98, 0x81, 0x03} },
};

static struct LCM_setting_table init_setting_vdo[] = {

{0xFF,3,{0x98,0x81,0x01}},
{0x00,1,{0x44}},
{0x01,1,{0x13}},
{0x02,1,{0x10}},
{0x03,1,{0x20}},
{0x04,1,{0xCA}},
{0x05,1,{0x13}},
{0x06,1,{0x10}},
{0x07,1,{0x20}},
{0x08,1,{0x82}},
{0x09,1,{0x09}},
{0x0a,1,{0xb3}},
{0x0b,1,{0x00}},
{0x0c,1,{0x17}},
{0x0d,1,{0x17}},
{0x0e,1,{0x04}},
{0x0f,1,{0x04}},
{0x10,1,{0x0A}},
{0x11,1,{0x0A}},
{0x12,1,{0x09}},
{0x1E,1,{0x0A}},
{0x1F,1,{0x0A}},
{0x16,1,{0x82}},
{0x17,1,{0x09}},
{0x18,1,{0x33}},
{0x19,1,{0x00}},
{0x1a,1,{0x17}},
{0x1b,1,{0x17}},
{0x1c,1,{0x04}},
{0x1d,1,{0x04}},
{0x20,1,{0x09}},
{0x24,1,{0x02}},
{0x25,1,{0x0b}},
{0x26,1,{0x10}},
{0x27,1,{0x20}},
{0x2c,1,{0x34}},
{0x31,1,{0x07}},
{0x32,1,{0x2a}},
{0x33,1,{0x2a}},
{0x34,1,{0x0D}},
{0x35,1,{0x28}},
{0x36,1,{0x29}},
{0x37,1,{0x11}},
{0x38,1,{0x13}},
{0x39,1,{0x15}},
{0x3a,1,{0x17}},
{0x3b,1,{0x19}},
{0x3c,1,{0x1b}},
{0x3d,1,{0x09}},
{0x3e,1,{0x07}},
{0x3f,1,{0x07}},
{0x40,1,{0x07}},
{0x41,1,{0x07}},
{0x42,1,{0x07}},
{0x43,1,{0x07}},
{0x44,1,{0x07}},
{0x45,1,{0x07}},
{0x46,1,{0x07}},
{0x47,1,{0x07}},
{0x48,1,{0x2a}},
{0x49,1,{0x2a}},
{0x4a,1,{0x0C}},
{0x4b,1,{0x28}},
{0x4c,1,{0x29}},
{0x4d,1,{0x10}},
{0x4e,1,{0x12}},
{0x4f,1,{0x14}},
{0x50,1,{0x16}},
{0x51,1,{0x18}},
{0x52,1,{0x1a}},
{0x53,1,{0x08}},
{0x54,1,{0x07}},
{0x55,1,{0x07}},
{0x56,1,{0x07}},
{0x57,1,{0x07}},
{0x58,1,{0x07}},
{0x59,1,{0x07}},
{0x5a,1,{0x07}},
{0x5b,1,{0x07}},
{0x5c,1,{0x07}},
{0x61,1,{0x07}},
{0x62,1,{0x2a}},
{0x63,1,{0x2a}},
{0x64,1,{0x0D}},
{0x65,1,{0x28}},
{0x66,1,{0x29}},
{0x67,1,{0x11}},
{0x68,1,{0x13}},
{0x69,1,{0x15}},
{0x6a,1,{0x17}},
{0x6b,1,{0x19}},
{0x6c,1,{0x1b}},
{0x6d,1,{0x09}},
{0x6e,1,{0x07}},
{0x6f,1,{0x07}},
{0x70,1,{0x07}},
{0x71,1,{0x07}},
{0x72,1,{0x07}},
{0x73,1,{0x07}},
{0x74,1,{0x07}},
{0x75,1,{0x07}},
{0x76,1,{0x07}},
{0x77,1,{0x07}},
{0x78,1,{0x2a}},
{0x79,1,{0x2a}},
{0x7a,1,{0x0C}},
{0x7b,1,{0x28}},
{0x7c,1,{0x29}},
{0x7d,1,{0x10}},
{0x7e,1,{0x12}},
{0x7f,1,{0x14}},
{0x80,1,{0x16}},
{0x81,1,{0x18}},
{0x82,1,{0x1a}},
{0x83,1,{0x08}},
{0x84,1,{0x07}},
{0x85,1,{0x07}},
{0x86,1,{0x07}},
{0x87,1,{0x07}},
{0x88,1,{0x07}},
{0x89,1,{0x07}},
{0x8a,1,{0x07}},
{0x8b,1,{0x07}},
{0x8c,1,{0x07}},
{0xA0,1,{0x01}},
{0xA2,1,{0x00}},
{0xA3,1,{0x00}},
{0xA4,1,{0x00}},
{0xA5,1,{0x00}},
{0xA6,1,{0x00}},
{0xA7,1,{0x05}},
{0xA8,1,{0x05}},
{0xA9,1,{0x04}},
{0xAa,1,{0x04}},
{0xAb,1,{0x05}},
{0xAc,1,{0x05}},
{0xAd,1,{0x04}},
{0xAe,1,{0x04}},
{0xb0,1,{0x33}},
{0xb1,1,{0x33}},
{0xb2,1,{0x00}},
{0xb9,1,{0x40}},
{0xc3,1,{0xff}},
{0xca,1,{0x44}},
{0xd0,1,{0x01}},
{0xd1,1,{0x00}},
{0xdc,1,{0x37}},
{0xdd,1,{0x42}},
{0xe2,1,{0x00}},
{0xe6,1,{0x23}},
{0xe7,1,{0x54}},
{0xED,1,{0x00}},
{0xFF,3,{0x98,0x81,0x02}},
{0x4B,1,{0x5A}},
{0x06,1,{0x90}},
{0x01,1,{0x10}},
{0x4D,1,{0x4E}},
{0x4E,1,{0x00}},
{0x1A,1,{0x48}},
{0x52,1,{0x48}},
{0x57,1,{0x48}},
{0xFF,3,{0x98,0x81,0x05}},
{0x03,1,{0x01}},
{0x04,1,{0x24}},
{0x63,1,{0x9C}},
{0x64,1,{0x9C}},
{0x68,1,{0x8F}},
{0x69,1,{0x95}},
{0x6A,1,{0xCB}},
{0x6B,1,{0xBD}},
{0xE0,1,{0x03}},
{0x85,1,{0x7F}},
{0xE5,1,{0x6D}},
{0xE6,1,{0x40}},
{0xE7,1,{0xD8}},
{0xE8,1,{0xD8}},
{0xE9,1,{0xA5}},
{0xC0,1,{0x01}},
{0xC1,1,{0x00}},
{0xC2,1,{0x01}},
{0xC3,1,{0x00}},
{0xC4,1,{0x01}},
{0xC5,1,{0x00}},
{0xC6,1,{0x01}},
{0xC7,1,{0x00}},
{0xC8,1,{0x8F}},
{0xC9,1,{0x8F}},
{0xCA,1,{0x53}},
{0xCB,1,{0x53}},
{0xCC,1,{0xCB}},
{0xCD,1,{0xCB}},
{0xCE,1,{0xCB}},
{0xCF,1,{0xCB}},
{0xD0,1,{0x95}},
{0xD1,1,{0x95}},
{0xD2,1,{0x59}},
{0xD3,1,{0x59}},
{0xD4,1,{0xBD}},
{0xD5,1,{0xBD}},
{0xD6,1,{0xBD}},
{0xD7,1,{0xBD}},
{0xFF,3,{0x98,0x81,0x06}},
{0x11,1,{0x03}},
{0x13,1,{0x44}},
{0x14,1,{0x41}},
{0x15,1,{0xF1}},
{0x16,1,{0x40}},
{0x17,1,{0xFF}},
{0x18,1,{0x00}},
{0x94,1,{0x00}},
{0x48,1,{0x0F}},
{0x4D,1,{0x80}},
{0x4E,1,{0x40}},
{0xC7,1,{0x05}},
{0xFF,3,{0x98,0x81,0x08}},
{0xE0,27,{0x00,0x09,0x41,0x6B,0xA1,0x50,0xCF,0xF5,0x22,0x46,0x95,0x7F,0xAF,0xD9,0x02,0xAA,0x2C,0x5F,0x81,0xAB,0xFE,0xD1,0x04,0x42,0x81,0x03,0x96}},
{0xE1,27,{0x00,0x09,0x41,0x6B,0xA1,0x50,0xCF,0xF5,0x22,0x46,0x95,0x7F,0xAF,0xD9,0x02,0xAA,0x2C,0x5F,0x81,0xAB,0xFE,0xD1,0x04,0x42,0x81,0x03,0xBA}},
{0xFF,3,{0x98,0x81,0x06}},
{0xD3,1,{0x09}},
{0xD6,1,{0x85}},
{0x27,1,{0xFF}},
{0x28,1,{0x20}},
{0x2E,1,{0x01}},
{0xC0,1,{0x1f}},
{0xC1,1,{0x03}},
{0xC2,1,{0x04}},
{0x7C,1,{0x40}},
{0xDD,1,{0x10}},
{0xFF,3,{0x98,0x81,0x0E}},
{0x00,1,{0xA0}},
{0x01,1,{0x28}},
{0x02,1,{0x10}},
{0x13,1,{0x10}},
{0x41,1,{0x50}},
{0x20,1,{0x07}},
{0x29,1,{0xBE}},
{0x25,1,{0x0A}},
{0x26,1,{0x23}},
{0x2D,1,{0x95}},
{0x40,1,{0x07}},
{0x49,1,{0xBE}},
{0x45,1,{0x0A}},
{0x46,1,{0x23}},
{0x4D,1,{0x96}},
{0xFF,3,{0x98,0x81,0x0E}},
{0xC0,1,{0x11}},
{0xC1,1,{0x80}},
{0xC2,1,{0x08}},
{0xC3,1,{0x01}},
{0xC4,1,{0x44}},
{0xC5,1,{0x44}},
{0xC6,1,{0x44}},
{0xC7,1,{0x40}},
{0xD0,1,{0x36}},
{0xD1,1,{0x00}},
{0xD2,1,{0x00}},
{0xD3,1,{0x95}},
{0xD4,1,{0x75}},
{0xD5,1,{0x75}},
{0xD6,1,{0x75}},
{0xD7,1,{0x75}},
{0xD8,1,{0x75}},
{0xD9,1,{0x80}},
{0xDA,1,{0x44}},
{0xDB,1,{0x44}},
{0xDC,1,{0x44}},
{0xDD,1,{0x44}},
{0xE1,1,{0x08}},
{0xE2,1,{0x19}},
{0xE3,1,{0x10}},
{0x43,1,{0x60}},
{0xFF,3,{0x98,0x81,0x07}},
{0x0F,1,{0x02}},
{0xFF,3,{0x98,0x81,0x03}},
{0x83,1,{0x30}},
{0x84,1,{0x01}},
{0xFF,3,{0x98,0x81,0x00}},
{0x35,1,{0x00}},
{0x68,2,{0x05,0x02}},
{0x51,2,{0x00,0x00}},
{0x53,1,{0x2C}},
{0x11,1,{0x00}},
{REGFLAG_DELAY, 120, {}},
{0x29,1,{0x00}},
{REGFLAG_DELAY, 20, {}},
};

static struct LCM_setting_table bl_level[] = {
	/* { 0xFF, 0x03, {0x98, 0x81, 0x00} }, */
	{0x51, 2, {0x00,0xFF} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static void push_table(void *cmdq, struct LCM_setting_table *table,
	unsigned int count, unsigned char force_update)
{
	unsigned int i;
	unsigned cmd;

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
			dsi_set_cmdq_V22(cmdq, cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}

static void lcm_get_params(struct LCM_PARAMS *params)
{
	memset(params, 0, sizeof(struct LCM_PARAMS));
	params->type = LCM_TYPE_DSI;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;
#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
	lcm_dsi_mode = CMD_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
	lcm_dsi_mode = SYNC_PULSE_VDO_MODE;
#endif
	pr_debug("lcm_get_params lcm_dsi_mode %d\n", lcm_dsi_mode);
	params->dsi.switch_mode_enable = 0;
	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_THREE_LANE;
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
	params->dsi.vertical_backporch = 16;
	params->dsi.vertical_frontporch = 240;
	//params->dsi.vertical_frontporch_for_low_power = 540;
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.horizontal_sync_active = 8;
	params->dsi.horizontal_backporch = 18;
	params->dsi.horizontal_frontporch = 18;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.ssc_disable = 1;
	//params->dsi.HS_TRAIL = 6;
	//params->dsi.HS_PRPR = 5;
	params->dsi.CLK_HS_PRPR = 7;
	// jump pll_clk
		//hongwu.2020/3/17 night updata
		//params->dsi.horizontal_sync_active_ext = 4;
		//params->dsi.horizontal_backporch_ext = 30;

#ifndef CONFIG_FPGA_EARLY_PORTING
#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 360;	/* this value must be in MTK suggested table */
#else
	params->dsi.PLL_CLOCK = 365;	/* this value must be in MTK suggested table */
#endif
	//params->dsi.PLL_CK_CMD = 360;
	//params->dsi.PLL_CK_VDO = 360;
#else
	params->dsi.pll_div1 = 0;
	params->dsi.pll_div2 = 0;
	params->dsi.fbk_div = 0x1;
#endif
	//params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#if 0
#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	params->round_corner_en = 1;
	params->full_content = 1;
	params->corner_pattern_width = 720;
	params->corner_pattern_height = 75;
	params->corner_pattern_height_bot = 75;
#endif
	params->blmap = blmap_table;
	params->blmap_size = sizeof(blmap_table)/sizeof(blmap_table[0]);
	params->brightness_max = 2012;
	params->brightness_min = 1;
	register_device_proc("lcd", "ili9881h", "truly_hix");
#endif

}

static void lcm_init_power(void)
{
	pr_debug("lcm_init_power\n");
	MDELAY(1);
	SET_LCM_VSP_PIN(1);
	MDELAY(3);
	SET_LCM_VSN_PIN(1);
}

extern void lcd_queue_load_tp_fw(void);
extern int tp_gesture_enable_flag(void);
static void lcm_suspend_power(void)
{
    pr_debug("%s: tp_gesture_enable_flag = %d \n", __func__, tp_gesture_enable_flag());
    if (0 == tp_gesture_enable_flag()) {
		pr_debug("lcm_suspend_power\n");
		SET_LCM_VSN_PIN(0);
		MDELAY(2);
		SET_LCM_VSP_PIN(0);
	}
}

static void lcm_resume_power(void)
{
	pr_debug("lcm_resume_power\n");
	SET_LCM_VSP_PIN(1);
	MDELAY(3);
	SET_LCM_VSN_PIN(1);
	//base voltage = 4.0 each step = 100mV; 4.0+20 * 0.1 = 6.0v;
	if ( display_bias_setting(0x14) )
		pr_err("fatal error: lcd gate ic setting failed \n");
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(2);
	SET_RESET_PIN(1);
	MDELAY(10);
}

static void lcm_init(void)
{
	pr_debug("lcm_init\n");
	SET_RESET_PIN(0);
	MDELAY(2);
	SET_RESET_PIN(1);

	MDELAY(11);
	lcd_queue_load_tp_fw();
	if (lcm_dsi_mode == CMD_MODE) {
		push_table(NULL, init_setting_cmd, sizeof(init_setting_cmd) / sizeof(struct LCM_setting_table), 1);
		pr_debug("ili9881h_txd_lcm_mode = cmd mode :%d----\n", lcm_dsi_mode);
	} else {
		push_table(NULL, init_setting_vdo, sizeof(init_setting_vdo) / sizeof(struct LCM_setting_table), 1);
		pr_debug("ili9881h_txd_lcm_mode = vdo mode :%d\n", lcm_dsi_mode);
	}
}

static void lcm_suspend(void)
{
	pr_debug("lcm_suspend\n");

	push_table(NULL, lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);


}

static void lcm_resume(void)
{
	pr_debug("lcm_resume\n");
	lcm_init();
}

static void lcm_setbacklight_cmdq(void *handle, unsigned int level)
{
	// set 12bit 
	pr_debug(" lcm ili9881h_txd backlight_level = %d\n", level);
	 bl_level[1].para_list[0] = (level&0xF00)>>8; 
 	bl_level[1].para_list[1] = (level&0xFF); 
	//MDELAY(5);
	//pr_err("[ HW check backlight ili9881+txd]para_list[0]=%x,para_list[1]=%x\n",bl_level[1].para_list[0],bl_level[1].para_list[1]); 
 
 	bl_level[1].para_list[0] = 0x0F; 
 	bl_level[1].para_list[1] = 0xFF; 
	push_table(handle, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
	//push_table_cust(handle, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table_V3), 0);
	//dump_stack();
	 
}

#if 0
static struct LCM_setting_table lcm_cabc_enter_setting[] = {
	{0x53,1,{0x2c}},
	{0x55,1,{0x01}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_cabc_exit_setting[] = {
	{0x53,1,{0x2c}},
	{0x55,1,{0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table set_cabc[] = {
	{ 0x53, 0x01, {0x2C} },
	{REGFLAG_DELAY, 20, {} },
	{ 0x55, 0x01, {0x00} },
	{REGFLAG_DELAY, 100, {} },
	//{ 0x53, 0x01, {0x24} },
	{ REGFLAG_END_OF_TABLE, 0x00, {} }
};

static int cabc_status;
 static void lcm_set_cabc_cmdq(void *handle, unsigned int level){
	pr_debug("[lcm] cabc set level %d\n", level);
	if (level==0){
		set_cabc[2].para_list[0]=0x00;
		push_table(handle, set_cabc, sizeof(set_cabc) / sizeof(struct LCM_setting_table), 1);
		pr_err("[lcm] cabc set disable, set_cabc[0x%2x]=%x \n",set_cabc[2].cmd,set_cabc[2].para_list[0]);
	}else if (level == 1){
		set_cabc[2].para_list[0]=0x01;
		push_table(handle, set_cabc, sizeof(set_cabc) / sizeof(struct LCM_setting_table), 1);
		pr_err("[lcm] cabc set enable, set_cabc[0x%2x]=%x \n",set_cabc[2].cmd,set_cabc[2].para_list[0]);
	}else if(level==2){
		set_cabc[2].para_list[0]=0x02;
		push_table(handle,set_cabc,sizeof(set_cabc)/sizeof(struct LCM_setting_table),1);
		pr_err("[lcm] cabc set enable, set_cabc[0x%2x]=%x \n",set_cabc[2].cmd,set_cabc[2].para_list[0]);
	}else if(level==3){
		set_cabc[2].para_list[0]=0x03;
		push_table(handle,set_cabc,sizeof(set_cabc)/sizeof(struct LCM_setting_table),1);
		pr_err("[lcm] cabc set enable, set_cabc[0x%2x]=%x \n",set_cabc[2].cmd,set_cabc[2].para_list[0]);
	}else{
			pr_err("[lcm]  level %d is not support\n", level);
	}
	cabc_status = level;
}
 static void lcm_get_cabc_status(int *status){
	pr_err("[lcm] cabc get to %d\n", cabc_status);
	*status = cabc_status;
}

static void push_table_cust(void *cmdq, struct LCM_setting_table_V3*table,
	unsigned int count, bool hs)
{
	set_lcm(table, count, hs);
}
#endif

static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK
	lcm_resume_power();
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(10);

	if (lcm_dsi_mode == CMD_MODE) {
		push_table(NULL, init_setting_cmd, sizeof(init_setting_cmd) / sizeof(struct LCM_setting_table), 1);
		pr_debug("ili9881h_txd_lcm_mode = cmd mode esd recovery :%d----\n", lcm_dsi_mode);
	} else {
		push_table(NULL, init_setting_vdo, sizeof(init_setting_vdo) / sizeof(struct LCM_setting_table), 1);
		pr_debug("ili9881h_txd_lcm_mode = vdo mode esd recovery :%d----\n", lcm_dsi_mode);
	}
	pr_debug("lcm_esd_recovery\n");
	push_table(NULL, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
	//push_table_cust(NULL, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table_V3), 0);
	return FALSE;
#else
	return FALSE;
#endif
}

struct LCM_DRIVER ilt9881h_txd_psc_hdp_dsi_vdo_lcm_drv = {
	.name = "ilt9881h_txd_psc_hdp_dsi_vdo_lcm",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.esd_recover = lcm_esd_recover,
	.set_backlight_cmdq = lcm_setbacklight_cmdq,
	//.set_cabc_cmdq = lcm_set_cabc_cmdq,
	//.set_cabc_mode_cmdq = lcm_set_cabc_cmdq,
	//.get_cabc_status = lcm_get_cabc_status,
};

