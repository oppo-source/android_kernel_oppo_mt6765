/***********************************************************
** Copyright (C), 2008-2016, OPPO Mobile Comm Corp., Ltd.
** ODM_WT_EDIT
** File: - oplus20361_boe_htl9882n_hdp_dsi_vdo.c
** Description: source file for lcm boe_htl9882n in kernel stage
**
** Version: 1.0
** Date : 2020/11/25
**
** ------------------------------- Revision History: -------------------------------
**  	<author>		<data> 	   <version >	       <desc>
**  lisheng       2020/11/25     1.0     source file for lcm boe_htl9882n in kernel stage
**
****************************************************************/

#define LOG_TAG "LCM"

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
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include "disp_dts_gpio.h"
#endif

#include "disp_cust.h"
/* #include <linux/update_tpfw_notifier.h> */
#include <soc/oplus/device_info.h>
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
#define FRAME_HEIGHT									(1612)

#define LCM_PHYSICAL_WIDTH									(67932)
#define LCM_PHYSICAL_HEIGHT									(152092)

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
extern int tp_control_reset_gpio(bool enable);
/* extern unsigned int esd_recovery_backlight_level; */
extern int __attribute__((weak)) tp_gesture_enable_flag(void) {
	return 0;
}
/*extern void __attribute__((weak)) tp_gpio_current_leakage_handler(bool normal)
{
	return;
};
*/
struct LCM_setting_table
{
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[256];
};

static struct LCM_setting_table lcm_suspend_setting[] =
{
	{0x28, 0, {} },
	{REGFLAG_DELAY, 20, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 120, {} }
};

static int lcm_resume_flag = 0;
#if 1
static int blmap_table[] =
{
	36, 7,
	16, 11,
	17, 12,
	19, 13,
	19, 15,
	20, 14,
	22, 14,
	22, 14,
	24, 10,
	24, 8,
	26, 4,
	27, 0,
	29, 9,
	29, 9,
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

static struct LCM_setting_table init_setting_cmd[] =
{
	{ 0xFF, 0x03, {0x98, 0x81, 0x03} },
};

static struct LCM_setting_table init_setting_vdo[] =
{
	{0xB0, 1, {0x04}},
	{0xD6, 1, {0x00}},
	{0xB6, 6, {0x20, 0x62, 0x00, 0x86, 0xc3, 0x03}},
	{0xC1, 39, {0x30, 0x41, 0x50, 0xfa, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00,
				0x00, 0x00, 0x40, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0xEB, 8, {0x08, 0x30, 0x83, 0x0b, 0x41, 0x01, 0x08, 0x00}},
	{0xED, 33, {0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0xc6,
				0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x02, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x10, 0x00}},
	{REGFLAG_DELAY, 20, {}},
/* cabc start */
	{0xB0, 1,  {0x00}},
	{0xD6, 1,  {0x00}},
	{0xB8, 6,  {0x02, 0x4A, 0x06, 0x00, 0x04, 0x25}},
	{0xB9, 6,  {0x02, 0x4a, 0x18, 0x00, 0x09, 0x3F}},
	{0xBA, 6,  {0x02, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0xCE, 24, {0x16, 0x40, 0x60, 0x78, 0x87, 0x94, 0x9e, 0xab, 0xb8, 0xc4,
				0xcd, 0xd6, 0xde, 0xe5, 0xec, 0xf3, 0xff, 0x07, 0x0F, 0x00, 0x00, 0x00, 0x04, 0x8C}},
	{0xB0, 1,  {0x04}},
	{0xF9, 11,  {0x20, 0x3F, 0xE0, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x00, 0x07, 0xFF}},
	{0xC6, 76, {0x00, 0x00, 0x00, 0x00, 0x01, 0x22, 0x04, 0x22, 0x01, 0x00, 0x53, 0x00, 0x00, 0x00, 0x01,
                    0x00, 0x53, 0x00, 0x01, 0x05, 0x01, 0x0b, 0x01, 0x35, 0xff, 0x8f, 0x06, 0x15, 0x01, 0xc0,
                    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0xc0, 0x11,
                    0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x50, 0x00, 0x33, 0x03, 0x00, 0x00}},
/* cabc end */
	{0xB0, 1, {0x04}},
	{0xC4, 84,  {0x00, 0x5f, 0x5f, 0x0c, 0x5d, 0x04, 0x06, 0x08, 0x01, 0x14,
				 0x14, 0x16, 0x16, 0x18, 0x18, 0x1a, 0x1a, 0x1c, 0x1c, 0x1e,
				 0x1e, 0x5f, 0x00, 0x00, 0x5f, 0x1d, 0x1d, 0x1b, 0x1b, 0x19,
				 0x19, 0x17, 0x17, 0x15, 0x15, 0x13, 0x13, 0x01, 0x07, 0x05,
				 0x03, 0x5d, 0x0b, 0x5f, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0xff, 0x0f, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33,
				 0x33, 0x00, 0x00, 0x55, 0x55, 0x55, 0x55, 0x55, 0x41, 0x55,
				 0x55, 0x55, 0x55, 0x55}},
	{0xD1, 37,  {0xd0, 0xd0, 0x1b, 0x33, 0x33, 0x17, 0x01, 0xbb, 0x22, 0x52,
				 0x22, 0x52, 0x00, 0x3b, 0x77, 0x07, 0x3b, 0x30, 0x06, 0x72,
				 0x33, 0x13, 0x00, 0xd7, 0x0c, 0x22, 0x02, 0x00, 0x18, 0x70,
				 0x18, 0x77, 0x11, 0x11, 0x11, 0x20, 0x20}},
	{0xD7, 46,  {0x01, 0x00, 0x12, 0x12, 0x00, 0x54, 0x01, 0x04, 0x00, 0x54,
				 0x01, 0x04, 0x03, 0x83, 0x80, 0x85, 0x85, 0x85, 0x87, 0x84,
				 0x05, 0x86, 0x87, 0x80, 0x88, 0x86, 0x89, 0x83, 0x83, 0x87,
				 0x84, 0x88, 0x8a, 0x0c, 0x0b, 0x0a, 0x0a, 0x0a, 0x07, 0x07,
				 0x06, 0x06, 0x00, 0x08, 0x0a, 0x0a}},
	{0xEE, 121, {0x05, 0x00, 0x01, 0x00, 0xc0, 0x57, 0x00, 0xc0, 0x57, 0x00,
				 0xc0, 0x57, 0x00, 0x00, 0x03, 0x00, 0xc0, 0x57, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xc0, 0x57, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
				 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
				 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
				 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}},
	{0x51, 2, {0x00, 0x00}},
	{0x53, 1, {0x24}},
	{0x55, 1, {0x00}},
	{0x35, 1, {0x00}},
	{0x11, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 20, {}},
};

static struct LCM_setting_table bl_level[] =
{
	/* { 0xFF, 0x03, {0x98, 0x81, 0x00} }, */
	{0x51, 0x02, {0x0F, 0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table set_cabc_off[] = {
	{0x55, 0x01, {0x03} },
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

static void push_table_cust(void *cmdq, struct LCM_setting_table_V3*table,
	unsigned int count, bool hs)
{
	set_lcm(table, count, hs);
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
	params->physical_width = LCM_PHYSICAL_WIDTH / 1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT / 1000;
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
	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 20;
	params->dsi.vertical_frontporch = 171;
	/* params->dsi.vertical_frontporch_for_low_power = 540; */
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.horizontal_sync_active = 4;
	params->dsi.horizontal_backporch = 55;
	params->dsi.horizontal_frontporch = 23;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.ssc_disable = 1;
	/* params->dsi.HS_TRAIL = 7; */
	/* params->dsi.CLK_TRAIL = 8; */
	/* params->dsi.HS_PRPR = 5; */
	params->dsi.CLK_HS_PRPR = 7;
	/*  jump pll_clk */
	/* params->dsi.horizontal_sync_active_ext = 8; */
	/* params->dsi.horizontal_backporch_ext = 40; */
	params->dsi.dynamic_switch_mipi = 1;
	params->dsi.horizontal_backporch_dyn = 30;
	params->dsi.data_rate_dyn = 720;
	params->lcd_serial_number = 1;

#ifndef CONFIG_FPGA_EARLY_PORTING
#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 360;	/* this value must be in MTK suggested table */
#else
	params->dsi.data_rate = 737;	/* this value must be in MTK suggested table */
#endif
	/* params->dsi.PLL_CK_CMD = 360; */
	/* params->dsi.PLL_CK_VDO = 360; */
#else
	params->dsi.pll_div1 = 0;
	params->dsi.pll_div2 = 0;
	params->dsi.fbk_div = 0x1;
#endif
	/* params->dsi.clk_lp_per_line_enable = 0; */
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	params->round_corner_en = 0;
	params->full_content = 1;
	params->corner_pattern_width = 720;
	params->corner_pattern_height = 136;
	params->corner_pattern_height_bot = 133;
#endif

	params->blmap = blmap_table;
	params->blmap_size = sizeof(blmap_table) / sizeof(blmap_table[0]);
	params->brightness_max = 3276;
	params->brightness_min = 13;
	register_device_proc("lcd", "td4160_limu", "22261_truly_td4160");
}

static void lcm_init_power(void)
{
	pr_info("td4160_truly lcm_init_power\n");
	MDELAY(1);
	SET_LCM_VSP_PIN(1);
	MDELAY(3);
	SET_LCM_VSN_PIN(1);
}

static void lcm_suspend_power(void)
{
	if (0 == tp_gesture_enable_flag()) {
		pr_info("td4160_truly lcm_suspend_power\n");
		SET_RESET_PIN(0);
		MDELAY(2);
		tp_control_reset_gpio(false);
		MDELAY(2);
		SET_LCM_VSN_PIN(0);
		MDELAY(2);
		SET_LCM_VSP_PIN(0);
	}
}

static void lcm_shudown_power(void)
{
	pr_info("samir lcm_shudown_power\n");
	SET_RESET_PIN(0);
	MDELAY(2);
	tp_control_reset_gpio(false);
	MDELAY(2);
	SET_LCM_VSN_PIN(0);
	MDELAY(2);
	SET_LCM_VSP_PIN(0);
}

static void lcm_resume_power(void)
{
	pr_info("lcm_resume_power\n");
	SET_LCM_VSP_PIN(1);
	MDELAY(3);
	SET_LCM_VSN_PIN(1);
	/* base voltage = 4.0 each step = 100mV; 4.0+20 * 0.1 = 6.0v; */
	if (display_bias_setting(0x14)) {
		pr_info("fatal error: lcd gate ic setting failed \n");
	}
	MDELAY(5);
}

extern void __attribute((weak)) lcd_queue_load_tp_fw(void)
{
	return;
};

static void lcm_init(void)
{
	pr_info("lcm_init\n");
	tp_control_reset_gpio(true);
	MDELAY(2);
	SET_RESET_PIN(0);
	MDELAY(2);
	SET_RESET_PIN(1);
	MDELAY(5);
	lcd_queue_load_tp_fw();
	MDELAY(22);
	if (lcm_dsi_mode == CMD_MODE) {
		push_table(NULL, init_setting_cmd, sizeof(init_setting_cmd) / sizeof(struct LCM_setting_table), 1);
		pr_info("oplus20361_boe = cmd mode :%d----\n", lcm_dsi_mode);
	}
	else {
		push_table(NULL, init_setting_vdo, sizeof(init_setting_vdo) / sizeof(struct LCM_setting_table), 1);
		pr_info("oplus20361_boe = vdo mode :%d\n", lcm_dsi_mode);
	}
}

static void lcm_suspend(void)
{
	pr_info("lcm_suspend\n");
	push_table(NULL, lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
	pr_info("lcm_resume\n");
	lcm_init();
	lcm_resume_flag = 1;
}

static struct LCM_setting_table lcm_dimming_on[] =
{
	{0xFF, 0x03, {0x98, 0x83, 0x00}},
	{0x53, 01, {0x2C}},
};

static struct LCM_setting_table lcm_dimming_off[] =
{
	{0xFF, 0x03, {0x98, 0x83, 0x00}},
	{0x53, 01, {0x24}},
};

static int lcm_dimming_flag = 0;

static struct LCM_setting_table lcm_display_on[] =
{
	{0x29, 0x01, {0x00}},
	{REGFLAG_DELAY, 20, {}},
};

static void lcm_setbacklight_cmdq(void *handle, unsigned int level)
{
	static  unsigned int level_last = 0;
	if (level_last == 0) {
		push_table(handle, lcm_display_on, sizeof(lcm_display_on) / sizeof(struct LCM_setting_table), 1);
	}
	bl_level[0].para_list[0] = 0x000F & (level >> 8);
	bl_level[0].para_list[1] = 0x00FF & (level);
	if (lcm_resume_flag) {
		MDELAY(15);
		lcm_resume_flag = 0;
	}
	if(level == 0 && lcm_dimming_flag == 0) {
		push_table(handle, lcm_dimming_on, sizeof(lcm_dimming_on) / sizeof(struct LCM_setting_table), 1);
		MDELAY(1);
		lcm_dimming_flag = 1;
	} else if (level > 0 && lcm_dimming_flag == 1) {
		push_table(handle, lcm_dimming_off, sizeof(lcm_dimming_off) / sizeof(struct LCM_setting_table), 1);
		MDELAY(1);
		lcm_dimming_flag = 0;
	}
	pr_info("[HW check backlight td4160_truly],para_list[0]=%x, para_list[1]=%x, lcm_dimming_flag=%d, level=%d\n", \
	bl_level[0].para_list[0], bl_level[0].para_list[1], lcm_dimming_flag, level);
	push_table(handle, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
	level_last = level;
}

static struct LCM_setting_table set_cabc_1[] = {
	{0x55, 0x01, {0x01} },
};
static struct LCM_setting_table set_cabc_2[] = {
	{0x55, 0x01, {0x02} },
};

static void lcm_set_cabc_cmdq(void *handle, unsigned int level)
{
	pr_info("[lcm] cabc set level %d\n", level);
	if (level == 3) {
		level = 2;
		pr_info("[lcm] cabc set level_2 %d\n", level);
	}
	push_table(handle, lcm_dimming_on, sizeof(lcm_dimming_on) / sizeof(struct LCM_setting_table), 1);
	if (level == 0) {
		push_table(handle, set_cabc_off, sizeof(set_cabc_off) / sizeof(struct LCM_setting_table), 1);
	} else if (level == 1) {
		push_table(handle, set_cabc_1, sizeof(set_cabc_1) / sizeof(struct LCM_setting_table), 1);
	} else if (level == 2) {
		push_table(handle, set_cabc_2, sizeof(set_cabc_2) / sizeof(struct LCM_setting_table), 1);
	} else {
		pr_info("[lcm]  level %d is not support\n", level);
	}
}
static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK
	lcm_resume_power();
	tp_control_reset_gpio(true);
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(10);
	/* lcd_queue_load_tp_fw(); */
	MDELAY(1);
	if (lcm_dsi_mode == CMD_MODE) {
		push_table(NULL, init_setting_cmd, sizeof(init_setting_cmd) / sizeof(struct LCM_setting_table), 1);
		pr_info("td4160_truly = cmd mode esd recovery :%d----\n", lcm_dsi_mode);
	}
	else {
		push_table(NULL, init_setting_vdo, sizeof(init_setting_vdo) / sizeof(struct LCM_setting_table), 1);
		push_table(NULL, lcm_display_on, sizeof(lcm_display_on) / sizeof(struct LCM_setting_table), 1);
		pr_info("td4160_truly = vdo mode esd recovery :%d----\n", lcm_dsi_mode);
	}
	pr_info("lcm_esd_recovery\n");
	push_table(NULL, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
	return FALSE;
#else
	return FALSE;
#endif
}

static struct LCM_setting_table_V3 set_gamma_enter[] = {
	{0x23, 0xB0, 0x01, {0x00}},
	{0x29, 0xC7, 0x4C, {0x02, 0xFE, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xB5, 0x02, 0x6A, 0x02, 0x2F, \
	0x02, 0x11, 0x01, 0xF3, 0x01, 0xDE, 0x01, 0xD3, 0x01, 0xB8, 0x01, 0xBA, 0x02, 0x44, 0x02, 0x7C, 0x02, 0xEB, 0x02, 0xFF, 0x02, \
	0xFE, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xB5, 0x02, 0x6A, 0x02, 0x2F, 0x02, 0x11, 0x01, 0xF3, \
	0x01, 0xDE, 0x01, 0xD3, 0x01, 0xB8, 0x01, 0xBA, 0x02, 0x44, 0x02, 0x7C, 0x02, 0xEB, 0x02, 0xFF}},
};

static struct LCM_setting_table_V3 set_gamma_exit[] = {
	{0x23, 0xB0, 0x01, {0x00}},
	{0x29, 0xC7, 0x4C, {0x00, 0x00, 0x00, 0x55, 0x00, 0xc2, 0x01, 0x0f, 0x01, 0x82, 0x01, 0xb5, 0x01, 0xb5, 0x01, 0xba, 0x01, 0xb2, \
	0x01, 0xb1, 0x01, 0xad, 0x01, 0xa7, 0x01, 0xa3, 0x01, 0x8c, 0x01, 0x99, 0x02, 0x2c, 0x02, 0x6f, 0x02, 0x9d, 0x02, 0xb2, 0x00, \
	0x00, 0x00, 0x55, 0x00, 0xc2, 0x01, 0x0f, 0x01, 0x82, 0x01, 0xb5, 0x01, 0xb5, 0x01, 0xba, 0x01, 0xb2, 0x01, 0xb1, 0x01, 0xad, \
	0x01, 0xa7, 0x01, 0xa3, 0x01, 0x8c, 0x01, 0x99, 0x02, 0x2c, 0x02, 0x6f, 0x02, 0x9d, 0x02, 0xb2}},
};

static void lcm_set_gamma_mode_cmdq(void *handle, unsigned int gamma_flag)
{
	pr_info("%s [lcd] gamma_flag is %d \n", __func__, gamma_flag);
	if (1 == gamma_flag) {
		push_table_cust(handle, set_gamma_enter, sizeof(set_gamma_enter) / sizeof(struct LCM_setting_table_V3), 0);
	}
	else {
		push_table_cust(handle, set_gamma_exit, sizeof(set_gamma_exit) / sizeof(struct LCM_setting_table_V3), 0);
	}
}

struct LCM_DRIVER oplus22261_td4160_truly_hdp_dsi_vdo_lcm_drv =
{
	.name = "oplus22261_td4160_truly_hdp_dsi_vdo_lcm",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.shutdown_power = lcm_shudown_power,
	.esd_recover = lcm_esd_recover,
	.set_backlight_cmdq = lcm_setbacklight_cmdq,
	.set_cabc_mode_cmdq = lcm_set_cabc_cmdq,
	.set_gamma_mode_cmdq = lcm_set_gamma_mode_cmdq,
};
