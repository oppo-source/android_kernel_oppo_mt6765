/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef _KD_IMGSENSOR_H
#define _KD_IMGSENSOR_H

#include <linux/ioctl.h>

#ifndef ASSERT
#define ASSERT(expr)        WARN_ON(!(expr))
#endif

#define IMGSENSORMAGIC 'i'
/* IOCTRL(inode * ,file * ,cmd ,arg ) */
/* S means "set through a ptr" */
/* T means "tell by a arg value" */
/* G means "get by a ptr" */
/* Q means "get by return a value" */
/* X means "switch G and S atomically" */
/* H means "switch T and Q atomically" */

/******************************************************************************
 *
 ******************************************************************************/

/* sensorOpen */
#define KDIMGSENSORIOC_T_OPEN \
	_IO(IMGSENSORMAGIC, 0)
/* sensorGetInfo */
#define KDIMGSENSORIOC_X_GET_CONFIG_INFO \
	_IOWR(IMGSENSORMAGIC, 5, struct IMGSENSOR_GET_CONFIG_INFO_STRUCT)

#define KDIMGSENSORIOC_X_GETINFO \
	_IOWR(IMGSENSORMAGIC, 5, struct ACDK_SENSOR_GETINFO_STRUCT)
/* sensorGetResolution */
#define KDIMGSENSORIOC_X_GETRESOLUTION \
	_IOWR(IMGSENSORMAGIC, 10, struct ACDK_SENSOR_RESOLUTION_INFO_STRUCT)
/* For kernel 64-bit */
#define KDIMGSENSORIOC_X_GETRESOLUTION2 \
	_IOWR(IMGSENSORMAGIC, 10, struct ACDK_SENSOR_PRESOLUTION_STRUCT)
/* sensorFeatureControl */
#define KDIMGSENSORIOC_X_FEATURECONCTROL \
	_IOWR(IMGSENSORMAGIC, 15, struct ACDK_SENSOR_FEATURECONTROL_STRUCT)
/* sensorControl */
#define KDIMGSENSORIOC_X_CONTROL \
	_IOWR(IMGSENSORMAGIC, 20, struct ACDK_SENSOR_CONTROL_STRUCT)
/* sensorClose */
#define KDIMGSENSORIOC_T_CLOSE \
	_IO(IMGSENSORMAGIC, 25)
/* sensorSearch */
#define KDIMGSENSORIOC_T_CHECK_IS_ALIVE \
	_IO(IMGSENSORMAGIC, 30)
/* set sensor driver */
#define KDIMGSENSORIOC_X_SET_DRIVER \
	_IOWR(IMGSENSORMAGIC, 35, struct SENSOR_DRIVER_INDEX_STRUCT)
/* get socket postion */
#define KDIMGSENSORIOC_X_GET_SOCKET_POS \
	_IOWR(IMGSENSORMAGIC, 40, u32)
/* set I2C bus */
#define KDIMGSENSORIOC_X_SET_I2CBUS \
	_IOWR(IMGSENSORMAGIC, 45, u32)
/* set I2C bus */
#define KDIMGSENSORIOC_X_RELEASE_I2C_TRIGGER_LOCK \
	_IO(IMGSENSORMAGIC, 50)
/* Set Shutter Gain Wait Done */
#define KDIMGSENSORIOC_X_SET_SHUTTER_GAIN_WAIT_DONE \
	_IOWR(IMGSENSORMAGIC, 55, u32)
/* set mclk */
#define KDIMGSENSORIOC_X_SET_MCLK_PLL \
	_IOWR(IMGSENSORMAGIC, 60, struct ACDK_SENSOR_MCLK_STRUCT)
#define KDIMGSENSORIOC_X_GETINFO2 \
	_IOWR(IMGSENSORMAGIC, 65, struct IMAGESENSOR_GETINFO_STRUCT)
/* set open/close sensor index */
#define KDIMGSENSORIOC_X_SET_CURRENT_SENSOR \
	_IOWR(IMGSENSORMAGIC, 70, u32)
/* set GPIO */
#define KDIMGSENSORIOC_X_SET_GPIO \
	_IOWR(IMGSENSORMAGIC, 75, struct IMGSENSOR_GPIO_STRUCT)
/* Get ISP CLK */
#define KDIMGSENSORIOC_X_GET_ISP_CLK \
	_IOWR(IMGSENSORMAGIC, 80, u32)
/* Get CSI CLK */
#define KDIMGSENSORIOC_X_GET_CSI_CLK \
	_IOWR(IMGSENSORMAGIC, 85, u32)

/* Get ISP CLK via MMDVFS*/
#define KDIMGSENSORIOC_DFS_UPDATE \
	_IOWR(IMGSENSORMAGIC, 90, unsigned int)
#define KDIMGSENSORIOC_GET_SUPPORTED_ISP_CLOCKS \
	_IOWR(IMGSENSORMAGIC, 95, struct IMAGESENSOR_GET_SUPPORTED_ISP_CLK)
#define KDIMGSENSORIOC_GET_CUR_ISP_CLOCK \
	_IOWR(IMGSENSORMAGIC, 100, unsigned int)

#ifdef CONFIG_COMPAT
#define COMPAT_KDIMGSENSORIOC_X_GET_CONFIG_INFO \
	_IOWR(IMGSENSORMAGIC, 5, struct COMPAT_IMGSENSOR_GET_CONFIG_INFO_STRUCT)

#define COMPAT_KDIMGSENSORIOC_X_GETINFO \
	_IOWR(IMGSENSORMAGIC, 5, struct COMPAT_ACDK_SENSOR_GETINFO_STRUCT)
#define COMPAT_KDIMGSENSORIOC_X_FEATURECONCTROL \
	_IOWR(IMGSENSORMAGIC, 15, \
		struct COMPAT_ACDK_SENSOR_FEATURECONTROL_STRUCT)
#define COMPAT_KDIMGSENSORIOC_X_CONTROL \
	_IOWR(IMGSENSORMAGIC, 20, struct COMPAT_ACDK_SENSOR_CONTROL_STRUCT)
#define COMPAT_KDIMGSENSORIOC_X_GETINFO2 \
	_IOWR(IMGSENSORMAGIC, 65, struct COMPAT_IMAGESENSOR_GETINFO_STRUCT)
#define COMPAT_KDIMGSENSORIOC_X_GETRESOLUTION2 \
	_IOWR(IMGSENSORMAGIC, 10, struct COMPAT_ACDK_SENSOR_PRESOLUTION_STRUCT)
#endif

/************************************************************************
 *
 ************************************************************************/
/* SENSOR CHIP VERSION */
/*IMX*/
#define IMX499_SENSOR_ID                        0x0499
#define IMX486_SENSOR_ID                        0x0486
#define IMX586_SENSOR_ID                        0x0586
#define IMX519_SENSOR_ID                        0x0519
#define IMX576_SENSOR_ID                        0x0576
#define IMX350_SENSOR_ID                        0x0350
#define IMX398_SENSOR_ID                        0x0398
#define IMX268_SENSOR_ID                        0x0268
#define IMX386_SENSOR_ID                        0x0386
#define IMX300_SENSOR_ID                        0x0300
#define IMX386_MONO_SENSOR_ID                   0x0286
#define IMX362_SENSOR_ID                        0x0362
#define IMX338_SENSOR_ID                        0x0338
#define IMX376_SENSOR_ID                        0x0376
#define IMX318_SENSOR_ID                        0x0318
#define IMX377_SENSOR_ID                        0x0377
#define IMX278_SENSOR_ID                        0x0278
#define IMX258_SENSOR_ID                        0x0258
#define IMX258_MONO_SENSOR_ID                   0x0259
#define IMX230_SENSOR_ID                        0x0230
#define IMX220_SENSOR_ID                        0x0220
#define IMX219_SENSOR_ID                        0x0219
#define IMX214_SENSOR_ID                        0x0214
#define IMX214_MONO_SENSOR_ID                   0x0215
#define IMX179_SENSOR_ID                        0x0179
#define IMX178_SENSOR_ID                        0x0178
#define IMX135_SENSOR_ID                        0x0135
#define IMX132MIPI_SENSOR_ID                    0x0132
#define IMX119_SENSOR_ID                        0x0119
#define IMX105_SENSOR_ID                        0x0105
#define IMX091_SENSOR_ID                        0x0091
#define IMX073_SENSOR_ID                        0x0046
#define IMX058_SENSOR_ID                        0x0058
/*OV*/
#define OV23850_SENSOR_ID                       0x023850
#define OV16880_SENSOR_ID                       0x016880
#define OV16825MIPI_SENSOR_ID                   0x016820
#define OV13855_SENSOR_ID                       0xD855
#define OV13850_SENSOR_ID                       0xD850
#define OV12A10_SENSOR_ID                       0x1241
#define OV13870_SENSOR_ID                       0x013870
#define OV16885_SENSOR_ID                       0x16885
#define OV13855MAIN2_SENSOR_ID                  0xD856
#define OV12830_SENSOR_ID                       0xC830
#define OV9760MIPI_SENSOR_ID                    0x9760
#define OV9740MIPI_SENSOR_ID                    0x9740
#define OV9726_SENSOR_ID                        0x9726
#define OV9726MIPI_SENSOR_ID                    0x9726
#define OV8865_SENSOR_ID                        0x8865
#define OV8858_SENSOR_ID                        0x8858
#define OV8858S_SENSOR_ID                      (0x8858+1)
#define OV8856_SENSOR_ID                        0x885A
#define OV8830_SENSOR_ID                        0x8830
#define OV8825_SENSOR_ID                        0x8825
#define OV7675_SENSOR_ID                        0x7673
#define OV5693_SENSOR_ID                        0x5690
#define OV5670MIPI_SENSOR_ID                    0x5670
#define OV5670MIPI_SENSOR_ID_2                  (0x5670+010000)
#define OV2281MIPI_SENSOR_ID                    0x5670
#define OV5675MIPI_SENSOR_ID                    0x5675
#define OV5671MIPI_SENSOR_ID                    0x5671
#define OV5650_SENSOR_ID                        0x5651
#define OV5650MIPI_SENSOR_ID                    0x5651
#define OV5648MIPI_SENSOR_ID                    0x5648
#define OV5647_SENSOR_ID                        0x5647
#define OV5647MIPI_SENSOR_ID                    0x5647
#define OV5645MIPI_SENSOR_ID                    0x5645
#define OV5642_SENSOR_ID                        0x5642
#define OV4688MIPI_SENSOR_ID                    0x4688
#define OV3640_SENSOR_ID                        0x364C
#define OV2724MIPI_SENSOR_ID                    0x2724
#define OV2722MIPI_SENSOR_ID                    0x2722
#define OV2680MIPI_SENSOR_ID                    0x2680
#define OV2680_SENSOR_ID                        0x2680
#define OV2659_SENSOR_ID                        0x2656
#define OV2655_SENSOR_ID                        0x2656
#define OV2650_SENSOR_ID                        0x2652
#define OV2650_SENSOR_ID_1                      0x2651
#define OV2650_SENSOR_ID_2                      0x2652
#define OV2650_SENSOR_ID_3                      0x2655
#define OV20880MIPI_SENSOR_ID                   0x20880
#define OV13B10_SENSOR_ID                       0x560D42
/*S5K*/
#define S5K2LQSX_SENSOR_ID                      0x2c1a
#define S5K4H7_SENSOR_ID                        0x487B
#define S5K3P8SP_SENSOR_ID                      0x3108
#define S5K2T7SP_SENSOR_ID                      0x2147
#define S5K3P8SX_SENSOR_ID                      0x3108
#define S5K2L7_SENSOR_ID                        0x20C7
#define S5K3L6_SENSOR_ID                        0x30C6
#define S5K3L8_SENSOR_ID                        0x30C8
#define S5K3M3_SENSOR_ID                        0x30D3
#define S5K2X8_SENSOR_ID                        0x2188
#define S5K2P7_SENSOR_ID                        0x2107
#define S5K2P8_SENSOR_ID                        0x2108
#define S5K3P3_SENSOR_ID                        0x3103
#define S5K3P3SX_SENSOR_ID                      0x3103
#define S5K3P8_SENSOR_ID                        0x3108
#define S5K3P8STECH_SENSOR_ID                   0xf3108
#define S5K3M2_SENSOR_ID                        0x30D2
#define S5K4E6_SENSOR_ID                        0x4e60
#define S5K3AAEA_SENSOR_ID                      0x07AC
#define S5K3BAFB_SENSOR_ID                      0x7070
#define S5K3H7Y_SENSOR_ID                       0x3087
#define S5K3H2YX_SENSOR_ID                      0x382b
#define S5KA3DFX_SENSOR_ID                      0x00AB
#define S5K3E2FX_SENSOR_ID                      0x3E2F
#define S5K4B2FX_SENSOR_ID                      0x5080
#define S5K4E1GA_SENSOR_ID                      0x4E10
#define S5K4ECGX_SENSOR_ID                      0x4EC0
#define S5K53BEX_SENSOR_ID                      0x45A8
#define S5K53BEB_SENSOR_ID                      0x87A8
#define S5K5BAFX_SENSOR_ID                      0x05BA
#define S5K5E2YA_SENSOR_ID                      0x5e20
#define S5K4H5YX_2LANE_SENSOR_ID                0x485B
#define S5K4H5YC_SENSOR_ID                      0x485B
#define S5K83AFX_SENSOR_ID                      0x01C4
#define S5K5CAGX_SENSOR_ID                      0x05ca
#define S5K8AAYX_MIPI_SENSOR_ID                 0x08aa
#define S5K8AAYX_SENSOR_ID                      0x08aa
#define S5K5E8YX_SENSOR_ID                      0x5e80
#define S5K5E8YXREAR2_SENSOR_ID                 0x5e81
#define S5K5E9_SENSOR_ID                        0x559b
#define S5KGD1SP_SENSOR_ID                      0x0841
/*HI*/
#define HI841_SENSOR_ID                         0x0841
#define HI707_SENSOR_ID                         0x00b8
#define HI704_SENSOR_ID                         0x0096
#define HI556_SENSOR_ID                         0x0556
#define HI551_SENSOR_ID                         0x0551
#define HI553_SENSOR_ID                         0x0553
#define HI545MIPI_SENSOR_ID                     0x0545
#define HI544MIPI_SENSOR_ID                     0x0544
#define HI542_SENSOR_ID                         0x00B1
#define HI542MIPI_SENSOR_ID                     0x00B1
#define HI253_SENSOR_ID                         0x0092
#define HI251_SENSOR_ID                         0x0084
#define HI191MIPI_SENSOR_ID                     0x0191
#define HIVICF_SENSOR_ID                        0x0081
/*MT*/
#define MT9D011_SENSOR_ID                       0x1511
#define MT9D111_SENSOR_ID                       0x1511
#define MT9D112_SENSOR_ID                       0x1580
#define MT9M011_SENSOR_ID                       0x1433
#define MT9M111_SENSOR_ID                       0x143A
#define MT9M112_SENSOR_ID                       0x148C
#define MT9M113_SENSOR_ID                       0x2480
#define MT9P012_SENSOR_ID                       0x2800
#define MT9P012_SENSOR_ID_REV7                  0x2801
#define MT9T012_SENSOR_ID                       0x1600
#define MT9T013_SENSOR_ID                       0x2600
#define MT9T113_SENSOR_ID                       0x4680
#define MT9V112_SENSOR_ID                       0x1229
#define MT9DX11_SENSOR_ID                       0x1519
#define MT9D113_SENSOR_ID                       0x2580
#define MT9D115_SENSOR_ID                       0x2580
#define MT9D115MIPI_SENSOR_ID                   0x2580
#define MT9V113_SENSOR_ID                       0x2280
#define MT9V114_SENSOR_ID                       0x2283
#define MT9V115_SENSOR_ID                       0x2284
#define MT9P015_SENSOR_ID                       0x2803
#define MT9P017_SENSOR_ID                       0x4800
#define MT9P017MIPI_SENSOR_ID                   0x4800
#define MT9T113MIPI_SENSOR_ID                   0x4680
/*GC*/
#define GC5035_SENSOR_ID                        0x5035
#define GC2375_SENSOR_ID                        0x2375
#define GC2375H_SENSOR_ID                       0x2375
#define GC2375SUB_SENSOR_ID                     0x2376
#define GC2365_SENSOR_ID                        0x2365
#define GC2366_SENSOR_ID                        0x2366
#define GC2355_SENSOR_ID                        0x2355
#define GC2235_SENSOR_ID                        0x2235
#define GC2035_SENSOR_ID                        0x2035
#define GC2145_SENSOR_ID                        0x2145
#define GC0330_SENSOR_ID                        0xC1
#define GC0329_SENSOR_ID                        0xC0
#define GC0310_SENSOR_ID                        0xa310
#define GC0313MIPI_YUV_SENSOR_ID                0xD0
#define GC0312_SENSOR_ID                        0xb310
#define GC8034_SENSOR_ID                        0x8044
#define GC8C34_SENSOR_ID                        0x80C4
/*SP*/
#define SP0A19_YUV_SENSOR_ID                    0xA6
#define SP2518_YUV_SENSOR_ID                    0x53
#define SP2509_SENSOR_ID                        0x2509
#define SP250A_SENSOR_ID                        0x250A
/*A*/
#define A5141MIPI_SENSOR_ID                     0x4800
#define A5142MIPI_SENSOR_ID                     0x4800
/*HM*/
#define HM3451_SENSOR_ID                        0x345
/*AR*/
#define AR0833_SENSOR_ID                        0x4B03
/*SIV*/
#define SID020A_SENSOR_ID                       0x12B4
#define SIV100B_SENSOR_ID                       0x0C11
#define SIV100A_SENSOR_ID                       0x0C10
#define SIV120A_SENSOR_ID                       0x1210
#define SIV120B_SENSOR_ID                       0x0012
#define SIV121D_SENSOR_ID                       0xDE
#define SIM101B_SENSOR_ID                       0x09A0
#define SIM120C_SENSOR_ID                       0x0012
#define SID130B_SENSOR_ID                       0x001b
#define SIC110A_SENSOR_ID                       0x000D
#define SIV120B_SENSOR_ID                       0x0012
/*PAS (PixArt Image)*/
#define PAS105_SENSOR_ID                        0x0065
#define PAS302_SENSOR_ID                        0x0064
#define PAS5101_SENSOR_ID                       0x0067
#define PAS6180_SENSOR_ID                       0x6179
/*Panasoic*/
#define MN34152_SENSOR_ID                       0x01
/*Toshiba*/
#define T4KA7_SENSOR_ID                         0x2c30
/*Others*/
#define SHARP3D_SENSOR_ID                       0x003d
#define T8EV5_SENSOR_ID                         0x1011
#ifdef OPLUS_FEATURE_CAMERA_COMMON
#define S5K3P9SP_SENSOR_ID                      0x310A

#define SENSOR_DRVNAME_S5K3P9SP_MIPI_RAW        "s5k3p9sp_mipi_raw"
#define SENSOR_DRVNAME_HI846P2Q_MIPI_RAW         "hi846p2q_mipi_raw"
#define SENSOR_DRVNAME_OV02A10P2Q_MIPI_MONO      "ov02a10p2q_mipi_mono"
#define SENSOR_DRVNAME_OV02A10P2Q_MIPI_MONO1     "ov02a10p2q_mipi_mono1"
#define SENSOR_DRVNAME_GC02M0P2Q_MIPI_MONO       "gc02m0p2q_mipi_mono"
#define SENSOR_DRVNAME_GC02M0P2Q_MIPI_MONO1      "gc02m0p2q_mipi_mono1"
#define SENSOR_DRVNAME_GC5035_MIPI_RAW          "gc5035_mipi_raw"
#define SENSOR_DRVNAME_S5KGD1SP_MIPI_RAW        "s5kgd1sp_mipi_raw"
#endif

/* CAMERA DRIVER NAME */
#define CAMERA_HW_DEVNAME                       "kd_camera_hw"
/* SENSOR DEVICE DRIVER NAME */
/*IMX*/
#define SENSOR_DRVNAME_IMX499_MIPI_RAW          "imx499_mipi_raw"
#define SENSOR_DRVNAME_IMX499_MIPI_RAW_13M      "imx499_mipi_raw_13m"
#define SENSOR_DRVNAME_IMX486_MIPI_RAW          "imx486_mipi_raw"
#define SENSOR_DRVNAME_IMX586_MIPI_RAW          "imx586_mipi_raw"
#define SENSOR_DRVNAME_IMX586P2Q2_MIPI_RAW       "imx586p2q2_mipi_raw"
#define SENSOR_DRVNAME_IMX586P2Q_MIPI_RAW       "imx586p2q_mipi_raw"
#define SENSOR_DRVNAME_IMX519_MIPI_RAW          "imx519_mipi_raw"
#define SENSOR_DRVNAME_IMX519DUAL_MIPI_RAW      "imx519dual_mipi_raw"
#define SENSOR_DRVNAME_IMX576_MIPI_RAW          "imx576_mipi_raw"
#define SENSOR_DRVNAME_IMX350_MIPI_RAW          "imx350_mipi_raw"
#define SENSOR_DRVNAME_IMX398_MIPI_RAW          "imx398_mipi_raw"
#define SENSOR_DRVNAME_IMX268_MIPI_RAW          "imx268_mipi_raw"
#define SENSOR_DRVNAME_IMX386_MIPI_RAW          "imx386_mipi_raw"
#define SENSOR_DRVNAME_IMX300_MIPI_RAW          "imx300_mipi_raw"
#define SENSOR_DRVNAME_IMX386_MIPI_MONO         "imx386_mipi_mono"
#define SENSOR_DRVNAME_IMX362_MIPI_RAW          "imx362_mipi_raw"
#define SENSOR_DRVNAME_IMX338_MIPI_RAW          "imx338_mipi_raw"
#define SENSOR_DRVNAME_IMX376_MIPI_RAW          "imx376_mipi_raw"
#define SENSOR_DRVNAME_IMX318_MIPI_RAW          "imx318_mipi_raw"
#define SENSOR_DRVNAME_IMX377_MIPI_RAW          "imx377_mipi_raw"
#define SENSOR_DRVNAME_IMX278_MIPI_RAW          "imx278_mipi_raw"
#define SENSOR_DRVNAME_IMX258_MIPI_RAW          "imx258_mipi_raw"
#define SENSOR_DRVNAME_IMX258_MIPI_MONO         "imx258_mipi_mono"
#define SENSOR_DRVNAME_IMX230_MIPI_RAW          "imx230_mipi_raw"
#define SENSOR_DRVNAME_IMX220_MIPI_RAW          "imx220_mipi_raw"
#define SENSOR_DRVNAME_IMX219_MIPI_RAW          "imx219_mipi_raw"
#define SENSOR_DRVNAME_IMX214_MIPI_MONO         "imx214_mipi_mono"
#define SENSOR_DRVNAME_IMX214_MIPI_RAW          "imx214_mipi_raw"
#define SENSOR_DRVNAME_IMX179_MIPI_RAW          "imx179_mipi_raw"
#define SENSOR_DRVNAME_IMX178_MIPI_RAW          "imx178_mipi_raw"
#define SENSOR_DRVNAME_IMX135_MIPI_RAW          "imx135_mipi_raw"
#define SENSOR_DRVNAME_IMX132_MIPI_RAW          "imx132_mipi_raw"
#define SENSOR_DRVNAME_IMX119_MIPI_RAW          "imx119_mipi_raw"
#define SENSOR_DRVNAME_IMX105_MIPI_RAW          "imx105_mipi_raw"
#define SENSOR_DRVNAME_IMX091_MIPI_RAW          "imx091_mipi_raw"
#define SENSOR_DRVNAME_IMX073_MIPI_RAW          "imx073_mipi_raw"
/*OV*/
#define SENSOR_DRVNAME_OV23850_MIPI_RAW         "ov23850_mipi_raw"
#define SENSOR_DRVNAME_OV16880_MIPI_RAW         "ov16880_mipi_raw"
#define SENSOR_DRVNAME_OV16885_MIPI_RAW         "ov16885_mipi_raw"
#define SENSOR_DRVNAME_OV16825_MIPI_RAW         "ov16825_mipi_raw"
#define SENSOR_DRVNAME_OV13870_MIPI_RAW         "ov13870_mipi_raw"
#define SENSOR_DRVNAME_OV13855_MIPI_RAW         "ov13855_mipi_raw"
#define SENSOR_DRVNAME_OV13855MAIN2_MIPI_RAW    "ov13855main2_mipi_raw"
#define SENSOR_DRVNAME_OV13850_MIPI_RAW         "ov13850_mipi_raw"
#define SENSOR_DRVNAME_OV12A10_MIPI_RAW         "ov12a10_mipi_raw"
#define SENSOR_DRVNAME_OV12830_MIPI_RAW         "ov12830_mipi_raw"
#define SENSOR_DRVNAME_OV9760_MIPI_RAW          "ov9760_mipi_raw"
#define SENSOR_DRVNAME_OV9740_MIPI_YUV          "ov9740_mipi_yuv"
#define SENSOR_DRVNAME_0V9726_RAW               "ov9726_raw"
#define SENSOR_DRVNAME_OV9726_MIPI_RAW          "ov9726_mipi_raw"
#define SENSOR_DRVNAME_OV8865_MIPI_RAW          "ov8865_mipi_raw"
#define SENSOR_DRVNAME_OV8858_MIPI_RAW          "ov8858_mipi_raw"
#define SENSOR_DRVNAME_OV8858S_MIPI_RAW         "ov8858s_mipi_raw"
#define SENSOR_DRVNAME_OV8856_MIPI_RAW          "ov8856_mipi_raw"
#define SENSOR_DRVNAME_OV8830_RAW               "ov8830_raw"
#define SENSOR_DRVNAME_OV8825_MIPI_RAW          "ov8825_mipi_raw"
#define SENSOR_DRVNAME_OV7675_YUV               "ov7675_yuv"
#define SENSOR_DRVNAME_OV5693_MIPI_RAW          "ov5693_mipi_raw"
#define SENSOR_DRVNAME_OV5670_MIPI_RAW          "ov5670_mipi_raw"
#define SENSOR_DRVNAME_OV5670_MIPI_RAW_2        "ov5670_mipi_raw_2"
#define SENSOR_DRVNAME_OV2281_MIPI_RAW          "ov2281_mipi_raw"
#define SENSOR_DRVNAME_OV5675_MIPI_RAW          "ov5675mipiraw"
#define SENSOR_DRVNAME_OV5671_MIPI_RAW          "ov5671_mipi_raw"
#define SENSOR_DRVNAME_OV5647MIPI_RAW           "ov5647_mipi_raw"
#define SENSOR_DRVNAME_OV5645_MIPI_RAW          "ov5645_mipi_raw"
#define SENSOR_DRVNAME_OV5645_MIPI_YUV          "ov5645_mipi_yuv"
#define SENSOR_DRVNAME_OV5650MIPI_RAW           "ov5650_mipi_raw"
#define SENSOR_DRVNAME_OV5650_RAW               "ov5650_raw"
#define SENSOR_DRVNAME_OV5648_MIPI_RAW          "ov5648_mipi_raw"
#define SENSOR_DRVNAME_OV5647_RAW               "ov5647_raw"
#define SENSOR_DRVNAME_OV5642_RAW               "ov5642_raw"
#define SENSOR_DRVNAME_OV5642_MIPI_YUV          "ov5642_mipi_yuv"
#define SENSOR_DRVNAME_OV5642_MIPI_RGB          "ov5642_mipi_rgb"
#define SENSOR_DRVNAME_OV5642_MIPI_JPG          "ov5642_mipi_jpg"
#define SENSOR_DRVNAME_OV5642_YUV               "ov5642_yuv"
#define SENSOR_DRVNAME_OV5642_YUV_SWI2C         "ov5642_yuv_swi2c"
#define SENSOR_DRVNAME_OV4688_MIPI_RAW          "ov4688_mipi_raw"
#define SENSOR_DRVNAME_OV3640_RAW               "ov3640_raw"
#define SENSOR_DRVNAME_OV3640_YUV               "ov3640_yuv"
#define SENSOR_DRVNAME_OV2724_MIPI_RAW          "ov2724_mipi_raw"
#define SENSOR_DRVNAME_OV2722_MIPI_RAW          "ov2722_mipi_raw"
#define SENSOR_DRVNAME_OV2680_MIPI_RAW          "ov2680_mipi_raw"
#define SENSOR_DRVNAME_OV2659_YUV               "ov2659_yuv"
#define SENSOR_DRVNAME_OV2655_YUV               "ov2655_yuv"
#define SENSOR_DRVNAME_OV2650_RAW               "ov265x_raw"
#define SENSOR_DRVNAME_OV20880_MIPI_RAW         "ov20880_mipi_raw"
#define SENSOR_DRVNAME_OV13B10_MIPI_RAW         "ov13b10_mipi_raw"
/*S5K*/
#define SENSOR_DRVNAME_S5K2LQSX_MIPI_RAW        "s5k2lqsx_mipi_raw"
#define SENSOR_DRVNAME_S5K4H7_MIPI_RAW          "s5k4h7_mipi_raw"
#define SENSOR_DRVNAME_S5K3P8SP_MIPI_RAW        "s5k3p8sp_mipi_raw"
#define SENSOR_DRVNAME_S5K2T7SP_MIPI_RAW        "s5k2t7sp_mipi_raw"
#define SENSOR_DRVNAME_S5K2T7SP_MIPI_RAW_5M     "s5k2t7sp_mipi_raw_5m"
#define SENSOR_DRVNAME_S5K3P8SX_MIPI_RAW        "s5k3p8sx_mipi_raw"
#define SENSOR_DRVNAME_S5K2L7_MIPI_RAW          "s5k2l7_mipi_raw"
#define SENSOR_DRVNAME_S5K3L6_MIPI_RAW          "s5k3l6_mipi_raw"
#define SENSOR_DRVNAME_S5K3L8_MIPI_RAW          "s5k3l8_mipi_raw"
#define SENSOR_DRVNAME_S5K3M3_MIPI_RAW          "s5k3m3_mipi_raw"
#define SENSOR_DRVNAME_S5K2X8_MIPI_RAW          "s5k2x8_mipi_raw"
#define SENSOR_DRVNAME_S5K2P7_MIPI_RAW          "s5k2p7_mipi_raw"
#define SENSOR_DRVNAME_S5K2P8_MIPI_RAW          "s5k2p8_mipi_raw"
#define SENSOR_DRVNAME_S5K3P3SX_MIPI_RAW        "s5k3p3sx_mipi_raw"
#define SENSOR_DRVNAME_S5K3P3_MIPI_RAW          "s5k3p3_mipi_raw"
#define SENSOR_DRVNAME_S5K3P8_MIPI_RAW          "s5k3p8_mipi_raw"
#define SENSOR_DRVNAME_S5K3M2_MIPI_RAW          "s5k3m2_mipi_raw"
#define SENSOR_DRVNAME_S5K4E6_MIPI_RAW          "s5k4e6_mipi_raw"
#define SENSOR_DRVNAME_S5K3H2YX_MIPI_RAW        "s5k3h2yx_mipi_raw"
#define SENSOR_DRVNAME_S5K3H7Y_MIPI_RAW         "s5k3h7y_mipi_raw"
#define SENSOR_DRVNAME_S5K4H5YC_MIPI_RAW        "s5k4h5yc_mipi_raw"
#define SENSOR_DRVNAME_S5K4E1GA_MIPI_RAW        "s5k4e1ga_mipi_raw"
#define SENSOR_DRVNAME_S5K4ECGX_MIPI_YUV        "s5k4ecgx_mipi_yuv"
#define SENSOR_DRVNAME_S5K5CAGX_YUV             "s5k5cagx_yuv"
#define SENSOR_DRVNAME_S5K4H5YX_2LANE_MIPI_RAW  "s5k4h5yx_2lane_mipi_raw"
#define SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW        "s5k5e2ya_mipi_raw"
#define SENSOR_DRVNAME_S5K8AAYX_MIPI_YUV        "s5k8aayx_mipi_yuv"
#define SENSOR_DRVNAME_S5K8AAYX_YUV             "s5k8aayx_yuv"
#define SENSOR_DRVNAME_S5K5E8YX_MIPI_RAW        "s5k5e8yx_mipi_raw"
#define SENSOR_DRVNAME_S5K5E8YXREAR2_MIPI_RAW   "s5k5e8yxrear2_mipi_raw"
#define SENSOR_DRVNAME_S5K5E9_MIPI_RAW          "s5k5e9_mipi_raw"
/*HI*/
#define SENSOR_DRVNAME_HI841_MIPI_RAW           "hi841_mipi_raw"
#define SENSOR_DRVNAME_HI707_YUV                "hi707_yuv"
#define SENSOR_DRVNAME_HI704_YUV                "hi704_yuv"
#define SENSOR_DRVNAME_HI556_MIPI_RAW           "hi556_mipi_raw"
#define SENSOR_DRVNAME_HI551_MIPI_RAW           "hi551_mipi_raw"
#define SENSOR_DRVNAME_HI553_MIPI_RAW           "hi553_mipi_raw"
#define SENSOR_DRVNAME_HI545_MIPI_RAW           "hi545_mipi_raw"
#define SENSOR_DRVNAME_HI542_RAW                "hi542_raw"
#define SENSOR_DRVNAME_HI542MIPI_RAW            "hi542_mipi_raw"
#define SENSOR_DRVNAME_HI544_MIPI_RAW           "hi544_mipi_raw"
#define SENSOR_DRVNAME_HI253_YUV                "hi253_yuv"
#define SENSOR_DRVNAME_HI191_MIPI_RAW           "hi191_mipi_raw"
/*MT*/
#define SENSOR_DRVNAME_MT9P012_RAW              "mt9p012_raw"
#define SENSOR_DRVNAME_MT9P015_RAW              "mt9p015_raw"
#define SENSOR_DRVNAME_MT9P017_RAW              "mt9p017_raw"
#define SENSOR_DRVNAME_MT9P017_MIPI_RAW         "mt9p017_mipi_raw"
#define SENSOR_DRVNAME_MT9D115_MIPI_RAW         "mt9d115_mipi_raw"
#define SENSOR_DRVNAME_MT9V114_YUV              "mt9v114_yuv"
#define SENSOR_DRVNAME_MT9V115_YUV              "mt9v115_yuv"
#define SENSOR_DRVNAME_MT9T113_YUV              "mt9t113_yuv"
#define SENSOR_DRVNAME_MT9V113_YUV              "mt9v113_yuv"
#define SENSOR_DRVNAME_MT9T113_MIPI_YUV         "mt9t113_mipi_yuv"
/*GC*/
#define SENSOR_DRVNAME_GC5035MIPI_RAW           "gc5035_mipi_raw"
#define SENSOR_DRVNAME_GC2375_MIPI_RAW          "gc2375_mipi_raw"
#define SENSOR_DRVNAME_GC2375H_MIPI_RAW         "gc2375h_mipi_raw"
#define SENSOR_DRVNAME_GC2375SUB_MIPI_RAW       "gc2375sub_mipi_raw"
#define SENSOR_DRVNAME_GC2365_MIPI_RAW          "gc2365_mipi_raw"
#define SENSOR_DRVNAME_GC2366_MIPI_RAW          "gc2366_mipi_raw"
#define SENSOR_DRVNAME_GC2035_YUV               "gc2035_yuv"
#define SENSOR_DRVNAME_GC2235_RAW               "gc2235_raw"
#define SENSOR_DRVNAME_GC2355_MIPI_RAW          "gc2355_mipi_raw"
#define SENSOR_DRVNAME_GC2355_RAW               "gc2355_raw"
#define SENSOR_DRVNAME_GC0330_YUV               "gc0330_yuv"
#define SENSOR_DRVNAME_GC0329_YUV               "gc0329_yuv"
#define SENSOR_DRVNAME_GC2145_MIPI_YUV          "gc2145_mipi_yuv"
#define SENSOR_DRVNAME_GC0310_MIPI_YUV          "gc0310_mipi_yuv"
#define SENSOR_DRVNAME_GC0310_YUV               "gc0310_yuv"
#define SENSOR_DRVNAME_GC0312_YUV               "gc0312_yuv"
#define SENSOR_DRVNAME_GC0313MIPI_YUV           "gc0313_mipi_yuv"
#define SENSOR_DRVNAME_GC8C34_MIPI_RAW          "gc8c34_mipi_raw"
#define SENSOR_DRVNAME_GC8034_MIPI_RAW          "gc8034_mipi_raw"
/*SP*/
#define SENSOR_DRVNAME_SP0A19_YUV               "sp0a19_yuv"
#define SENSOR_DRVNAME_SP2518_YUV               "sp2518_yuv"
#define SENSOR_DRVNAME_SP2509_MIPI_RAW          "sp2509_mipi_raw"
#define SENSOR_DRVNAME_SP250A_MIPI_RAW          "sp250a_mipi_raw"
/*A*/
#define SENSOR_DRVNAME_A5141_MIPI_RAW           "a5141_mipi_raw"
#define SENSOR_DRVNAME_A5142_MIPI_RAW           "a5142_mipi_raw"
/*HM*/
#define SENSOR_DRVNAME_HM3451_RAW               "hm3451_raw"
/*AR*/
#define SENSOR_DRVNAME_AR0833_MIPI_RAW          "ar0833_mipi_raw"
/*SIV*/
#define SENSOR_DRVNAME_SIV121D_YUV              "siv121d_yuv"
#define SENSOR_DRVNAME_SIV120B_YUV              "siv120b_yuv"
/*PAS (PixArt Image)*/
#define SENSOR_DRVNAME_PAS6180_SERIAL_YUV       "pas6180_serial_yuv"
/*Panasoic*/
#define SENSOR_DRVNAME_MN34152_MIPI_RAW         "mn34152_mipi_raw"
/*Toshiba*/
#define SENSOR_DRVNAME_T4KA7_MIPI_RAW           "t4ka7_mipi_raw"
/*Others*/
#define SENSOR_DRVNAME_SHARP3D_MIPI_YUV         "sharp3d_mipi_yuv"
#define SENSOR_DRVNAME_T8EV5_YUV                "t8ev5_yuv"
/*Test*/
#define SENSOR_DRVNAME_IMX135_MIPI_RAW_5MP      "imx135_mipi_raw_5mp"
#define SENSOR_DRVNAME_IMX135_MIPI_RAW_8MP      "imx135_mipi_raw_8mp"
#define SENSOR_DRVNAME_OV13870_MIPI_RAW_5MP     "ov13870_mipi_raw_5mp"
#define SENSOR_DRVNAME_OV8856_MIPI_RAW_5MP      "ov8856_mipi_raw_5mp"
#ifdef OPLUS_FEATURE_CAMERA_COMMON
#define S5KGW1_SENSOR_ID                        0x0971
#define S5KGH1_SENSOR_ID                        0x0881
#define S5K3M5_SENSOR_ID                        0x30D5
//#define S5K3M5SX_SENSOR_ID                      0x30D6
#define GC8054_SENSOR_ID                        0x8054
#define GC8054F_SENSOR_ID                       0x8055
#define HI846_SENSOR_ID                         0x0846
#define GC2375H_SENSOR_ID                       0x2375
#define OV02A10_SENSOR_ID                       0x2509
#define GC02M0_SENSOR_ID                        0x02D0
#define GC02M0F_SENSOR_ID                       0x02D1
#define SENSOR_DRVNAME_S5KGW1_MIPI_RAW          "s5kgw1_mipi_raw"
#define SENSOR_DRVNAME_S5KGH1_MIPI_RAW          "s5kgh1_mipi_raw"
#define SENSOR_DRVNAME_S5K3M5_MIPI_RAW          "s5k3m5_mipi_raw"
#define SENSOR_DRVNAME_S5K3M5SX_MIPI_RAW        "s5k3m5sx_mipi_raw"
#define SENSOR_DRVNAME_GC8054_MIPI_RAW          "gc8054_mipi_raw"
#define SENSOR_DRVNAME_GC8054F_MIPI_RAW         "gc8054f_mipi_raw"
#define SENSOR_DRVNAME_HI846_MIPI_RAW           "hi846_mipi_raw"
#define SENSOR_DRVNAME_GC2375H_MIPI_RAW         "gc2375h_mipi_raw"
#define SENSOR_DRVNAME_OV02A10_MIPI_MONO        "ov02a10_mipi_mono"
#define SENSOR_DRVNAME_GC02M0_MIPI_MONO         "gc02m0_mipi_mono"
#define SENSOR_DRVNAME_GC02M0F_MIPI_MONO        "gc02m0f_mipi_mono"
#define GC02M0_SENSOR_ID0                       0x02d0
#define GC02M0_SENSOR_ID1                       0x02d1
#define GC02M0_SENSOR_ID2                       0x02d2
#define SENSOR_DRVNAME_GC02M0B_MIPI_MONO0       "gc02m0b_mipi_mono0"
#define SENSOR_DRVNAME_GC02M0B_MIPI_MONO1       "gc02m0b_mipi_mono1"
#define SENSOR_DRVNAME_GC02M0B_MIPI_MONO2       "gc02m0b_mipi_mono2"
#define GC02M1_SENSOR_ID0                       0x02e0
#define GC02M1_SENSOR_ID1                       0x02e1
#define GC02M1_SENSOR_ID2                       0x02e2
#define SENSOR_DRVNAME_GC02M1B_MIPI_MONO0       "gc02m1b_mipi_mono0"
#define SENSOR_DRVNAME_GC02M1B_MIPI_MONO1       "gc02m1b_mipi_mono1"
#define SENSOR_DRVNAME_GC02M1B_MIPI_MONO2       "gc02m1b_mipi_mono2"
#define GM1ST_SENSOR_ID                         0xF8D1
#define SENSOR_DRVNAME_GM1ST_MIPI_RAW           "gm1st_mipi_raw"
#define IMX471_SENSOR_ID                        0x20000
#define SENSOR_DRVNAME_IMX471_MIPI_RAW          "imx471_mipi_raw"

#ifdef OPLUS_FEATURE_CAMERA_COMMON
#define OV02A10_MONO_SENSOR_ID                  0x2509
#define OV48B_SENSOR_ID                         0x564842
#define GC02K0_SENSOR_ID                        0x2385
#define OV16A10_SENSOR_ID                       0x561641
#define SENSOR_DRVNAME_GC02M0_MIPI_RAW          "gc02m0_mipi_raw"
#define IMX616_SENSOR_ID                     0x0616
#define IMX686_SENSOR_ID                     0x0686
#define SENSOR_DRVNAME_GC02M0B_MIPI_MONO     "gc02m0b_mipi_mono"
#define GC02M1B_SENSOR_ID                     0x02e0
#define GC02M1B_SENSOR_ID1                    0x02e1
#define S5KGW3_SENSOR_ID                      0x0973
#define OV32A_SENSOR_ID                       0x563241
#define IMX481_SENSOR_ID                        0x0481
#define IMX319_SENSOR_ID                        0x0319
#define S5K3M5SX_SENSOR_ID                      0x30D5
#define OV02B10_SENSOR_ID                       0x002B
#define SENSOR_DRVNAME_IMX481_MIPI_RAW          "imx481_mipi_raw"
#define SENSOR_DRVNAME_IMX319_MIPI_RAW          "imx319_mipi_raw"
#define SENSOR_DRVNAME_GC02M0_MIPI_RAW          "gc02m0_mipi_raw"
#define SENSOR_DRVNAME_IMX686_MIPI_RAW          "imx686_mipi_raw"
#define SENSOR_DRVNAME_IMX616_MIPI_RAW          "imx616_mipi_raw"
#define SENSOR_DRVNAME_OV48B_MIPI_RAW           "ov48b_mipi_raw"
#define SENSOR_DRVNAME_GC8054_MIPI_RAW          "gc8054_mipi_raw"
#define SENSOR_DRVNAME_GC02M0B_MIPI_MONO1       "gc02m0b_mipi_mono1"
#define SENSOR_DRVNAME_GC02K0B_MIPI_MONO        "gc02k0b_mipi_mono"
#define SENSOR_DRVNAME_OV16A10_MIPI_RAW         "ov16a10_mipi_raw"
#define SENSOR_DRVNAME_GC02M1B_MIPI_MONO        "gc02m1b_mipi_mono"
#define SENSOR_DRVNAME_S5KGW3_MIPI_RAW          "s5kgw3_mipi_raw"
#define SENSOR_DRVNAME_OV32A_MIPI_RAW           "ov32a_mipi_raw"
#define SENSOR_DRVNAME_OV02B10_MIPI_RAW         "ov02b10_mipi_raw"

#define SENSOR_DRVNAME_GC02K0_MIPI_RAW          "gc02k0_mipi_raw"
#endif
#endif

#ifdef OPLUS_FEATURE_CAMERA_COMMON
#define PASCALD_TRULY_MAIN_OV13B10_SENSOR_ID               0x560D43
#define PASCALD_QTECH_MAIN_OV13B10_SENSOR_ID               0x560D44
#define PASCALD_HLT_FRONT_GC5035_SENSOR_ID                 0x5036
#define PASCALD_SHENGTAI_FRONT_GC5035_SENSOR_ID            0x5037
#define PASCALD_SHENGTAI2_FRONT_GC5035_SENSOR_ID           0x5038
#define PASCALD_HLT_FRONT_S5K4H7_SENSOR_ID                 0x487B
#define PASCALD_HLT_DEPTH_GC02K0B_SENSOR_ID                0x2385
#define PASCALD_HLT_DEPTH_GC02M1B_MIPI_SENSOR_ID           0x02E1
#define PASCALD_CXT_DEPTH_GC02M1B_SENSOR_ID                0x02E3
#define PASCALD_LHYX_DEPTH_OV02B1B_SENSOR_ID               0x002D
#define SENSOR_DRVNAME_PASCALD_TRULY_MAIN_OV13B10          "pascald_truly_main_ov13b10"
#define SENSOR_DRVNAME_PASCALD_QTECH_MAIN_OV13B10          "pascald_qtech_main_ov13b10"
#define SENSOR_DRVNAME_PASCALD_HLT_FRONT_GC5035            "pascald_hlt_front_gc5035"
#define SENSOR_DRVNAME_PASCALD_SHENGTAI_FRONT_GC5035       "pascald_shengtai_front_gc5035"
#define SENSOR_DRVNAME_PASCALD_SHENGTAI2_FRONT_GC5035      "pascald_shengtai2_front_gc5035"
#define SENSOR_DRVNAME_PASCALD_HLT_FRONT_S5K4H7            "pascald_hlt_front_s5k4h7"
#define SENSOR_DRVNAME_PASCALD_HLT_DEPTH_GC02K0B           "pascald_hlt_depth_gc02k0b"
#define SENSOR_DRVNAME_PASCALD_HLT_DEPTH_GC02M1B           "pascald_hlt_depth_gc02m1b"
#define SENSOR_DRVNAME_PASCALD_CXT_DEPTH_GC02M1B           "pascald_cxt_depth_gc02m1b"
#define SENSOR_DRVNAME_PASCALD_LHYX_DEPTH_OV02B1B          "pascald_lhyx_depth_ov02b1b"
#define PASCAL_HLT_MONO_GC02M1B_MIPI_SENSOR_ID             0x02E2
#define PASCAL_SHENGTAI_MONO1_OV02B1B_SENSOR_ID            0x002b
#define PASCAL_LHYX_MONO1_OV02B1B_SENSOR_ID                0x002c
#define PASCAL_CXT_MONO1_GC02M1B_SENSOR_ID                 0x02E4
#define PASCAL_CXT_MONO_OV02B1B_SENSOR_ID                  0x002E
#define PASCAL_SHENGTAI_WIDE_OV8856_SENSOR_ID              0x885A
#define PASCAL_SUNNY_WIDE_OV8856_SENSOR_ID                 0x885B
#define SENSOR_DRVNAME_PASCAL_HLT_MONO_GC02M1B             "pascal_hlt_mono_gc02m1b"
#define SENSOR_DRVNAME_PASCAL_SHENGTAI_MONO1_OV02B1B       "pascal_shengtai_mono1_ov02b1b"
#define SENSOR_DRVNAME_PASCAL_LHYX_MONO1_OV02B1B           "pascal_lhyx_mono1_ov02b1b"
#define SENSOR_DRVNAME_PASCAL_CXT_MONO1_GC02M1B            "pascal_cxt_mono1_gc02m1b"
#define SENSOR_DRVNAME_PASCAL_CXT_MONO_OV02B1B             "pascal_cxt_mono_ov02b1b"
#define SENSOR_DRVNAME_PASCAL_SHENGTAI_WIDE_OV8856         "pascal_shengtai_wide_ov8856"
#define SENSOR_DRVNAME_PASCAL_SUNNY_WIDE_OV8856            "pascal_sunny_wide_ov8856"
#define PASCALE_HLT_MACRO_OV02B10_SENSOR_ID                0x002F
#define SENSOR_DRVNAME_PASCALE_HLT_MACRO_OV02B10           "pascale_hlt_macro_ov02b10"
#define OV02B1B_MIPI_SENSOR_ID                             0x002B
#define GC02M1B_MIPI_SENSOR_ID                             0x02E0
#endif //OPLUS_FEATURE_CAMERA_COMMON

#ifdef OPLUS_FEATURE_CAMERA_COMMON
#define SENSOR_DRVNAME_YOGURT_TRULY_MAIN_S5K3L6          "yogurt_truly_main_s5k3l6"
#define YOGURT_TRULY_MAIN_S5K3L6_SENSOR_ID                0x30C6
#define SENSOR_DRVNAME_YOGURT_QTECH_MAIN_OV13B10         "yogurt_qtech_main_ov13b10"
#define YOGURT_QTECH_MAIN_OV13B10_SENSOR_ID              0x560D43
#define SENSOR_DRVNAME_YOGURT_QTECH_FRONT_S5K4H7         "yogurt_qtech_front_s5k4h7"
#define YOGURT_QTECH_FRONT_S5K4H7_SENSOR_ID               0x487B
#define SENSOR_DRVNAME_YOGURT_CXT_DEPTH_GC02M1B          "yogurt_cxt_depth_gc02m1b"
#define YOGURT_CXT_DEPTH_GC02M1B_MIPI_SENSOR_ID          0x02E2
#define SENSOR_DRVNAME_YOGURT_LHYX_MICRO_GC02K0          "yogurt_lhyx_micro_gc02k0"
#define YOGURT_LHYX_MICRO_GC02K0_SENSOR_ID               0x2386
#define SENSOR_DRVNAME_YOGURT_HLT_FRONT_GC5035           "yogurt_hlt_front_gc5035"
#define YOGURT_HLT_FRONT_GC5035_SENSOR_ID                0x5035
#define SENSOR_DRVNAME_YOGURT_SHENGTAI_FRONT_GC5035      "yogurt_shengtai_front_gc5035"
#define YOGURT_SHENGTAI_FRONT_GC5035_SENSOR_ID           0x5036
#define SENSOR_DRVNAME_YOGURT_SUNNY_FRONT_GC5035         "yogurt_sunny_front_gc5035"
#define YOGURT_SUNNY_FRONT_GC5035_SENSOR_ID              0x5037
#define SENSOR_DRVNAME_YOGURT_LCE_FRONT_HI556            "yogurt_lce_front_hi556"
#define YOGURT_LCE_FRONT_HI556_SENSOR_ID                 0x0556
#define SENSOR_DRVNAME_YOGURT_SHENGTAI_FRONT_OV8856      "yogurt_shengtai_front_ov8856"
#define YOGURT_SHENGTAI_FRONT_OV8856_SENSOR_ID           0x885A
#define SENSOR_DRVNAME_YOGURT_HLT_DEPTH_GC02M1B          "yogurt_hlt_depth_gc02m1b"
#define YOGURT_HLT_DEPTH_GC02M1B_MIPI_SENSOR_ID          0x02E0
#define SENSOR_DRVNAME_YOGURT_LHYX_DEPTH_GC02M1B         "yogurt_lhyx_depth_gc02m1b"
#define YOGURT_LHYX_DEPTH_GC02M1B_MIPI_SENSOR_ID         0x02E1
#define SENSOR_DRVNAME_YOGURT_SHENGTAI_MICRO_GC02K0      "yogurt_shengtai_micro_gc02k0"
#define YOGURT_SHENGTAI_MICRO_GC02K0_SENSOR_ID           0x2385
#define SENSOR_DRVNAME_YOGURT_CXT_MICRO_GC02K0           "yogurt_cxt_micro_gc02k0"
#define YOGURT_CXT_MICRO_GC02K0_SENSOR_ID                0x2387
#define SENSOR_DRVNAME_YOGURT_CXT_MICRO_GC02M1           "yogurt_cxt_micro_gc02m1"
#define YOGURT_CXT_MICRO_GC02M1_SENSOR_ID                0x02EA/*0x02E0 +6 +4*/
#define SENSOR_DRVNAME_YOGURT_SUNNY_FRONT_S5K3P9SP       "yogurt_sunny_front_s5k3p9sp"
#define YOGURT_SUNNY_FRONT_S5K3P9SP_SENSOR_ID             0x3109
#define SENSOR_DRVNAME_YOGURT_SHENGTAI_MONO_OV02B1B      "yogurt_shengtai_mono_ov02b1b"
#define YOGURT_SHENGTAI_MONO_OV02B1B_SENSOR_ID            0x002C
#define SENSOR_DRVNAME_YOGURT_HLT_MICRO_GC02M1           "yogurt_hlt_micro_gc02m1"
#define YOGURT_HLT_MICRO_GC02M1_SENSOR_ID                 0x02E8
#define SENSOR_DRVNAME_YOGURT_HLT_MICRO_OV02B10          "yogurt_hlt_micro_ov02b10"
#define YOGURT_HLT_MICRO_OV02B10_SENSOR_ID                0x002B
/* Yogurt 2G Project */
#define YOGURT_QTECH_MAIN_OV13B10_SENSOR_ID_2G           0x560D47 /*0x560D44 + 3 */
#define YOGURT_TRULY_MAIN_S5K3L6_SENSOR_ID_2G            0x30C7
#define YOGURT_SHENGTAI_FRONT_GC5035_SENSOR_ID_2G        0x5039
#define YOGURT_LCE_FRONT_HI556_SENSOR_ID_2G              0x0558 /*0x0557 + 1 */
#define YOGURT_QTECH_FRONT_S5K4H7_SENSOR_ID_2G           0x487C
#define YOGURT_SHENGTAI_FRONT_OV8856_SENSOR_ID_2G        0x885C /*0x885B + 1*/
#define YOGURT_HLT_DEPTH_GC02M1B_MIPI_SENSOR_ID_2G       0x02EB /*0x02E0 + 11*/
#define YOGURT_LHYX_DEPTH_GC02M1B_MIPI_SENSOR_ID_2G      0x02EC /*0x02E0 + 12*/
#define YOGURT_CXT_DEPTH_GC02M1B_MIPI_SENSOR_ID_2G       0x02ED /*0x02E0 + 13*/
#define YOGURT_SHENGTAI_MICRO_GC02K0_SENSOR_ID_2G        0x2388
#define YOGURT_LHYX_MICRO_GC02K0_SENSOR_ID_2G            0x2389
#define YOGURT_CXT_MICRO_GC02K0_SENSOR_ID_2G             0x238A
#define YOGURT_CXT_MICRO_GC02M1_SENSOR_ID_2G             0x02EE /*0x02E0 + 14*/
/* Yogurt 2G Project END*/
/* Yogurt 2G Project */
#define SENSOR_DRVNAME_YOGURT_QTECH_MAIN_OV13B10_2G         "yogurt_qtech_main_ov13b10_2g"
#define SENSOR_DRVNAME_YOGURT_TRULY_MAIN_S5K3L6_2G          "yogurt_truly_main_s5k3l6_2g"
#define SENSOR_DRVNAME_YOGURT_HLT_FRONT_GC5035_2G           "yogurt_hlt_front_gc5035_2g"
#define SENSOR_DRVNAME_YOGURT_SHENGTAI_FRONT_GC5035_2G      "yogurt_shengtai_front_gc5035_2g"
#define SENSOR_DRVNAME_YOGURT_SUNNY_FRONT_GC5035_2G         "yogurt_sunny_front_gc5035_2g"
#define SENSOR_DRVNAME_YOGURT_LCE_FRONT_HI556_2G            "yogurt_lce_front_hi556_2g"
#define SENSOR_DRVNAME_YOGURT_QTECH_FRONT_S5K4H7_2G         "yogurt_qtech_front_s5k4h7_2g"
#define SENSOR_DRVNAME_YOGURT_SHENGTAI_FRONT_OV8856_2G      "yogurt_shengtai_front_ov8856_2g"
#define SENSOR_DRVNAME_YOGURT_HLT_DEPTH_GC02M1B_2G          "yogurt_hlt_depth_gc02m1b_2g"
#define SENSOR_DRVNAME_YOGURT_LHYX_DEPTH_GC02M1B_2G         "yogurt_lhyx_depth_gc02m1b_2g"
#define SENSOR_DRVNAME_YOGURT_CXT_DEPTH_GC02M1B_2G          "yogurt_cxt_depth_gc02m1b_2g"
#define SENSOR_DRVNAME_YOGURT_SHENGTAI_MICRO_GC02K0_2G      "yogurt_shengtai_micro_gc02k0_2g"
#define SENSOR_DRVNAME_YOGURT_LHYX_MICRO_GC02K0_2G          "yogurt_lhyx_micro_gc02k0_2g"
#define SENSOR_DRVNAME_YOGURT_CXT_MICRO_GC02K0_2G           "yogurt_cxt_micro_gc02k0_2g"
#define SENSOR_DRVNAME_YOGURT_CXT_MICRO_GC02M1_2G           "yogurt_cxt_micro_gc02m1_2g"
/* Yogurt 2G Project END*/
#define PARKERA_QTECH_MAIN_OV13B10_SENSOR_ID               0x560D45 /*0x560D42+3*/
#define SENSOR_DRVNAME_PARKERA_QTECH_MAIN_OV13B10          "parkera_qtech_main_ov13b10"
#define PARKERA_QTECH_FRONT_IMX355_SENSOR_ID               0x0357 /*0x0355 + 2*/
#define SENSOR_DRVNAME_PARKERA_QTECH_FRONT_IMX355          "parkera_qtech_front_imx355"
#define PARKERA_SHINETECH_MONO_GC02M1B_SENSOR_ID           0x02E7  /*0x02E0 + 7*/
#define SENSOR_DRVNAME_PARKERA_SHINETECH_MONO_GC02M1B      "parkera_shinetech_mono_gc02m1b"
#define PARKERA_SHINETECH_MONO_OV02B1B_SENSOR_ID           0x002A/*0x2B - 1*/
#define SENSOR_DRVNAME_PARKERA_SHINETECH_MONO_OV02B1B      "parkera_shinetech_mono_ov02b1b"
#define PARKERA_HOLITECH_MACRO_GC02M1_SENSOR_ID            0x02E6/*0x02E0 + 6*/
#define SENSOR_DRVNAME_PARKERA_HOLITECH_MACRO_GC02M1       "parkera_holitech_macro_gc02m1"
#define PARKERA_SHINETECH_MACRO_OV02B10_SENSOR_ID          0x0029/*0x2B - 2*/
#define SENSOR_DRVNAME_PARKERA_SHINETECH_MACRO_OV02B10     "parkera_shinetech_macro_ov02b10"
#define PARKERA_HOLITECH_FRONT_S5K4H7_SENSOR_ID            0x487E/*0x487B + 3*/
#define SENSOR_DRVNAME_PARKERA_HOLITECH_FRONT_S5K4H7       "parkera_holitech_front_s5k4h7"
#define PARKERA_SHINETECH_FRONT_OV08D10_SENSOR_ID          0x56084700
#define SENSOR_DRVNAME_PARKERA_SHINETECH_FRONT_OV08D10     "parkera_shinetech_front_ov08d10"
#define PARKERA_SHINETECH_MAIN_S5KJN103_SENSOR_ID          0x38E1
#define SENSOR_DRVNAME_PARKERA_SHINETECH_MAIN_S5KJN103     "parkera_shinetech_main_s5kjn103"
#define PARKERA_QTECH_MAIN_OV13B2A_SENSOR_ID               0x560D46 /*0x560D43+3*/
#define SENSOR_DRVNAME_PARKERA_QTECH_MAIN_OV13B2A          "parkera_qtech_main_ov13b2a"
#define PARKERA_SHINETECH_MONO_GC02M1B_50M_SENSOR_ID           0x02E9  /*0x02E0 + 9*/
#define SENSOR_DRVNAME_PARKERA_SHINETECH_MONO_GC02M1B_50M      "parkera_shinetech_mono_gc02m1b_50m"
#define PARKERA_SHINETECH_MONO_OV02B1B_50M_SENSOR_ID           0x0028  /*0x2B - 3*/
#define SENSOR_DRVNAME_PARKERA_SHINETECH_MONO_OV02B1B_50M      "parkera_shinetech_mono_ov02b1b_50m"
#define PARKERA_PROJECT                                    4
/********************** ROI_D **********************/
#define ROID_HI556_SENSOR_ID                    0x0556
#define ROID_GC5035_SENSOR_ID                   0x5035
#define ROID_HI556_TXD_SENSOR_ID                0x0557
#define ROID_HI1336_SENSOR_ID                   0x1336
#define ROID_OV13B10_SENSOR_ID                  0x560D42
#define ROID_GC02M1_SENSOR_ID                   0x02E6 + 3
#define ROID_OV02B10_SENSOR_ID                  0x002F + 5
#define ROID_GC02M1_SW_SENSOR_ID                0x02E7 + 1
#define ROID_GC02M1B_SENSOR_ID                  0x02E0 + 3
#define ROID_OV02B1B_SENSOR_ID                  0x002C + 7
#define ROID_GC02M1B_SW_SENSOR_ID               0x02E1 + 3
#define SENSOR_DRVNAME_HI556_MIPI_RAW           "hi556_mipi_raw"
#define SENSOR_DRVNAME_GC5035_MIPI_RAW          "gc5035_mipi_raw"
#define SENSOR_DRVNAME_HI556_TXD_MIPI_RAW       "hi556_txd_mipi_raw"
#define SENSOR_DRVNAME_HI1336_MIPI_RAW          "hi1336_mipi_raw"
#define SENSOR_DRVNAME_OV13B10_MIPI_RAW         "ov13b10_mipi_raw"
#define SENSOR_DRVNAME_GC02M1_MIPI_RAW          "gc02m1_mipi_raw"
#define SENSOR_DRVNAME_OV02B10_MIPI_RAW         "ov02b10_mipi_raw"
#define SENSOR_DRVNAME_GC02M1_SW_MIPI_RAW       "gc02m1_sw_mipi_raw"
#define SENSOR_DRVNAME_GC02M1B_MIPI_RAW         "gc02m1b_mipi_raw"
#define SENSOR_DRVNAME_OV02B1B_MIPI_RAW         "ov02b1b_mipi_raw"
#define SENSOR_DRVNAME_GC02M1B_SW_MIPI_RAW      "gc02m1b_sw_mipi_raw"
/*Parker-B*/
#define PARKERB_SHINETECH_MAIN_S5KJN103_SENSOR_ID          0x38E2  /*0x38E1 + 1*/
#define SENSOR_DRVNAME_PARKERB_SHINETECH_MAIN_S5KJN103     "parkerb_shinetech_main_s5kjn103"
#define PARKERB_SUNNY_FRONT_S5K3P9SP_SENSOR_ID             0x310B
#define SENSOR_DRVNAME_PARKERB_SUNNY_FRONT_S5K3P9SP        "parkerb_sunny_front_s5k3p9sp"
#define PARKERB_SHINETECH_MONO_GC02M1B_SENSOR_ID           0x02EF  /*0x02E0 + 15*/
#define SENSOR_DRVNAME_PARKERB_SHINETECH_MONO_GC02M1B      "parkerb_shinetech_mono_gc02m1b"
#define PARKERB_SHINETECH_MONO_OV02B1B_SENSOR_ID           0x0031/*0x2B + 6*/
#define SENSOR_DRVNAME_PARKERB_SHINETECH_MONO_OV02B1B      "parkerb_shinetech_mono_ov02b1b"
#define PARKERB_HLT_MICRO_GC02M1_SENSOR_ID                 0x02F0  /*0x02E0 + 16*/
#define SENSOR_DRVNAME_PARKERB_HLT_MICRO_GC02M1            "parkerb_hlt_micro_gc02m1"
#define PARKERB_SHINETECH_MACRO_OV02B10_SENSOR_ID          0x0030  /*0x002B + 5 */
#define SENSOR_DRVNAME_PARKERB_SHINETECH_MACRO_OV02B10     "parkerb_shinetech_macro_ov02b10"

/*YogurtA*/
#define SENSOR_DRVNAME_YOGURTA_QTECH_MAIN_OV13B10         "yogurta_qtech_main_ov13b10"
#define YOGURTA_QTECH_MAIN_OV13B10_SENSOR_ID              0x560D42 + 6
#define SENSOR_DRVNAME_YOGURTA_SHENGTAI_FRONT_GC5035      "yogurta_shengtai_front_gc5035"
#define YOGURTA_SHENGTAI_FRONT_GC5035_SENSOR_ID           0x5035 + 6
#define SENSOR_DRVNAME_YOGURTA_TXD_FRONT_HI556            "yogurta_txd_front_hi556"
#define YOGURTA_TXD_FRONT_HI556_SENSOR_ID                 0x0556 + 6

#endif //OPLUS_FEATURE_CAMERA_COMMON

/******************************************************************************
 *
 ******************************************************************************/
void KD_IMGSENSOR_PROFILE_INIT(void);
void KD_IMGSENSOR_PROFILE(char *tag);
void KD_IMGSENSOR_PROFILE_INIT_I2C(void);
void KD_IMGSENSOR_PROFILE_I2C(char *tag, int trans_num);

#define mDELAY(ms)     mdelay(ms)
#define uDELAY(us)       udelay(us)
#endif              /* _KD_IMGSENSOR_H */
