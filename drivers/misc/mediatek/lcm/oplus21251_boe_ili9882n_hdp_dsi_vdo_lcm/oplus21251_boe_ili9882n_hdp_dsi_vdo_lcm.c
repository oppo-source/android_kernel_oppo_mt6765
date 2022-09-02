/***********************************************************
** Copyright (C), 2008-2016, OPPO Mobile Comm Corp., Ltd.
** ODM_WT_EDIT
** File: - oplus20361_boe_ili9882n_hdp_dsi_vdo.c
** Description: source file for lcm boe_ili9882n in kernel stage
**
** Version: 1.0
** Date : 2020/11/25
**
** ------------------------------- Revision History: -------------------------------
**	 <author>    <data>		<version >	   <desc>
** lisheng	  2020/11/25	 1.0   source file for lcm boe_ili9882n in kernel stage
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

/* 2019/10/07, add gesture_flag flag.
#include <linux/update_tpfw_notifier.h>
*/
#include "disp_cust.h"
/*add /proc/devinfo/lcd node */
#include <soc/oplus/device_info.h>
static struct LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)   (lcm_util.set_reset_pin((v)))
#define SET_LCM_VDD18_PIN(v)	(lcm_util.set_gpio_lcm_vddio_ctl((v)))
#define SET_LCM_VSP_PIN(v) (lcm_util.set_gpio_lcd_enp_bias((v)))
#define SET_LCM_VSN_PIN(v) (lcm_util.set_gpio_lcd_enn_bias((v)))
#define MDELAY(n)	 (lcm_util.mdelay(n))
#define UDELAY(n)	 (lcm_util.udelay(n))

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

#define LCM_DSI_CMD_MODE						 0
#define FRAME_WIDTH						   (720)
#define FRAME_HEIGHT					(1600)

#define LCM_PHYSICAL_WIDTH					 (67932)
#define LCM_PHYSICAL_HEIGHT					    (150960)

#define REGFLAG_DELAY	  0xFFFC
#define REGFLAG_UDELAY  0xFFFB
#define REGFLAG_END_OF_TABLE	0xFFFD
#define REGFLAG_RESET_LOW  0xFFFE
#define REGFLAG_RESET_HIGH 0xFFFF

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
/*extern unsigned int esd_recovery_backlight_level;*/
extern int __attribute__((weak)) tp_gesture_enable_flag(void)
{
	return 0;
}
extern void __attribute__((weak)) tp_gpio_current_leakage_handler(bool normal)
{
	return;
};
__attribute__((weak)) void lcd_resume_load_ili_fw(void)
{
	return;
}
extern void core_config_sleep_ctrl(bool out);
struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{REGFLAG_DELAY, 20, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 120, {} }
};

static int lcm_resume_flag = 0;
#if 1
static int blmap_table[] = {
	36, 7,
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
	/*cabc setting parameters start */
	{0xFF, 0x03, {0x98, 0x82, 0x06}},
	{0x06, 0x01, {0xA4}},          /*PWM 15.6kHz*/
	{0x48, 0x01, {0x05}},
	{0x4D, 0x01, {0x80}},
	{0x4E, 0x01, {0x40}},
	{0xC7, 0x01, {0x05}},
	{0xFF, 0x03, {0x98, 0x82, 0x03}},
	{0x83, 0x01, {0x20}},          /*PWM 12bit*/
	{0x84, 0x01, {0x00}},
	{0x88, 0x01, {0xE1}},
	{0x89, 0x01, {0xE8}},
	{0x8A, 0x01, {0xF0}},
	{0x8B, 0x01, {0xF7}},
	{0x8C, 0x01, {0xBF}},
	{0x8D, 0x01, {0xC5}},
	{0x8E, 0x01, {0xC8}},
	{0x8F, 0x01, {0xCE}},
	{0x90, 0x01, {0xD1}},
	{0x91, 0x01, {0xD6}},
	{0x92, 0x01, {0xDC}},
	{0x93, 0x01, {0xE3}},
	{0x94, 0x01, {0xED}},
	{0x95, 0x01, {0xFA}},
	{0xAF, 0x01, {0x18}},
	{0x86, 0x01, {0x3F}},
	/*cabc setting parameters end*/
	{0xFF, 0x03, {0x98, 0x82, 0x00}},
	{0x53, 0x01, {0x24}},
	{0x55, 0x01, {0x01}},
	{0x35, 0x01, {0x00}},
	{0x11, 0x01, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0xFF, 0x03, {0x98, 0x82, 0x08}},
	{0xE0, 0x1B, {0x00, 0x00, 0x36, 0x60, 0x99, 0x50, 0xCB, 0xF3, 0x25, 0x4E, 0x95, 0x91, 0xC7, \
	0xF9, 0x28, 0xAA, 0x5A, 0x94, 0xB8, 0xE4, 0xFF, 0x0A, 0x39, 0x72, 0xA1, 0x03, 0xFF}},
	{0xE1, 0x1B, {0x00, 0x00, 0x36, 0x60, 0x99, 0x50, 0xCB, 0xF3, 0x25, 0x4E, 0x95, 0x91, 0xC7, \
	0xF9, 0x28, 0xAA, 0x5A, 0x94, 0xB8, 0xE4, 0xFF, 0x0A, 0x39, 0x72, 0xA1, 0x03, 0xFF}},
	{0xFF, 0x03, {0x98, 0x82, 0x02}},
	{0x01, 0x01, {0x14}}, /*mipi time out 160us*/
	{0x02, 0x01, {0x0A}},
	/*pwm setting*/
	{0xFF, 0x03, {0x98, 0x82, 0x06}},
	{0x08, 0x01, {0x01}},
};


static struct LCM_setting_table bl_level[] = {
	/* {0xFF, 0x03, {0x98, 0x81, 0x00}} */
	{0x51, 2, {0x00, 0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
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
	params->dsi.vertical_frontporch = 243;
	/* params->dsi.vertical_frontporch_for_low_power = 540; */
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.horizontal_sync_active = 8;
	params->dsi.horizontal_backporch = 16;
	params->dsi.horizontal_frontporch = 38;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.ssc_disable = 1;
	params->dsi.HS_TRAIL = 7;
	params->dsi.CLK_TRAIL = 8;
	/* params->dsi.HS_PRPR = 5;*/
	params->dsi.CLK_HS_PRPR = 7;
	/* jump pll_clk*/
	/* params->dsi.horizontal_sync_active_ext = 4;*/
	/* params->dsi.horizontal_backporch_ext = 4;*/
	params->dsi.dynamic_switch_mipi = 1;
	params->dsi.horizontal_sync_active_dyn = 4;
	params->dsi.horizontal_backporch_dyn = 4;
	params->dsi.data_rate_dyn = 720;

#ifndef CONFIG_FPGA_EARLY_PORTING
#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 360;  /* this value must be in MTK suggested table */
#else
	params->dsi.data_rate = 737; /* this value must be in MTK suggested table */
#endif
	/* params->dsi.PLL_CK_CMD = 360;*/
	/* params->dsi.PLL_CK_VDO = 360;*/
#else
	params->dsi.pll_div1 = 0;
	params->dsi.pll_div2 = 0;
	params->dsi.fbk_div = 0x1;
#endif
	/* params->dsi.clk_lp_per_line_enable = 0;*/
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x09;
	params->dsi.lcm_esd_check_table[0].count = 3;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x80;
	params->dsi.lcm_esd_check_table[0].para_list[1] = 0x03;
	params->dsi.lcm_esd_check_table[0].para_list[2] = 0x06;

#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	params->round_corner_en = 1;
	params->full_content = 1;
	params->corner_pattern_width = 720;
	params->corner_pattern_height = 136;
	params->corner_pattern_height_bot = 133;
#endif

	params->blmap = blmap_table;
	params->blmap_size = sizeof(blmap_table)/sizeof(blmap_table[0]);
	params->brightness_max = 2047;
	params->brightness_min = 3;

	/* 2020/04/04, add /proc/devinfo/lcd node*/
	register_device_proc("lcd", "ili9882q", "21251_boe_ilitek");
}

static void lcm_init_power(void)
{
	pr_debug("boe_ili9882n lcm_init_power\n");
	MDELAY(1);
	SET_LCM_VSP_PIN(1);
	MDELAY(3);
	SET_LCM_VSN_PIN(1);
}

static void lcm_suspend_power(void)
{
	pr_debug("boe_ili9882n lcm_suspend_power\n");
	if (!tp_gesture_enable_flag()) {
		printk("lcm_tp_suspend_power_on\n");
		SET_LCM_VSN_PIN(0);
		MDELAY(2);
		SET_LCM_VSP_PIN(0);
	}
}
static void lcm_shudown_power(void)
{
        printk("lcm_shudown_power\n");
        tp_gpio_current_leakage_handler(false);
	MDELAY(6);
        SET_RESET_PIN(0);
        MDELAY(2);
        SET_LCM_VSN_PIN(0);
        MDELAY(2);
        SET_LCM_VSP_PIN(0);
}

static void lcm_resume_power(void)
{
	pr_debug("lcm_resume_power\n");
	SET_LCM_VSP_PIN(1);
	MDELAY(3);
	SET_LCM_VSN_PIN(1);
	/* base voltage = 4.0 each step = 100mV; 4.0+20 * 0.1 = 6.0v;*/
	if (display_bias_setting(0x14))
		pr_err("fatal error: lcd gate ic setting failed \n");
	MDELAY(5);
}

extern void __attribute((weak)) lcd_queue_load_tp_fw(void) {
	return;
}

static void lcm_init(void)
{
	pr_debug("lcm_init\n");
	SET_RESET_PIN(0);
	MDELAY(2);
	SET_RESET_PIN(1);

	MDELAY(5);
	/* lcd_resume_load_ili_fw();*/
	lcd_queue_load_tp_fw();
	MDELAY(10);

	if (lcm_dsi_mode == CMD_MODE) {
		push_table(NULL, init_setting_cmd, sizeof(init_setting_cmd) / sizeof(struct LCM_setting_table), 1);
		pr_debug("oplus20361_boe = cmd mode :%d----\n", lcm_dsi_mode);
	} else {
		push_table(NULL, init_setting_vdo, sizeof(init_setting_vdo) / sizeof(struct LCM_setting_table), 1);
		pr_debug("oplus20361_boe = vdo mode :%d\n", lcm_dsi_mode);
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
	lcm_resume_flag = 1;
}
static struct LCM_setting_table lcm_display_on[] = {
        {0xFF, 0x03, {0x98, 0x82, 0x00}},
        {0x29, 0x01, {0x00}},
        {REGFLAG_DELAY, 20, {}},
};

static void lcm_setbacklight_cmdq(void *handle, unsigned int level)
{
        static  unsigned int level_last = 0;
        if (level_last == 0) {
                push_table(handle, lcm_display_on, sizeof(lcm_display_on) / sizeof(struct LCM_setting_table), 1);
        }
	bl_level[0].para_list[0] = 0x000F&(level >> 8);
	bl_level[0].para_list[1] = 0x00FF&(level);

	if (lcm_resume_flag) {
		MDELAY(21);
		lcm_resume_flag = 0;
	}
	pr_err("[HW check backlight boe_ili9882n]level = %d, para_list[0] = %x, para_list[1] = %x\n",
			level, bl_level[0].para_list[0], bl_level[0].para_list[1]);

	push_table(handle, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
        level_last = level;
}

/* modify lcd cabc to hs download */
static struct LCM_setting_table set_cabc_off[] = {
	{0xFF, 0x03, {0x98, 0x82, 0x00}},
	{0x55, 0x01, {0x00} }
};
static struct LCM_setting_table set_cabc_ui[] = {
	{0xFF, 0x03, {0x98, 0x82, 0x00}},
	{0x55, 0x01, {0x01} }
};
static struct LCM_setting_table set_cabc_still[] = {
	{0xFF, 0x03, {0x98, 0x82, 0x00}},
	{0x55, 0x01, {0x02} }
};
static struct LCM_setting_table set_cabc_move[] = {
	{0xFF, 0x03, {0x98, 0x82, 0x00}},
	{0x55, 0x01, {0x02} }
};

static int cabc_status;
static void lcm_set_cabc_cmdq(void *handle, unsigned int level) {
	pr_err("[lcm] cabc set level %d\n", level);
	if (level == 0) {
		push_table(handle, set_cabc_off, sizeof(set_cabc_off) / sizeof(struct LCM_setting_table), 1);
	} else if (level == 1) {
		push_table(handle, set_cabc_ui, sizeof(set_cabc_ui) / sizeof(struct LCM_setting_table), 1);
	} else if (level == 2) {
		push_table(handle, set_cabc_still, sizeof(set_cabc_still) / sizeof(struct LCM_setting_table), 1);
	} else if (level == 3) {
		push_table(handle, set_cabc_move, sizeof(set_cabc_move) / sizeof(struct LCM_setting_table), 1);
	} else {
		pr_debug("[lcm]  level %d is not support\n", level);
	}
	cabc_status = level;
}
static void lcm_get_cabc_status(int *status) {
	pr_debug("[lcm] cabc get to %d\n", cabc_status);
	*status = cabc_status;
}

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
	lcd_queue_load_tp_fw();
	MDELAY(1);

	if (lcm_dsi_mode == CMD_MODE) {
		push_table(NULL, init_setting_cmd, sizeof(init_setting_cmd) / sizeof(struct LCM_setting_table), 1);
		pr_debug("boe_ili9882n = cmd mode esd recovery :%d----\n", lcm_dsi_mode);
	} else {
		push_table(NULL, init_setting_vdo, sizeof(init_setting_vdo) / sizeof(struct LCM_setting_table), 1);
                push_table(NULL, lcm_display_on, sizeof(lcm_display_on) / sizeof(struct LCM_setting_table), 1);
		pr_debug("boe_ili9882n = vdo mode esd recovery :%d----\n", lcm_dsi_mode);
	}
	pr_debug("lcm_esd_recovery\n");
	push_table(NULL, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
	return FALSE;
#else
	return FALSE;
#endif
}

/* 2020/7/15, add LCD dimming control*/
static struct LCM_setting_table lcm_dimming_enable_setting[] = {
	{0x53, 0x01, {0x2c} },
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_dimming_disable_setting[] = {
	{0x53, 0x01, {0x24} },
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void lcm_set_dimming_mode_cmdq(void *handle, unsigned int level)
{
	pr_err("%s [lcd] dimming_mode is %d \n", __func__, level);

	if (1 == level)
		push_table(handle, lcm_dimming_enable_setting, sizeof(lcm_dimming_enable_setting) / sizeof(struct LCM_setting_table), 1);
	else
		push_table(handle, lcm_dimming_disable_setting, sizeof(lcm_dimming_disable_setting) / sizeof(struct LCM_setting_table), 1);
}

static struct LCM_setting_table_V3 set_gamma_enter[] = {
	{0x29, 0xFF, 0x03, {0x98, 0x82, 0x08}},
	{0x29, 0xE0, 0x1B, {0x50, 0xEE, 0xFE, 0x0D, 0x25, 0x55, 0x3F, 0x56, 0x74, 0x90, 0xA5, 0xC0, 0xEC, \
	0x16, 0x41, 0xAA, 0x6F, 0xA7, 0xCA, 0xF4, 0xFF, 0x19, 0x46, 0x7D, 0xAA, 0x03, 0xFF}},
	{0x29, 0xE1, 0x1B, {0x50, 0xEE, 0xFE, 0x0D, 0x25, 0x55, 0x3F, 0x56, 0x74, 0x90, 0xA5, 0xC0, 0xEC, \
	0x16, 0x41, 0xAA, 0x6F, 0xA7, 0xCA, 0xF4, 0xFF, 0x19, 0x46, 0x7D, 0xAA, 0x03, 0xFF}},
	{0x29, 0xFF, 0x03, {0x98, 0x82, 0x00}},
};

static struct LCM_setting_table_V3 set_gamma_exit[] = {
	{0x29, 0xFF, 0x03, {0x98, 0x82, 0x08}},
	{0x29, 0xE0, 0x1B, {0x00, 0x00, 0x36, 0x60, 0x99, 0x50, 0xCB, 0xF3, 0x25, 0x4E, 0x95, 0x91, 0xC7, \
	0xF9, 0x28, 0xAA, 0x5A, 0x94, 0xB8, 0xE4, 0xFF, 0x0A, 0x39, 0x72, 0xA1, 0x03, 0xFF}},
	{0x29, 0xE1, 0x1B, {0x00, 0x00, 0x36, 0x60, 0x99, 0x50, 0xCB, 0xF3, 0x25, 0x4E, 0x95, 0x91, 0xC7, \
	0xF9, 0x28, 0xAA, 0x5A, 0x94, 0xB8, 0xE4, 0xFF, 0x0A, 0x39, 0x72, 0xA1, 0x03, 0xFF}},
	{0x29, 0xFF, 0x03, {0x98, 0x82, 0x00}},
};

static void lcm_set_gamma_mode_cmdq(void *handle, unsigned int level)
{
	pr_err("%s [lcd] gamma_mode is %d \n", __func__, level);

	if (1 == level)
		push_table_cust(handle, set_gamma_enter, sizeof(set_gamma_enter) / sizeof(struct LCM_setting_table_V3), 0);
	else
		push_table_cust(handle, set_gamma_exit, sizeof(set_gamma_exit) / sizeof(struct LCM_setting_table_V3), 0);
/*
	   int backlight_level = 0;
	   backlight_level = current_level*1.4;

	   bl_level[0].para_list[0] = 0x000F&(backlight_level >> 8);
	   bl_level[0].para_list[1] = 0x00FF&(backlight_level);

	   pr_err("[HW check backlight ili9882n_huaxian]current_level= %d, backlight_level=%d, para_list[0]=%x, para_list[1]=%x\n",
	   current_level, backlight_level, bl_level[0].para_list[0], bl_level[0].para_list[1]);

	   push_table(handle, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
*/
}

struct LCM_DRIVER oplus21251_boe_ili9882n_hdp_dsi_vdo_lcm_drv = {
	.name = "oplus21251_boe_ili9882q_hdp_dsi_vdo_lcm",
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
	/* 2020/7/15, add LCD dimming control*/
	.set_dimming_mode_cmdq = lcm_set_dimming_mode_cmdq,
	.set_gamma_mode_cmdq = lcm_set_gamma_mode_cmdq,
	.set_cabc_mode_cmdq = lcm_set_cabc_cmdq,
	.get_cabc_status = lcm_get_cabc_status,
};

