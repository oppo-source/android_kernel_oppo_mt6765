/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
*/
/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#ifndef __IMGSENSOR_HW_GPIO_H__
#define __IMGSENSOR_HW_GPIO_H__

#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/pinctrl.h>
#include "imgsensor_hw.h"
#include "imgsensor_common.h"

#ifdef OPLUS_FEATURE_CAMERA_COMMON
#include <soc/oplus/system/oplus_project.h>
#endif
enum GPIO_CTRL_STATE_CAM {
	/* Main */
	GPIO_CTRL_STATE_PDN_H,
	GPIO_CTRL_STATE_PDN_L,
	GPIO_CTRL_STATE_RST_H,
	GPIO_CTRL_STATE_RST_L,
	GPIO_CTRL_STATE_LDO_VCAMA_H,
	GPIO_CTRL_STATE_LDO_VCAMA_L,
	GPIO_CTRL_STATE_LDO_VCAMD_H,
	GPIO_CTRL_STATE_LDO_VCAMD_L,
	GPIO_CTRL_STATE_LDO_VCAMIO_H,
	GPIO_CTRL_STATE_LDO_VCAMIO_L,
	GPIO_CTRL_STATE_MAX_NUM_CAM,
};

#ifdef MIPI_SWITCH
enum GPIO_CTRL_STATE_SWITCH {
	GPIO_CTRL_STATE_MIPI_SWITCH_EN_H,
	GPIO_CTRL_STATE_MIPI_SWITCH_EN_L,
	GPIO_CTRL_STATE_MIPI_SWITCH_SEL_H,
	GPIO_CTRL_STATE_MIPI_SWITCH_SEL_L,
	GPIO_CTRL_STATE_MAX_NUM_SWITCH,
};
#endif

enum GPIO_STATE {
	GPIO_STATE_H,
	GPIO_STATE_L,
};

struct GPIO_PINCTRL {
	char *ppinctrl_lookup_names;
};

struct GPIO {
	struct pinctrl       *ppinctrl;
	struct pinctrl_state *ppinctrl_state_cam[
		IMGSENSOR_SENSOR_IDX_MAX_NUM][GPIO_CTRL_STATE_MAX_NUM_CAM];
#ifdef MIPI_SWITCH
	struct pinctrl       *ppinctrl_switch;
	struct pinctrl_state *ppinctrl_state_switch[
		GPIO_CTRL_STATE_MAX_NUM_SWITCH];
#endif
#ifdef OPLUS_FEATURE_CAMERA_COMMON
	#ifdef SENSOR_PLATFORM_4G_20682
	struct pinctrl_state *pinctrl_state_ldo_enable;
	#endif
#endif
	struct mutex         *pgpio_mutex;
};

enum IMGSENSOR_RETURN
imgsensor_hw_gpio_open(struct IMGSENSOR_HW_DEVICE **pdevice);

#endif

