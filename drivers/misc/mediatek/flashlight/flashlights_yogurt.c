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
#ifndef YOGURT_DTNAME
#define YOGURT_DTNAME "mediatek,flashlights_yogurt"
#endif
#ifndef YOGURT_DTNAME_I2C
#define YOGURT_DTNAME_I2C "mediatek,strobe_main"
#endif

#define YOGURT_NAME "flashlights_yogurt"
/* define registers */
#define YOGURT_REG_ENABLE             (0x01)
#define YOGURT_MASK_ENABLE_LED1       (0x01)
#define YOGURT_DISABLE                (0x00)
#define YOGURT_ENABLE_LED1            (0x03)
#define YOGURT_ENABLE_LED1_TORCH      (0x0B)
#define YOGURT_ENABLE_LED1_FLASH      (0x0F)

#define YOGURT_REG_FLASH_LEVEL_LED1   (0x03)
#define YOGURT_REG_TORCH_LEVEL_LED1   (0x05)


#define YOGURT_REG_TIMING_CONF        (0x08)
#define YOGURT_TORCH_RAMP_TIME        (0x10)
#define YOGURT_FLASH_TIMEOUT          (0x0A)
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
#define YOGURT_CHANNEL_NUM          1
#define YOGURT_CHANNEL_CH1          0

#define YOGURT_LEVEL_NUM            11
#define YOGURT_LEVEL_TORCH          2

/* define pinctrl */
#define YOGURT_PINCTRL_PIN_HWEN 0
#define YOGURT_PINCTRL_PINSTATE_LOW 0
#define YOGURT_PINCTRL_PINSTATE_HIGH 1
#define YOGURT_PINCTRL_STATE_HWEN_HIGH "hwen_high"
#define YOGURT_PINCTRL_STATE_HWEN_LOW  "hwen_low"
static struct pinctrl *yogurt_pinctrl;
static struct pinctrl_state *yogurt_hwen_high;
static struct pinctrl_state *yogurt_hwen_low;

/* define mutex and work queue */
static DEFINE_MUTEX(yogurt_mutex);
static struct work_struct yogurt_work_ch1;
static int YOGURT_FLAHSH_IC = 0;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static ssize_t yogurt_get_reg(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t yogurt_set_reg(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);

static DEVICE_ATTR(reg, 0660, yogurt_get_reg,  yogurt_set_reg);
/* define usage count */
static int use_count;

/* define i2c */
static struct i2c_client *yogurt_i2c_client;

/* platform data */
struct yogurt_platform_data {
	u8 torch_pin_enable;         /* 1: TX1/TORCH pin isa hardware TORCH enable */
	u8 pam_sync_pin_enable;      /* 1: TX2 Mode The ENVM/TX2 is a PAM Sync. on input */
	u8 thermal_comp_mode_enable; /* 1: LEDI/NTC pin in Thermal Comparator Mode */
	u8 strobe_pin_disable;       /* 1: STROBE Input disabled */
	u8 vout_mode_enable;         /* 1: Voltage Out Mode enable */
};

/* yogurt chip data */
struct yogurt_chip_data {
	struct i2c_client *client;
	struct yogurt_platform_data *pdata;
	struct mutex lock;
	u8 last_flag;
	u8 no_pdata;
};

extern int parker_project(void);
/******************************************************************************
 * yogurt operations
 *****************************************************************************/
static const int *yogurt_current;
static const unsigned char *yogurt_torch_level;
static const unsigned char *yogurt_flash_level;
static const int aw3642_current[YOGURT_LEVEL_NUM] = {
	93,  187, 281,  375,  469,  563, 656, 750,844, 938, 1031
};

static const unsigned char aw3642_torch_level[YOGURT_LEVEL_NUM] = {
	0x8f, 0x9f, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00
};

static const unsigned char aw3642_flash_level[YOGURT_LEVEL_NUM] = {
	0x83,0x87, 0x8B, 0x8F, 0x93, 0x97,0x9B,0x9f, 0xA3, 0xA7, 0xAB
};

static const int lm3642_current[YOGURT_LEVEL_NUM] = {
   	93,  187, 281,  375,  469,  563, 656, 750,844, 938, 1031
};

static const unsigned char lm3642_torch_level[YOGURT_LEVEL_NUM] = {
   	0x10, 0x30, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char lm3642_flash_level[YOGURT_LEVEL_NUM] = {
 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0A
};

// Add for Parker flashlight
static const unsigned char parker_aw3642_flash_level[YOGURT_LEVEL_NUM] = {
	0x83,0x87, 0x8B, 0x8F, 0x93, 0x97,0x9B,0x9f, 0xA3, 0xA7, 0xAF
};

static const unsigned char parker_lm3642_flash_level[YOGURT_LEVEL_NUM] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0B
};

static volatile unsigned char yogurt_reg_enable;
static volatile int yogurt_level_ch1 = -1;

/******************************************************************************
 * Pinctrl configuration
 *****************************************************************************/
static int yogurt_pinctrl_init(struct platform_device *pdev)
{
	int ret = 0;

    pr_info("yogurt_pinctrl_init start\n");
	/* get pinctrl */
	yogurt_pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(yogurt_pinctrl)) {
		pr_err("Failed to get flashlight pinctrl.\n");
		ret = PTR_ERR(yogurt_pinctrl);
	}
	pr_info("devm_pinctrl_get finish %p\n", yogurt_pinctrl);

	/* Flashlight HWEN pin initialization */
	yogurt_hwen_high = pinctrl_lookup_state(yogurt_pinctrl, YOGURT_PINCTRL_STATE_HWEN_HIGH);
	if (IS_ERR(yogurt_hwen_high)) {
		pr_err("Failed to init (%s)\n", YOGURT_PINCTRL_STATE_HWEN_HIGH);
		ret = PTR_ERR(yogurt_hwen_high);
	}
	pr_info("pinctrl_lookup_state yogurt_hwen_high finish %p\n", yogurt_hwen_high);
	yogurt_hwen_low = pinctrl_lookup_state(yogurt_pinctrl, YOGURT_PINCTRL_STATE_HWEN_LOW);
	if (IS_ERR(yogurt_hwen_low)) {
		pr_err("Failed to init (%s)\n", YOGURT_PINCTRL_STATE_HWEN_LOW);
		ret = PTR_ERR(yogurt_hwen_low);
	}
	pr_info("pinctrl_lookup_state yogurt_hwen_low finish\n");

	return ret;
}

static int yogurt_pinctrl_set(int pin, int state)
{
	int ret = 0;

	if (IS_ERR(yogurt_pinctrl)) {
		pr_info("pinctrl is not available\n");
		return -1;
	}

	switch (pin) {
	case YOGURT_PINCTRL_PIN_HWEN:
		if (state == YOGURT_PINCTRL_PINSTATE_LOW && !IS_ERR(yogurt_hwen_low))
			pinctrl_select_state(yogurt_pinctrl, yogurt_hwen_low);
		else if (state == YOGURT_PINCTRL_PINSTATE_HIGH && !IS_ERR(yogurt_hwen_high))
			pinctrl_select_state(yogurt_pinctrl, yogurt_hwen_high);
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

static int yogurt_is_torch(int level)
{
	if (level >= YOGURT_LEVEL_TORCH)
		return -1;

	return 0;
}

static int yogurt_verify_level(int level)
{
	if (level < 0)
		level = 0;
	else if (level >= YOGURT_LEVEL_NUM)
		level = YOGURT_LEVEL_NUM - 1;

	return level;
}

/* i2c wrapper function */
static int yogurt_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	struct yogurt_chip_data *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);
	ret = i2c_smbus_write_byte_data(client, reg, val);
	mutex_unlock(&chip->lock);

	if (ret < 0)
		pr_info("YOGURT_FLASH""failed writing at 0x%02x\n", reg);

	return ret;
}

static int yogurt_read_reg(struct i2c_client *client, u8 reg)
{
	int val;
	struct yogurt_chip_data *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);
	val = i2c_smbus_read_byte_data(client, reg);
	mutex_unlock(&chip->lock);

	if (val < 0)
		pr_info("YOGURT_FLASH""failed read at 0x%02x\n", reg);

	return val;
}
enum FLASHLIGHT_DEVICE {
	AW3642_SM = 0x36,//Chip ID Register ->0x00
	LM3642_SM = 0x00,//Silicon Revision Register (LM3642LT) ->0x00
};

#define USE_AW3642_IC	0x0001
#define USE_LM3642_IC	0x0011
#define USE_NOT_PRO		0x1111

static int yogurt_chip_id(void)
{
	int chip_id;
	int i = 0;
	for(i = 0; i < 3; i++) {
		yogurt_pinctrl_set(YOGURT_PINCTRL_PIN_HWEN, YOGURT_PINCTRL_PINSTATE_HIGH);
		chip_id = yogurt_read_reg(yogurt_i2c_client, 0x00);
		pr_info("AAAAflashlight chip id: reg:0x0c, data:0x%x", chip_id);
		yogurt_pinctrl_set(YOGURT_PINCTRL_PIN_HWEN, YOGURT_PINCTRL_PINSTATE_LOW);

		if (chip_id == AW3642_SM) {
			YOGURT_FLAHSH_IC = AW3642_SM;
			pr_err(" the device's flashlight driver IC is AW3642\n");
			break;
		} else if (chip_id == LM3642_SM){
			YOGURT_FLAHSH_IC = LM3642_SM;
			pr_err(" the device's flashlight driver IC is LM3642\n");
			break;
		} else {
			pr_err(" the device's flashlight driver IC is not used in our project chip_id:%d!\n",chip_id);
			msleep(10);
		}
	}
	return YOGURT_FLAHSH_IC;
}

/* flashlight enable function */
static int yogurt_enable_ch1(void)
{
	unsigned char reg, val;

	reg = YOGURT_REG_ENABLE;
	if (!yogurt_is_torch(yogurt_level_ch1)) {
		/* torch mode */
		yogurt_reg_enable |= YOGURT_ENABLE_LED1_TORCH;
	} else {
		/* flash mode */
		yogurt_reg_enable |= YOGURT_ENABLE_LED1_FLASH;
	}

	val = yogurt_reg_enable;

	return yogurt_write_reg(yogurt_i2c_client, reg, val);
}



/* flashlight disable function */
static int yogurt_disable_ch1(void)
{
	unsigned char reg, val;

	reg = YOGURT_REG_ENABLE;
	yogurt_reg_enable &= (~YOGURT_ENABLE_LED1_FLASH);
	yogurt_reg_enable &= (~YOGURT_ENABLE_LED1_TORCH);
	val = yogurt_reg_enable;

	return yogurt_write_reg(yogurt_i2c_client, reg, val);
}

static int yogurt_enable(int channel)
{
	if(YOGURT_FLAHSH_IC == AW3642_SM){
		if (channel == YOGURT_CHANNEL_CH1) {
			yogurt_enable_ch1();
		}else {
			pr_info("AW3642_FLASH""Error channel\n");
			return -1;
		}
	} else if (YOGURT_FLAHSH_IC == LM3642_SM){
		unsigned char reg, val;
		reg = LM3642_REG_ENABLE;
		if (!yogurt_is_torch(yogurt_level_ch1)) {
			/* torch mode */
			val = LM3642_ENABLE_TORCH;
			yogurt_write_reg(yogurt_i2c_client, reg, val);
		} else {
			/* flash mode */
			val = LM3642_ENABLE_FLASH;
			yogurt_write_reg(yogurt_i2c_client, reg, val);
		}
	}
	return 0;
}

static int yogurt_disable(int channel)
{
	if(YOGURT_FLAHSH_IC == AW3642_SM){
		if (channel == YOGURT_CHANNEL_CH1) {
			yogurt_disable_ch1();
		}else {
			pr_info("YOGURT_FLASH""Error channel\n");
			return -1;
		}
	}else if (YOGURT_FLAHSH_IC == LM3642_SM){
		unsigned char reg, val;
		reg = LM3642_REG_ENABLE;
		val = LM3642_ENABLE_STANDBY;
		return yogurt_write_reg(yogurt_i2c_client, reg, val);
	}
	return 0;
}

/* set flashlight level */
static int yogurt_set_level_ch1(int level)
{
	int ret;
	unsigned char reg, val;

	level = yogurt_verify_level(level);
	yogurt_level_ch1 = level;

	if(YOGURT_FLAHSH_IC == AW3642_SM){
		if (!yogurt_is_torch(yogurt_level_ch1)) {
			/* set torch brightness level */
			reg = YOGURT_REG_TORCH_LEVEL_LED1;
			val = aw3642_torch_level[level];
			ret = yogurt_write_reg(yogurt_i2c_client, reg, val);
		} else {
			/* set flash brightness level */
			reg = YOGURT_REG_FLASH_LEVEL_LED1;
			if (parker_project()==1) {
				val = parker_aw3642_flash_level[level];
			} else {
				val = aw3642_flash_level[level];
			}
			ret = yogurt_write_reg(yogurt_i2c_client, reg, val);
		}
	}else if (YOGURT_FLAHSH_IC == LM3642_SM){
		if (!yogurt_is_torch(yogurt_level_ch1)) {
			/* torch mode */
			reg = LM3642_REG_CURRENT_CONTROL;
			val = lm3642_torch_level[level];
			ret = yogurt_write_reg(yogurt_i2c_client, reg, val);
		} else {
			/* flash mode */
			reg = LM3642_REG_CURRENT_CONTROL;
			if (parker_project()==1) {
				val = parker_lm3642_flash_level[level];
			} else {
				val = lm3642_flash_level[level];
			}
			ret = yogurt_write_reg(yogurt_i2c_client, reg, val);
		}
	}
	return ret;
}



static int yogurt_set_level(int channel, int level)
{
	if (channel == YOGURT_CHANNEL_CH1)
		yogurt_set_level_ch1(level);
	else {
		pr_info("YOGURT_FLASH""Error channel\n");
		return -1;
	}

	return 0;
}

/* flashlight init */
int yogurt_init(void)
{
	int ret;
	unsigned char reg, val;
	yogurt_pinctrl_set( YOGURT_PINCTRL_PIN_HWEN,  YOGURT_PINCTRL_PINSTATE_HIGH);
	if(YOGURT_FLAHSH_IC == AW3642_SM){
		msleep(2);
		/* clear enable register */
		reg =  YOGURT_REG_ENABLE;
		val =  YOGURT_DISABLE;
		ret = yogurt_write_reg(yogurt_i2c_client, reg, val);

		yogurt_reg_enable = val;

		/* set torch current ramp time and flash timeout */
		reg = YOGURT_REG_TIMING_CONF;
		val = YOGURT_TORCH_RAMP_TIME | YOGURT_FLASH_TIMEOUT;
		ret = yogurt_write_reg(yogurt_i2c_client, reg, val);
	}else if (YOGURT_FLAHSH_IC == LM3642_SM){
		/* get silicon revision */
		is_lm3642lt = yogurt_read_reg(yogurt_i2c_client, LM3642_REG_SILICON_REVISION);
		pr_info("LM3642(LT) revision(%d).\n", is_lm3642lt);

		/* disable */
		ret = yogurt_write_reg(yogurt_i2c_client, LM3642_REG_ENABLE,LM3642_ENABLE_STANDBY);

		/* set flash ramp time and timeout */
		ret = yogurt_write_reg(yogurt_i2c_client, LM3642_REG_FLASH_FEATURE,LM3642_INDUCTOR_CURRENT_LIMIT |
		LM3642_FLASH_RAMP_TIME |LM3642_FLASH_TIMEOUT);
	}
	return ret;
}

/* flashlight uninit */
int yogurt_uninit(void)
{
	yogurt_disable(YOGURT_CHANNEL_CH1);
	yogurt_pinctrl_set(YOGURT_PINCTRL_PIN_HWEN, YOGURT_PINCTRL_PINSTATE_LOW);

	return 0;
}


/******************************************************************************
 * Timer and work queue
 *****************************************************************************/
static struct hrtimer yogurt_timer_ch1;
static unsigned int yogurt_timeout_ms[YOGURT_CHANNEL_NUM];

static void yogurt_work_disable_ch1(struct work_struct *data)
{
	pr_info("YOGURT_FLASH""ht work queue callback\n");
	yogurt_disable_ch1();
}

static enum hrtimer_restart yogurt_timer_func_ch1(struct hrtimer *timer)
{
	schedule_work(&yogurt_work_ch1);
	return HRTIMER_NORESTART;
}


int yogurt_timer_start(int channel, ktime_t ktime)
{
	if (channel == YOGURT_CHANNEL_CH1)
		hrtimer_start(&yogurt_timer_ch1, ktime, HRTIMER_MODE_REL);
	else {
		pr_info("YOGURT_FLASH""Error channel\n");
		return -1;
	}

	return 0;
}

int yogurt_timer_cancel(int channel)
{
	if (channel == YOGURT_CHANNEL_CH1)
		hrtimer_cancel(&yogurt_timer_ch1);
	else {
		pr_info("YOGURT_FLASH""Error channel\n");
		return -1;
	}

	return 0;
}


/******************************************************************************
 * Flashlight operations
 *****************************************************************************/
static int yogurt_ioctl(unsigned int cmd, unsigned long arg)
{
	struct flashlight_dev_arg *fl_arg;
	int channel;
	ktime_t ktime;

	fl_arg = (struct flashlight_dev_arg *)arg;
	channel = fl_arg->channel;

	/* verify channel */
	if (channel < 0 || channel >= YOGURT_CHANNEL_NUM) {
		pr_info("YOGURT_FLASH""Failed with error channel\n");
		return -EINVAL;
	}

	switch (cmd) {
	case FLASH_IOC_SET_TIME_OUT_TIME_MS:
		pr_info("YOGURT_FLASH""FLASH_IOC_SET_TIME_OUT_TIME_MS(%d): %d\n",
				channel, (int)fl_arg->arg);
		yogurt_timeout_ms[channel] = fl_arg->arg;
		break;

	case FLASH_IOC_SET_DUTY:
		pr_info("YOGURT_FLASH""FLASH_IOC_SET_DUTY(%d): %d\n",
				channel, (int)fl_arg->arg);
		yogurt_set_level(channel, fl_arg->arg);
		break;

	case FLASH_IOC_SET_ONOFF:
		pr_info("YOGURT_FLASH""FLASH_IOC_SET_ONOFF(%d): %d\n",
				channel, (int)fl_arg->arg);
		if (fl_arg->arg == 1) {
			if (yogurt_timeout_ms[channel]) {
				ktime = ktime_set(yogurt_timeout_ms[channel] / 1000,
						(yogurt_timeout_ms[channel] % 1000) * 1000000);
				yogurt_timer_start(channel, ktime);
			}
			//yogurt_pinctrl_set(YOGURT_PINCTRL_PIN_HWEN, YOGURT_PINCTRL_PINSTATE_HIGH);
			yogurt_enable(channel);
		} else {
			yogurt_disable(channel);
			//yogurt_pinctrl_set(YOGURT_PINCTRL_PIN_HWEN, YOGURT_PINCTRL_PINSTATE_LOW);
			yogurt_timer_cancel(channel);
		}
		break;

	case FLASH_IOC_GET_DUTY_NUMBER:
 		pr_debug("FLASH_IOC_GET_DUTY_NUMBER(%d)\n", channel);
 		fl_arg->arg = YOGURT_LEVEL_NUM;
 		break;

	case FLASH_IOC_GET_MAX_TORCH_DUTY:
 		pr_debug("FLASH_IOC_GET_MAX_TORCH_DUTY(%d)\n", channel);
 		fl_arg->arg = YOGURT_LEVEL_TORCH - 1;
 		break;

	case FLASH_IOC_GET_DUTY_CURRENT:
 		fl_arg->arg = yogurt_verify_level(fl_arg->arg);
 		pr_debug("FLASH_IOC_GET_DUTY_CURRENT(%d): %d\n",
 			channel, (int)fl_arg->arg);
 		fl_arg->arg = yogurt_current[fl_arg->arg];
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
		pr_info("YOGURT_FLASH""No such command and arg(%d): (%d, %d)\n",
				channel, _IOC_NR(cmd), (int)fl_arg->arg);
		return -ENOTTY;
	}

	return 0;
}

static int yogurt_open(void)
{
	/* Actual behavior move to set driver function since power saving issue */
	return 0;
}

static int yogurt_release(void)
{
	/* uninit chip and clear usage count */
	mutex_lock(&yogurt_mutex);
	use_count--;
	if (!use_count)
		yogurt_uninit();
	if (use_count < 0)
		use_count = 0;
	mutex_unlock(&yogurt_mutex);

	pr_info("YOGURT_FLASH""Release: %d\n", use_count);

	return 0;
}

static int yogurt_set_driver(int set)
{
	/* init chip and set usage count */
	mutex_lock(&yogurt_mutex);
	if (!use_count)
		yogurt_init();
	use_count++;
	mutex_unlock(&yogurt_mutex);

	pr_info("YOGURT_FLASH""Set driver: %d\n", use_count);

	return 0;
}

static ssize_t yogurt_strobe_store(struct flashlight_arg arg)
{
	yogurt_set_driver(1);
	yogurt_set_level(arg.ct, arg.level);
	yogurt_enable(arg.ct);
	msleep(arg.dur);
	yogurt_disable(arg.ct);
	yogurt_set_driver(0);

	return 0;
}

static struct flashlight_operations yogurt_ops = {
	yogurt_open,
	yogurt_release,
	yogurt_ioctl,
	yogurt_strobe_store,
	yogurt_set_driver
};


/******************************************************************************
 * I2C device and driver
 *****************************************************************************/
static int yogurt_chip_init(struct yogurt_chip_data *chip)
{
	/* NOTE: Chip initialication move to "set driver" operation for power saving issue.
	 * yogurt_init();
	 */

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//YOGURT Debug file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static ssize_t yogurt_get_reg(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char reg_val;
	unsigned char i;
	ssize_t len = 0;
	for(i=0;i<0x0E;i++)
	{
		if(! (i==0x09 ||i==0x0A ))
		{
			reg_val = yogurt_read_reg(yogurt_i2c_client,i);
			len += snprintf(buf+len, PAGE_SIZE-len, "reg0x%2X = 0x%2X \n", i,reg_val);
		}
	}
	len += snprintf(buf+len, PAGE_SIZE-len, "\r\n");
	return len;
}

static ssize_t yogurt_set_reg(struct device* cd, struct device_attribute *attr, const char* buf, size_t len)
{
	unsigned int databuf[2];
	if(2 == sscanf(buf,"%x %x",&databuf[0], &databuf[1]))
	{
		//i2c_write_reg(databuf[0],databuf[1]);
		yogurt_write_reg(yogurt_i2c_client,databuf[0],databuf[1]);
	}
	return len;
}



static int yogurt_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);

	err = device_create_file(dev, &dev_attr_reg);

	return err;
}

static int yogurt_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct yogurt_chip_data *chip;
	struct yogurt_platform_data *pdata = client->dev.platform_data;
	int err;
	int chip_id;

	pr_info("YOGURT_FLASH""Probe start.\n");

	/* check i2c */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_info("YOGURT_FLASH""Failed to check i2c functionality.\n");
		err = -ENODEV;
		goto err_out;
	}

	/* init chip private data */
	chip = kzalloc(sizeof(struct yogurt_chip_data), GFP_KERNEL);
	if (!chip) {
		err = -ENOMEM;
		goto err_out;
	}
	chip->client = client;

	/* init platform data */
	if (!pdata) {
		pr_info("YOGURT_FLASH""Platform data does not exist\n");
		pdata = kzalloc(sizeof(struct yogurt_platform_data), GFP_KERNEL);
		if (!pdata) {
			err = -ENOMEM;
			goto err_init_pdata;
		}
		chip->no_pdata = 1;
	}
	chip->pdata = pdata;
	i2c_set_clientdata(client, chip);
	yogurt_i2c_client = client;

	/* init mutex and spinlock */
	mutex_init(&chip->lock);

	/* init work queue */
	INIT_WORK(&yogurt_work_ch1, yogurt_work_disable_ch1);

	/* init timer */
	hrtimer_init(&yogurt_timer_ch1, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	yogurt_timer_ch1.function = yogurt_timer_func_ch1;
	yogurt_timeout_ms[YOGURT_CHANNEL_CH1] = 100;

	/* init chip hw */
	yogurt_chip_init(chip);

	chip_id = yogurt_chip_id();
	if(YOGURT_FLAHSH_IC ==AW3642_SM){
		yogurt_current = aw3642_current;
		yogurt_torch_level = aw3642_torch_level;
		yogurt_flash_level = aw3642_flash_level;
	}
	else if (YOGURT_FLAHSH_IC== LM3642_SM){
		yogurt_current = lm3642_current;
		yogurt_torch_level = lm3642_torch_level;
		yogurt_flash_level = lm3642_flash_level;
	} else {
		pr_err("Failed to get flashlight device chip_id:%d.\n",chip_id);
		goto err_free;
	}
	/* register flashlight operations */
	if (flashlight_dev_register(YOGURT_NAME, &yogurt_ops)) {
		pr_info("YOGURT_FLASH""Failed to register flashlight device.\n");
		err = -EFAULT;
		goto err_free;
	}

	/* clear usage count */
	use_count = 0;

	yogurt_create_sysfs(client);

	pr_info("YOGURT_FLASH""Probe done.\n");

	return 0;

err_free:
	kfree(chip->pdata);
err_init_pdata:
	i2c_set_clientdata(client, NULL);
	kfree(chip);
err_out:
	return err;
}

static int yogurt_i2c_remove(struct i2c_client *client)
{
	struct yogurt_chip_data *chip = i2c_get_clientdata(client);

	pr_info("YOGURT_FLASH""Remove start.\n");

	/* flush work queue */
	flush_work(&yogurt_work_ch1);

	/* unregister flashlight operations */
	flashlight_dev_unregister(YOGURT_NAME);

	/* free resource */
	if (chip->no_pdata)
		kfree(chip->pdata);
	kfree(chip);

	pr_info("YOGURT_FLASH""Remove done.\n");

	return 0;
}

static const struct i2c_device_id yogurt_i2c_id[] = {
	{YOGURT_NAME, 0},
	{}
};

#ifdef CONFIG_OF
static const struct of_device_id yogurt_i2c_of_match[] = {
	{.compatible = YOGURT_DTNAME_I2C},
	{},
};
#endif

static struct i2c_driver yogurt_i2c_driver = {
	.driver = {
		   .name = YOGURT_NAME,
#ifdef CONFIG_OF
		   .of_match_table = yogurt_i2c_of_match,
#endif
		   },
	.probe = yogurt_i2c_probe,
	.remove = yogurt_i2c_remove,
	.id_table = yogurt_i2c_id,
};


/******************************************************************************
 * Platform device and driver
 *****************************************************************************/
static int yogurt_probe(struct platform_device *dev)
{
	pr_info("YOGURT_FLASH""Probe start.\n");

	if (yogurt_pinctrl_init(dev)) {
		pr_info("Failed to init pinctrl.\n");
		return -1;
	}
	if (i2c_add_driver(&yogurt_i2c_driver)) {
		pr_info("YOGURT_FLASH""Failed to add i2c driver.\n");
		return -1;
	}

	pr_info("YOGURT_FLASH""Probe done.\n");

	return 0;
}

static int yogurt_remove(struct platform_device *dev)
{
	pr_info("YOGURT_FLASH""Remove start.\n");

	i2c_del_driver(&yogurt_i2c_driver);

	pr_info("YOGURT_FLASH""Remove done.\n");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id yogurt_of_match[] = {
	{.compatible = YOGURT_DTNAME},
	{},
};
MODULE_DEVICE_TABLE(of, yogurt_of_match);
#else
static struct platform_device yogurt_platform_device[] = {
	{
		.name = YOGURT_NAME,
		.id = 0,
		.dev = {}
	},
	{}
};
MODULE_DEVICE_TABLE(platform, yogurt_platform_device);
#endif

static struct platform_driver yogurt_platform_driver = {
	.probe = yogurt_probe,
	.remove = yogurt_remove,
	.driver = {
		.name = YOGURT_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = yogurt_of_match,
#endif
	},
};

static int __init flashlight_yogurt_init(void)
{
	int ret;

	pr_info("YOGURT_FLASH""flashlight_yogurt-Init start.\n");

#ifndef CONFIG_OF
	ret = platform_device_register(&yogurt_platform_device);
	if (ret) {
		pr_info("YOGURT_FLASH""Failed to register platform device\n");
		return ret;
	}
#endif

	ret = platform_driver_register(&yogurt_platform_driver);
	if (ret) {
		pr_info("YOGURT_FLASH""Failed to register platform driver\n");
		return ret;
	}

	pr_info("YOGURT_FLASH""flashlight_yogurt Init done.\n");

	return 0;
}

static void __exit flashlight_yogurt_exit(void)
{
	pr_info("YOGURT_FLASH""flashlight_yogurt-Exit start.\n");

	platform_driver_unregister(&yogurt_platform_driver);

	pr_info("YOGURT_FLASH""flashlight_yogurt Exit done.\n");
}

module_init(flashlight_yogurt_init);
module_exit(flashlight_yogurt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joseph <zhangzetao@awinic.com.cn>");
MODULE_DESCRIPTION("AW Flashlight YOGURT Driver");

