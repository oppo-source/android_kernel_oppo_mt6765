/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 MediaTek Inc.
*/
#include <linux/delay.h>
#include <linux/time.h>

#include <mt-plat/upmu_common.h>
#include <mt-plat/v1/mtk_battery.h>

#include <mt-plat/mtk_auxadc_intf.h>
#include <mach/mtk_pmic.h>

#if defined(CONFIG_MACH_MT6785)
	#include<mtk_gauge.h>
#endif

#include <mtk_battery_internal.h>
#if defined (CONFIG_MACH_MT6833) || defined(CONFIG_MACH_MT6885) || defined(CONFIG_MACH_MT6893) || defined(CONFIG_MACH_MT6771) || defined(CONFIG_MACH_MT6853) || defined(CONFIG_MACH_MT6873) || defined(CONFIG_MACH_MT6785)
#include <mt-plat/v1/mtk_charger.h>
#else
#include <mtk_charger.h>
#endif
#include "include/pmic_auxadc.h"

bool __attribute__ ((weak)) is_power_path_supported(void)
{
	pr_notice_once("%s: check mtk_charger\n", __func__);
	return 0;
}

int pmic_get_battery_voltage(void)
{
	int bat = 0;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_FPGA_EARLY_PORTING)
	bat = 4201;
#else
#if defined(CONFIG_MACH_MT6785)
	bat = mt6359_gauge_get_batadc();
	if (bat == -1) {
		if (is_isense_supported() && is_power_path_supported())
			bat = pmic_get_auxadc_value(AUXADC_LIST_ISENSE);
		else
			bat = pmic_get_auxadc_value(AUXADC_LIST_BATADC);
	}
#else
	if (is_isense_supported() && is_power_path_supported())
		bat = pmic_get_auxadc_value(AUXADC_LIST_ISENSE);
	else
		bat = pmic_get_auxadc_value(AUXADC_LIST_BATADC);
#endif
#endif
	return bat;
}

bool pmic_is_battery_exist(void)
{
	int temp;
	bool is_bat_exist;
	int hw_id = pmic_get_register_value(PMIC_HWCID);

#if defined(CONFIG_FPGA_EARLY_PORTING)
	is_bat_exist = 0;
	return is_bat_exist;
#endif

#if defined(CONFIG_MTK_PMIC_CHIP_MT6358) \
|| defined(CONFIG_MTK_PMIC_CHIP_MT6359) \
|| defined(CONFIG_MTK_PMIC_CHIP_MT6359P)
	temp = pmic_get_register_value(PMIC_AD_BATON_UNDET);
#else
	temp = pmic_get_register_value(PMIC_RGS_BATON_UNDET);
#endif

	if (temp == 0)
		is_bat_exist = true;
	else
		is_bat_exist = false;


	bm_debug(
		"[fg_is_battery_exist] PMIC_RGS_BATON_UNDET = %d\n",
		is_bat_exist);

	if (is_bat_exist == 0) {
		if (hw_id == 0x3510) {
			pmic_set_register_value(
				PMIC_AUXADC_ADC_RDY_WAKEUP_CLR, 1);
			mdelay(1);
			pmic_set_register_value(
				PMIC_AUXADC_ADC_RDY_WAKEUP_CLR, 0);
		} else {
			pmic_set_register_value(
				PMIC_AUXADC_ADC_RDY_PWRON_CLR, 1);
			mdelay(1);
			pmic_set_register_value(
				PMIC_AUXADC_ADC_RDY_PWRON_CLR, 0);
		}
	}
	return is_bat_exist;

}


int pmic_get_vbus(void)
{
	int vchr = 0;
#if defined(CONFIG_POWER_EXT) || (CONFIG_MTK_GAUGE_VERSION != 30)
	vchr = 5001;
#elif defined(CONFIG_FPGA_EARLY_PORTING)
	vchr = 5001;
#else
	vchr = pmic_get_auxadc_value(AUXADC_LIST_VCDT);
	vchr =
		(((fg_cust_data.r_charger_1 +
		fg_cust_data.r_charger_2) * 100 * vchr) /
		fg_cust_data.r_charger_2) / 100;

#endif
	return vchr;
}

int pmic_get_v_bat_temp(void)
{
	int adc;
#if defined(CONFIG_POWER_EXT)
	adc = 0;
#else
#ifdef CONFIG_MTK_PMIC_CHIP_MT6335
	adc = pmic_get_auxadc_value(AUXADC_LIST_BATTEMP_35);
#else
#if defined(CONFIG_MACH_MT6785)
	adc = mt6359_gauge_get_v_bat_temp();
	if(adc == -1)
		adc =  pmic_get_auxadc_value(AUXADC_LIST_BATTEMP);
#else
	adc = pmic_get_auxadc_value(AUXADC_LIST_BATTEMP);
#endif
#endif
#endif
	return adc;
}

int pmic_get_ibus(void)
{
	return 0;
}

int __attribute__ ((weak))
	charger_get_rsense(void)
{
	pr_notice_once("%s: do not define r_sense\n", __func__);
	return 56;
}


int pmic_get_charging_current(void)
{
#if defined(CONFIG_POWER_EXT) || defined(CONFIG_FPGA_EARLY_PORTING)
	return 0;
#else
	int v_batsns = 0, v_isense = 0;

	if (is_isense_supported() && !is_power_path_supported()) {
		v_isense = pmic_get_auxadc_value(AUXADC_LIST_ISENSE);
		v_batsns = pmic_get_auxadc_value(AUXADC_LIST_BATADC);

		return (v_isense - v_batsns) * 1000 / charger_get_rsense();
	}

	return 0;
#endif
}

bool __attribute__ ((weak))
	mtk_bif_is_hw_exist(void)
{
	pr_notice_once("%s: do not have bif driver\n", __func__);
	return false;
}

int __attribute__ ((weak))
	mtk_bif_get_vbat(int *vbat)
{
	pr_notice_once("%s: do not have bif driver\n", __func__);
	return -ENOTSUPP;
}

int __attribute__ ((weak))
	mtk_bif_get_tbat(int *tmp)
{
	pr_notice_once("%s: do not have bif driver\n", __func__);
	return -ENOTSUPP;
}

int __attribute__ ((weak))
	mtk_bif_init(void)
{
	pr_notice_once("%s: do not have bif driver\n", __func__);
	return -ENOTSUPP;
}

int pmic_is_bif_exist(void)
{
	return mtk_bif_is_hw_exist();
}

int pmic_get_bif_battery_voltage(int *vbat)
{
	return mtk_bif_get_vbat(vbat);
}

int pmic_get_bif_battery_temperature(int *tmp)
{
	return mtk_bif_get_tbat(tmp);
}

int pmic_bif_init(void)
{
	return mtk_bif_init();
}

int pmic_enable_hw_vbus_ovp(bool enable)
{
	int ret = 0;
#if defined(CONFIG_MTK_PMIC_CHIP_MT6359) \
|| defined(CONFIG_MTK_PMIC_CHIP_MT6359P)
	/* TODO check replace by which RG*/
#else
	ret = pmic_set_register_value(PMIC_RG_VCDT_HV_EN, enable);
#endif
	if (ret != 0)
		pr_notice("%s: failed, ret = %d\n", __func__, ret);

	return ret;
}
