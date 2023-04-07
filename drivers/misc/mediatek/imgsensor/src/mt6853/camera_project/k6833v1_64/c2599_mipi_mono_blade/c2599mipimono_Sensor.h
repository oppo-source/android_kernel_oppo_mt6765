/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
/*****************************************************************************
 *
 * Filename:
 * ---------
 *     n26_c2519_dep_delta_mipi_mono_Sensor.h
 *
 * Project:
 * --------
 *     ALPS
 *
 * Description:
 * ------------
 *     CMOS sensor header file
 *
 ****************************************************************************/
#ifndef __N26_C2519_DEP_DELTA_MIPI_MONO_SENSOR_H__
#define __N26_C2519_DEP_DELTA_MIPI_MONO_SENSOR_H__


/* SENSOR MIRROR FLIP INFO */
//+bug596767,zhouyikuan.wt,MODIFY,2020/20/22,gc20m1b depth camera mirror and flip
#define C2519_MIRROR_NORMAL    0
#define C2519_MIRROR_H         0
#define C2519_MIRROR_V         0
#define C2519_MIRROR_HV        1
//-bug596767,zhouyikuan.wt,MODIFY,2020/20/22,gc20m1b depth camera mirror and flip

#if C2519_MIRROR_NORMAL
#define C2519_MIRROR	        0x80
#elif C2519_MIRROR_H
#define C2519_MIRROR	        0x81
#elif C2519_MIRROR_V
#define C2519_MIRROR	        0x82
#elif C2519_MIRROR_HV
#define C2519_MIRROR	        0x83
#else
#define C2519_MIRROR	        0x80
#endif


/* SENSOR PRIVATE INFO FOR GAIN SETTING */
#define C2519_SENSOR_GAIN_BASE             0x400
#define C2519_SENSOR_GAIN_MAX              (12 * C2519_SENSOR_GAIN_BASE)
#define C2519_SENSOR_GAIN_MAX_VALID_INDEX  16
#define C2519_SENSOR_GAIN_MAP_SIZE         16
#define C2519_SENSOR_DGAIN_BASE            0x400

enum{
	IMGSENSOR_MODE_INIT,
	IMGSENSOR_MODE_PREVIEW,
	IMGSENSOR_MODE_CAPTURE,
	IMGSENSOR_MODE_VIDEO,
	IMGSENSOR_MODE_HIGH_SPEED_VIDEO,
	IMGSENSOR_MODE_SLIM_VIDEO,
};

struct imgsensor_mode_struct {
	kal_uint32 pclk;
	kal_uint32 linelength;
	kal_uint32 framelength;
	kal_uint8 startx;
	kal_uint8 starty;
	kal_uint16 grabwindow_width;
	kal_uint16 grabwindow_height;
	kal_uint8 mipi_data_lp2hs_settle_dc;
	kal_uint32 mipi_pixel_rate;
	kal_uint16 max_framerate;
};

/* SENSOR PRIVATE STRUCT FOR VARIABLES */
struct imgsensor_struct {
	kal_uint8 mirror;
	kal_uint8 sensor_mode;
	kal_uint32 shutter;
	kal_uint16 gain;
	kal_uint32 pclk;
	kal_uint32 frame_length;
	kal_uint32 line_length;
	kal_uint32 min_frame_length;
	kal_uint16 dummy_pixel;
	kal_uint16 dummy_line;
	kal_uint16 current_fps;
	kal_bool   autoflicker_en;
	kal_bool   test_pattern;
	enum MSDK_SCENARIO_ID_ENUM current_scenario_id;
	kal_uint8  ihdr_en;
	kal_uint8 i2c_write_id;
};

/* SENSOR PRIVATE STRUCT FOR CONSTANT*/
struct imgsensor_info_struct {
	kal_uint32 sensor_id; //record sensor id defined in Kd_imgsensor.h
	kal_uint32 checksum_value; //checksum value for Camera Auto Test
	struct imgsensor_mode_struct pre; //preview scenario information
	struct imgsensor_mode_struct cap; //capture scenario  information
	struct imgsensor_mode_struct cap1; //capture for PIP 24fps info
	//normal video  scenario  information
	struct imgsensor_mode_struct normal_video;
	//high speed video scenario  information
	struct imgsensor_mode_struct hs_video;
	//slim video for VT scenario  information
	struct imgsensor_mode_struct slim_video;
	struct imgsensor_mode_struct custom1; //custom1 scenario information
	struct imgsensor_mode_struct custom2; //custom2 scenario information
	struct imgsensor_mode_struct custom3; //custom3 scenario information
	
	kal_uint8  ae_shut_delay_frame;	//shutter delay frame for AE cycle
	//sensor gain delay frame for AE cycle
	kal_uint8  ae_sensor_gain_delay_frame;
	kal_uint8  ae_ispGain_delay_frame; //ispgaindelayframe for AEcycle
	//+bug584789 zhouyikuan.wt, add, 2020/10/22, first supply dualcam(s5k3l6+c2515) bring up
        kal_uint8  frame_time_delay_frame;
        //-bug584789 zhouyikuan.wt, add, 2020/10/22, first supply dualcam(s5k3l6+c2515) bring up
	kal_uint8  ihdr_support;		//
	kal_uint8  ihdr_le_firstline;	//
	kal_uint8  sensor_mode_num;		//support sensor mode num

	kal_uint8  cap_delay_frame; //enter capture delay frame num
	kal_uint8  pre_delay_frame; //enter preview delay frame num
	kal_uint8  video_delay_frame; //enter video delay frame num
	kal_uint8  hs_video_delay_frame; //enter hspeedvideo delayframe num
	kal_uint8  slim_video_delay_frame; //enter svideo delay frame num
	kal_uint8  custom1_delay_frame;     //enter custom1 delay frame num
	kal_uint8  custom2_delay_frame;     //enter custom2 delay frame num
	kal_uint8  custom3_delay_frame;     //enter custom3 delay frame num
	kal_uint8  margin; //sensor framelength & shutter margin
	kal_uint32 min_shutter; //min shutter
	kal_uint32 max_frame_length; //max framelength by sensor limitation
	//+bug 558061, zhanglinfeng.wt, modify, 2020/07/02, modify codes for factory mode of photo black screen
	kal_uint32 min_gain;
	kal_uint32 max_gain;
	kal_uint32 min_gain_iso;
	kal_uint32 gain_step;
	kal_uint32 exp_step;
	kal_uint32 gain_type;
	//-bug 558061, zhanglinfeng.wt, modify, 2020/07/02, modify codes for factory mode of photo black screen
	kal_uint8  isp_driving_current;	//mclk driving current
	kal_uint8  sensor_interface_type;//sensor_interface_type

	kal_uint8  mipi_sensor_type;
	kal_uint8  mipi_settle_delay_mode;
	kal_uint8  sensor_output_dataformat;
	kal_uint8  mclk; //mclk value, suggest 24Mhz or 26Mhz

	kal_uint8  mipi_lane_num;		//mipi lane num

	kal_uint8  i2c_addr_table[5];
	kal_uint32  i2c_speed; //khz
};

extern int iReadRegI2C(u8 *a_pSendData, u16 a_sizeSendData, u8 *a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData, u16 a_sizeSendData, u16 i2cId);
extern int iWriteReg(u16 a_u2Addr, u32 a_u4Data, u32 a_u4Bytes, u16 i2cId);
extern void kdSetI2CSpeed(u16 i2cSpeed);
extern int iBurstWriteReg(u8 *pData, u32 bytes, u16 i2cId);
extern int iBurstWriteReg_multi(u8 *pData, u32 bytes, u16 i2cId, u16 transfer_length, u16 timing);
extern int iWriteRegI2CTiming(u8 *a_pSendData, u16 a_sizeSendData, u16 i2cId, u16 timing);

#endif
