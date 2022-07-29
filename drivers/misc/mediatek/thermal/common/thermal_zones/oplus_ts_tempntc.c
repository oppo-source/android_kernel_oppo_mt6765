// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/acpi.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <mt-plat/aee.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "mt-plat/mtk_thermal_monitor.h"
#include "mach/mtk_thermal.h"
#include <linux/uidgid.h>
#include <linux/slab.h>
#include <soc/oplus/system/oppo_project.h>
#include <linux/power_supply.h>
#include <linux/iio/consumer.h>
#include <linux/kthread.h>	/* For Kthread_run */
#include <linux/of_gpio.h>


#define CHGNTC_ENABLE_LEVEL		     	(1)
#define CHGNTC_DISABLE_LEVEL			(0)
#define  BOARD_TEMP_GPIO_NUM   154
struct temp_data {
	struct platform_device *pdev;
    int batt_id_volt;
    int flash_ntc_volt;
    int charger_ntc_volt;
    int pa_ntc_volt;
    struct task_struct      *oplus_tempntc_kthread;
    struct iio_channel      *iio_channel_flash;
    struct iio_channel      *iio_channel_pa;
    struct iio_channel      *iio_channel3;
    struct delayed_work init_work;
    struct completion temp_det_start;
    struct mutex det_lock;
    bool is_kthread_get_adc;
    int tempntc_pin;
};
static struct temp_data *pinfo;
extern void oplus_gpio_switch_lock(void);
extern void oplus_gpio_switch_unlock(void);
extern void oplus_gpio_value_switch(unsigned int pin, unsigned int val);

int get_flash_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->flash_ntc_volt;
}

int get_batt_id_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->batt_id_volt;
}

int get_charger_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->charger_ntc_volt;
}

int get_pa_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->pa_ntc_volt;
}

bool is_kthread_get_adc(void)
{
    if (!pinfo)
        return 0;
    return pinfo->is_kthread_get_adc;
}

int oplus_tempntc_get_volt(struct temp_data *info)
{
    int ret;
    int iio_chan3_volt;
    int iio_chan1_volt;

    if ((!info->iio_channel_flash) || (!info->iio_channel3)
                || (!info->iio_channel_pa)) {
        pr_err("%s conntinue\n", __func__);
        return 0;
    }

    if (pinfo->tempntc_pin > 0)
        gpio_set_value(pinfo->tempntc_pin, CHGNTC_DISABLE_LEVEL);
    else
        return 0;

    msleep(300);
    ret = iio_read_channel_processed(info->iio_channel_flash, &iio_chan1_volt);
    if (ret < 0) {
        pr_err("%s flash_ntc_volt read error!\n");
    } else {
        info->flash_ntc_volt = iio_chan1_volt;
    }

    ret = iio_read_channel_processed(info->iio_channel3, &iio_chan3_volt);
    if (ret < 0) {
        pr_err("%s batt_id_volt read error!\n");
    } else {
        info->batt_id_volt = iio_chan3_volt;
    }

    if (pinfo->tempntc_pin > 0)
    	gpio_set_value(pinfo->tempntc_pin, CHGNTC_ENABLE_LEVEL);
    else
        return 0;

    msleep(300);
    ret = iio_read_channel_processed(info->iio_channel_pa, &iio_chan1_volt);
    if (ret < 0) {
        pr_err("%s pa_ntc_volt read error!\n");
    } else {
        info->pa_ntc_volt = iio_chan1_volt;
    }

    ret = iio_read_channel_processed(info->iio_channel3, &iio_chan3_volt);
    if (ret < 0) {
        pr_err("%s charger_ntc_volt read error!\n");
    } else {
        info->charger_ntc_volt = iio_chan3_volt;
    }

    pinfo->is_kthread_get_adc = true;

	return 0;
}

static void oplus_tempntc_det_work(struct work_struct *work)
{
    mutex_lock(&pinfo->det_lock);
    oplus_tempntc_get_volt(pinfo);
    mutex_unlock(&pinfo->det_lock);
    schedule_delayed_work(&pinfo->init_work, msecs_to_jiffies(5000));
}

extern struct iio_channel	*batt_id;
extern struct iio_channel   *thermistor_ch0;
extern struct iio_channel   *thermistor_ch1;
static int oplus_tempntc_parse_dt(struct temp_data *info,
				struct device *dev)
{
    struct device_node *np = dev->of_node;

	info->iio_channel3 = iio_channel_get(dev, "auxadc3-batt_id_volt");
	if (IS_ERR(info->iio_channel3)) {
		pr_err("battery ID CHANNEL ERR \n");
		info->iio_channel3 = batt_id;
	}

    info->iio_channel_flash = iio_channel_get(dev, "auxadc1-flash_volt");
	if (IS_ERR(info->iio_channel_flash)){
		pr_err("Flash_PA CHANNEL ERR \n");
		info->iio_channel_flash = thermistor_ch0;
	}

    info->iio_channel_pa = iio_channel_get(dev, "auxadc1-pa_volt");
	if (IS_ERR(info->iio_channel_pa)){
		pr_err("Flash_PA CHANNEL ERR \n");
		info->iio_channel_pa = thermistor_ch1;
	}

	info->tempntc_pin = of_get_named_gpio(np, "tempntc_gpio", 0);
	if(info->tempntc_pin < 0)	{
		pr_err("[%s]: tempntc_pin < 0 !!! \r\n", __func__);
	}

	if (gpio_request(info->tempntc_pin, "TEMPNTC_GPIO") < 0)
		pr_err("[%s]: tempntc_pin gpio_request fail !!! \r\n", __func__);

	return 0;
}

static int oplus_tempntc_data_init(struct temp_data *info)
{
    info->flash_ntc_volt = -1;
    info->batt_id_volt = -1;
    info->pa_ntc_volt = -1;
    info->charger_ntc_volt = -1;
    mutex_init(&info->det_lock);
	return 0;
}

static int oplus_tempntc_pdrv_probe(struct platform_device *pdev)
{
    struct temp_data *info;

    pr_err("%s: starts\n", __func__);
	info = devm_kzalloc(&pdev->dev,sizeof(struct temp_data), GFP_KERNEL);
	if (!info) {
		pr_err(" kzalloc() failed\n");
		return -ENOMEM;
	}

    pinfo = info;
    platform_set_drvdata(pdev, info);
	info->pdev = pdev;

    oplus_tempntc_data_init(info);
    oplus_tempntc_parse_dt(info, &pdev->dev);
    INIT_DELAYED_WORK(&info->init_work, oplus_tempntc_det_work);
    schedule_delayed_work(&info->init_work, msecs_to_jiffies(5000));

	return 0;
}

static int oplus_tempntc_pdrv_remove(struct platform_device *pdev)
{
	return 0;
}

static void oplus_tempntc_pdrv_shutdown(struct platform_device *dev)
{

}

static const struct of_device_id oplus_tempntc_of_match[] = {
	{.compatible = "oplus-tempntc",},
	{},
};
MODULE_DEVICE_TABLE(of, oplus_tempntc_of_match)

static int __maybe_unused tempntc_suspend(struct device *dev)
{
	cancel_delayed_work_sync(&pinfo->init_work);
	return 0;
}

static int __maybe_unused tempntc_resume(struct device *dev)
{
	/* Schedule timer to check current status */
	schedule_delayed_work(&pinfo->init_work,
			msecs_to_jiffies(5000));
	return 0;
}

static SIMPLE_DEV_PM_OPS(tempntc_pm_ops, tempntc_suspend, tempntc_resume);

static struct platform_driver oplus_tempntc_driver = {
	.probe = oplus_tempntc_pdrv_probe,
	.remove = oplus_tempntc_pdrv_remove,
    .shutdown = oplus_tempntc_pdrv_shutdown,
    .driver = {
            .name = "oplus_tempntc",
            .owner = THIS_MODULE,
            .of_match_table = oplus_tempntc_of_match,
            .pm	= &tempntc_pm_ops,
		},
};

static int __init oplus_tempntc_init(void)
{
    int ret;
	ret = platform_driver_register(&oplus_tempntc_driver);
    if (ret) {
		pr_err("%s fail to tempntc device\n", __func__);
		return ret;
    }
    return ret;
}

late_initcall(oplus_tempntc_init);

static void __exit oplus_tempntc_exit(void)
{
    platform_driver_unregister(&oplus_tempntc_driver);
}
module_exit(oplus_tempntc_exit);

MODULE_AUTHOR("wy.chuang <wy.chuang@mediatek.com>");
MODULE_DESCRIPTION("MTK Gauge Device Driver");
MODULE_LICENSE("GPL");
