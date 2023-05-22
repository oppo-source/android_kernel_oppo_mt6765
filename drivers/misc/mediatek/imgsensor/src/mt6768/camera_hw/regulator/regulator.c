/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include "regulator.h"
#include "upmu_common.h"


#include <mt-plat/aee.h>
#include <asm/siginfo.h>

#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/notifier.h>
#include <linux/regulator/consumer.h>
#include <linux/sched/signal.h>

#ifdef OPLUS_FEATURE_CAMERA_COMMON
#include <linux/regulator/driver.h>
#include "../../../../../../../../drivers/regulator/internal.h"
#include "kd_imgsensor.h"
#include <linux/mfd/mt6397/core.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/regmap.h>
#endif /* ALPS07853033 */

static struct REGULATOR *preg_own;
static bool Is_Notify_call[IMGSENSOR_SENSOR_IDX_MAX_NUM][REGULATOR_TYPE_MAX_NUM];

struct reg_oc_debug_t {
	const char *name;
	struct notifier_block nb;
	struct regulator *regulator;
	struct work_struct work;
	unsigned int times;
	unsigned int md_reg_idx;
	bool is_md_reg;
};

static struct reg_oc_debug_t
	reg_oc_debug[IMGSENSOR_SENSOR_IDX_MAX_NUM][REGULATOR_TYPE_MAX_NUM];

static const int regulator_voltage[] = {
	REGULATOR_VOLTAGE_0,
	REGULATOR_VOLTAGE_1000,
#ifdef OPLUS_FEATURE_CAMERA_COMMON
	REGULATOR_VOLTAGE_1050,
#endif
	REGULATOR_VOLTAGE_1100,
	REGULATOR_VOLTAGE_1200,
	REGULATOR_VOLTAGE_1210,
	REGULATOR_VOLTAGE_1220,
	REGULATOR_VOLTAGE_1500,
	REGULATOR_VOLTAGE_1800,
	REGULATOR_VOLTAGE_2500,
	REGULATOR_VOLTAGE_2800,
	REGULATOR_VOLTAGE_2900,
};

struct REGULATOR_CTRL regulator_control[REGULATOR_TYPE_MAX_NUM] = {
	{"vcama"},
	{"vcamd"},
	{"vcamio"},
};

static struct REGULATOR reg_instance;

#ifdef OPLUS_FEATURE_CAMERA_COMMON
struct regmap *g_regmap;
#endif /* ALPS07853033 */

static int regulator_oc_notify(
	struct notifier_block *nb, unsigned long event, void *data)
{
		struct reg_oc_debug_t *reg_oc_dbg =
			container_of(nb, struct reg_oc_debug_t, nb);

		if (event != REGULATOR_EVENT_OVER_CURRENT)
			return NOTIFY_OK;

		/* Do OC handling */
		pr_info("Imgsensor OC notify regulator: %s OC pid %ld\n",
			reg_oc_dbg->name, (long)reg_instance.pid);

		gimgsensor.status.oc = 1;
		aee_kernel_warning("Imgsensor OC", "Over current");
		if (reg_instance.pid != -1 &&
		pid_task(find_get_pid(reg_instance.pid), PIDTYPE_PID) != NULL) {
			force_sig(SIGKILL,
				pid_task(find_get_pid(reg_instance.pid),
				PIDTYPE_PID));
		}
		return NOTIFY_OK;
}

#define OC_MODULE "camera"
enum IMGSENSOR_RETURN imgsensor_oc_interrupt(
	enum IMGSENSOR_SENSOR_IDX sensor_idx, bool enable)
{
	int i = 0;
	int ret = 0;

	mutex_lock(&oc_mutex);
	if (enable) {
		mdelay(5);
		for (i = 0; i < REGULATOR_TYPE_MAX_NUM; i++) {
			if (preg_own->pregulator[(unsigned int)sensor_idx][i] &&
			regulator_is_enabled(preg_own->pregulator[(unsigned int)sensor_idx][i]) &&
			!Is_Notify_call[(unsigned int)sensor_idx][i]
			) {
				/* oc notifier callback function */
			reg_oc_debug[(unsigned int)sensor_idx][i].name =
					regulator_control[i].pregulator_type;
				reg_oc_debug[(unsigned int)sensor_idx][i].regulator =
					preg_own->pregulator[(unsigned int)sensor_idx][i];
				reg_oc_debug[(unsigned int)sensor_idx][i].nb.notifier_call =
					regulator_oc_notify;
				ret = devm_regulator_register_notifier(
					preg_own->pregulator[(unsigned int)sensor_idx][i],
					&reg_oc_debug[(unsigned int)sensor_idx][i].nb);
				Is_Notify_call[(unsigned int)sensor_idx][i] = true;

				if (ret) {
					pr_info(
					"regulator notifier request error\n");
				}
				pr_debug(
					"[regulator] %s idx=%d %s enable=%d oc enabled\n",
					__func__,
					sensor_idx,
					regulator_control[i].pregulator_type,
					enable);
			}
		}
		rcu_read_lock();
		reg_instance.pid = current->tgid;
		rcu_read_unlock();
	} else {
		reg_instance.pid = -1;
		/* Disable interrupt before power off */

		for (i = 0; i < REGULATOR_TYPE_MAX_NUM; i++) {
			if (preg_own->pregulator[(unsigned int)sensor_idx][i] &&
			regulator_is_enabled(preg_own->pregulator[(unsigned int)sensor_idx][i]) &&
			Is_Notify_call[(unsigned int)sensor_idx][i]
			) {
				/* oc notifier callback function */
				devm_regulator_unregister_notifier(
					preg_own->pregulator[(unsigned int)sensor_idx][i],
					&reg_oc_debug[(unsigned int)sensor_idx][i].nb);
				Is_Notify_call[(unsigned int)sensor_idx][i] = false;
				pr_info("Unregister OC notifier");
			}
		}

	}
	mutex_unlock(&oc_mutex);
	return IMGSENSOR_RETURN_SUCCESS;
}

enum IMGSENSOR_RETURN imgsensor_oc_init(void)
{
	/* Register your interrupt handler of OC interrupt at first */

	gimgsensor.status.oc  = 0;
	gimgsensor.imgsensor_oc_irq_enable = imgsensor_oc_interrupt;
	reg_instance.pid = -1;

	return IMGSENSOR_RETURN_SUCCESS;
}

static enum IMGSENSOR_RETURN regulator_init(void *pinstance)
{
	struct REGULATOR *preg = (struct REGULATOR *)pinstance;
	struct device            *pdevice;
	struct device_node       *pof_node;
	int j, i;
	char str_regulator_name[LENGTH_FOR_SNPRINTF];

#ifdef OPLUS_FEATURE_CAMERA_COMMON
	struct device_node	 *pmic_node;
	struct platform_device	 *pmic_pdev;
	struct mt6397_chip	 *chip;
#endif /* ALPS07853033 */

	pdevice  = gimgsensor_device;
	pof_node = pdevice->of_node;
	pdevice->of_node =
		of_find_compatible_node(NULL, NULL, "mediatek,camera_hw");

	if (pdevice->of_node == NULL) {
		pr_debug("regulator get cust camera node failed!\n");
		pdevice->of_node = pof_node;
		return IMGSENSOR_RETURN_ERROR;
	}

#ifdef OPLUS_FEATURE_CAMERA_COMMON
	pmic_node = of_parse_phandle(pdevice->of_node, "pmic", 0);
	if (!pmic_node)	{
		pr_debug("regulator get pmic_node fail!\n");
		return IMGSENSOR_RETURN_ERROR;
	}
	pmic_pdev = of_find_device_by_node(pmic_node);
	if (!pmic_pdev)	{
		pr_debug("get pmic_pdev fail!\n");
		return IMGSENSOR_RETURN_ERROR;
	}
	chip = dev_get_drvdata(&(pmic_pdev->dev));
	if (!chip) {
		pr_debug("get chip fail!\n");
		return IMGSENSOR_RETURN_ERROR;
	}
	g_regmap = chip->regmap;
	if (IS_ERR_VALUE(g_regmap)) {
		g_regmap = NULL;
		pr_debug("get g_regmap fail\n");
	}
#endif /* ALPS07853033 */

	for (j = IMGSENSOR_SENSOR_IDX_MIN_NUM;
		j < IMGSENSOR_SENSOR_IDX_MAX_NUM;
		j++) {
		for (i = 0; i < REGULATOR_TYPE_MAX_NUM; i++) {
			snprintf(str_regulator_name,
					sizeof(str_regulator_name),
					"cam%d_%s",
					j,
					regulator_control[i].pregulator_type);
			preg->pregulator[j][i] =
			    regulator_get_optional(
				pdevice, str_regulator_name);
			if (IS_ERR(preg->pregulator[j][i]))
				preg->pregulator[j][i] = NULL;
			if (preg->pregulator[j][i] == NULL)
				pr_debug("regulator[%d][%d]  %s fail!\n",
					j, i, str_regulator_name);

			atomic_set(&preg->enable_cnt[j][i], 0);
		}
	}
	pdevice->of_node = pof_node;
	imgsensor_oc_init();
	preg_own = (struct REGULATOR *)pinstance;
	return IMGSENSOR_RETURN_SUCCESS;
}
static enum IMGSENSOR_RETURN regulator_release(void *pinstance)
{
	struct REGULATOR *preg = (struct REGULATOR *)pinstance;
	int type, idx;
	struct regulator *pregulator = NULL;
	atomic_t *enable_cnt = NULL;

	for (idx = IMGSENSOR_SENSOR_IDX_MIN_NUM;
		idx < IMGSENSOR_SENSOR_IDX_MAX_NUM;
		idx++) {

		for (type = 0; type < REGULATOR_TYPE_MAX_NUM; type++) {
			pregulator = preg->pregulator[idx][type];
			enable_cnt = &preg->enable_cnt[idx][type];
			if (pregulator != NULL) {
				for (; atomic_read(enable_cnt) > 0; ) {
					regulator_disable(pregulator);
					atomic_dec(enable_cnt);
				}
			}
		}
	}
	return IMGSENSOR_RETURN_SUCCESS;
}

static enum IMGSENSOR_RETURN regulator_set(
	void *pinstance,
	enum IMGSENSOR_SENSOR_IDX   sensor_idx,
	enum IMGSENSOR_HW_PIN       pin,
	enum IMGSENSOR_HW_PIN_STATE pin_state)
{
	struct regulator     *pregulator;
	struct REGULATOR     *preg = (struct REGULATOR *)pinstance;
	int reg_type_offset;
	atomic_t	*enable_cnt;

#ifdef OPLUS_FEATURE_CAMERA_COMMON
	int regval;
#endif /* ALPS07853033 */

	if (pin > IMGSENSOR_HW_PIN_DOVDD   ||
	    pin < IMGSENSOR_HW_PIN_AVDD    ||
	    pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_0 ||
	    pin_state >= IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)
		return IMGSENSOR_RETURN_ERROR;

	reg_type_offset = REGULATOR_TYPE_VCAMA;

	pregulator =
		preg->pregulator[(unsigned int)sensor_idx][
			reg_type_offset + pin - IMGSENSOR_HW_PIN_AVDD];

	enable_cnt =
		&preg->enable_cnt[(unsigned int)sensor_idx][
			reg_type_offset + pin - IMGSENSOR_HW_PIN_AVDD];

	if (pregulator) {
		if (pin_state != IMGSENSOR_HW_PIN_STATE_LEVEL_0) {

		#ifdef OPLUS_FEATURE_CAMERA_COMMON
			unsigned int voltage = regulator_voltage[pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0];
		#endif /* ALPS07853033 */

			if (regulator_set_voltage(
				pregulator,
				regulator_voltage[
				    pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0],
				regulator_voltage[
				 pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0])) {

				pr_err(
				    "[regulator]fail to regulator_set_voltage, powertype:%d powerId:%d\n",
				    pin,
				    regulator_voltage[
				   pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0]);
			}

		#ifdef OPLUS_FEATURE_CAMERA_COMMON
			if (voltage == REGULATOR_VOLTAGE_1050 && sensor_idx == 0) {
				pr_debug("set_sub_sensor vcamd to %dmV\n", voltage/1000);
				regmap_write(g_regmap, MT6358_VCAMD_ANA_CON0, 0x405);
				regmap_read(g_regmap, MT6358_VCAMD_ANA_CON0, &regval);
				pr_debug("regval=%d\n", regval);
				if (regval == 0x405) {
				    pr_debug("set vcamd to %dmV success!\n", voltage/1000);
				}
			}
		#endif /* ALPS07853033 */

			if (regulator_enable(pregulator)) {
				pr_err(
				    "[regulator]fail to regulator_enable, powertype:%d powerId:%d\n",
				    pin,
				    regulator_voltage[
				   pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0]);

				return IMGSENSOR_RETURN_ERROR;
			}
			atomic_inc(enable_cnt);
		} else {
			if (regulator_is_enabled(pregulator)) {
				/*pr_debug("[regulator]%d is enabled\n", pin);*/

				if (regulator_disable(pregulator)) {
					pr_err(
					    "[regulator]fail to regulator_disable, powertype: %d\n",
					    pin);
					return IMGSENSOR_RETURN_ERROR;
				}
			}
			atomic_dec(enable_cnt);
		}
	} else {
		pr_err("regulator == NULL %d %d %d\n",
		    reg_type_offset,
		    pin,
		    IMGSENSOR_HW_PIN_AVDD);
	}

	return IMGSENSOR_RETURN_SUCCESS;
}

static struct IMGSENSOR_HW_DEVICE device = {
	.pinstance = (void *)&reg_instance,
	.init      = regulator_init,
	.set       = regulator_set,
	.release   = regulator_release,
	.id        = IMGSENSOR_HW_ID_REGULATOR
};

enum IMGSENSOR_RETURN imgsensor_hw_regulator_open(
	struct IMGSENSOR_HW_DEVICE **pdevice)
{
	*pdevice = &device;
	return IMGSENSOR_RETURN_SUCCESS;
}

