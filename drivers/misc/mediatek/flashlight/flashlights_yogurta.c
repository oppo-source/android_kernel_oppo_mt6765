/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/of.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/pinctrl/consumer.h>
#include "flashlight.h"
#include "flashlight-dt.h"
#include "flashlight-core.h"


/* device tree should be defined in flashlight-dt.h */
#ifndef YOGURTA_DTNAME
#define YOGURTA_DTNAME "mediatek,flashlights_yogurta"
#endif
#ifndef YOGURTA_DTNAME_I2C
#define YOGURTA_DTNAME_I2C "mediatek,strobe_main"
#endif

#define YOGURTA_NAME "flashlights_yogurta"
/* define registers */
#define YOGURTA_REG_ENABLE             (0x01)
#define YOGURTA_MASK_ENABLE_LED1       (0x01)
#define YOGURTA_DISABLE                (0x00)
#define YOGURTA_ENABLE_LED1            (0x03)
#define YOGURTA_ENABLE_LED1_TORCH      (0x0B)
#define YOGURTA_ENABLE_LED1_FLASH      (0x0F)

#define YOGURTA_REG_FLASH_LEVEL_LED1   (0x03)
#define YOGURTA_REG_TORCH_LEVEL_LED1   (0x05)


#define YOGURTA_REG_TIMING_CONF        (0x08)
#define YOGURTA_TORCH_RAMP_TIME        (0x10)
#define YOGURTA_FLASH_TIMEOUT          (0x0A)
//add sec flash IC
#define LM3642_REG_SILICON_REVISION   (0x00)
#define LM3642_REG_FLASH_FEATURE      (0x08)
#define LM3642_INDUCTOR_CURRENT_LIMIT (0x40)
#define LM3642_FLASH_RAMP_TIME        (0x00)
#define LM3642_FLASH_TIMEOUT          (0x07)
#define LM3642_REG_CURRENT_CONTROL    (0x09)
#define LM3642_REG_ENABLE             (0x0A)
#define LM3642_ENABLE_STANDBY         (0x00)
#define LM3642_ENABLE_TORCH           (0x02)
#define LM3642_ENABLE_FLASH           (0x03)
#define LM3642_REG_FLAG               (0x0B)
/* lm3642 revision */
static int is_lm3642lt;
//add sec flash IC

/* define channel, level */
#define YOGURTA_CHANNEL_NUM          1
#define YOGURTA_CHANNEL_CH1          0

#define YOGURTA_LEVEL_NUM            11
#define YOGURTA_LEVEL_TORCH          2

/* define pinctrl */
#define YOGURTA_PINCTRL_PIN_HWEN 0
#define YOGURTA_PINCTRL_PINSTATE_LOW 0
#define YOGURTA_PINCTRL_PINSTATE_HIGH 1
#define YOGURTA_PINCTRL_STATE_HWEN_HIGH "hwen_high"
#define YOGURTA_PINCTRL_STATE_HWEN_LOW  "hwen_low"
static struct pinctrl *yogurta_pinctrl;
static struct pinctrl_state *yogurta_hwen_high;
static struct pinctrl_state *yogurta_hwen_low;

/* define mutex and work queue */
static DEFINE_MUTEX(yogurta_mutex);
static struct work_struct yogurta_work_ch1;
static int YOGURTA_FLAHSH_IC = 0;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static ssize_t yogurta_get_reg(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t yogurta_set_reg(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);

static DEVICE_ATTR(reg, 0660, yogurta_get_reg,  yogurta_set_reg);
/* define usage count */
static int use_count;

/* define i2c */
static struct i2c_client *yogurta_i2c_client;

/* platform data */
struct yogurta_platform_data {
	u8 torch_pin_enable;         /* 1: TX1/TORCH pin isa hardware TORCH enable */
	u8 pam_sync_pin_enable;      /* 1: TX2 Mode The ENVM/TX2 is a PAM Sync. on input */
	u8 thermal_comp_mode_enable; /* 1: LEDI/NTC pin in Thermal Comparator Mode */
	u8 strobe_pin_disable;       /* 1: STROBE Input disabled */
	u8 vout_mode_enable;         /* 1: Voltage Out Mode enable */
};

/* yogurta chip data */
struct yogurta_chip_data {
	struct i2c_client *client;
	struct yogurta_platform_data *pdata;
	struct mutex lock;
	u8 last_flag;
	u8 no_pdata;
};

/******************************************************************************
 * yogurta operations
 *****************************************************************************/
static const int *yogurta_current;
static const unsigned char *yogurta_torch_level;
static const unsigned char *yogurta_flash_level;
static const int aw3642_current[YOGURTA_LEVEL_NUM] = {
	93, 187, 281, 375, 469, 563, 656, 750,844, 938, 1031
};

static const unsigned char aw3642_torch_level[YOGURTA_LEVEL_NUM] = {
	0x8f, 0x9f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char aw3642_flash_level[YOGURTA_LEVEL_NUM] = {
	0x83, 0x87, 0x8B, 0x8F, 0x93, 0x97, 0x9B, 0x9f, 0xA3, 0xA7, 0xAB
};

static const int lm3642_current[YOGURTA_LEVEL_NUM] = {
	93, 187, 281, 375, 469, 563, 656, 750, 844, 938, 1031
};

static const unsigned char lm3642_torch_level[YOGURTA_LEVEL_NUM] = {
	0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char lm3642_flash_level[YOGURTA_LEVEL_NUM] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0A
};

static volatile unsigned char yogurta_reg_enable;
static volatile int yogurta_level_ch1 = -1;

/******************************************************************************
 * Pinctrl configuration
 *****************************************************************************/
static int yogurta_pinctrl_init(struct platform_device *pdev)
{
	int ret = 0;

    pr_info("yogurta_pinctrl_init start\n");
	/* get pinctrl */
	yogurta_pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(yogurta_pinctrl)) {
		pr_err("Failed to get flashlight pinctrl.\n");
		ret = PTR_ERR(yogurta_pinctrl);
	}
	pr_info("devm_pinctrl_get finish %p\n", yogurta_pinctrl);

	/* Flashlight HWEN pin initialization */
	yogurta_hwen_high = pinctrl_lookup_state(yogurta_pinctrl, YOGURTA_PINCTRL_STATE_HWEN_HIGH);
	if (IS_ERR(yogurta_hwen_high)) {
		pr_err("Failed to init (%s)\n", YOGURTA_PINCTRL_STATE_HWEN_HIGH);
		ret = PTR_ERR(yogurta_hwen_high);
	}
	pr_info("pinctrl_lookup_state yogurta_hwen_high finish %p\n", yogurta_hwen_high);
	yogurta_hwen_low = pinctrl_lookup_state(yogurta_pinctrl, YOGURTA_PINCTRL_STATE_HWEN_LOW);
	if (IS_ERR(yogurta_hwen_low)) {
		pr_err("Failed to init (%s)\n", YOGURTA_PINCTRL_STATE_HWEN_LOW);
		ret = PTR_ERR(yogurta_hwen_low);
	}
	pr_info("pinctrl_lookup_state yogurta_hwen_low finish\n");

	return ret;
}

static int yogurta_pinctrl_set(int pin, int state)
{
	int ret = 0;

	if (IS_ERR(yogurta_pinctrl)) {
		pr_info("pinctrl is not available\n");
		return -1;
	}

	switch (pin) {
	case YOGURTA_PINCTRL_PIN_HWEN:
		if (state == YOGURTA_PINCTRL_PINSTATE_LOW && !IS_ERR(yogurta_hwen_low))
			pinctrl_select_state(yogurta_pinctrl, yogurta_hwen_low);
		else if (state == YOGURTA_PINCTRL_PINSTATE_HIGH && !IS_ERR(yogurta_hwen_high))
			pinctrl_select_state(yogurta_pinctrl, yogurta_hwen_high);
		else
			pr_info("set err, pin(%d) state(%d)\n", pin, state);
		break;
	default:
		pr_info("set err, pin(%d) state(%d)\n", pin, state);
		break;
	}
	pr_info("pin(%d) state(%d)\n", pin, state);

	return ret;
}

static int yogurta_is_torch(int level)
{
	if (level >= YOGURTA_LEVEL_TORCH)
		return -1;

	return 0;
}

static int yogurta_verify_level(int level)
{
	if (level < 0)
		level = 0;
	else if (level >= YOGURTA_LEVEL_NUM)
		level = YOGURTA_LEVEL_NUM - 1;

	return level;
}

/* i2c wrapper function */
static int yogurta_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	struct yogurta_chip_data *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);
	ret = i2c_smbus_write_byte_data(client, reg, val);
	mutex_unlock(&chip->lock);

	if (ret < 0)
		pr_info("YOGURTA_FLASH""failed writing at 0x%02x\n", reg);

	return ret;
}

static int yogurta_read_reg(struct i2c_client *client, u8 reg)
{
	int val;
	struct yogurta_chip_data *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);
	val = i2c_smbus_read_byte_data(client, reg);
	mutex_unlock(&chip->lock);

	if (val < 0)
		pr_info("YOGURTA_FLASH""failed read at 0x%02x\n", reg);

	return val;
}
enum FLASHLIGHT_DEVICE {
	AW3642_SM = 0x36,//Chip ID Register ->0x00
	LM3642_SM = 0x00,//Silicon Revision Register (LM3642LT) ->0x00
};

#define USE_AW3642_IC	0x0001
#define USE_LM3642_IC	0x0011
#define USE_NOT_PRO		0x1111

static int yogurta_chip_id(void)
{
	int chip_id;
	int i = 0;
	for(i = 0; i < 3; i++) {
		yogurta_pinctrl_set(YOGURTA_PINCTRL_PIN_HWEN, YOGURTA_PINCTRL_PINSTATE_HIGH);
		chip_id = yogurta_read_reg(yogurta_i2c_client, 0x00);
		pr_info("AAAAflashlight chip id: reg:0x0c, data:0x%x", chip_id);
		yogurta_pinctrl_set(YOGURTA_PINCTRL_PIN_HWEN, YOGURTA_PINCTRL_PINSTATE_LOW);

		if (chip_id == AW3642_SM) {
			YOGURTA_FLAHSH_IC = AW3642_SM;
			pr_err(" the device's flashlight driver IC is AW3642\n");
			break;
		} else if (chip_id == LM3642_SM){
			YOGURTA_FLAHSH_IC = LM3642_SM;
			pr_err(" the device's flashlight driver IC is LM3642\n");
			break;
		} else {
			pr_err(" the device's flashlight driver IC is not used in our project chip_id:%d!\n",chip_id);
			msleep(10);
		}
	}
	return YOGURTA_FLAHSH_IC;
}

/* flashlight enable function */
static int yogurta_enable_ch1(void)
{
	unsigned char reg, val;

	reg = YOGURTA_REG_ENABLE;
	if (!yogurta_is_torch(yogurta_level_ch1)) {
		/* torch mode */
		yogurta_reg_enable |= YOGURTA_ENABLE_LED1_TORCH;
	} else {
		/* flash mode */
		yogurta_reg_enable |= YOGURTA_ENABLE_LED1_FLASH;
	}

	val = yogurta_reg_enable;

	return yogurta_write_reg(yogurta_i2c_client, reg, val);
}



/* flashlight disable function */
static int yogurta_disable_ch1(void)
{
	unsigned char reg, val;

	reg = YOGURTA_REG_ENABLE;
	yogurta_reg_enable &= (~YOGURTA_ENABLE_LED1_FLASH);
	yogurta_reg_enable &= (~YOGURTA_ENABLE_LED1_TORCH);
	val = yogurta_reg_enable;

	return yogurta_write_reg(yogurta_i2c_client, reg, val);
}

static int yogurta_enable(int channel)
{
	if(YOGURTA_FLAHSH_IC == AW3642_SM){
		if (channel == YOGURTA_CHANNEL_CH1) {
			yogurta_enable_ch1();
		}else {
			pr_info("AW3642_FLASH""Error channel\n");
			return -1;
		}
	} else if (YOGURTA_FLAHSH_IC == LM3642_SM){
		unsigned char reg, val;
		reg = LM3642_REG_ENABLE;
		if (!yogurta_is_torch(yogurta_level_ch1)) {
			/* torch mode */
			val = LM3642_ENABLE_TORCH;
			yogurta_write_reg(yogurta_i2c_client, reg, val);
		} else {
			/* flash mode */
			val = LM3642_ENABLE_FLASH;
			yogurta_write_reg(yogurta_i2c_client, reg, val);
		}
	}
	return 0;
}

static int yogurta_disable(int channel)
{
	if(YOGURTA_FLAHSH_IC == AW3642_SM){
		if (channel == YOGURTA_CHANNEL_CH1) {
			yogurta_disable_ch1();
		}else {
			pr_info("YOGURTA_FLASH""Error channel\n");
			return -1;
		}
	}else if (YOGURTA_FLAHSH_IC == LM3642_SM){
		unsigned char reg, val;
		reg = LM3642_REG_ENABLE;
		val = LM3642_ENABLE_STANDBY;
		return yogurta_write_reg(yogurta_i2c_client, reg, val);
	}
	return 0;
}

/* set flashlight level */
static int yogurta_set_level_ch1(int level)
{
	int ret;
	unsigned char reg, val;

	level = yogurta_verify_level(level);
	yogurta_level_ch1 = level;

	if(YOGURTA_FLAHSH_IC == AW3642_SM){
		if (!yogurta_is_torch(yogurta_level_ch1)) {
			/* set torch brightness level */
			reg = YOGURTA_REG_TORCH_LEVEL_LED1;
			val = aw3642_torch_level[level];
			ret = yogurta_write_reg(yogurta_i2c_client, reg, val);
		} else {
			/* set flash brightness level */
			reg = YOGURTA_REG_FLASH_LEVEL_LED1;
			val = aw3642_flash_level[level];
			ret = yogurta_write_reg(yogurta_i2c_client, reg, val);
		}
	}else if (YOGURTA_FLAHSH_IC == LM3642_SM){
		if (!yogurta_is_torch(yogurta_level_ch1)) {
			/* torch mode */
			reg = LM3642_REG_CURRENT_CONTROL;
			val = lm3642_torch_level[level];
			ret = yogurta_write_reg(yogurta_i2c_client, reg, val);
		} else {
			/* flash mode */
			reg = LM3642_REG_CURRENT_CONTROL;
			val = lm3642_flash_level[level];
			ret = yogurta_write_reg(yogurta_i2c_client, reg, val);
		}
	}
	return ret;
}



static int yogurta_set_level(int channel, int level)
{
	if (channel == YOGURTA_CHANNEL_CH1)
		yogurta_set_level_ch1(level);
	else {
		pr_info("YOGURTA_FLASH""Error channel\n");
		return -1;
	}

	return 0;
}

/* flashlight init */
int yogurta_init(void)
{
	int ret;
	unsigned char reg, val;
	int retry = 3;

	while(retry > 0){
		yogurta_pinctrl_set( YOGURTA_PINCTRL_PIN_HWEN,  YOGURTA_PINCTRL_PINSTATE_HIGH);
		if (YOGURTA_FLAHSH_IC == LM3642_SM){
		/* get silicon revision */
			is_lm3642lt = yogurta_read_reg(yogurta_i2c_client, LM3642_REG_SILICON_REVISION);
			pr_info("LM3642(LT) revision(%d).\n", is_lm3642lt);

			/* disable */
			ret = yogurta_write_reg(yogurta_i2c_client, LM3642_REG_ENABLE,LM3642_ENABLE_STANDBY);
			if (ret < 0){
				retry --;
				pr_info("yogurta_init PIN_HWEN failed, retry %d", retry);
			} else {
				pr_info("yogurta_init PIN_HWEN success");
				break;
			}

			/* set flash ramp time and timeout */
			ret = yogurta_write_reg(yogurta_i2c_client, LM3642_REG_FLASH_FEATURE,LM3642_INDUCTOR_CURRENT_LIMIT |
			LM3642_FLASH_RAMP_TIME |LM3642_FLASH_TIMEOUT);
		}else{
			msleep(2);
			/* clear enable register */
			reg =  YOGURTA_REG_ENABLE;
			val =  YOGURTA_DISABLE;
			ret = yogurta_write_reg(yogurta_i2c_client, reg, val);
			if (ret < 0){
				retry --;
				pr_info("yogurta_init PIN_HWEN failed, retry %d", retry);
			} else {
				pr_info("yogurta_init PIN_HWEN success");
				break;
			}

			yogurta_reg_enable = val;

			/* set torch current ramp time and flash timeout */
			reg = YOGURTA_REG_TIMING_CONF;
			val = YOGURTA_TORCH_RAMP_TIME | YOGURTA_FLASH_TIMEOUT;
			ret = yogurta_write_reg(yogurta_i2c_client, reg, val);
		}
	}
	return ret;
}

/* flashlight uninit */
int yogurta_uninit(void)
{
	yogurta_disable(YOGURTA_CHANNEL_CH1);
	yogurta_pinctrl_set(YOGURTA_PINCTRL_PIN_HWEN, YOGURTA_PINCTRL_PINSTATE_LOW);

	return 0;
}


/******************************************************************************
 * Timer and work queue
 *****************************************************************************/
static struct hrtimer yogurta_timer_ch1;
static unsigned int yogurta_timeout_ms[YOGURTA_CHANNEL_NUM];

static void yogurta_work_disable_ch1(struct work_struct *data)
{
	pr_info("YOGURTA_FLASH""ht work queue callback\n");
	yogurta_disable_ch1();
}

static enum hrtimer_restart yogurta_timer_func_ch1(struct hrtimer *timer)
{
	schedule_work(&yogurta_work_ch1);
	return HRTIMER_NORESTART;
}


int yogurta_timer_start(int channel, ktime_t ktime)
{
	if (channel == YOGURTA_CHANNEL_CH1)
		hrtimer_start(&yogurta_timer_ch1, ktime, HRTIMER_MODE_REL);
	else {
		pr_info("YOGURTA_FLASH""Error channel\n");
		return -1;
	}

	return 0;
}

int yogurta_timer_cancel(int channel)
{
	if (channel == YOGURTA_CHANNEL_CH1)
		hrtimer_cancel(&yogurta_timer_ch1);
	else {
		pr_info("YOGURTA_FLASH""Error channel\n");
		return -1;
	}

	return 0;
}


/******************************************************************************
 * Flashlight operations
 *****************************************************************************/
static int yogurta_ioctl(unsigned int cmd, unsigned long arg)
{
	struct flashlight_dev_arg *fl_arg;
	int channel;
	ktime_t ktime;

	fl_arg = (struct flashlight_dev_arg *)arg;
	channel = fl_arg->channel;

	/* verify channel */
	if (channel < 0 || channel >= YOGURTA_CHANNEL_NUM) {
		pr_info("YOGURTA_FLASH""Failed with error channel\n");
		return -EINVAL;
	}

	switch (cmd) {
	case FLASH_IOC_SET_TIME_OUT_TIME_MS:
		pr_info("YOGURTA_FLASH""FLASH_IOC_SET_TIME_OUT_TIME_MS(%d): %d\n",
				channel, (int)fl_arg->arg);
		yogurta_timeout_ms[channel] = fl_arg->arg;
		break;

	case FLASH_IOC_SET_DUTY:
		pr_info("YOGURTA_FLASH""FLASH_IOC_SET_DUTY(%d): %d\n",
				channel, (int)fl_arg->arg);
		yogurta_set_level(channel, fl_arg->arg);
		break;

	case FLASH_IOC_SET_ONOFF:
		pr_info("YOGURTA_FLASH""FLASH_IOC_SET_ONOFF(%d): %d\n",
				channel, (int)fl_arg->arg);
		if (fl_arg->arg == 1) {
			if (yogurta_timeout_ms[channel]) {
				ktime = ktime_set(yogurta_timeout_ms[channel] / 1000,
						(yogurta_timeout_ms[channel] % 1000) * 1000000);
				yogurta_timer_start(channel, ktime);
			}
			//yogurta_pinctrl_set(YOGURTA_PINCTRL_PIN_HWEN, YOGURTA_PINCTRL_PINSTATE_HIGH);
			yogurta_enable(channel);
		} else {
			yogurta_disable(channel);
			//yogurta_pinctrl_set(YOGURTA_PINCTRL_PIN_HWEN, YOGURTA_PINCTRL_PINSTATE_LOW);
			yogurta_timer_cancel(channel);
		}
		break;

	case FLASH_IOC_GET_DUTY_NUMBER:
 		pr_debug("FLASH_IOC_GET_DUTY_NUMBER(%d)\n", channel);
 		fl_arg->arg = YOGURTA_LEVEL_NUM;
 		break;

	case FLASH_IOC_GET_MAX_TORCH_DUTY:
 		pr_debug("FLASH_IOC_GET_MAX_TORCH_DUTY(%d)\n", channel);
 		fl_arg->arg = YOGURTA_LEVEL_TORCH - 1;
 		break;

	case FLASH_IOC_GET_DUTY_CURRENT:
 		fl_arg->arg = yogurta_verify_level(fl_arg->arg);
 		pr_debug("FLASH_IOC_GET_DUTY_CURRENT(%d): %d\n",
 			channel, (int)fl_arg->arg);
 		fl_arg->arg = yogurta_current[fl_arg->arg];
		break;
#if 0
	case FLASH_IOC_GET_HW_TIMEOUT:
 		pr_debug("FLASH_IOC_GET_HW_TIMEOUT(%d)\n", channel);
 		fl_arg->arg = LM3642_HW_TIMEOUT;
 		break;

	case FLASH_IOC_GET_HW_FAULT:
 		pr_debug("FLASH_IOC_GET_HW_FAULT(%d)\n", channel);
 		fl_arg->arg = lm3642_get_flag();
 		break;
#endif

	default:
		pr_info("YOGURTA_FLASH""No such command and arg(%d): (%d, %d)\n",
				channel, _IOC_NR(cmd), (int)fl_arg->arg);
		return -ENOTTY;
	}

	return 0;
}

static int yogurta_open(void)
{
	/* Actual behavior move to set driver function since power saving issue */
	return 0;
}

static int yogurta_release(void)
{
	/* uninit chip and clear usage count */
	mutex_lock(&yogurta_mutex);
	use_count--;
	if (!use_count)
		yogurta_uninit();
	if (use_count < 0)
		use_count = 0;
	mutex_unlock(&yogurta_mutex);

	pr_info("YOGURTA_FLASH""Release: %d\n", use_count);

	return 0;
}

static int yogurta_set_driver(int set)
{
	int ret = 0;
	/* init chip and set usage count */
	mutex_lock(&yogurta_mutex);

	if (set) {
		ret = yogurta_init();
		pr_info("Set driver: %d\n", set);
	} else {
		ret = yogurta_uninit();
		pr_info("Unset driver: %d\n", set);
	}

	mutex_unlock(&yogurta_mutex);

	pr_info("YOGURTA_FLASH""Set driver: %d\n", use_count);

	return 0;
}

static ssize_t yogurta_strobe_store(struct flashlight_arg arg)
{
	yogurta_set_driver(1);
	yogurta_set_level(arg.ct, arg.level);
	yogurta_enable(arg.ct);
	msleep(arg.dur);
	yogurta_disable(arg.ct);
	yogurta_set_driver(0);

	return 0;
}

static struct flashlight_operations yogurta_ops = {
	yogurta_open,
	yogurta_release,
	yogurta_ioctl,
	yogurta_strobe_store,
	yogurta_set_driver
};


/******************************************************************************
 * I2C device and driver
 *****************************************************************************/
static int yogurta_chip_init(struct yogurta_chip_data *chip)
{
	/* NOTE: Chip initialication move to "set driver" operation for power saving issue.
	 * yogurta_init();
	 */

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//YOGURTA Debug file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static ssize_t yogurta_get_reg(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char reg_val;
	unsigned char i;
	ssize_t len = 0;
	for(i=0;i<0x0E;i++)
	{
		if(! (i==0x09 ||i==0x0A ))
		{
			reg_val = yogurta_read_reg(yogurta_i2c_client,i);
			len += snprintf(buf+len, PAGE_SIZE-len, "reg0x%2X = 0x%2X \n", i,reg_val);
		}
	}
	len += snprintf(buf+len, PAGE_SIZE-len, "\r\n");
	return len;
}

static ssize_t yogurta_set_reg(struct device* cd, struct device_attribute *attr, const char* buf, size_t len)
{
	unsigned int databuf[2];
	if(2 == sscanf(buf,"%x %x",&databuf[0], &databuf[1]))
	{
		//i2c_write_reg(databuf[0],databuf[1]);
		yogurta_write_reg(yogurta_i2c_client,databuf[0],databuf[1]);
	}
	return len;
}



static int yogurta_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);

	err = device_create_file(dev, &dev_attr_reg);

	return err;
}

static int yogurta_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct yogurta_chip_data *chip;
	struct yogurta_platform_data *pdata = client->dev.platform_data;
	int err;
	int chip_id;

	pr_info("YOGURTA_FLASH""Probe start.\n");

	/* check i2c */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_info("YOGURTA_FLASH""Failed to check i2c functionality.\n");
		err = -ENODEV;
		goto err_out;
	}

	/* init chip private data */
	chip = kzalloc(sizeof(struct yogurta_chip_data), GFP_KERNEL);
	if (!chip) {
		err = -ENOMEM;
		goto err_out;
	}
	chip->client = client;

	/* init platform data */
	if (!pdata) {
		pr_info("YOGURTA_FLASH""Platform data does not exist\n");
		pdata = kzalloc(sizeof(struct yogurta_platform_data), GFP_KERNEL);
		if (!pdata) {
			err = -ENOMEM;
			goto err_init_pdata;
		}
		chip->no_pdata = 1;
	}
	chip->pdata = pdata;
	i2c_set_clientdata(client, chip);
	yogurta_i2c_client = client;

	/* init mutex and spinlock */
	mutex_init(&chip->lock);

	/* init work queue */
	INIT_WORK(&yogurta_work_ch1, yogurta_work_disable_ch1);

	/* init timer */
	hrtimer_init(&yogurta_timer_ch1, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	yogurta_timer_ch1.function = yogurta_timer_func_ch1;
	yogurta_timeout_ms[YOGURTA_CHANNEL_CH1] = 100;

	/* init chip hw */
	yogurta_chip_init(chip);

	chip_id = yogurta_chip_id();
	if(YOGURTA_FLAHSH_IC ==AW3642_SM){
		yogurta_current = aw3642_current;
		yogurta_torch_level = aw3642_torch_level;
		yogurta_flash_level = aw3642_flash_level;
	}
	else if (YOGURTA_FLAHSH_IC== LM3642_SM){
		yogurta_current = lm3642_current;
		yogurta_torch_level = lm3642_torch_level;
		yogurta_flash_level = lm3642_flash_level;
	} else {
		pr_err("Failed to get flashlight device chip_id:%d.\n",chip_id);
		goto err_free;
	}
	/* register flashlight operations */
	if (flashlight_dev_register(YOGURTA_NAME, &yogurta_ops)) {
		pr_info("YOGURTA_FLASH""Failed to register flashlight device.\n");
		err = -EFAULT;
		goto err_free;
	}

	/* clear usage count */
	use_count = 0;

	yogurta_create_sysfs(client);

	pr_info("YOGURTA_FLASH""Probe done.\n");

	return 0;

err_free:
	kfree(chip->pdata);
err_init_pdata:
	i2c_set_clientdata(client, NULL);
	kfree(chip);
err_out:
	return err;
}

static int yogurta_i2c_remove(struct i2c_client *client)
{
	struct yogurta_chip_data *chip = i2c_get_clientdata(client);

	pr_info("YOGURTA_FLASH""Remove start.\n");

	/* flush work queue */
	flush_work(&yogurta_work_ch1);

	/* unregister flashlight operations */
	flashlight_dev_unregister(YOGURTA_NAME);

	/* free resource */
	if (chip->no_pdata)
		kfree(chip->pdata);
	kfree(chip);

	pr_info("YOGURTA_FLASH""Remove done.\n");

	return 0;
}

static const struct i2c_device_id yogurta_i2c_id[] = {
	{YOGURTA_NAME, 0},
	{}
};

#ifdef CONFIG_OF
static const struct of_device_id yogurta_i2c_of_match[] = {
	{.compatible = YOGURTA_DTNAME_I2C},
	{},
};
#endif

static struct i2c_driver yogurta_i2c_driver = {
	.driver = {
		   .name = YOGURTA_NAME,
#ifdef CONFIG_OF
		   .of_match_table = yogurta_i2c_of_match,
#endif
		   },
	.probe = yogurta_i2c_probe,
	.remove = yogurta_i2c_remove,
	.id_table = yogurta_i2c_id,
};


/******************************************************************************
 * Platform device and driver
 *****************************************************************************/
static int yogurta_probe(struct platform_device *dev)
{
	pr_info("YOGURTA_FLASH""Probe start.\n");

	if (yogurta_pinctrl_init(dev)) {
		pr_info("Failed to init pinctrl.\n");
		return -1;
	}
	if (i2c_add_driver(&yogurta_i2c_driver)) {
		pr_info("YOGURTA_FLASH""Failed to add i2c driver.\n");
		return -1;
	}

	pr_info("YOGURTA_FLASH""Probe done.\n");

	return 0;
}

static int yogurta_remove(struct platform_device *dev)
{
	pr_info("YOGURTA_FLASH""Remove start.\n");

	i2c_del_driver(&yogurta_i2c_driver);

	pr_info("YOGURTA_FLASH""Remove done.\n");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id yogurta_of_match[] = {
	{.compatible = YOGURTA_DTNAME},
	{},
};
MODULE_DEVICE_TABLE(of, yogurta_of_match);
#else
static struct platform_device yogurta_platform_device[] = {
	{
		.name = YOGURTA_NAME,
		.id = 0,
		.dev = {}
	},
	{}
};
MODULE_DEVICE_TABLE(platform, yogurta_platform_device);
#endif

static struct platform_driver yogurta_platform_driver = {
	.probe = yogurta_probe,
	.remove = yogurta_remove,
	.driver = {
		.name = YOGURTA_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = yogurta_of_match,
#endif
	},
};

static int __init flashlight_yogurta_init(void)
{
	int ret;

	pr_info("YOGURTA_FLASH""flashlight_yogurta-Init start.\n");

#ifndef CONFIG_OF
	ret = platform_device_register(&yogurta_platform_device);
	if (ret) {
		pr_info("YOGURTA_FLASH""Failed to register platform device\n");
		return ret;
	}
#endif

	ret = platform_driver_register(&yogurta_platform_driver);
	if (ret) {
		pr_info("YOGURTA_FLASH""Failed to register platform driver\n");
		return ret;
	}

	pr_info("YOGURTA_FLASH""flashlight_yogurta Init done.\n");

	return 0;
}

static void __exit flashlight_yogurta_exit(void)
{
	pr_info("YOGURTA_FLASH""flashlight_yogurta-Exit start.\n");

	platform_driver_unregister(&yogurta_platform_driver);

	pr_info("YOGURTA_FLASH""flashlight_yogurta Exit done.\n");
}

module_init(flashlight_yogurta_init);
module_exit(flashlight_yogurta_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joseph <zhangzetao@awinic.com.cn>");
MODULE_DESCRIPTION("AW Flashlight YOGURTA Driver");

