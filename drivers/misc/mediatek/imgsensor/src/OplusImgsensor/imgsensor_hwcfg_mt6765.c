/*
 * Copyright (C) 2017 MediaTek Inc.
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
#ifndef __IMGSENSOR_HWCFG_20181_CTRL_H__
#define __IMGSENSOR_HWCFG_20181_CTRL_H__
#include "imgsensor_hwcfg_custom.h"

struct IMGSENSOR_HW_CFG *oplus_imgsensor_custom_config = NULL;
struct IMGSENSOR_HW_POWER_SEQ *oplus_sensor_power_sequence = NULL;
struct CAMERA_DEVICE_INFO gImgEepromInfo;

struct IMGSENSOR_HW_CFG imgsensor_custom_config_21331[] = {
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

    {IMGSENSOR_SENSOR_IDX_NONE}
};

struct IMGSENSOR_HW_POWER_SEQ sensor_power_sequence_21331[] = {
#if defined(ZHAOYUN_SHINETECH_MAIN_S5KJN103)
        {
            SENSOR_DRVNAME_ZHAOYUN_SHINETECH_MAIN_S5KJN103,
            {
                {RST, Vol_Low, 0},
                {DOVDD, Vol_1800, 1},
                {DVDD, Vol_1100, 2},
                {AVDD, Vol_2800, 1},
                {AFVDD, Vol_2800, 9},
                {RST, Vol_High, 6},
                {SensorMCLK, Vol_High, 1},
            },
        },
#endif
#if defined(ZHAOYUN_QTECH_MAIN_OV13B10)
            {
                  SENSOR_DRVNAME_ZHAOYUN_QTECH_MAIN_OV13B10,
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
#if defined(ZHAOYUNLITE_QTECH_MAIN_OV13B10)
		{
			SENSOR_DRVNAME_ZHAOYUNLITE_QTECH_MAIN_OV13B10,
			{
				{RST, Vol_Low, 0},
				{AVDD, Vol_2800, 1},
				{DOVDD, Vol_1800, 1},
				{DVDD, Vol_1200, 1},
				{AFVDD, Vol_2800, 9},
				{RST, Vol_High, 10},
				{SensorMCLK, Vol_High, 3},
			},
		},
#endif
#if defined(ZHAOYUN_OPTICS_FRONT_GC5035)
            {
                  SENSOR_DRVNAME_ZHAOYUN_OPTICS_FRONT_GC5035,
                  {
                        {RST, Vol_Low, 1},
                        {PDN, Vol_Low, 5},
                        {DOVDD, Vol_1800, 5},
                        {DVDD, Vol_1200, 5},
                        {AVDD, Vol_2800, 5},
                        {SensorMCLK, Vol_High, 5},
                        {RST, Vol_High, 1},
                        {PDN, Vol_High, 5},
                  },
            },
#endif
#if defined(ZHAOYUNLITE_QTECH_MAIN_S5K3L6)
            {
                  SENSOR_DRVNAME_ZHAOYUNLITE_QTECH_MAIN_S5K3L6,
                  {
                        {RST, Vol_Low, 1},
                        {DOVDD, Vol_1800, 1},
                        {DVDD, Vol_1050, 1},
                        {AVDD, Vol_2800, 1},
                        {AFVDD, Vol_2800, 1},
                        {RST, Vol_High, 1},
                        {SensorMCLK, Vol_High, 5},
                  },
            },
#endif
#if defined(ZHAOYUN_SHINETECH_MONO_GC02M1B)
            {
                  SENSOR_DRVNAME_ZHAOYUN_SHINETECH_MONO_GC02M1B,
                  {
                        {PDN, Vol_Low, 1},
                        {DOVDD, Vol_1800, 5},
                        {AVDD, Vol_2800, 0},
                        {PDN, Vol_High, 5},
                        {SensorMCLK, Vol_High, 3},
                  },
            },
#endif
#if defined(ZHAOYUN_SHINETECH_FRONT_OV08D10)
        {
            SENSOR_DRVNAME_ZHAOYUN_SHINETECH_FRONT_OV08D10,
            {
                {RST, Vol_Low, 3},
                {DOVDD, Vol_1800, 1},
                {AVDD, Vol_2800, 5},
                {DVDD, Vol_1200, 10},
                {RST, Vol_High, 3},
                {SensorMCLK, Vol_High, 8},
            },
        },
#endif
#if defined(ZHAOYUN_SHINETECH_MONO_GC02M1B_50M)
            {
                  SENSOR_DRVNAME_ZHAOYUN_SHINETECH_MONO_GC02M1B_50M,
                  {
                        {PDN, Vol_Low, 1},
                        {DOVDD, Vol_1800, 5},
                        {AVDD, Vol_2800, 0},
                        {PDN, Vol_High, 5},
                        {SensorMCLK, Vol_High, 3},
                  },
            },
#endif
    /* add new sensor before this line */
    {NULL,},
};


struct CAMERA_DEVICE_INFO gImgEepromInfo_21331 = {
    .i4SensorNum = 3,
    .pCamModuleInfo = {
        {ZHAOYUN_SHINETECH_MAIN_S5KJN103_SENSOR_ID,   0xA0, {0x00, 0x06}, 0xB0, 1, {0x92,0xFF,0xFF,0x94}, "Cam_r0", "s5kjn103"},
        {ZHAOYUN_SHINETECH_FRONT_OV08D10_SENSOR_ID,  0xA0, {0x00, 0x06}, 0xB0, 0, {0xFF,0xFF,0xFF,0xFF}, "Cam_f",  "ov08d10"},
        {ZHAOYUN_SHINETECH_MONO_GC02M1B_50M_SENSOR_ID,  0xFF, {0x00, 0x06}, 0xB0, 0, {0xFF,0xFF,0xFF,0xFF}, "Cam_r1", "gc02m1b"},
    },
    .i4MWDataIdx = IMGSENSOR_SENSOR_IDX_MAIN2,
    .i4MTDataIdx = 0xFF,
    .i4FrontDataIdx = 0xFF,
    .i4NormDataLen = 40,
    .i4MWDataLen = 3230,
    .i4MWStereoAddr = {0xFFFF, 0xFFFF},
    .i4MTStereoAddr = {0xFFFF, 0xFFFF},
    .i4FrontStereoAddr = {0xFFFF, 0xFFFF},
};

struct CAMERA_DEVICE_INFO gImgEepromInfo_21333 = {
    .i4SensorNum = 3,
    .pCamModuleInfo = {
        {ZHAOYUN_QTECH_MAIN_OV13B10_SENSOR_ID,   0xA0, {0x00, 0x06}, 0xB0, 1, {0x92,0xFF,0xFF,0x94}, "Cam_r0", "ov13b10"},
        {ZHAOYUN_SHINETECH_FRONT_OV08D10_SENSOR_ID,  0xA0, {0x00, 0x06}, 0xB0, 0, {0xFF,0xFF,0xFF,0xFF}, "Cam_f",  "ov08d10"},
        {ZHAOYUN_SHINETECH_MONO_GC02M1B_SENSOR_ID,  0xFF, {0x00, 0x06}, 0xB0, 0, {0xFF,0xFF,0xFF,0xFF}, "Cam_r1", "gc02m1b"},
    },
    .i4MWDataIdx = IMGSENSOR_SENSOR_IDX_MAIN2,
    .i4MTDataIdx = 0xFF,
    .i4FrontDataIdx = 0xFF,
    .i4NormDataLen = 40,
    .i4MWDataLen = 3230,
    .i4MWStereoAddr = {0xFFFF, 0xFFFF},
    .i4MTStereoAddr = {0xFFFF, 0xFFFF},
    .i4FrontStereoAddr = {0xFFFF, 0xFFFF},
};

struct CAMERA_DEVICE_INFO gImgEepromInfo_21107 = {
    .i4SensorNum = 3,
    .pCamModuleInfo = {
        {ZHAOYUN_QTECH_MAIN_OV13B10_SENSOR_ID,   0xA0, {0x00, 0x06}, 0xB0, 1, {0x92,0xFF,0xFF,0x94}, "Cam_r0", "ov13b10"},
        {ZHAOYUN_OPTICS_FRONT_GC5035_SENSOR_ID,  0xA8, {0x00, 0x06}, 0xB0, 0, {0xFF,0xFF,0xFF,0xFF}, "Cam_f",  "gc5035"},
    },
    .i4MWDataIdx = IMGSENSOR_SENSOR_IDX_SUB,
    .i4MTDataIdx = 0xFF,
    .i4FrontDataIdx = 0xFF,
    .i4NormDataLen = 40,
    .i4MWDataLen = 3230,
    .i4MWStereoAddr = {0xFFFF, 0xFFFF},
    .i4MTStereoAddr = {0xFFFF, 0xFFFF},
    .i4FrontStereoAddr = {0xFFFF, 0xFFFF},
};

void oplus_imgsensor_hwcfg()
{
    //zhaoyun 50M : 21331 21332 21337 21338
    //zhaoyun 13M : 21333 21334 21335 21336 21339
    if (is_project(21331) || is_project(21332) || is_project(21337)
        || is_project(21338)) {
        oplus_imgsensor_custom_config = imgsensor_custom_config_21331;
        oplus_sensor_power_sequence = sensor_power_sequence_21331;
        gImgEepromInfo = gImgEepromInfo_21331;
    } else if (is_project(21333) || is_project(21334) || is_project(21335)
        || is_project(21336) || is_project(21339)) {
        oplus_imgsensor_custom_config = imgsensor_custom_config_21331;
        oplus_sensor_power_sequence = sensor_power_sequence_21331;
        gImgEepromInfo = gImgEepromInfo_21333;
    } else if (is_project(21107) || is_project(21361) || is_project(21362)
        || is_project(21363)) {
        oplus_imgsensor_custom_config = imgsensor_custom_config_21331;
        oplus_sensor_power_sequence = sensor_power_sequence_21331;
        gImgEepromInfo = gImgEepromInfo_21107;
    } else {
        oplus_imgsensor_custom_config = imgsensor_custom_config_21331;
        oplus_sensor_power_sequence = sensor_power_sequence_21331;
    }
}

void oplus_imgsensor_delay_set(struct IMGSENSOR_HW_POWER_INFO *ppwr_info, struct IMGSENSOR_HW_POWER_SEQ *ppwr_seq)
{
      if (is_project(21331) || is_project(21332) || is_project(21333)
        || is_project(21334) || is_project(21335) || is_project(21336)
        || is_project(21337) || is_project(21338) || is_project(21339)) {
            if (ppwr_info->pin == IMGSENSOR_HW_PIN_MCLK &&
                  (!strcmp(ppwr_seq->name, "zhaoyun_shinetech_mono_gc02m1b") || !strcmp(ppwr_seq->name, "zhaoyun_shinetech_mono_gc02m1b_50m")))
                  //zhaoyun mono GC02M1B MCLK power off delay 2ms
                  mdelay(2);
            if (ppwr_info->pin == IMGSENSOR_HW_PIN_AVDD && !strcmp(ppwr_seq->name, "zhaoyun_shinetech_front_ov08d10"))
                  //zhaoyun front OV08D10 AVDD power off delay 2ms
                  mdelay(2);
            if (ppwr_info->pin == IMGSENSOR_HW_PIN_DVDD && !strcmp(ppwr_seq->name, "zhaoyun_shinetech_front_ov08d10"))
                  //zhaoyun front OV08D10 DVDD power off delay 2ms
                  mdelay(2);
      }
}
#endif
