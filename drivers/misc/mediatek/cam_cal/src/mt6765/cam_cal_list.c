// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 MediaTek Inc.
 */
#include <linux/kernel.h>
#include "cam_cal_list.h"
#include "eeprom_i2c_common_driver.h"
#include "eeprom_i2c_custom_driver.h"
#include "kd_imgsensor.h"

struct stCAM_CAL_LIST_STRUCT g_camCalList[] = {
	#ifdef OPLUS_FEATURE_CAMERA_COMMON
	{YOGURT_TRULY_MAIN_S5K3L6_SENSOR_ID, 0xA0, Common_read_region},
	{YOGURT_QTECH_MAIN_OV13B10_SENSOR_ID, 0xA0, Common_read_region},
	{YOGURT_HLT_FRONT_GC5035_SENSOR_ID, 0xA8, Common_read_region},
	{YOGURT_SHENGTAI_FRONT_GC5035_SENSOR_ID, 0xA8, Common_read_region},
	{YOGURT_SUNNY_FRONT_GC5035_SENSOR_ID, 0xA8, Common_read_region},
	{YOGURT_LCE_FRONT_HI556_SENSOR_ID, 0xA8, Common_read_region},
	{YOGURT_SHENGTAI_FRONT_OV8856_SENSOR_ID, 0xA0, Common_read_region},
	{YOGURT_HLT_DEPTH_GC02M1B_MIPI_SENSOR_ID, 0xA2, Common_read_region},
	{YOGURT_SHENGTAI_MICRO_GC02K0_SENSOR_ID, 0xA4, Common_read_region},
	{YOGURT_LHYX_MICRO_GC02K0_SENSOR_ID, 0xA4, Common_read_region},
	{YOGURT_CXT_MICRO_GC02K0_SENSOR_ID, 0xA4, Common_read_region},
	{YOGURT_CXT_MICRO_GC02M1_SENSOR_ID, 0xA4, Common_read_region},
	{YOGURT_HLT_MICRO_GC02M1_SENSOR_ID, 0xA4, Common_read_region},
	{YOGURT_SUNNY_FRONT_S5K3P9SP_SENSOR_ID, 0xA8, Common_read_region},
	{YOGURT_HLT_MICRO_OV02B10_SENSOR_ID, 0xA4, Common_read_region},

	{YOGURT_TRULY_MAIN_S5K3L6_SENSOR_ID_2G, 0xA0, Common_read_region},
	{YOGURT_QTECH_MAIN_OV13B10_SENSOR_ID_2G, 0xA0, Common_read_region},
	{YOGURT_SHENGTAI_FRONT_GC5035_SENSOR_ID_2G, 0xA8, Common_read_region},
	{YOGURT_HLT_DEPTH_GC02M1B_MIPI_SENSOR_ID_2G, 0xA2, Common_read_region},
	{YOGURT_LCE_FRONT_HI556_SENSOR_ID_2G, 0xA8, Common_read_region},
	{YOGURT_SHENGTAI_FRONT_OV8856_SENSOR_ID_2G, 0xA0, Common_read_region},
	{YOGURT_SHENGTAI_MICRO_GC02K0_SENSOR_ID_2G, 0xA4, Common_read_region},
	{YOGURT_LHYX_MICRO_GC02K0_SENSOR_ID_2G, 0xA4, Common_read_region},
	{YOGURT_CXT_MICRO_GC02K0_SENSOR_ID_2G, 0xA4, Common_read_region},
	{YOGURT_CXT_MICRO_GC02M1_SENSOR_ID_2G, 0xA4, Common_read_region},
	{PASCALD_QTECH_MAIN_OV13B10_SENSOR_ID, 0xA0, Common_read_region},
	{PASCALD_HLT_FRONT_S5K4H7_SENSOR_ID, 0xA8, Common_read_region},
	{PASCAL_SHENGTAI_WIDE_OV8856_SENSOR_ID, 0xA2, Common_read_region},

	{PASCALD_SHENGTAI_FRONT_GC5035_SENSOR_ID, 0xA8, Common_read_region},
	{PASCALE_HLT_MACRO_OV02B10_SENSOR_ID, 0xA4, Common_read_region},
	{PARKERA_QTECH_MAIN_OV13B10_SENSOR_ID, 0xA0, Common_read_region},
	{PARKERA_QTECH_MAIN_OV13B2A_SENSOR_ID, 0xA0, Common_read_region},
	{PARKERA_SHINETECH_MAIN_S5KJN103_SENSOR_ID, 0xA0, Common_read_region},
	{PARKERA_SHINETECH_FRONT_OV08D10_SENSOR_ID, 0xA0, Common_read_region},
	{PARKERA_QTECH_FRONT_IMX355_SENSOR_ID, 0xA8, Common_read_region},
	{PARKERA_HOLITECH_MACRO_GC02M1_SENSOR_ID, 0xA4, Common_read_region},
	{PARKERA_SHINETECH_MACRO_OV02B10_SENSOR_ID, 0xA4, Common_read_region},

	{PARKERB_SHINETECH_MAIN_S5KJN103_SENSOR_ID, 0xA0, Common_read_region},
	{PARKERB_SUNNY_FRONT_S5K3P9SP_SENSOR_ID, 0xA8, Common_read_region},
	{PARKERB_HLT_MICRO_GC02M1_SENSOR_ID, 0xA4, Common_read_region},
	{PARKERB_SHINETECH_MACRO_OV02B10_SENSOR_ID, 0xA4, Common_read_region},

	{YOGURTA_QTECH_MAIN_OV13B10_SENSOR_ID, 0xA0, Common_read_region},
	{YOGURTA_SHENGTAI_FRONT_GC5035_SENSOR_ID, 0xA8, Common_read_region},
	{YOGURTA_TXD_FRONT_HI556_SENSOR_ID, 0x40, yogurta_hi556_read_region},

	{ROID_HI556_SENSOR_ID, 0x40, Hi556_read_region},
	{ROID_GC5035_SENSOR_ID, 0xA0, Common_read_region},
	{ROID_HI556_TXD_SENSOR_ID, 0x50, Hi556_TXD_read_region},
	{ROID_HI1336_SENSOR_ID, 0xA0, Common_read_region},
	{ROID_OV13B10_SENSOR_ID, 0xA0, Common_read_region},
	{ROID_GC02M1_SENSOR_ID, 0xA4, Common_read_region},
	{ROID_OV02B10_SENSOR_ID, 0xA4, Common_read_region},
	{ROID_GC02M1_SW_SENSOR_ID, 0xA4, Common_read_region},
	#endif /* OPLUS_FEATURE_CAMERA_COMMON */
	/*Below is commom sensor */
	{IMX230_SENSOR_ID, 0xA0, Common_read_region},
	{S5K2T7SP_SENSOR_ID, 0xA4, Common_read_region},
	{IMX338_SENSOR_ID, 0xA0, Common_read_region},
	{S5K4E6_SENSOR_ID, 0xA8, Common_read_region},
	{IMX386_SENSOR_ID, 0xA0, Common_read_region},
	{S5K3M3_SENSOR_ID, 0xA0, Common_read_region},
	{S5K2L7_SENSOR_ID, 0xA0, Common_read_region},
	{IMX398_SENSOR_ID, 0xA0, Common_read_region},
	{IMX318_SENSOR_ID, 0xA0, Common_read_region},
	{OV8858_SENSOR_ID, 0xA8, Common_read_region},
	{IMX386_MONO_SENSOR_ID, 0xA0, Common_read_region},
	/*B+B*/
	{S5K2P7_SENSOR_ID, 0xA0, Common_read_region},
	{OV8856_SENSOR_ID, 0xA0, Common_read_region},
	/*61*/
	{IMX499_SENSOR_ID, 0xA0, Common_read_region},
	{S5K3L8_SENSOR_ID, 0xA0, Common_read_region},
	{S5K5E8YX_SENSOR_ID, 0xA2, Common_read_region},
	/*99*/
	{IMX258_SENSOR_ID, 0xA0, Common_read_region},
	{IMX258_MONO_SENSOR_ID, 0xA0, Common_read_region},
	/*97*/
	{OV23850_SENSOR_ID, 0xA0, Common_read_region},
	{OV23850_SENSOR_ID, 0xA8, Common_read_region},
	{S5K3M2_SENSOR_ID, 0xA0, Common_read_region},
	/*55*/
	{S5K2P8_SENSOR_ID, 0xA2, Common_read_region},
	{S5K2P8_SENSOR_ID, 0xA0, Common_read_region},
	{OV8858_SENSOR_ID, 0xA2, Common_read_region},
	/* Others */
	{S5K2X8_SENSOR_ID, 0xA0, Common_read_region},
	{IMX377_SENSOR_ID, 0xA0, Common_read_region},
	{IMX214_SENSOR_ID, 0xA0, Common_read_region},
	{IMX214_MONO_SENSOR_ID, 0xA0, Common_read_region},
	{IMX486_SENSOR_ID, 0xA8, Common_read_region},
	{OV12A10_SENSOR_ID, 0xA8, Common_read_region},
	{OV13855_SENSOR_ID, 0xA0, Common_read_region},
	{S5K3L8_SENSOR_ID, 0xA0, Common_read_region},
	{HI556_SENSOR_ID, 0x51, Common_read_region},
	{S5K5E8YX_SENSOR_ID, 0x5a, Common_read_region},
	{S5K5E8YXREAR2_SENSOR_ID, 0x5a, Common_read_region},
	/*  ADD before this line */
	{0, 0, 0}       /*end of list */
};

unsigned int cam_cal_get_sensor_list(
	struct stCAM_CAL_LIST_STRUCT **ppCamcalList)
{
	if (ppCamcalList == NULL)
		return 1;

	*ppCamcalList = &g_camCalList[0];
	return 0;
}


