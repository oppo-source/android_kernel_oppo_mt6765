// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include "kd_imgsensor.h"

#include "regulator/regulator.h"
#include "gpio/gpio.h"
/*#include "mt6306/mt6306.h"*/
#include "mclk/mclk.h"



#include "imgsensor_cfg_table.h"

#ifdef OPLUS_FEATURE_CAMERA_COMMON
#include <soc/oplus/system/oplus_project.h>
#include <soc/oplus/system/oplus_project_oldcdt.h>
#endif


enum IMGSENSOR_RETURN
	(*hw_open[IMGSENSOR_HW_ID_MAX_NUM])(struct IMGSENSOR_HW_DEVICE **) = {
	imgsensor_hw_regulator_open,
	imgsensor_hw_gpio_open,
	/*imgsensor_hw_mt6306_open,*/
	imgsensor_hw_mclk_open
};

struct IMGSENSOR_HW_CFG imgsensor_custom_config[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN3,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},

	{IMGSENSOR_SENSOR_IDX_NONE}
};

#ifdef OPLUS_FEATURE_CAMERA_COMMON
struct IMGSENSOR_HW_CFG imgsensor_custom_config_20701_D[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{IMGSENSOR_SENSOR_IDX_NONE}
};
struct IMGSENSOR_HW_CFG imgsensor_custom_config_parkera[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR,IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
};

struct IMGSENSOR_HW_CFG imgsensor_custom_config_pascald[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR,IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN3,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN4,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},

	{IMGSENSOR_SENSOR_IDX_NONE}
};


struct IMGSENSOR_HW_CFG imgsensor_custom_config_pascal[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR,IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_MIPI_SWITCH_SEL},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			//{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN3,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			//{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN4,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},

	{IMGSENSOR_SENSOR_IDX_NONE}
};

struct IMGSENSOR_HW_CFG imgsensor_custom_config_pascale[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR,IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			//{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN3,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN4,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},

	{IMGSENSOR_SENSOR_IDX_NONE}
};

struct IMGSENSOR_HW_CFG imgsensor_custom_config_yogurt[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR,IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			//{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},

	{IMGSENSOR_SENSOR_IDX_NONE}
};

struct IMGSENSOR_HW_CFG imgsensor_custom_config_yogurta[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR,IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{IMGSENSOR_SENSOR_IDX_NONE}
};

struct IMGSENSOR_HW_CFG imgsensor_custom_config_parker[] = {
        {
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR,IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			//{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_3,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
		  /*{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},*/
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
		  /*{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},*/
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},

		},
	},
	{IMGSENSOR_SENSOR_IDX_NONE}
};

/*Parker-B*/
struct IMGSENSOR_HW_CFG imgsensor_custom_config_parkerb[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR,IMGSENSOR_HW_PIN_AFVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			//{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			//{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			//{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{IMGSENSOR_SENSOR_IDX_NONE}
};

int yogurt_project(void){//// yogurt 1.yogurt 2 yogurte 3 yogurt 11
	int returnvalue = 0;
	const char *projectcdt = "0";
	struct device_node *of_node
		= of_find_compatible_node(NULL, NULL, "mediatek,camera_hw");
	if (of_property_read_string(
			of_node,
			"camera_yogurt",
			&projectcdt) < 0) {
			printk("Property camera_yogurt not defined\n");
	}
	if (strcmp(projectcdt, "20271") == 0) {
		returnvalue = 1;
        printk("cfg_setting_imgsensor,This is YOGURT_20271\n");
	} else {
        returnvalue = 0;
        printk("cfg_setting_imgsensor,This is NOT YOGURT_20271\n");
    }

    return returnvalue;
}

int yogurta_project(void){//// yogurta 1.yogurta
	int returnvalue = 0;
	const char *projectcdt = "0";
	struct device_node *of_node
		= of_find_compatible_node(NULL, NULL, "mediatek,camera_hw");
	if (of_property_read_string(
			of_node,
			"camera_yogurta",
			&projectcdt) < 0) {
			printk("Property camera_yogurta not defined\n");
	}
	if (strcmp(projectcdt, "21281") == 0) {
		returnvalue = 1;
        printk("cfg_setting_imgsensor,This is YOGURTA_21281\n");
	} else {
        returnvalue = 0;
        printk("cfg_setting_imgsensor,This is NOT YOGURTA_21281\n");
    }

    return returnvalue;
}

int pascal_project(void){//// pascald 1.pascal 2 pascale 3
	int projectvalue = 0;
	if (get_Operator_Version() == 131 || get_Operator_Version() == 132
		|| get_Operator_Version() == 133 || get_Operator_Version() == 134
		|| get_Operator_Version() == 135 || get_Operator_Version() == 171
		|| get_Operator_Version() == 172 || get_Operator_Version() == 173
		|| get_Operator_Version() == 174 || get_Operator_Version() == 175
		|| get_Operator_Version() == 176) {
		printk("This is pascald board\n");
		projectvalue = 1;
	} else if (get_Operator_Version() == 141 || get_Operator_Version() == 142
		|| get_Operator_Version() == 143 || get_Operator_Version() == 144
		|| get_Operator_Version() == 145 || get_Operator_Version() == 146) {
		printk("This is pascal board\n");
		projectvalue = 2;
	} else if (get_Operator_Version() == 136 || get_Operator_Version() == 137
		|| get_Operator_Version() == 138 || get_Operator_Version() == 139
		|| get_Operator_Version() == 140) {
		printk("This is pascale board\n");
		projectvalue = 3;
	} else if (get_Operator_Version() == 9 || is_project(20375)
		|| is_project(20376) || is_project(20377)
		|| is_project(20378) || is_project(20379)
		|| is_project(0x2037A)) {
		printk("This is parkera board\n");
		projectvalue = PARKERA_PROJECT;
	} else if (get_Operator_Version() >= 177 && get_Operator_Version() <= 184) {
		printk("This is ROI-D board\n");
		projectvalue = 5;
	} else if(is_project(21251) || is_project(21253) || is_project(21254)){
		printk("This is parker-B board\n");
		projectvalue = 6;
	} else {
		projectvalue = 0;
	}

	printk("pascal_project projectvalue = %d\n",projectvalue);
	return projectvalue;
}

int parker_project(void){
    int returnvalue = 0;
    const char *projectcdt = "0";
	struct device_node *of_node
		= of_find_compatible_node(NULL, NULL, "mediatek,camera_hw");
	if (of_property_read_string(
			of_node,
			"camera_parker",
			&projectcdt) < 0) {
			printk("Property camera_parker not defined\n");
	}
	if (strcmp(projectcdt, "20361") == 0) {
		returnvalue = 1;
        printk("cfg_setting_imgsensor,This is PARKER_20361\n");
	} else {
        returnvalue = 0;
        printk("cfg_setting_imgsensor,This is NOT PARKER_20361\n");
    }

    return returnvalue;
}
#endif  //OPLUS_FEATURE_CAMERA_COMMON

struct IMGSENSOR_HW_POWER_SEQ platform_power_sequence[] = {
#ifdef MIPI_SWITCH
	{
		PLATFORM_POWER_SEQ_NAME,
		{
			{
				IMGSENSOR_HW_PIN_MIPI_SWITCH_EN,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0,
				IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH,
				0
			},
			{
				IMGSENSOR_HW_PIN_MIPI_SWITCH_SEL,
				IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH,
				0,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0
			},
		},
		IMGSENSOR_SENSOR_IDX_SUB,
	},
	{
		PLATFORM_POWER_SEQ_NAME,
		{
			{
				IMGSENSOR_HW_PIN_MIPI_SWITCH_EN,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0,
				IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH,
				0
			},
			{
				IMGSENSOR_HW_PIN_MIPI_SWITCH_SEL,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0
			},
		},
		IMGSENSOR_SENSOR_IDX_MAIN2,
	},
#endif

	{NULL}
};

/* Legacy design */
struct IMGSENSOR_HW_POWER_SEQ sensor_power_sequence[] = {

#ifdef OPLUS_FEATURE_CAMERA_COMMON
#if defined(YOGURT_TRULY_MAIN_S5K3L6)
    {
        SENSOR_DRVNAME_YOGURT_TRULY_MAIN_S5K3L6,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1100, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 6},
            {SensorMCLK, Vol_High, 5},
        },
    },
#endif
#if defined(YOGURT_QTECH_MAIN_OV13B10)
    {
        SENSOR_DRVNAME_YOGURT_QTECH_MAIN_OV13B10,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(YOGURT_QTECH_FRONT_S5K4H7)
    {
        SENSOR_DRVNAME_YOGURT_QTECH_FRONT_S5K4H7,
        {
            {PDN, Vol_Low, 0},
            {RST, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 1},
            {RST, Vol_High, 1},
            {PDN, Vol_High, 1},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(YOGURT_CXT_DEPTH_GC02M1B)
    {
        SENSOR_DRVNAME_YOGURT_CXT_DEPTH_GC02M1B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 0},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(YOGURT_LHYX_MICRO_GC02K0)
    {
        SENSOR_DRVNAME_YOGURT_LHYX_MICRO_GC02K0,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 5},
            {SensorMCLK, Vol_High, 3},
            {PDN, Vol_High, 5},
        },
    },
#endif
#if defined(YOGURT_LCE_FRONT_HI556)
    {
        SENSOR_DRVNAME_YOGURT_LCE_FRONT_HI556,
        {
            {RST, Vol_Low, 0},
            {PDN, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {SensorMCLK, Vol_High, 1},
            {PDN, Vol_High, 1},
            {RST, Vol_High, 0},
        },
    },
#endif
#if defined(YOGURT_SHENGTAI_FRONT_GC5035)
    {
        SENSOR_DRVNAME_YOGURT_SHENGTAI_FRONT_GC5035,
        {
            {PDN, Vol_Low, 0},
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 1},
            {RST, Vol_High, 1},
            {PDN, Vol_High, 1},
            {SensorMCLK, Vol_High, 0},
        },
    },
#endif
#if defined(YOGURT_SHENGTAI_FRONT_OV8856)
    {
        SENSOR_DRVNAME_YOGURT_SHENGTAI_FRONT_OV8856,
        {
            {RST, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {SensorMCLK, Vol_High, 1},
            {RST, Vol_High, 5},
        },
    },
#endif
#if defined(YOGURT_LHYX_DEPTH_GC02M1B)
    {
        SENSOR_DRVNAME_YOGURT_LHYX_DEPTH_GC02M1B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 0},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(YOGURT_HLT_DEPTH_GC02M1B)
    {
        SENSOR_DRVNAME_YOGURT_HLT_DEPTH_GC02M1B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 0},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(YOGURT_SHENGTAI_MICRO_GC02K0)
    {
        SENSOR_DRVNAME_YOGURT_SHENGTAI_MICRO_GC02K0,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 5},
            {SensorMCLK, Vol_High, 3},
            {PDN, Vol_High, 5},
        },
    },
#endif
#if defined(YOGURT_CXT_MICRO_GC02K0)
    {
        SENSOR_DRVNAME_YOGURT_CXT_MICRO_GC02K0,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 5},
            {SensorMCLK, Vol_High, 3},
            {PDN, Vol_High, 5},
        },
    },
#endif
#if defined(YOGURT_CXT_MICRO_GC02M1)
    {
        SENSOR_DRVNAME_YOGURT_CXT_MICRO_GC02M1,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {PDN, Vol_High, 1},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(YOGURT_SUNNY_FRONT_S5K3P9SP)
    {
        SENSOR_DRVNAME_YOGURT_SUNNY_FRONT_S5K3P9SP,
        {
            {RST, Vol_Low, 1},
            {DVDD, Vol_1100, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {RST, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PARKERA_SHINETECH_MAIN_S5KJN103)
    {
        SENSOR_DRVNAME_PARKERA_SHINETECH_MAIN_S5KJN103,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1100, 2},
            {AVDD, Vol_2800, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 6},
            {SensorMCLK, Vol_High, 1},
        },
    },
#endif
#if defined(YOGURT_SHENGTAI_MONO_OV02B1B)
    {
        SENSOR_DRVNAME_YOGURT_SHENGTAI_MONO_OV02B1B,
        {
            {RST, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 9},
            {RST, Vol_High, 9},
        },
    },
#endif
#if defined(YOGURT_HLT_MICRO_GC02M1)
    {
	SENSOR_DRVNAME_YOGURT_HLT_MICRO_GC02M1,
		{
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {PDN, Vol_High, 1},
            {SensorMCLK, Vol_High, 3},
		},
	},
#endif
#if defined(YOGURT_HLT_MICRO_OV02B10)
    {
        SENSOR_DRVNAME_YOGURT_HLT_MICRO_OV02B10,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 9},
            {PDN, Vol_High, 9},
        },
    },
#endif
#if defined(PARKERA_QTECH_MAIN_OV13B10)
    {
        SENSOR_DRVNAME_PARKERA_QTECH_MAIN_OV13B10,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PARKERA_QTECH_MAIN_OV13B2A)
    {
        SENSOR_DRVNAME_PARKERA_QTECH_MAIN_OV13B2A,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 6},
            {SensorMCLK, Vol_High, 5},
        },
    },
#endif
#if defined(PARKERA_QTECH_FRONT_IMX355)
    {
        SENSOR_DRVNAME_PARKERA_QTECH_FRONT_IMX355,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {SensorMCLK, Vol_High, 5},
            {RST, Vol_High, 6},
        },
    },
#endif
#if defined(PARKERA_HOLITECH_FRONT_S5K4H7)
    {
        SENSOR_DRVNAME_PARKERA_HOLITECH_FRONT_S5K4H7,
        {
            {PDN, Vol_Low, 0},
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 1},
            {RST, Vol_High, 3},
            {PDN, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PARKERA_SHINETECH_FRONT_OV08D10)
    {
        SENSOR_DRVNAME_PARKERA_SHINETECH_FRONT_OV08D10,
        {
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 5},
            {RST, Vol_High, 3},
            {SensorMCLK, Vol_High, 8},
        },
    },
#endif
#if defined(PARKERA_SHINETECH_MONO_GC02M1B)
    {
        SENSOR_DRVNAME_PARKERA_SHINETECH_MONO_GC02M1B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 0},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PARKERA_SHINETECH_MONO_OV02B1B)
    {
        SENSOR_DRVNAME_PARKERA_SHINETECH_MONO_OV02B1B,
        {
            {PDN, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 5},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PARKERA_HOLITECH_MACRO_GC02M1)
    {
        SENSOR_DRVNAME_PARKERA_HOLITECH_MACRO_GC02M1,
        {
            {PDN, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 5},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PARKERA_SHINETECH_MACRO_OV02B10)
    {
        SENSOR_DRVNAME_PARKERA_SHINETECH_MACRO_OV02B10,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 0},
            {AVDD, Vol_2800, 9},
            {SensorMCLK, Vol_High, 1},
            {PDN, Vol_High, 4},
        },
    },
#endif
#if defined(PARKERA_SHINETECH_MONO_GC02M1B_50M)
    {
        SENSOR_DRVNAME_PARKERA_SHINETECH_MONO_GC02M1B_50M,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 0},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PARKERA_SHINETECH_MONO_OV02B1B_50M)
    {
        SENSOR_DRVNAME_PARKERA_SHINETECH_MONO_OV02B1B_50M,
        {
            {PDN, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 5},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCALD_TRULY_MAIN_OV13B10)
    {
        SENSOR_DRVNAME_PASCALD_TRULY_MAIN_OV13B10,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 6},
            {SensorMCLK, Vol_High, 5},
        },
    },
#endif
#if defined(PASCALD_QTECH_MAIN_OV13B10)
    {
        SENSOR_DRVNAME_PASCALD_QTECH_MAIN_OV13B10,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 6},
            {SensorMCLK, Vol_High, 5},
        },
    },
#endif

#if defined(PASCALD_HLT_FRONT_GC5035)
    {
        SENSOR_DRVNAME_PASCALD_HLT_FRONT_GC5035,
        {
            {PDN, Vol_Low, 0},
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 1},
            #ifdef OPLUS_FEATURE_CAMERA_COMMON
            {MIPISEL, Vol_High, 3},
            #endif
            {RST, Vol_High, 3},
            {PDN, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCALD_SHENGTAI_FRONT_GC5035)
    {
        SENSOR_DRVNAME_PASCALD_SHENGTAI_FRONT_GC5035,
        {
            {PDN, Vol_Low, 0},
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 1},
            {MIPISEL, Vol_High, 3},
            {RST, Vol_High, 3},
            {PDN, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCALD_SHENGTAI2_FRONT_GC5035)
    {
        SENSOR_DRVNAME_PASCALD_SHENGTAI2_FRONT_GC5035,
        {
            {PDN, Vol_Low, 0},
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 1},
            {MIPISEL, Vol_High, 3},
            {RST, Vol_High, 3},
            {PDN, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCALD_HLT_FRONT_S5K4H7)
    {
        SENSOR_DRVNAME_PASCALD_HLT_FRONT_S5K4H7,
        {
            {PDN, Vol_Low, 0},
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 1},
            {MIPISEL, Vol_High, 3},
            {RST, Vol_High, 3},
            {PDN, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCALD_HLT_DEPTH_GC02K0B)
    {
        SENSOR_DRVNAME_PASCALD_HLT_DEPTH_GC02K0B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 5},
            {SensorMCLK, Vol_High, 3},
            {PDN, Vol_High, 5},
        },
    },
#endif
#if defined(PASCALD_HLT_DEPTH_GC02M1B)
    {
        SENSOR_DRVNAME_PASCALD_HLT_DEPTH_GC02M1B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 0},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCALD_CXT_DEPTH_GC02M1B)
    {
        SENSOR_DRVNAME_PASCALD_CXT_DEPTH_GC02M1B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 0},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCALD_LHYX_DEPTH_OV02B1B)
    {
        SENSOR_DRVNAME_PASCALD_LHYX_DEPTH_OV02B1B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 5},
            {SensorMCLK, Vol_High, 3},
            {PDN, Vol_High, 5},
        },
    },
#endif
#if defined(PASCAL_HLT_MONO_GC02M1B)
    {
        SENSOR_DRVNAME_PASCAL_HLT_MONO_GC02M1B,
        {
            {RST, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {RST, Vol_High, 5},
            {SensorMCLK, Vol_High, 1},
        },
    },
#endif
#if defined(PASCAL_SHENGTAI_MONO1_OV02B1B)
    {
        SENSOR_DRVNAME_PASCAL_SHENGTAI_MONO1_OV02B1B,
        {
            //{PDN, Vol_Low, 0},
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            //{DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 5},
            //{MIPISEL, Vol_High, 3},
            {RST, Vol_High, 5},
            //{PDN, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCAL_LHYX_MONO1_OV02B1B)
    {
        SENSOR_DRVNAME_PASCAL_LHYX_MONO1_OV02B1B,
        {
            //{PDN, Vol_Low, 0},
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            //{DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 5},
            //{MIPISEL, Vol_High, 3},
            {RST, Vol_High, 5},
            //{PDN, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCAL_CXT_MONO1_GC02M1B)
    {
        SENSOR_DRVNAME_PASCAL_CXT_MONO1_GC02M1B,
        {
            {RST, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 5},
            {RST, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
#if defined(PASCAL_CXT_MONO_OV02B1B)
    {
        SENSOR_DRVNAME_PASCAL_CXT_MONO_OV02B1B,
        {
            {RST, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 9},
            {RST, Vol_High, 5},
            {SensorMCLK, Vol_High, 1},
        },
    },
#endif
#if defined(PASCAL_SHENGTAI_WIDE_OV8856)
    {
        SENSOR_DRVNAME_PASCAL_SHENGTAI_WIDE_OV8856,
        {
            {RST, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 0},
            {SensorMCLK, Vol_High, 1},
            {RST, Vol_High, 5},
        },
    },
#endif
#if defined(PASCAL_SUNNY_WIDE_OV8856)
    {
        SENSOR_DRVNAME_PASCAL_SUNNY_WIDE_OV8856,
        {
            {RST, Vol_Low, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 0},
            {SensorMCLK, Vol_High, 1},
            {RST, Vol_High, 5},
        },
    },
#endif
#if defined(PASCALE_HLT_MACRO_OV02B10)
    {
        SENSOR_DRVNAME_PASCALE_HLT_MACRO_OV02B10,
        {
            {RST, Vol_Low, 1},
            {DOVDD, Vol_1800, 0},
            {AVDD, Vol_2800, 9},
            {SensorMCLK, Vol_High, 1},
            {RST, Vol_High, 4},
        },
    },
#endif

/* Parker-B */
/* Main */
#if defined(PARKERB_SHINETECH_MAIN_S5KJN103)
    {
        SENSOR_DRVNAME_PARKERB_SHINETECH_MAIN_S5KJN103,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1100, 2},
            {AVDD, Vol_2800, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 6},
            {SensorMCLK, Vol_High, 1},
        },
    },
#endif
/* Front */
#if defined(PARKERB_SUNNY_FRONT_S5K3P9SP)
    {
        SENSOR_DRVNAME_PARKERB_SUNNY_FRONT_S5K3P9SP,
        {
            {RST, Vol_Low, 1},
            {DVDD, Vol_1100, 1},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {RST, Vol_High, 3},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
/* Depth */
#if defined(PARKERB_SHINETECH_MONO_GC02M1B)
    {
        SENSOR_DRVNAME_PARKERB_SHINETECH_MONO_GC02M1B,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 5},
            {AVDD, Vol_2800, 0},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
/* Depth */
#if defined(PARKERB_SHINETECH_MONO_OV02B1B)
    {
        SENSOR_DRVNAME_PARKERB_SHINETECH_MONO_OV02B1B,
        {
            {PDN, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 5},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif

/* Macro */
#if defined(PARKERB_HLT_MICRO_GC02M1)
    {
        SENSOR_DRVNAME_PARKERB_HLT_MICRO_GC02M1,
        {
            {PDN, Vol_Low, 3},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 5},
            {PDN, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
/* Macro */
#if defined(PARKERB_SHINETECH_MACRO_OV02B10)
    {
        SENSOR_DRVNAME_PARKERB_SHINETECH_MACRO_OV02B10,
        {
            {PDN, Vol_Low, 1},
            {DOVDD, Vol_1800, 0},
            {AVDD, Vol_2800, 9},
            {SensorMCLK, Vol_High, 1},
            {PDN, Vol_High, 4},
        },
    },
#endif
#else //OPLUS_FEATURE_CAMERA_COMMON
#if defined(IMX398_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX398_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 0},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(OV23850_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV23850_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 2},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif
#if defined(IMX386_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX386_MIPI_RAW,
		{
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1100, 0},
			{DOVDD, Vol_1800, 0},
			{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif
#if defined(IMX386_MIPI_MONO)
	{
		SENSOR_DRVNAME_IMX386_MIPI_MONO,
		{
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1100, 0},
			{DOVDD, Vol_1800, 0},
			{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif

#if defined(IMX338_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX338_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2500, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 0},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(S5K4E6_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K4E6_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2900, 0},
			{DVDD, Vol_1200, 2},
			{AFVDD, Vol_2800, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K3P8SP_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3P8SP_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0},
		},
	},
#endif
#if defined(S5K2T7SP_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2T7SP_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0},
		},
	},
#endif
#if defined(IMX230_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX230_MIPI_RAW,
		{
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{AVDD, Vol_2500, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 10}
		},
	},
#endif
#if defined(S5K3M2_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3M2_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K3P3SX_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3P3SX_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K5E2YA_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K4ECGX_MIPI_YUV)
	{
		SENSOR_DRVNAME_S5K4ECGX_MIPI_YUV,
		{
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_2800, 1},
			{DOVDD, Vol_1800, 1},
			{AFVDD, Vol_2800, 0},
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(OV16880_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV16880_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 1},
			{RST, Vol_High, 2}
		},
	},
#endif
#if defined(S5K2P7_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2P7_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1000, 1},
			{DOVDD, Vol_1800, 1},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_Low, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0},
		},
	},
#endif
#if defined(S5K2P8_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2P8_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX258_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX258_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX258_MIPI_MONO)
	{
		SENSOR_DRVNAME_IMX258_MIPI_MONO,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX377_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX377_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(OV8858_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV8858_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 1},
			{RST, Vol_High, 2}
		},
	},
#endif
#if defined(OV8856_MIPI_RAW)
	{SENSOR_DRVNAME_OV8856_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 2},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif
#if defined(S5K2X8_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2X8_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX214_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX214_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(IMX214_MIPI_MONO)
	{
		SENSOR_DRVNAME_IMX214_MIPI_MONO,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(S5K3L8_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3L8_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX362_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX362_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 10},
			{DOVDD, Vol_1800, 10},
			{DVDD, Vol_1200, 10},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K2L7_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2L7_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 3},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(IMX318_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX318_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 10},
			{DOVDD, Vol_1800, 10},
			{DVDD, Vol_1200, 10},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(OV8865_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV8865_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 5},
			{RST, Vol_Low, 5},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 5},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(IMX219_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX219_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 10},
			{DOVDD, Vol_1800, 10},
			{DVDD, Vol_1000, 10},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K3M3_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3M3_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 2}
		},
	},
#endif
#if defined(OV5670_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV5670_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 5},
			{RST, Vol_Low, 5},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 5},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV5670_MIPI_RAW_2)
	{
		SENSOR_DRVNAME_OV5670_MIPI_RAW_2,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 5},
			{RST, Vol_Low, 5},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 5},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV20880_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV20880_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_Low, 1},
			{AVDD, Vol_2800, 1},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1100, 1},
			{RST, Vol_High, 5}
		},
	},
#endif

#endif //OPLUS_FEATURE_CAMERA_COMMON

	/* add new sensor before this line */
	{NULL,},
};

struct IMGSENSOR_HW_POWER_SEQ  sensor_power_sequence_yogurta[] = {
/* YogurtA */
/* Main */
#if defined(YOGURTA_QTECH_MAIN_OV13B10)
    {
        SENSOR_DRVNAME_YOGURTA_QTECH_MAIN_OV13B10,
        {
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {AFVDD, Vol_2800, 1},
            {RST, Vol_High, 5},
            {SensorMCLK, Vol_High, 3},
        },
    },
#endif
/* Front */
#if defined(YOGURTA_SHENGTAI_FRONT_GC5035)
    {
        SENSOR_DRVNAME_YOGURTA_SHENGTAI_FRONT_GC5035,
        {
            {PDN, Vol_Low, 0},
            {RST, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {DVDD, Vol_1200, 1},
            {AVDD, Vol_2800, 1},
            {RST, Vol_High, 1},
            {PDN, Vol_High, 1},
            {SensorMCLK, Vol_High, 0},
        },
    },
#endif
#if defined(YOGURTA_TXD_FRONT_HI556)
    {
        SENSOR_DRVNAME_YOGURTA_TXD_FRONT_HI556,
        {
            {RST, Vol_Low, 0},
            {PDN, Vol_Low, 0},
            {DOVDD, Vol_1800, 1},
            {AVDD, Vol_2800, 1},
            {DVDD, Vol_1200, 1},
            {SensorMCLK, Vol_High, 1},
            {PDN, Vol_High, 1},
            {RST, Vol_High, 0},
        },
    },
#endif

    /* add new sensor before this line */
    {NULL,},
};

struct IMGSENSOR_HW_POWER_SEQ sensor_power_sequence_20701_D[] = {
#if defined(HI846_MIPI_RAW)
		{
			SENSOR_DRVNAME_HI846_MIPI_RAW,
			{
				{RST, Vol_Low, 1},
				{DOVDD, Vol_1800, 0},
				{AVDD, Vol_High, 0},
				{DVDD, Vol_1200, 0},
				{AFVDD, Vol_2800, 1},
				{SensorMCLK, Vol_High, 3},
				{RST, Vol_High, 1}
			},
		},
#endif
#if defined(HI556_MIPI_RAW)
		{
			SENSOR_DRVNAME_HI556_MIPI_RAW,
			{
				{RST, Vol_Low, 1},
				{DOVDD, Vol_1800, 1},
				{AVDD, Vol_High, 1},
				{DVDD, Vol_1200, 1},
				{SensorMCLK, Vol_High, 2},
				{RST, Vol_High, 1}
			},
		},
#endif
#if defined(GC5035_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC5035_MIPI_RAW,
		{
			{RST, Vol_Low, 5},
			{DOVDD, Vol_1800, 5},
			{DVDD, Vol_1200, 5},
			{AVDD, Vol_High, 5},
			{SensorMCLK, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV13B10_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV13B10_MIPI_RAW,
		{
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_High, 1},
			{DVDD, Vol_1200, 1},
			{AFVDD, Vol_2800, 1},
			{RST, Vol_High, 6},
			{SensorMCLK, Vol_High, 5},
		},
	},
#endif
#if defined(GC8034_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC8034_MIPI_RAW,
		{
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_High, 1},
			{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 3},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(HI1336_MIPI_RAW)
	{
		SENSOR_DRVNAME_HI1336_MIPI_RAW,
		{
			{RST, Vol_Low, 5},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_High, 1},
			{DVDD, Vol_1200, 1},
			{SensorMCLK, Vol_High, 5},
			{AFVDD, Vol_2800, 5},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(GC030A_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC030A_MIPI_RAW,
		{
			{PDN, Vol_High, 5},
			{DOVDD, Vol_1800, 2},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 1},
			{SensorMCLK, Vol_High, 5},
			{PDN, Vol_Low, 2},
		},
	},
#endif
#if defined(GC02M1B_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC02M1B_MIPI_RAW,
		{
			{PDN, Vol_Low, 1},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 1},
			{PDN, Vol_High, 5},
			{SensorMCLK, Vol_High, 5},
		},
	},
#endif
#if defined(GC02M1B_SW_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC02M1B_SW_MIPI_RAW,
		{
			{PDN, Vol_Low, 1},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 1},
			{PDN, Vol_High, 5},
			{SensorMCLK, Vol_High, 5},
		},
	},
#endif
#if defined(S5K4H7_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K4H7_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 1},
			{RST, Vol_Low, 1},
			{AVDD, Vol_High, 1},
			{DVDD, Vol_1200, 1},
			{DOVDD, Vol_1800, 1},
			{AFVDD, Vol_2800, 1},
			{RST, Vol_High, 5},
		},
	},
#endif
#if defined(HI556_TXD_MIPI_RAW)
	{
		SENSOR_DRVNAME_HI556_TXD_MIPI_RAW,
		{
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_High, 1},
			{DVDD, Vol_1200, 1},
			{SensorMCLK, Vol_High, 2},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(OV02B1B_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV02B1B_MIPI_RAW,
		{
			{PDN, Vol_Low, 1},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 5},
			{SensorMCLK, Vol_High, 3},
			{PDN, Vol_High, 5 , Vol_High, 1},
		},
	},
#endif
#if defined(OV02B10_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV02B10_MIPI_RAW,
		{
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 5},
			{SensorMCLK, Vol_High, 3},
			{RST, Vol_High, 5 , Vol_High, 1},
		},
	},
#endif
#if defined(GC02M1_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC02M1_MIPI_RAW,
		{
			{RST, Vol_Low, 3},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{RST, Vol_High, 5},
			{SensorMCLK, Vol_High, 3},
		},
	},
#endif
#if defined(GC02M1_SW_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC02M1_SW_MIPI_RAW,
		{
			{RST, Vol_Low, 3},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{RST, Vol_High, 5},
			{SensorMCLK, Vol_High, 3},
		},
	},
#endif
	{NULL}
};


