/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
  ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/types.h>
#include "imgsensor_eeprom.h"
#include "imx319mipiraw_Sensor.h"

/***************Modify Following Strings for Debug**********************/
#define PFX "IMX319_camera_sensor"
/****************************   Modify end    **************************/
#define LOG_INF(format, args...) pr_debug(PFX "[%s] " format, __func__, ##args)
#define I2C_BUFFER_LEN 765 /* trans# max is 255, each 3 bytes */
static kal_uint32 streaming_control(kal_bool enable);
static DEFINE_SPINLOCK(imgsensor_drv_lock);

static struct imgsensor_info_struct imgsensor_info = {
    .sensor_id = IMX319_SENSOR_ID,
    .checksum_value = 0x8ac2d94a,

    .pre = {
        .pclk = 350000000,
        .linelength = 3968,
        .framelength = 2940,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 3264,
        .grabwindow_height = 2448,
        .mipi_pixel_rate = 320000000,
        .mipi_data_lp2hs_settle_dc = 85,
        .max_framerate = 300,
    },
    .cap = {
      .pclk = 350000000,
      .linelength = 3968,
      .framelength = 2940,
      .startx = 0,
      .starty = 0,
      .grabwindow_width = 3264,
      .grabwindow_height = 2448,
      .mipi_pixel_rate = 320000000,
      .mipi_data_lp2hs_settle_dc = 85,
      .max_framerate = 300,
    },
    .normal_video = {
      .pclk = 418000000,
      .linelength = 3968,
      .framelength = 3510,
      .startx = 0,
      .starty = 0,
      .grabwindow_width = 3264,
      .grabwindow_height = 1840,
      .mipi_pixel_rate = 416000000,
      .mipi_data_lp2hs_settle_dc = 85,
      .max_framerate = 300,
    },
    .hs_video = {
      .pclk = 210000000,
      .linelength = 2080,
      .framelength = 3364,
      .startx = 0,
      .starty = 0,
      .grabwindow_width = 1632,
      .grabwindow_height = 1224,
      .mipi_pixel_rate = 251200000,
      .mipi_data_lp2hs_settle_dc = 85,
      .max_framerate = 300,
    },
    .slim_video = {
      .pclk = 468000000,
      .linelength = 3968,
      .framelength = 1965,
      .startx = 0,
      .starty = 0,
      .grabwindow_width = 3264,
      .grabwindow_height = 1840,
      .mipi_pixel_rate = 406400000,
      .mipi_data_lp2hs_settle_dc = 85,
      .max_framerate = 600,
    },
    .custom1 = {
      .pclk = 209000000,
      .linelength = 3968,
      .framelength = 2194,
      .startx = 0,
      .starty = 0,
      .grabwindow_width = 2304,
      .grabwindow_height = 1728,
      .mipi_pixel_rate = 251200000,
      .mipi_data_lp2hs_settle_dc = 85,
      .max_framerate = 240,
    },
    .custom2 = {
      .pclk = 210000000,
      .linelength = 2080,
      .framelength = 3364,
      .startx = 0,
      .starty = 0,
      .grabwindow_width = 1632,
      .grabwindow_height = 1224,
      .mipi_pixel_rate = 251200000,
      .mipi_data_lp2hs_settle_dc = 85,
      .max_framerate = 300,
    },
      .margin = 18,
      .min_shutter = 1,
      .min_gain = 64, /*1x gain*/
      .max_gain = 1024, /*16x gain*/
      .min_gain_iso = 100,
      .gain_step = 1,
      .gain_type = 0,
      .max_frame_length = 0xffff,
      .ae_shut_delay_frame = 0,
      .ae_sensor_gain_delay_frame = 0,
      .ae_ispGain_delay_frame = 2,
      .ihdr_support = 0,
      .ihdr_le_firstline = 0,
      .sensor_mode_num = 7,
      .cap_delay_frame = 2,
      .pre_delay_frame = 2,
      .video_delay_frame = 2,
      .hs_video_delay_frame = 2,
      .slim_video_delay_frame = 2,
      .custom1_delay_frame = 2,
      .custom2_delay_frame = 2,
      .frame_time_delay_frame = 3,
      .isp_driving_current = ISP_DRIVING_6MA,
      .sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
      .mipi_sensor_type = MIPI_OPHY_NCSI2,
      .mipi_settle_delay_mode = 0,
      .sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_R,
      .mclk = 24,
      .mipi_lane_num = SENSOR_MIPI_4_LANE,
      .i2c_addr_table = {0x20, 0xff},
      .i2c_speed = 1000,
};

static kal_uint16 sensor_init_setting_array[] = {
     0x0136, 0x18,
     0x0137, 0x00,
     0x3c7e, 0x05,
     0x3c7f, 0x07,
     0x4d39, 0x0b,
     0x4d41, 0x33,
     0x4d43, 0x0c,
     0x4d49, 0x89,
     0x4e05, 0x0b,
     0x4e0d, 0x33,
     0x4e0f, 0x0c,
     0x4e15, 0x89,
     0x4e49, 0x2a,
     0x4e51, 0x33,
     0x4e53, 0x0c,
     0x4e59, 0x89,
     0x5601, 0x4f,
     0x560b, 0x45,
     0x562f, 0x0a,
     0x5643, 0x0a,
     0x5645, 0x0c,
     0x56ef, 0x51,
     0x586f, 0x33,
     0x5873, 0x89,
     0x5905, 0x33,
     0x5907, 0x89,
     0x590d, 0x33,
     0x590f, 0x89,
     0x5915, 0x33,
     0x5917, 0x89,
     0x5969, 0x1c,
     0x596b, 0x72,
     0x5971, 0x33,
     0x5973, 0x89,
     0x5975, 0x33,
     0x5977, 0x89,
     0x5979, 0x1c,
     0x597b, 0x72,
     0x5985, 0x33,
     0x5987, 0x89,
     0x5999, 0x1c,
     0x599b, 0x72,
     0x59a5, 0x33,
     0x59a7, 0x89,
     0x7485, 0x08,
     0x7487, 0x0c,
     0x7489, 0xc7,
     0x748b, 0x8b,
     0x9004, 0x09,
     0x9200, 0x6a,
     0x9201, 0x22,
     0x9202, 0x6a,
     0x9203, 0x23,
     0x9204, 0x5f,
     0x9205, 0x23,
     0x9206, 0x5f,
     0x9207, 0x24,
     0x9208, 0x5f,
     0x9209, 0x26,
     0x920a, 0x5f,
     0x920b, 0x27,
     0x920c, 0x5f,
     0x920d, 0x29,
     0x920e, 0x5f,
     0x920f, 0x2a,
     0x9210, 0x5f,
     0x9211, 0x2c,
     0xbc22, 0x1a,
     0xf01f, 0x04,
     0xf021, 0x03,
     0xf023, 0x02,
     0xf03d, 0x05,
     0xf03f, 0x03,
     0xf041, 0x02,
     0xf0af, 0x04,
     0xf0b1, 0x03,
     0xf0b3, 0x02,
     0xf0cd, 0x05,
     0xf0cf, 0x03,
     0xf0d1, 0x02,
     0xf13f, 0x04,
     0xf141, 0x03,
     0xf143, 0x02,
     0xf15d, 0x05,
     0xf15f, 0x03,
     0xf161, 0x02,
     0xf1cf, 0x04,
     0xf1d1, 0x03,
     0xf1d3, 0x02,
     0xf1ed, 0x05,
     0xf1ef, 0x03,
     0xf1f1, 0x02,
     0xf287, 0x04,
     0xf289, 0x03,
     0xf28b, 0x02,
     0xf2a5, 0x05,
     0xf2a7, 0x03,
     0xf2a9, 0x02,
     0xf2b7, 0x04,
     0xf2b9, 0x03,
     0xf2bb, 0x02,
     0xf2d5, 0x05,
     0xf2d7, 0x03,
     0xf2d9, 0x02,
     //image_quality
     0x7980, 0x00,
     0x7981, 0x00,
     0x8C1F, 0x00,
     0x8C27, 0x00,
     0x9963, 0x64,
     0x9964, 0x50,
     0xB6D9, 0x00,
     0xF000, 0x3F,
     0xF001, 0x3F,
     0xF002, 0x3F,
     0xF015, 0x3F,
     0xF016, 0x3F,
     0xF017, 0x3F,
     0xF08B, 0xD0,
     0xF08D, 0xD0,
     0xF08F, 0xD0,
     0xF090, 0x2F,
     0xF091, 0x2F,
     0xF092, 0x2F,
     0xF120, 0x3F,
     0xF121, 0x3F,
     0xF122, 0x3F,
     0xF135, 0x3F,
     0xF136, 0x3F,
     0xF137, 0x3F,
     0xF1AB, 0xD0,
     0xF1AD, 0xD0,
     0xF1AF, 0xD0,
     0xF250, 0x3F,
     0xF251, 0x3F,
     0xF252, 0x3F,
};

static kal_uint16 preview_setting_array[] = {
    0x0112, 0x0A,
    0x0113, 0x0A,
    0x0114, 0x03,
    0x0342, 0x0F,
    0x0343, 0x80,
    0x0340, 0x0B,
    0x0341, 0x7C,
    0x0344, 0x00,
    0x0345, 0x00,
    0x0346, 0x00,
    0x0347, 0x08,
    0x0348, 0x0C,
    0x0349, 0xCF,
    0x034A, 0x09,
    0x034B, 0x97,
    0x0220, 0x00,
    0x0221, 0x11,
    0x0381, 0x01,
    0x0383, 0x01,
    0x0385, 0x01,
    0x0387, 0x01,
    0x0900, 0x00,
    0x0901, 0x11,
    0x0902, 0x0A,
    0x3140, 0x02,
    0x3141, 0x00,
    0x3F0D, 0x0A,
    0x3F14, 0x01,
    0x3F3C, 0x01,
    0x3F4D, 0x01,
    0x3F4C, 0x01,
    0x4254, 0x7F,
    0x0401, 0x00,
    0x0404, 0x00,
    0x0405, 0x10,
    0x0408, 0x00,
    0x0409, 0x08,
    0x040A, 0x00,
    0x040B, 0x00,
    0x040C, 0x0C,
    0x040D, 0xC0,
    0x040E, 0x09,
    0x040F, 0x90,
    0x034C, 0x0C,
    0x034D, 0xC0,
    0x034E, 0x09,
    0x034F, 0x90,
    0x3261, 0x00,
    0x3264, 0x00,
    0x3265, 0x10,
    0x0301, 0x06,
    0x0303, 0x04,
    0x0305, 0x04,
    0x0306, 0x01,
    0x0307, 0x5E,
    0x0309, 0x0A,
    0x030B, 0x02,
    0x030D, 0x03,
    0x030E, 0x00,
    0x030F, 0xC8,
    0x0310, 0x01,
    0x0820, 0x0C,
    0x0821, 0x80,
    0x0822, 0x00,
    0x0823, 0x00,
    0x3E20, 0x01,
    0x3E37, 0x01,
    0x3E3B, 0x01,
    0x38A3, 0x01,
    0x38A8, 0x00,
    0x38A9, 0x00,
    0x38AA, 0x00,
    0x38AB, 0x00,
    0x3234, 0x00,
    0x3FC1, 0x00,
    0x3235, 0x00,
    0x3802, 0x00,
    0x3143, 0x04,
    0x360A, 0x00,
    0x0B00, 0x00,
    0x0106, 0x00,
    0x0B05, 0x01,
    0x0B06, 0x01,
    0x3230, 0x00,
    0x3602, 0x01,
    0x3607, 0x01,
    0x3C00, 0x00,
    0x3C01, 0x48,
    0x3C02, 0xC8,
    0x3C03, 0xAA,
    0x3C04, 0x91,
    0x3C05, 0x54,
    0x3C06, 0x26,
    0x3C07, 0x20,
    0x3C08, 0x51,
    0x3D80, 0x00,
    0x3F50, 0x00,
    0x3F56, 0x00,
    0x3F57, 0x30,
    0x3F78, 0x01,
    0x3F79, 0x18,
    0x3F7C, 0x00,
    0x3F7D, 0x00,
    0x3FBA, 0x00,
    0x3FBB, 0x00,
    0xA081, 0x00,
    0xE014, 0x00,
    0x0202, 0x0B,
    0x0203, 0x6A,
    0x0224, 0x01,
    0x0225, 0xF4,
    0x0204, 0x00,
    0x0205, 0x00,
    0x0216, 0x00,
    0x0217, 0x00,
    0x020E, 0x01,
    0x020F, 0x00,
    0x0210, 0x01,
    0x0211, 0x00,
    0x0212, 0x01,
    0x0213, 0x00,
    0x0214, 0x01,
    0x0215, 0x00,
    0x0218, 0x01,
    0x0219, 0x00,
    0x3614, 0x00,
    0x3616, 0x0D,
    0x3617, 0x56,
    0xB612, 0x20,
    0xB613, 0x20,
    0xB614, 0x20,
    0xB615, 0x20,
    0xB616, 0x0A,
    0xB617, 0x0A,
    0xB618, 0x20,
    0xB619, 0x20,
    0xB61A, 0x20,
    0xB61B, 0x20,
    0xB61C, 0x0A,
    0xB61D, 0x0A,
    0xB666, 0x30,
    0xB667, 0x30,
    0xB668, 0x30,
    0xB669, 0x30,
    0xB66A, 0x14,
    0xB66B, 0x14,
    0xB66C, 0x20,
    0xB66D, 0x20,
    0xB66E, 0x20,
    0xB66F, 0x20,
    0xB670, 0x10,
    0xB671, 0x10,
    0x3237, 0x00,
    0x3900, 0x00,
    0x3901, 0x00,
    0x3902, 0x00,
    0x3904, 0x00,
    0x3905, 0x00,
    0x3906, 0x00,
    0x3907, 0x00,
    0x3908, 0x00,
    0x3909, 0x00,
    0x3912, 0x00,
    0x3930, 0x00,
    0x3931, 0x00,
    0x3933, 0x00,
    0x3934, 0x00,
    0x3935, 0x00,
    0x3936, 0x00,
    0x3937, 0x00,
    0x30AC, 0x00,
};


static kal_uint16 capture_setting_array[] = {
    0x0112, 0x0A,
    0x0113, 0x0A,
    0x0114, 0x03,
    0x0342, 0x0F,
    0x0343, 0x80,
    0x0340, 0x0B,
    0x0341, 0x7C,
    0x0344, 0x00,
    0x0345, 0x00,
    0x0346, 0x00,
    0x0347, 0x08,
    0x0348, 0x0C,
    0x0349, 0xCF,
    0x034A, 0x09,
    0x034B, 0x97,
    0x0220, 0x00,
    0x0221, 0x11,
    0x0381, 0x01,
    0x0383, 0x01,
    0x0385, 0x01,
    0x0387, 0x01,
    0x0900, 0x00,
    0x0901, 0x11,
    0x0902, 0x0A,
    0x3140, 0x02,
    0x3141, 0x00,
    0x3F0D, 0x0A,
    0x3F14, 0x01,
    0x3F3C, 0x01,
    0x3F4D, 0x01,
    0x3F4C, 0x01,
    0x4254, 0x7F,
    0x0401, 0x00,
    0x0404, 0x00,
    0x0405, 0x10,
    0x0408, 0x00,
    0x0409, 0x08,
    0x040A, 0x00,
    0x040B, 0x00,
    0x040C, 0x0C,
    0x040D, 0xC0,
    0x040E, 0x09,
    0x040F, 0x90,
    0x034C, 0x0C,
    0x034D, 0xC0,
    0x034E, 0x09,
    0x034F, 0x90,
    0x3261, 0x00,
    0x3264, 0x00,
    0x3265, 0x10,
    0x0301, 0x06,
    0x0303, 0x04,
    0x0305, 0x04,
    0x0306, 0x01,
    0x0307, 0x5E,
    0x0309, 0x0A,
    0x030B, 0x02,
    0x030D, 0x03,
    0x030E, 0x00,
    0x030F, 0xC8,
    0x0310, 0x01,
    0x0820, 0x0C,
    0x0821, 0x80,
    0x0822, 0x00,
    0x0823, 0x00,
    0x3E20, 0x01,
    0x3E37, 0x01,
    0x3E3B, 0x01,
    0x38A3, 0x01,
    0x38A8, 0x00,
    0x38A9, 0x00,
    0x38AA, 0x00,
    0x38AB, 0x00,
    0x3234, 0x00,
    0x3FC1, 0x00,
    0x3235, 0x00,
    0x3802, 0x00,
    0x3143, 0x04,
    0x360A, 0x00,
    0x0B00, 0x00,
    0x0106, 0x00,
    0x0B05, 0x01,
    0x0B06, 0x01,
    0x3230, 0x00,
    0x3602, 0x01,
    0x3607, 0x01,
    0x3C00, 0x00,
    0x3C01, 0x48,
    0x3C02, 0xC8,
    0x3C03, 0xAA,
    0x3C04, 0x91,
    0x3C05, 0x54,
    0x3C06, 0x26,
    0x3C07, 0x20,
    0x3C08, 0x51,
    0x3D80, 0x00,
    0x3F50, 0x00,
    0x3F56, 0x00,
    0x3F57, 0x30,
    0x3F78, 0x01,
    0x3F79, 0x18,
    0x3F7C, 0x00,
    0x3F7D, 0x00,
    0x3FBA, 0x00,
    0x3FBB, 0x00,
    0xA081, 0x00,
    0xE014, 0x00,
    0x0202, 0x0B,
    0x0203, 0x6A,
    0x0224, 0x01,
    0x0225, 0xF4,
    0x0204, 0x00,
    0x0205, 0x00,
    0x0216, 0x00,
    0x0217, 0x00,
    0x020E, 0x01,
    0x020F, 0x00,
    0x0210, 0x01,
    0x0211, 0x00,
    0x0212, 0x01,
    0x0213, 0x00,
    0x0214, 0x01,
    0x0215, 0x00,
    0x0218, 0x01,
    0x0219, 0x00,
    0x3614, 0x00,
    0x3616, 0x0D,
    0x3617, 0x56,
    0xB612, 0x20,
    0xB613, 0x20,
    0xB614, 0x20,
    0xB615, 0x20,
    0xB616, 0x0A,
    0xB617, 0x0A,
    0xB618, 0x20,
    0xB619, 0x20,
    0xB61A, 0x20,
    0xB61B, 0x20,
    0xB61C, 0x0A,
    0xB61D, 0x0A,
    0xB666, 0x30,
    0xB667, 0x30,
    0xB668, 0x30,
    0xB669, 0x30,
    0xB66A, 0x14,
    0xB66B, 0x14,
    0xB66C, 0x20,
    0xB66D, 0x20,
    0xB66E, 0x20,
    0xB66F, 0x20,
    0xB670, 0x10,
    0xB671, 0x10,
    0x3237, 0x00,
    0x3900, 0x00,
    0x3901, 0x00,
    0x3902, 0x00,
    0x3904, 0x00,
    0x3905, 0x00,
    0x3906, 0x00,
    0x3907, 0x00,
    0x3908, 0x00,
    0x3909, 0x00,
    0x3912, 0x00,
    0x3930, 0x00,
    0x3931, 0x00,
    0x3933, 0x00,
    0x3934, 0x00,
    0x3935, 0x00,
    0x3936, 0x00,
    0x3937, 0x00,
    0x30AC, 0x00,
};

static kal_uint16 normal_video_setting_array[] = {
     0x0112, 0x0a,
     0x0113, 0x0a,
     0x0114, 0x03,
     0x0342, 0x0f,
     0x0343, 0x80,
     0x0340, 0x0d,
     0x0341, 0xb6,
     0x0344, 0x00,
     0x0345, 0x00,
     0x0346, 0x01,
     0x0347, 0x38,
     0x0348, 0x0c,
     0x0349, 0xcf,
     0x034a, 0x08,
     0x034b, 0x67,
     0x0220, 0x00,
     0x0221, 0x11,
     0x0381, 0x01,
     0x0383, 0x01,
     0x0385, 0x01,
     0x0387, 0x01,
     0x0900, 0x00,
     0x0901, 0x11,
     0x0902, 0x0a,
     0x3140, 0x02,
     0x3141, 0x00,
     0x3f0d, 0x0a,
     0x3f14, 0x01,
     0x3f3c, 0x01,
     0x3f4d, 0x01,
     0x3f4c, 0x01,
     0x4254, 0x7f,
     0x0401, 0x00,
     0x0404, 0x00,
     0x0405, 0x10,
     0x0408, 0x00,
     0x0409, 0x08,
     0x040a, 0x00,
     0x040b, 0x00,
     0x040c, 0x0c,
     0x040d, 0xc0,
     0x040e, 0x07,
     0x040f, 0x30,
     0x034c, 0x0c,
     0x034d, 0xc0,
     0x034e, 0x07,
     0x034f, 0x30,
     0x3261, 0x00,
     0x3264, 0x00,
     0x3265, 0x10,
     0x0301, 0x06,
     0x0303, 0x02,
     0x0305, 0x04,
     0x0306, 0x00,
     0x0307, 0xd1,
     0x0309, 0x0a,
     0x030b, 0x02,
     0x030d, 0x03,
     0x030e, 0x01,
     0x030f, 0x04,
     0x0310, 0x01,
     0x0820, 0x10,
     0x0821, 0x40,
     0x0822, 0x00,
     0x0823, 0x00,
     0x3e20, 0x01,
     0x3e37, 0x00,
     0x3e3b, 0x01,
     0x38a3, 0x01,
     0x38a8, 0x00,
     0x38a9, 0x00,
     0x38aa, 0x00,
     0x38ab, 0x00,
     0x3234, 0x00,
     0x3fc1, 0x00,
     0x3235, 0x00,
     0x3802, 0x00,
     0x3143, 0x04,
     0x360a, 0x00,
     0x0b00, 0x00,
     0x0106, 0x00,
     0x0b05, 0x01,
     0x0b06, 0x01,
     0x3230, 0x00,
     0x3602, 0x01,
     0x3607, 0x01,
     0x3c00, 0x00,
     0x3c01, 0x48,
     0x3c02, 0xc8,
     0x3c03, 0xaa,
     0x3c04, 0x91,
     0x3c05, 0x54,
     0x3c06, 0x26,
     0x3c07, 0x20,
     0x3c08, 0x51,
     0x3d80, 0x00,
     0x3f50, 0x00,
     0x3f56, 0x00,
     0x3F57, 0x30,
     0x3F78, 0x01,
     0x3F79, 0x18,
     0x3F7C, 0x00,
     0x3F7D, 0x00,
     0x3FBA, 0x00,
     0x3FBB, 0x00,
     0xA081, 0x00,
     0xE014, 0x00,
     0x0202, 0x0D,
     0x0203, 0xA4,
     0x0224, 0x01,
     0x0225, 0xF4,
     0x0204, 0x00,
     0x0205, 0x00,
     0x0216, 0x00,
     0x0217, 0x00,
     0x020E, 0x01,
     0x020F, 0x00,
     0x0210, 0x01,
     0x0211, 0x00,
     0x0212, 0x01,
     0x0213, 0x00,
     0x0214, 0x01,
     0x0215, 0x00,
     0x0218, 0x01,
     0x0219, 0x00,
     0x3614, 0x00,
     0x3616, 0x0D,
     0x3617, 0x56,
     0xB612, 0x20,
     0xB613, 0x20,
     0xB614, 0x20,
     0xB615, 0x20,
     0xB616, 0x0A,
     0xB617, 0x0A,
     0xB618, 0x20,
     0xB619, 0x20,
     0xB61A, 0x20,
     0xB61B, 0x20,
     0xB61C, 0x0A,
     0xB61D, 0x0A,
     0xB666, 0x30,
     0xB667, 0x30,
     0xB668, 0x30,
     0xB669, 0x30,
     0xB66A, 0x14,
     0xB66B, 0x14,
     0xB66C, 0x20,
     0xB66D, 0x20,
     0xB66E, 0x20,
     0xB66F, 0x20,
     0xB670, 0x10,
     0xB671, 0x10,
     0x3237, 0x00,
     0x3900, 0x00,
     0x3901, 0x00,
     0x3902, 0x00,
     0x3904, 0x00,
     0x3905, 0x00,
     0x3906, 0x00,
     0x3907, 0x00,
     0x3908, 0x00,
     0x3909, 0x00,
     0x3912, 0x00,
     0x3930, 0x00,
     0x3931, 0x00,
     0x3933, 0x00,
     0x3934, 0x00,
     0x3935, 0x00,
     0x3936, 0x00,
     0x3937, 0x00,
     0x30AC, 0x00,
};

static kal_uint16 hs_video_setting_array[] = {
     0x0112, 0x0a,
     0x0113, 0x0a,
     0x0114, 0x03,
     0x0342, 0x08,
     0x0343, 0x20,
     0x0340, 0x0d,
     0x0341, 0x24,
     0x0344, 0x00,
     0x0345, 0x00,
     0x0346, 0x00,
     0x0347, 0x08,
     0x0348, 0x0c,
     0x0349, 0xcf,
     0x034a, 0x09,
     0x034b, 0x97,
     0x0220, 0x00,
     0x0221, 0x11,
     0x0381, 0x01,
     0x0383, 0x01,
     0x0385, 0x01,
     0x0387, 0x01,
     0x0900, 0x01,
     0x0901, 0x22,
     0x0902, 0x08,
     0x3140, 0x02,
     0x3141, 0x00,
     0x3f0d, 0x0a,
     0x3f14, 0x01,
     0x3f3c, 0x02,
     0x3f4d, 0x01,
     0x3f4c, 0x01,
     0x4254, 0x7f,
     0x0401, 0x00,
     0x0404, 0x00,
     0x0405, 0x10,
     0x0408, 0x00,
     0x0409, 0x04,
     0x040a, 0x00,
     0x040b, 0x00,
     0x040c, 0x06,
     0x040d, 0x60,
     0x040e, 0x04,
     0x040f, 0xc8,
     0x034c, 0x06,
     0x034d, 0x60,
     0x034e, 0x04,
     0x034f, 0xc8,
     0x3261, 0x00,
     0x3264, 0x00,
     0x3265, 0x10,
     0x0301, 0x06,
     0x0303, 0x04,
     0x0305, 0x04,
     0x0306, 0x00,
     0x0307, 0xd2,
     0x0309, 0x0a,
     0x030b, 0x02,
     0x030d, 0x03,
     0x030e, 0x00,
     0x030f, 0x9d,
     0x0310, 0x01,
     0x0820, 0x09,
     0x0821, 0xd0,
     0x0822, 0x00,
     0x0823, 0x00,
     0x3e20, 0x01,
     0x3e37, 0x00,
     0x3e3b, 0x01,
     0x3234, 0x00,
     0x3fc1, 0x00,
     0x3235, 0x00,
     0x3802, 0x00,
     0x3143, 0x04,
     0x360a, 0x00,
     0x0b00, 0x00,
     0x0106, 0x00,
     0x0b05, 0x01,
     0x0b06, 0x01,
     0x3230, 0x00,
     0x3602, 0x01,
     0x3607, 0x01,
     0x3c00, 0x00,
     0x3c01, 0xba,
     0x3c02, 0xc8,
     0x3c03, 0xaa,
     0x3c04, 0x91,
     0x3c05, 0x54,
     0x3c06, 0x26,
     0x3c07, 0x20,
     0x3c08, 0x51,
     0x3d80, 0x00,
     0x3f50, 0x00,
     0x3f56, 0x00,
     0x3f57, 0x30,
     0x3f78, 0x00,
     0x3f79, 0x34,
     0x3f7c, 0x00,
     0x3f7d, 0x00,
     0x3fba, 0x00,
     0x3fbb, 0x00,
     0xa081, 0x04,
     0xe014, 0x00,
     0x0202, 0x0d,
     0x0203, 0x12,
     0x0224, 0x01,
     0x0225, 0xf4,
     0x0204, 0x00,
     0x0205, 0x00,
     0x0216, 0x00,
     0x0217, 0x00,
     0x020e, 0x01,
     0x020f, 0x00,
     0x0210, 0x01,
     0x0211, 0x00,
     0x0212, 0x01,
     0x0213, 0x00,
     0x0214, 0x01,
     0x0215, 0x00,
     0x0218, 0x01,
     0x0219, 0x00,
     0x3614, 0x00,
     0x3616, 0x0d,
     0x3617, 0x56,
     0xb612, 0x20,
     0xb613, 0x20,
     0xb614, 0x20,
     0xb615, 0x20,
     0xb616, 0x0a,
     0xb617, 0x0a,
     0xb618, 0x20,
     0xb619, 0x20,
     0xb61a, 0x20,
     0xb61b, 0x20,
     0xb61c, 0x0a,
     0xb61d, 0x0a,
     0xb666, 0x30,
     0xb667, 0x30,
     0xb668, 0x30,
     0xb669, 0x30,
     0xb66a, 0x14,
     0xb66b, 0x14,
     0xb66c, 0x20,
     0xb66d, 0x20,
     0xb66e, 0x20,
     0xb66f, 0x20,
     0xb670, 0x10,
     0xb671, 0x10,
     0x3237, 0x00,
     0x3900, 0x00,
     0x3901, 0x00,
     0x3902, 0x00,
     0x3904, 0x00,
     0x3905, 0x00,
     0x3906, 0x00,
     0x3907, 0x00,
     0x3908, 0x00,
     0x3909, 0x00,
     0x3912, 0x00,
     0x3930, 0x00,
     0x3931, 0x00,
     0x3933, 0x00,
     0x3934, 0x00,
     0x3935, 0x00,
     0x3936, 0x00,
     0x3937, 0x00,
     0x30ac, 0x00,
     //pdaf win mode
     0x38a3, 0x02,
     //fixed win setting
     0x38a4, 0x00,
     0x38a5, 0x00,
     0x38a6, 0x00,
     0x38a7, 0x00,
     0x38a8, 0x00,
     0x38a9, 0x00,
     0x38aa, 0x00,
     0x38ab, 0x00,
     //flexible setting
     0x38ac, 0x01,
     0x38ad, 0x00,
     0x38ae, 0x00,
     0x38af, 0x00,
     0x38b0, 0x00,
     0x38b1, 0x00,
     0x38b2, 0x00,
     0x38b3, 0x00,
     /*Area setting*/
     0x38b4, 0x04,
     0x38b5, 0x58,
     0x38b6, 0x02,
     0x38b7, 0xbb,
     0x38b8, 0x08,
     0x38b9, 0x68,
     0x38ba, 0x04,
     0x38bb, 0x75,
};

static kal_uint16 slim_video_setting_array[] = {
     0x0100, 0x00,
     0x0112, 0x0a,
     0x0113, 0x0a,
     0x0114, 0x01,
     0x0342, 0x0f,
     0x0343, 0x80,
     0x0340, 0x07,
     0x0341, 0xad,
     0x0344, 0x00,
     0x0345, 0x00,
     0x0346, 0x01,
     0x0347, 0x38,
     0x0348, 0x0c,
     0x0349, 0xcf,
     0x034a, 0x08,
     0x034b, 0x67,
     0x0220, 0x00,
     0x0221, 0x11,
     0x0381, 0x01,
     0x0383, 0x01,
     0x0385, 0x01,
     0x0387, 0x01,
     0x0900, 0x00,
     0x0901, 0x11,
     0x0902, 0x0a,
     0x3140, 0x02,
     0x3141, 0x00,
     0x3f0d, 0x0a,
     0x3f14, 0x01,
     0x3f3c, 0x01,
     0x3f4d, 0x01,
     0x3f4c, 0x01,
     0x4254, 0x7f,
     0x0401, 0x00,
     0x0404, 0x00,
     0x0405, 0x10,
     0x0408, 0x00,
     0x0409, 0x08,
     0x040a, 0x00,
     0x040b, 0x00,
     0x040c, 0x0c,
     0x040d, 0xc0,
     0x040e, 0x07,
     0x040f, 0x30,
     0x034c, 0x0c,
     0x034d, 0xc0,
     0x034e, 0x07,
     0x034f, 0x30,
     0x3261, 0x00,
     0x3264, 0x00,
     0x3265, 0x10,
     0x0301, 0x06,
     0x0303, 0x02,
     0x0305, 0x04,
     0x0306, 0x00,
     0x0307, 0xea,
     0x0309, 0x0a,
     0x030b, 0x01,
     0x030d, 0x03,
     0x030e, 0x00,
     0x030f, 0xfe,
     0x0310, 0x01,
     0x0820, 0x0f,
     0x0821, 0xe0,
     0x0822, 0x00,
     0x0823, 0x00,
     0x3e20, 0x01,
     0x3e37, 0x01,
     0x3e3b, 0x01,
     0x3234, 0x00,
     0x3fc1, 0x00,
     0x3235, 0x00,
     0x3802, 0x00,
     0x3143, 0x04,
     0x360a, 0x00,
     0x0b00, 0x00,
     0x0106, 0x00,
     0x0b05, 0x01,
     0x0b06, 0x01,
     0x3230, 0x00,
     0x3602, 0x01,
     0x3607, 0x01,
     0x3c00, 0x00,
     0x3c01, 0x48,
     0x3c02, 0xc8,
     0x3c03, 0xaa,
     0x3c04, 0x91,
     0x3c05, 0x54,
     0x3c06, 0x26,
     0x3c07, 0x20,
     0x3c08, 0x51,
     0x3d80, 0x00,
     0x3f50, 0x00,
     0x3f56, 0x00,
     0x3f57, 0x30,
     0x3f78, 0x01,
     0x3f79, 0x18,
     0x3f7c, 0x00,
     0x3f7d, 0x00,
     0x3fba, 0x00,
     0x3fbb, 0x00,
     0xa081, 0x00,
     0xe014, 0x00,
     0x0202, 0x07,
     0x0203, 0x9b,
     0x0224, 0x01,
     0x0225, 0xf4,
     0x0204, 0x00,
     0x0205, 0x00,
     0x0216, 0x00,
     0x0217, 0x00,
     0x020e, 0x01,
     0x020f, 0x00,
     0x0210, 0x01,
     0x0211, 0x00,
     0x0212, 0x01,
     0x0213, 0x00,
     0x0214, 0x01,
     0x0215, 0x00,
     0x0218, 0x01,
     0x0219, 0x00,
     0x3614, 0x00,
     0x3616, 0x0d,
     0x3617, 0x56,
     0xb612, 0x20,
     0xb613, 0x20,
     0xb614, 0x20,
     0xb615, 0x20,
     0xb616, 0x0a,
     0xb617, 0x0a,
     0xb618, 0x20,
     0xb619, 0x20,
     0xb61a, 0x20,
     0xb61b, 0x20,
     0xb61c, 0x0a,
     0xb61d, 0x0a,
     0xb666, 0x30,
     0xb667, 0x30,
     0xb668, 0x30,
     0xb669, 0x30,
     0xb66a, 0x14,
     0xb66b, 0x14,
     0xb66c, 0x20,
     0xb66d, 0x20,
     0xb66e, 0x20,
     0xb66f, 0x20,
     0xb670, 0x10,
     0xb671, 0x10,
     0x3237, 0x00,
     0x3900, 0x00,
     0x3901, 0x00,
     0x3902, 0x00,
     0x3904, 0x00,
     0x3905, 0x00,
     0x3906, 0x00,
     0x3907, 0x00,
     0x3908, 0x00,
     0x3909, 0x00,
     0x3912, 0x00,
     0x3930, 0x00,
     0x3931, 0x00,
     0x3933, 0x00,
     0x3934, 0x00,
     0x3935, 0x00,
     0x3936, 0x00,
     0x3937, 0x00,
     0x30ac, 0x00,
     //pdaf win mode
     0x38a3, 0x02,
     //fixed win setting
     0x38a4, 0x00,
     0x38a5, 0x00,
     0x38a6, 0x00,
     0x38a7, 0x00,
     0x38a8, 0x00,
     0x38a9, 0x00,
     0x38aa, 0x00,
     0x38ab, 0x00,
     //flexible setting
     0x38ac, 0x01,
     0x38ad, 0x00,
     0x38ae, 0x00,
     0x38af, 0x00,
     0x38b0, 0x00,
     0x38b1, 0x00,
     0x38b2, 0x00,
     0x38b3, 0x00,
     /*Area setting*/
     0x38b4, 0x04,
     0x38b5, 0x58,
     0x38b6, 0x03,
     0x38b7, 0xa2,
     0x38b8, 0x08,
     0x38b9, 0x68,
     0x38ba, 0x05,
     0x38bb, 0xee,
};
static kal_uint16 custom1_setting_array[] = {
    0x0112, 0x0A,
    0x0113, 0x0A,
    0x0114, 0x03,
    0x0342, 0x0F,
    0x0343, 0x80,
    0x0340, 0x08,
    0x0341, 0x92,
    0x0344, 0x00,
    0x0345, 0x00,
    0x0346, 0x01,
    0x0347, 0x70,
    0x0348, 0x0C,
    0x0349, 0xCF,
    0x034A, 0x08,
    0x034B, 0x2F,
    0x0220, 0x00,
    0x0221, 0x11,
    0x0381, 0x01,
    0x0383, 0x01,
    0x0385, 0x01,
    0x0387, 0x01,
    0x0900, 0x00,
    0x0901, 0x11,
    0x0902, 0x0A,
    0x3140, 0x02,
    0x3141, 0x00,
    0x3F0D, 0x0A,
    0x3F14, 0x01,
    0x3F3C, 0x01,
    0x3F4D, 0x01,
    0x3F4C, 0x01,
    0x4254, 0x7F,
    0x0401, 0x00,
    0x0404, 0x00,
    0x0405, 0x10,
    0x0408, 0x01,
    0x0409, 0xE8,
    0x040A, 0x00,
    0x040B, 0x00,
    0x040C, 0x09,
    0x040D, 0x00,
    0x040E, 0x06,
    0x040F, 0xC0,
    0x034C, 0x09,
    0x034D, 0x00,
    0x034E, 0x06,
    0x034F, 0xC0,
    0x3261, 0x00,
    0x3264, 0x00,
    0x3265, 0x10,
    0x0301, 0x06,
    0x0303, 0x04,
    0x0305, 0x04,
    0x0306, 0x00,
    0x0307, 0xD1,
    0x0309, 0x0A,
    0x030B, 0x02,
    0x030D, 0x03,
    0x030E, 0x00,
    0x030F, 0x9D,
    0x0310, 0x01,
    0x0820, 0x09,
    0x0821, 0xD0,
    0x0822, 0x00,
    0x0823, 0x00,
    0x3E20, 0x01,
    0x3E37, 0x01,
    0x3E3B, 0x01,
    0x38A3, 0x01,
    0x38A8, 0x00,
    0x38A9, 0x00,
    0x38AA, 0x00,
    0x38AB, 0x00,
    0x3234, 0x00,
    0x3FC1, 0x00,
    0x3235, 0x00,
    0x3802, 0x00,
    0x3143, 0x04,
    0x360A, 0x00,
    0x0B00, 0x00,
    0x0106, 0x00,
    0x0B05, 0x01,
    0x0B06, 0x01,
    0x3230, 0x00,
    0x3602, 0x01,
    0x3607, 0x01,
    0x3C00, 0x00,
    0x3C01, 0x48,
    0x3C02, 0xC8,
    0x3C03, 0xAA,
    0x3C04, 0x91,
    0x3C05, 0x54,
    0x3C06, 0x26,
    0x3C07, 0x20,
    0x3C08, 0x51,
    0x3D80, 0x00,
    0x3F50, 0x00,
    0x3F56, 0x00,
    0x3F57, 0x30,
    0x3F78, 0x01,
    0x3F79, 0x18,
    0x3F7C, 0x00,
    0x3F7D, 0x00,
    0x3FBA, 0x00,
    0x3FBB, 0x00,
    0xA081, 0x00,
    0xE014, 0x00,
    0x0202, 0x08,
    0x0203, 0x80,
    0x0224, 0x01,
    0x0225, 0xF4,
    0x0204, 0x00,
    0x0205, 0x00,
    0x0216, 0x00,
    0x0217, 0x00,
    0x020E, 0x01,
    0x020F, 0x00,
    0x0210, 0x01,
    0x0211, 0x00,
    0x0212, 0x01,
    0x0213, 0x00,
    0x0214, 0x01,
    0x0215, 0x00,
    0x0218, 0x01,
    0x0219, 0x00,
    0x3614, 0x00,
    0x3616, 0x0D,
    0x3617, 0x56,
    0xB612, 0x20,
    0xB613, 0x20,
    0xB614, 0x20,
    0xB615, 0x20,
    0xB616, 0x0A,
    0xB617, 0x0A,
    0xB618, 0x20,
    0xB619, 0x20,
    0xB61A, 0x20,
    0xB61B, 0x20,
    0xB61C, 0x0A,
    0xB61D, 0x0A,
    0xB666, 0x30,
    0xB667, 0x30,
    0xB668, 0x30,
    0xB669, 0x30,
    0xB66A, 0x14,
    0xB66B, 0x14,
    0xB66C, 0x20,
    0xB66D, 0x20,
    0xB66E, 0x20,
    0xB66F, 0x20,
    0xB670, 0x10,
    0xB671, 0x10,
    0x3237, 0x00,
    0x3900, 0x00,
    0x3901, 0x00,
    0x3902, 0x00,
    0x3904, 0x00,
    0x3905, 0x00,
    0x3906, 0x00,
    0x3907, 0x00,
    0x3908, 0x00,
    0x3909, 0x00,
    0x3912, 0x00,
    0x3930, 0x00,
    0x3931, 0x00,
    0x3933, 0x00,
    0x3934, 0x00,
    0x3935, 0x00,
    0x3936, 0x00,
    0x3937, 0x00,
    0x30AC, 0x00,
    0x0808, 0x02,
    0x080A, 0x00,
    0x080B, 0x67,
    0x080C, 0x00,
    0x080D, 0x2F,
    0x080E, 0x00,
    0x080F, 0x4F,
    0x0810, 0x00,
    0x0811, 0x2F,
    0x0812, 0x00,
    0x0813, 0x27,
    0x0814, 0x00,
    0x0815, 0x27,
    0x0816, 0x00,
    0x0817, 0xB7,
    0x0818, 0x00,
    0x0819, 0x27,
};

static kal_uint16 custom2_setting_array[] = {
     0x0100, 0x00,
     0x0112, 0x0a,
     0x0113, 0x0a,
     0x0114, 0x03,
     0x0342, 0x08,
     0x0343, 0x20,
     0x0340, 0x0d,
     0x0341, 0x24,
     0x0344, 0x00,
     0x0345, 0x00,
     0x0346, 0x00,
     0x0347, 0x08,
     0x0348, 0x0c,
     0x0349, 0xcf,
     0x034a, 0x09,
     0x034b, 0x97,
     0x0220, 0x00,
     0x0221, 0x11,
     0x0381, 0x01,
     0x0383, 0x01,
     0x0385, 0x01,
     0x0387, 0x01,
     0x0900, 0x01,
     0x0901, 0x22,
     0x0902, 0x08,
     0x3140, 0x02,
     0x3141, 0x00,
     0x3f0d, 0x0a,
     0x3f14, 0x01,
     0x3f3c, 0x02,
     0x3f4d, 0x01,
     0x3f4c, 0x01,
     0x4254, 0x7f,
     0x0401, 0x00,
     0x0404, 0x00,
     0x0405, 0x10,
     0x0408, 0x00,
     0x0409, 0x04,
     0x040a, 0x00,
     0x040b, 0x00,
     0x040c, 0x06,
     0x040d, 0x60,
     0x040e, 0x04,
     0x040f, 0xc8,
     0x034c, 0x06,
     0x034d, 0x60,
     0x034e, 0x04,
     0x034f, 0xc8,
     0x3261, 0x00,
     0x3264, 0x00,
     0x3265, 0x10,
     0x0301, 0x06,
     0x0303, 0x04,
     0x0305, 0x04,
     0x0306, 0x00,
     0x0307, 0xd2,
     0x0309, 0x0a,
     0x030b, 0x02,
     0x030d, 0x03,
     0x030e, 0x00,
     0x030f, 0x9d,
     0x0310, 0x01,
     0x0820, 0x09,
     0x0821, 0xd0,
     0x0822, 0x00,
     0x0823, 0x00,
     0x3e20, 0x01,
     0x3e37, 0x00,
     0x3e3b, 0x01,
     0x3234, 0x00,
     0x3fc1, 0x00,
     0x3235, 0x00,
     0x3802, 0x00,
     0x3143, 0x04,
     0x360a, 0x00,
     0x0b00, 0x00,
     0x0106, 0x00,
     0x0b05, 0x01,
     0x0b06, 0x01,
     0x3230, 0x00,
     0x3602, 0x01,
     0x3607, 0x01,
     0x3c00, 0x00,
     0x3c01, 0xba,
     0x3c02, 0xc8,
     0x3c03, 0xaa,
     0x3c04, 0x91,
     0x3c05, 0x54,
     0x3c06, 0x26,
     0x3c07, 0x20,
     0x3c08, 0x51,
     0x3d80, 0x00,
     0x3f50, 0x00,
     0x3f56, 0x00,
     0x3f57, 0x30,
     0x3f78, 0x00,
     0x3f79, 0x34,
     0x3f7c, 0x00,
     0x3f7d, 0x00,
     0x3fba, 0x00,
     0x3fbb, 0x00,
     0xa081, 0x04,
     0xe014, 0x00,
     0x0202, 0x0d,
     0x0203, 0x12,
     0x0224, 0x01,
     0x0225, 0xf4,
     0x0204, 0x00,
     0x0205, 0x00,
     0x0216, 0x00,
     0x0217, 0x00,
     0x020e, 0x01,
     0x020f, 0x00,
     0x0210, 0x01,
     0x0211, 0x00,
     0x0212, 0x01,
     0x0213, 0x00,
     0x0214, 0x01,
     0x0215, 0x00,
     0x0218, 0x01,
     0x0219, 0x00,
     0x3614, 0x00,
     0x3616, 0x0d,
     0x3617, 0x56,
     0xb612, 0x20,
     0xb613, 0x20,
     0xb614, 0x20,
     0xb615, 0x20,
     0xb616, 0x0a,
     0xb617, 0x0a,
     0xb618, 0x20,
     0xb619, 0x20,
     0xb61a, 0x20,
     0xb61b, 0x20,
     0xb61c, 0x0a,
     0xb61d, 0x0a,
     0xb666, 0x30,
     0xb667, 0x30,
     0xb668, 0x30,
     0xb669, 0x30,
     0xb66a, 0x14,
     0xb66b, 0x14,
     0xb66c, 0x20,
     0xb66d, 0x20,
     0xb66e, 0x20,
     0xb66f, 0x20,
     0xb670, 0x10,
     0xb671, 0x10,
     0x3237, 0x00,
     0x3900, 0x00,
     0x3901, 0x00,
     0x3902, 0x00,
     0x3904, 0x00,
     0x3905, 0x00,
     0x3906, 0x00,
     0x3907, 0x00,
     0x3908, 0x00,
     0x3909, 0x00,
     0x3912, 0x00,
     0x3930, 0x00,
     0x3931, 0x00,
     0x3933, 0x00,
     0x3934, 0x00,
     0x3935, 0x00,
     0x3936, 0x00,
     0x3937, 0x00,
     0x30ac, 0x00,
     //pdaf win mode
     0x38a3, 0x02,
     //fixed win setting
     0x38a4, 0x00,
     0x38a5, 0x00,
     0x38a6, 0x00,
     0x38a7, 0x00,
     0x38a8, 0x00,
     0x38a9, 0x00,
     0x38aa, 0x00,
     0x38ab, 0x00,
     //flexible setting
     0x38ac, 0x01,
     0x38ad, 0x00,
     0x38ae, 0x00,
     0x38af, 0x00,
     0x38b0, 0x00,
     0x38b1, 0x00,
     0x38b2, 0x00,
     0x38b3, 0x00,
     /*Area setting*/
     0x38b4, 0x04,
     0x38b5, 0x58,
     0x38b6, 0x03,
     0x38b7, 0xa2,
     0x38b8, 0x08,
     0x38b9, 0x68,
     0x38ba, 0x05,
     0x38bb, 0xee,
};

static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[] = {
{3280, 2464, 0, 8,   3280, 2448, 3280, 2448, 8,   0, 3264, 2448, 0, 0, 3264, 2448},//pre
{3280, 2464, 0, 8,   3280, 2448, 3280, 2448, 8,   0, 3264, 2448, 0, 0, 3264, 2448},//cap
{3280, 2464, 0, 312, 3280, 1840, 3280, 1840, 8,   0, 3264, 1840, 0, 0, 3264, 1840},//video
{3280, 2464, 0, 8,   3280, 2448, 1640, 1224, 4,   0, 1632, 1224, 0, 0, 1632, 1224},
{3280, 2464, 0, 312, 3280, 1840, 3280, 1840, 8,   0, 3264, 1840, 0, 0, 3264, 1840},
{3280, 2464, 0, 368, 3280, 1728, 3280, 1728, 488, 0, 2304, 1728, 0, 0, 2304, 1728},//custom1
{3280, 2464, 0, 8,   3280, 2448, 1640, 1224, 4,   0, 1632, 1224, 0, 0, 1632, 1224},
};
//customerconfig
static struct imgsensor_struct imgsensor = {
    .mirror = IMAGE_NORMAL,    /* mirrorflip information */
    .sensor_mode = IMGSENSOR_MODE_INIT,
    .shutter = 0x3D0,    /* current shutter */
    .gain = 0x100,        /* current gain */
    .dummy_pixel = 0,    /* current dummypixel */
    .dummy_line = 0,    /* current dummyline */
    .current_fps = 300,
    .autoflicker_en = KAL_FALSE,
    .test_pattern = 0,
    .current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
    .ihdr_mode = 0, /* sensor need support LE, SE with HDR feature */
    .i2c_write_id = 0x34, /* record current sensor's i2c write id */
    .current_ae_effective_frame = 2,
};

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte = 0;
    char pusendcmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF)};

    iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 2, imgsensor.i2c_write_id);
    return ((get_byte<<8)&0xff00) | ((get_byte>>8)&0x00ff);
}

static kal_uint16 read_cmos_sensor_8(kal_uint16 addr)
{
    kal_uint16 get_byte = 0;
    char pusendcmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

    iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 1, imgsensor.i2c_write_id);
    return get_byte;
}

static void write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para)
{
    char pusendcmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF),
            (char)(para & 0xFF)};

    iWriteRegI2C(pusendcmd, 3, imgsensor.i2c_write_id);
}


static void write_cmos_sensor(kal_uint16 addr, kal_uint8 para)
{
    char pusendcmd[4] = {
    (char)(addr >> 8), (char)(addr & 0xFF), (char)(para >> 8), (char)(para & 0xFF) };
    iWriteRegI2C(pusendcmd, 4, imgsensor.i2c_write_id);

}

static void imx319_get_pdaf_reg_setting(MUINT32 regNum, kal_uint16 *regDa)
{
    int i, idx;

    for (i = 0; i < regNum; i++) {
        idx = 2 * i;
        regDa[idx + 1] = read_cmos_sensor_8(regDa[idx]);
        //LOG_INF("%x %x", regDa[idx], regDa[idx+1]);
    }
}
static void imx319_set_pdaf_reg_setting(MUINT32 regNum, kal_uint16 *regDa)
{
    int i, idx;

    for (i = 0; i < regNum; i++) {
        idx = 2 * i;
        write_cmos_sensor_8(regDa[idx], regDa[idx + 1]);
        //LOG_INF("%x %x", regDa[idx], regDa[idx+1]);
    }
}

static void read_module_data(void)
{
    int i = 0;
    /*Read normal eeprom data*/
    gImgEepromInfo.camNormdata[2][0] = Eeprom_1ByteDataRead(0x00, 0xA2);
    gImgEepromInfo.camNormdata[2][1] = Eeprom_1ByteDataRead(0x01, 0xA2);
    imgsensor_info.module_id = Eeprom_1ByteDataRead(0x00, 0xA2);
    Oplusimgsensor_Registdeviceinfo(gImgEepromInfo.pCamModuleInfo[2].name,
                                    gImgEepromInfo.pCamModuleInfo[2].version,
                                    imgsensor_info.module_id);
    for(i = 2; i < 8; i++) {
       gImgEepromInfo.camNormdata[2][i] = Eeprom_1ByteDataRead(0x04+i, 0xA2);
    }

    for (i = 0; i < OPLUS_CAMERASN_LENS; i ++) {
       gImgEepromInfo.camNormdata[2][8+i] = Eeprom_1ByteDataRead(0xB0+i, 0xA2);
    }
    gImgEepromInfo.camNormdata[2][39] = 2;
    /*Read stereo eeprom data*/
    for (i = 0; i < CALI_DATA_SLAVE_LENGTH; i ++) {
        gImgEepromInfo.stereoMWdata[CALI_DATA_MASTER_LENGTH+i] =
                    Eeprom_1ByteDataRead(IMX319_STEREO_START_ADDR+i, 0xA2);
    }
    gImgEepromInfo.i4CurSensorIdx = 2;
    gImgEepromInfo.i4CurSensorId = imgsensor_info.sensor_id;
}

static kal_uint16 table_write_cmos_sensor(kal_uint16 *para, kal_uint32 len)
{
    char puSendCmd[I2C_BUFFER_LEN];
    kal_uint32 tosend, IDX;
    kal_uint16 addr = 0, addr_last = 0, data;
    tosend = 0;
    IDX = 0;

    while (len > IDX) {
        addr = para[IDX];
        {
            puSendCmd[tosend++] = (char)(addr >> 8);
            puSendCmd[tosend++] = (char)(addr & 0xFF);
            data = para[IDX + 1];
            puSendCmd[tosend++] = (char)(data & 0xFF);
            IDX += 2;
            addr_last = addr;
        }

        /* Write when remain buffer size is less than 3 bytes
         * or reach end of data
         */
        if ((I2C_BUFFER_LEN - tosend) < 3 || IDX == len || addr != addr_last) {
            iBurstWriteReg_multi(puSendCmd,
                        tosend,
                        imgsensor.i2c_write_id,
                        3,
                        imgsensor_info.i2c_speed);
            tosend = 0;
        }
    }

    return 0;
}

static void set_mirror_flip(kal_uint8 image_mirror)
{
    kal_uint8 itemp;

    LOG_INF("image_mirror = %d\n", image_mirror);
    itemp = read_cmos_sensor_8(0x0101);
    itemp &= ~0x03;

    switch (image_mirror) {

    case IMAGE_NORMAL:
    write_cmos_sensor_8(0x0101, itemp);
    break;

    case IMAGE_V_MIRROR:
    write_cmos_sensor_8(0x0101, itemp | 0x02);
    break;

    case IMAGE_H_MIRROR:
    write_cmos_sensor_8(0x0101, itemp | 0x01);
    break;

    case IMAGE_HV_MIRROR:
    write_cmos_sensor_8(0x0101, itemp | 0x03);
    break;
    }
}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
     kal_uint16 reg_gain = 0x0;

    reg_gain = 1024 - (1024*64)/gain;
    return (kal_uint16) reg_gain;
}



static kal_uint32 set_test_pattern_mode(kal_uint8 modes, struct SET_SENSOR_PATTERN_SOLID_COLOR *pTestpatterndata)
{
    pr_debug("set_test_pattern enum: %d\n", modes);

    if (modes) {
        write_cmos_sensor_8(0x0600, modes>>4);
        write_cmos_sensor_8(0x0601, modes);
        if (modes == 1 && (pTestpatterndata != NULL)) { //Solid Color
            write_cmos_sensor_8(0x0600, 0x0000);
            write_cmos_sensor_8(0x0601, 0x01);
        }
    } else {
        write_cmos_sensor(0x0600, 0x0000); /*No pattern*/
        write_cmos_sensor(0x0601, 0x0000);
    }

    spin_lock(&imgsensor_drv_lock);
    imgsensor.test_pattern = modes;
    spin_unlock(&imgsensor_drv_lock);
    return ERROR_NONE;
}


static kal_int32 get_sensor_temperature(void)
{
    UINT8 temperature = 0;
    INT32 temperature_convert = 0;

    temperature = read_cmos_sensor_8(0x013a);

    if (temperature >= 0x0 && temperature <= 0x60)
        temperature_convert = temperature;
    else if (temperature >= 0x61 && temperature <= 0x7F)
        temperature_convert = 97;
    else if (temperature >= 0x80 && temperature <= 0xE2)
        temperature_convert = -30;
    else
        temperature_convert = (INT8)temperature | 0xFFFFFF0;

/* LOG_INF("temp_c(%d), read_reg(%d)\n", temperature_convert, temperature); */

    return temperature_convert;
}

static void set_dummy(void)
{
    LOG_INF("dummyline = %d, dummypixels = %d\n",    imgsensor.dummy_line, imgsensor.dummy_pixel);
    write_cmos_sensor_8(0x0104, 0x01);
    write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
    write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
    write_cmos_sensor_8(0x0342, imgsensor.line_length >> 8);
    write_cmos_sensor_8(0x0343, imgsensor.line_length & 0xFF);
    write_cmos_sensor_8(0x0104, 0x00);

}    /*    set_dummy  */
static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
    /*kal_int16 dummy_line;*/
    kal_uint32 frame_length = imgsensor.frame_length;

    LOG_INF("framerate = %d, min framelength should enable %d\n", framerate,
        min_framelength_en);

    frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
    spin_lock(&imgsensor_drv_lock);
    if (frame_length >= imgsensor.min_frame_length)
        imgsensor.frame_length = frame_length;
    else
        imgsensor.frame_length = imgsensor.min_frame_length;

    imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;

    if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
        imgsensor.frame_length = imgsensor_info.max_frame_length;
        imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
    }
    if (min_framelength_en)
        imgsensor.min_frame_length = imgsensor.frame_length;
    spin_unlock(&imgsensor_drv_lock);
    set_dummy();
}    /*    set_max_framerate  */




#define MAX_CIT_LSHIFT 7
static void write_shutter(kal_uint32 shutter)
{
    kal_uint16 realtime_fps = 0;
    kal_uint16 l_shift = 1;

    spin_lock(&imgsensor_drv_lock);
    if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
        imgsensor.frame_length = shutter + imgsensor_info.margin;
    else
        imgsensor.frame_length = imgsensor.min_frame_length;
    if (imgsensor.frame_length > imgsensor_info.max_frame_length)
        imgsensor.frame_length = imgsensor_info.max_frame_length;
    spin_unlock(&imgsensor_drv_lock);
    if (shutter < imgsensor_info.min_shutter)
        shutter = imgsensor_info.min_shutter;

    if (imgsensor.autoflicker_en) {
        realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
        if (realtime_fps >= 297 && realtime_fps <= 305)
            set_max_framerate(296, 0);
        else if (realtime_fps >= 147 && realtime_fps <= 150)
            set_max_framerate(146, 0);
        else {
        /* Extend frame length*/
        write_cmos_sensor_8(0x0104, 0x01);
        write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
        write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
        write_cmos_sensor_8(0x0104, 0x00);
        }
    } else {
        /* Extend frame length*/
        write_cmos_sensor_8(0x0104, 0x01);
        write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
        write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
        write_cmos_sensor_8(0x0104, 0x00);

        LOG_INF("(else)imgsensor.frame_length = %d\n",
            imgsensor.frame_length);
    }

    /* long expsoure */
    if (shutter >    (imgsensor_info.max_frame_length - imgsensor_info.margin)) {

        for (l_shift = 1; l_shift < MAX_CIT_LSHIFT; l_shift++) {
            if ((shutter >> l_shift)  < (imgsensor_info.max_frame_length - imgsensor_info.margin))

                break;
        }
        if (l_shift > MAX_CIT_LSHIFT) {
            LOG_INF("Unable to set such a long exposure %d, set to max\n",   shutter);

            l_shift = MAX_CIT_LSHIFT;
        }
        shutter = shutter >> l_shift;
        imgsensor.frame_length = shutter + imgsensor_info.margin;

        LOG_INF("enter long exposure mode, time is %d", l_shift);

        write_cmos_sensor_8(0x0104, 0x01);
        write_cmos_sensor_8(0x3100,
                read_cmos_sensor(0x3100) | (l_shift & 0x7));
        write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
        write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
        write_cmos_sensor_8(0x0104, 0x00);

        /* Frame exposure mode customization for LE*/
        imgsensor.ae_frm_mode.frame_mode_1 = IMGSENSOR_AE_MODE_SE;
        imgsensor.ae_frm_mode.frame_mode_2 = IMGSENSOR_AE_MODE_SE;
        imgsensor.current_ae_effective_frame = 2;
    } else {
        write_cmos_sensor_8(0x0104, 0x01);
        write_cmos_sensor_8(0x3100, read_cmos_sensor(0x3100) & 0xf8);
        write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
        write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
        write_cmos_sensor_8(0x0104, 0x00);
        imgsensor.current_ae_effective_frame = 2;
        LOG_INF("exit long exposure mode");
    }

    /* Update Shutter */
    write_cmos_sensor_8(0x0104, 0x01);
    write_cmos_sensor_8(0x0202, (shutter >> 8) & 0xFF);
    write_cmos_sensor_8(0x0203, shutter  & 0xFF);
    write_cmos_sensor_8(0x0104, 0x00);

    LOG_INF("shutter =%d, framelength =%d\n", shutter, imgsensor.frame_length);

}    /*    write_shutter  */

/*************************************************************************
 * FUNCTION
 *    set_shutter
 *
 * DESCRIPTION
 *    This function set e-shutter of sensor to change exposure time.
 *
 * PARAMETERS
 *    iShutter : exposured lines
 *
 * RETURNS
 *    None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static void set_shutter(kal_uint32 shutter)
{
    unsigned long flags;

    spin_lock_irqsave(&imgsensor_drv_lock, flags);
    imgsensor.shutter = shutter;
    spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

    write_shutter(shutter);
} /* set_shutter */


/*************************************************************************
 * FUNCTION
 *    set_shutter_frame_length
 *
 * DESCRIPTION
 *    for frame & 3A sync
 *
 *************************************************************************/
static void set_shutter_frame_length(kal_uint32 shutter,
                                    kal_uint16 frame_length,
                                    kal_bool auto_extend_en)
{
    unsigned long flags;
    kal_uint16 realtime_fps = 0;
    kal_int32 dummy_line = 0;

    spin_lock_irqsave(&imgsensor_drv_lock, flags);
    imgsensor.shutter = shutter;
    spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

    spin_lock(&imgsensor_drv_lock);
    /* Change frame time */
    if (frame_length > 1)
        imgsensor.frame_length = frame_length;

    imgsensor.frame_length = imgsensor.frame_length + dummy_line;

    if (imgsensor.frame_length > imgsensor_info.max_frame_length)
        imgsensor.frame_length = imgsensor_info.max_frame_length;
    spin_unlock(&imgsensor_drv_lock);
    shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;

    if (imgsensor.autoflicker_en) {
        realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
        if (realtime_fps >= 297 && realtime_fps <= 305)
            set_max_framerate(296, 0);
        else if (realtime_fps >= 147 && realtime_fps <= 150)
            set_max_framerate(146, 0);
        else {
            /* Extend frame length */
            write_cmos_sensor_8(0x0104, 0x01);
            write_cmos_sensor_8(0x0340,
                    imgsensor.frame_length >> 8);
            write_cmos_sensor_8(0x0341,
                    imgsensor.frame_length & 0xFF);
            write_cmos_sensor_8(0x0104, 0x00);
        }
    } else {
        /* Extend frame length */
        write_cmos_sensor_8(0x0104, 0x01);
        write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
        write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
        write_cmos_sensor_8(0x0104, 0x00);
    }

    /* Update Shutter */
    write_cmos_sensor_8(0x0104, 0x01);
    if (auto_extend_en) {
        write_cmos_sensor_8(0x0350, 0x01); /* Enable auto extend */
    } else {
        write_cmos_sensor_8(0x0350, 0x00); /* Disable auto extend */
    }

    write_cmos_sensor_8(0x0202, (shutter >> 8) & 0xFF);
    write_cmos_sensor_8(0x0203, shutter  & 0xFF);
    write_cmos_sensor_8(0x0104, 0x00);
    LOG_INF(
        "Exit! shutter =%d, framelength =%d/%d, dummy_line=%d\n",
        shutter, imgsensor.frame_length, frame_length, dummy_line);

}    /* set_shutter_frame_length */

/*************************************************************************
 * FUNCTION
 *    set_gain
 *
 * DESCRIPTION
 *    This function is to set global gain to sensor.
 *
 * PARAMETERS
 *    iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *    the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
    kal_uint16 reg_gain;

    if (gain < imgsensor_info.min_gain || gain > imgsensor_info.max_gain) {
        LOG_INF("Error gain setting");

        if (gain < imgsensor_info.min_gain)
            gain = imgsensor_info.min_gain;
        else
            gain = imgsensor_info.max_gain;
    }

    reg_gain = gain2reg(gain);
    spin_lock(&imgsensor_drv_lock);
    imgsensor.gain = reg_gain;
    spin_unlock(&imgsensor_drv_lock);
    LOG_INF("gain = %d, reg_gain = 0x%x\n ", gain, reg_gain);

    write_cmos_sensor_8(0x0104, 0x01);
    write_cmos_sensor_8(0x0204, (reg_gain>>8) & 0xFF);
    write_cmos_sensor_8(0x0205, reg_gain & 0xFF);
    write_cmos_sensor_8(0x0104, 0x00);

    return gain;
} /* set_gain */

static BYTE imx319_SPC_data[SPC_DATA_SIZE] = { 0 };
static BYTE IMX319_DCC_data[DCC_DATA_SIZE] = { 0 };

static bool _read_imx319_eeprom(kal_uint16 addr, BYTE *data, int size)
{
    int i = 0;
    int offset = addr;
    int ret;
    u8 pu_send_cmd[2];

    #define MAX_READ_WRITE_SIZE 255
    for (i = 0; i < size; i += MAX_READ_WRITE_SIZE) {
        pu_send_cmd[0] = (u8) (offset >> 8);
        pu_send_cmd[1] = (u8) (offset & 0xFF);

        if (i + MAX_READ_WRITE_SIZE > size) {
            ret = iReadRegI2C(pu_send_cmd, 2,
                     (u8 *) (data + i),
                     (size - i),
                     IMX319_EEPROM_READ_ID);
        } else {
            ret = iReadRegI2C(pu_send_cmd, 2,
                     (u8 *) (data + i),
                     MAX_READ_WRITE_SIZE,
                     IMX319_EEPROM_READ_ID);
        }
        if (ret < 0) {
            pr_debug("read spc failed!\n");
            return false;
        }

        offset += MAX_READ_WRITE_SIZE;
    }

    if (addr == SPC_START_ADDR) {
        get_done_spc = true;
        last_size_spc = size;
    } else {
        get_done_dcc = true;
        last_size_dcc = size;
    }
    pr_debug("exit _read_eeprom size = %d\n", size);
    return true;
}

void read_imx319_SPC(BYTE *data)
{

    int addr = SPC_START_ADDR;
    int size = SPC_DATA_SIZE;

    pr_debug("read imx319 SPC, size = %d\n", size);
    if (!get_done_spc || last_size_spc != size) {
        if (!_read_imx319_eeprom(addr, data, size)) {
            get_done_spc = 0;
            last_size_spc = 0;
        }
    }
    /* return true; */
}

void read_imx319_DCC(kal_uint16 addr, BYTE *data, kal_uint32 size)
{
    /* int i; */
    addr = DCC_START_ADDR;
    size = DCC_DATA_SIZE;

    pr_debug("read imx319 DCC, size = %d\n", size);

    if (!get_done_dcc || last_size_dcc != size) {
        if (!_read_imx319_eeprom(addr, IMX319_DCC_data, size)) {
            get_done_dcc = 0;
            last_size_dcc = 0;
        }
    }

    memcpy(data, IMX319_DCC_data, size);
    /* return true; */
}

static void imx319_apply_SPC(void)
{
    unsigned int start_reg = 0x7500;
    char puSendCmd[SPC_DATA_SIZE + 2];
    kal_uint32 tosend = 0;

    read_imx319_SPC(imx319_SPC_data);

    LOG_INF("+");
    LOG_INF("spc_data(%x, %x, %x, %x)", imx319_SPC_data[0],  imx319_SPC_data[1], imx319_SPC_data[2], imx319_SPC_data[3]);

    puSendCmd[tosend++] = (char)(start_reg >> 8);
    puSendCmd[tosend++] = (char)(start_reg & 0xFF);
    memcpy((void *)&puSendCmd[tosend], imx319_SPC_data, SPC_DATA_SIZE);
    tosend += SPC_DATA_SIZE;
    iBurstWriteReg_multi(
        puSendCmd,
        tosend,
        imgsensor.i2c_write_id,
        tosend,
        imgsensor_info.i2c_speed);

#if 0
    /* for verify */
    for (i = 0x7c00; i < 0x7c00+352; i++)
        pr_info(
        "SPC read out : Addr[%3d] Data[%x], Ref[%x]",
        i, read_cmos_sensor(i), imx230_SPC_data[i-0x7c00]);
#endif
}

/*************************************************************************
 * FUNCTION
 *    night_mode
 *
 * DESCRIPTION
 *    This function night mode of sensor.
 *
 * PARAMETERS
 *    bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *    None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 streaming_control(kal_bool enable)
{
    LOG_INF("streaming_enable(0=Sw Standby,1=streaming): %d\n", enable);
    if (enable)
        write_cmos_sensor_8(0x0100, 0X01);
    else
        write_cmos_sensor_8(0x0100, 0x00);
    return ERROR_NONE;
}



static void sensor_init(void)
{
    LOG_INF("E \n");
    table_write_cmos_sensor(sensor_init_setting_array,
                    sizeof(sensor_init_setting_array)/sizeof(kal_uint16));

}

static void preview_setting(void)
{
    LOG_INF("E\n");
    table_write_cmos_sensor(preview_setting_array,
                    sizeof(preview_setting_array)/sizeof(kal_uint16));
    if (imgsensor.pdaf_mode == 1)
        imx319_apply_SPC();
    LOG_INF("X");
} /* preview_setting */

static void capture_setting(kal_uint16 currefps)
{
    LOG_INF(" E! currefps:%d\n",  currefps);
    table_write_cmos_sensor(capture_setting_array,
                    sizeof(capture_setting_array)/sizeof(kal_uint16));
    if (imgsensor.pdaf_mode == 1)
        imx319_apply_SPC();
}


static void normal_video_setting(kal_uint16 currefps)
{
    LOG_INF(" E! currefps:%d\n",  currefps);
    table_write_cmos_sensor(normal_video_setting_array,
                    sizeof(normal_video_setting_array)/sizeof(kal_uint16));
    if (imgsensor.pdaf_mode == 1)
        imx319_apply_SPC();
}


static void hs_video_setting(void)
{
    LOG_INF("E\n");
    table_write_cmos_sensor(hs_video_setting_array,
                    sizeof(hs_video_setting_array)/sizeof(kal_uint16));
}


static void slim_video_setting(void)
{
    LOG_INF("E\n");
    table_write_cmos_sensor(slim_video_setting_array,
                    sizeof(slim_video_setting_array)/sizeof(kal_uint16));
}

static void custom1_setting(void)
{
    LOG_INF("E\n");
    table_write_cmos_sensor(custom1_setting_array,
                    sizeof(custom1_setting_array)/sizeof(kal_uint16));
    if (imgsensor.pdaf_mode == 1)
        imx319_apply_SPC();
}


static void custom2_setting(void)
{
    LOG_INF("E\n");
    table_write_cmos_sensor(custom2_setting_array,
                    sizeof(custom2_setting_array)/sizeof(kal_uint16));
}

/*************************************************************************
 * FUNCTION
 *    get_imgsensor_id
 *
 * DESCRIPTION
 *    This function get the sensor ID
 *
 * PARAMETERS
 *    *sensorID : return the sensor ID
 *
 * RETURNS
 *    None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
    kal_uint8 i = 0;
    kal_uint8 retry = 2;

    while (imgsensor_info.i2c_addr_table[i] != 0xff) {
        spin_lock(&imgsensor_drv_lock);
        imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
        spin_unlock(&imgsensor_drv_lock);
        do {
            *sensor_id = ((read_cmos_sensor_8(0x0016) << 8)
                    | read_cmos_sensor_8(0x0017));
            if (*sensor_id == imgsensor_info.sensor_id) {
                read_module_data();
                LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id, *sensor_id);
                return ERROR_NONE;
            }
            LOG_INF("Read sensor id fail, id: 0x%x\n", imgsensor.i2c_write_id);
            retry--;
        } while (retry > 0);
        i++;
        retry = 2;
    }
    if (*sensor_id != imgsensor_info.sensor_id) {
        /*if Sensor ID is not correct,
         *Must set *sensor_id to 0xFFFFFFFF
         */
        *sensor_id = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;
}

/*************************************************************************
 * FUNCTION
 *    open
 *
 * DESCRIPTION
 *    This function initialize the registers of CMOS sensor
 *
 * PARAMETERS
 *    None
 *
 * RETURNS
 *    None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 open(void)
{
    kal_uint8 i = 0;
    kal_uint8 retry = 2;
    kal_uint32 sensor_id = 0;

    /*sensor have two i2c address 0x6c 0x6d & 0x21 0x20,
     *we should detect the module used i2c address
     */
    while (imgsensor_info.i2c_addr_table[i] != 0xff) {
        spin_lock(&imgsensor_drv_lock);
        imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
        spin_unlock(&imgsensor_drv_lock);
        do {
            sensor_id = ((read_cmos_sensor_8(0x0016) << 8)
                    | read_cmos_sensor_8(0x0017));
            if (sensor_id == imgsensor_info.sensor_id) {
                LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n",
                    imgsensor.i2c_write_id, sensor_id);
                break;
            }
            LOG_INF("Read sensor id fail, id: 0x%x\n",
                imgsensor.i2c_write_id);
            retry--;
        } while (retry > 0);
        i++;
        if (sensor_id == imgsensor_info.sensor_id)
            break;
        retry = 2;
    }
    if (imgsensor_info.sensor_id != sensor_id) {
        return ERROR_SENSORID_READ_FAIL;
    }
    /* initail sequence write in  */
    sensor_init();

    spin_lock(&imgsensor_drv_lock);

    imgsensor.autoflicker_en = KAL_FALSE;
    imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
    imgsensor.shutter = 0x3D0;
    imgsensor.gain = 0x100;
    imgsensor.pclk = imgsensor_info.pre.pclk;
    imgsensor.frame_length = imgsensor_info.pre.framelength;
    imgsensor.line_length = imgsensor_info.pre.linelength;
    imgsensor.min_frame_length = imgsensor_info.pre.framelength;
    imgsensor.dummy_pixel = 0;
    imgsensor.dummy_line = 0;
    imgsensor.ihdr_mode = 0;
    imgsensor.test_pattern = 0;
    imgsensor.current_fps = imgsensor_info.pre.max_framerate;
    spin_unlock(&imgsensor_drv_lock);

    return ERROR_NONE;
} /* open */



/*************************************************************************
 * FUNCTION
 *    close
 *
 * DESCRIPTION
 *
 *
 * PARAMETERS
 *    None
 *
 * RETURNS
 *    None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 close(void)
{
    LOG_INF("E\n");

    /*No Need to implement this function*/
    streaming_control(0);
    return ERROR_NONE;
} /* close */


/*************************************************************************
 * FUNCTION
 * preview
 *
 * DESCRIPTION
 *    This function start the sensor preview.
 *
 * PARAMETERS
 *    *image_window : address pointer of pixel numbers in one period of HSYNC
 *  *sensor_config_data : address pointer of line numbers in one period of VSYNC
 *
 * RETURNS
 *    None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
              MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("%s E\n", __func__);

    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
    imgsensor.pclk = imgsensor_info.pre.pclk;
    imgsensor.line_length = imgsensor_info.pre.linelength;
    imgsensor.frame_length = imgsensor_info.pre.framelength;
    imgsensor.min_frame_length = imgsensor_info.pre.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);

    preview_setting();
    set_mirror_flip(imgsensor.mirror);

    return ERROR_NONE;
} /* preview */

/*************************************************************************
 * FUNCTION
 *    capture
 *
 * DESCRIPTION
 *    This function setup the CMOS sensor in capture MY_OUTPUT mode
 *
 * PARAMETERS
 *
 * RETURNS
 *    None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
              MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;


    if (imgsensor.current_fps != imgsensor_info.cap.max_framerate) {
        LOG_INF(
            "Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n",
            imgsensor.current_fps,
            imgsensor_info.cap.max_framerate / 10);
        imgsensor.pclk = imgsensor_info.cap.pclk;
        imgsensor.line_length = imgsensor_info.cap.linelength;
        imgsensor.frame_length = imgsensor_info.cap.framelength;
        imgsensor.min_frame_length = imgsensor_info.cap.framelength;
        imgsensor.autoflicker_en = KAL_FALSE;

    } else {        /* PIP capture: 24fps for less than 13M, 20fps for 16M,15fps for 20M */
        LOG_INF("Warning:=== current_fps %d fps is not support, so use cap1's setting\n",
            imgsensor.current_fps / 10);
        imgsensor.pclk = imgsensor_info.cap.pclk;
        imgsensor.line_length = imgsensor_info.cap.linelength;
        imgsensor.frame_length = imgsensor_info.cap.framelength;
        imgsensor.min_frame_length = imgsensor_info.cap.framelength;
        imgsensor.autoflicker_en = KAL_FALSE;
    }
    spin_unlock(&imgsensor_drv_lock);

    capture_setting(imgsensor.current_fps);
    set_mirror_flip(imgsensor.mirror);

    return ERROR_NONE;
}    /* capture() */
static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");

    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
    imgsensor.pclk = imgsensor_info.normal_video.pclk;
    imgsensor.line_length = imgsensor_info.normal_video.linelength;
    imgsensor.frame_length = imgsensor_info.normal_video.framelength;
    imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    normal_video_setting(imgsensor.current_fps);
    set_mirror_flip(imgsensor.mirror);

    return ERROR_NONE;
}    /*    normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");

    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
    imgsensor.pclk = imgsensor_info.hs_video.pclk;
    /*imgsensor.video_mode = KAL_TRUE;*/
    imgsensor.line_length = imgsensor_info.hs_video.linelength;
    imgsensor.frame_length = imgsensor_info.hs_video.framelength;
    imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
    imgsensor.dummy_line = 0;
    imgsensor.dummy_pixel = 0;
    /*imgsensor.current_fps = 300;*/
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    hs_video_setting();
    set_mirror_flip(imgsensor.mirror);

    return ERROR_NONE;
}    /*    hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");

    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
    imgsensor.pclk = imgsensor_info.slim_video.pclk;
    /*imgsensor.video_mode = KAL_TRUE;*/
    imgsensor.line_length = imgsensor_info.slim_video.linelength;
    imgsensor.frame_length = imgsensor_info.slim_video.framelength;
    imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
    imgsensor.dummy_line = 0;
    imgsensor.dummy_pixel = 0;
    /*imgsensor.current_fps = 300;*/
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    slim_video_setting();
    set_mirror_flip(imgsensor.mirror);

    return ERROR_NONE;
}    /* slim_video */


static kal_uint32 custom1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
              MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("%s.\n", __func__);

    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
    imgsensor.pclk = imgsensor_info.custom1.pclk;
    imgsensor.line_length = imgsensor_info.custom1.linelength;
    imgsensor.frame_length = imgsensor_info.custom1.framelength;
    imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    custom1_setting();
    set_mirror_flip(imgsensor.mirror);
    return ERROR_NONE;
}    /* custom1 */

static kal_uint32 custom2(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
              MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("%s.\n", __func__);

    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM2;
    imgsensor.pclk = imgsensor_info.custom2.pclk;
    imgsensor.line_length = imgsensor_info.custom2.linelength;
    imgsensor.frame_length = imgsensor_info.custom2.framelength;
    imgsensor.min_frame_length = imgsensor_info.custom2.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    custom2_setting();
    set_mirror_flip(imgsensor.mirror);
    return ERROR_NONE;
}    /* custom2 */

static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
    LOG_INF("E\n");
    sensor_resolution->SensorFullWidth =
        imgsensor_info.cap.grabwindow_width;
    sensor_resolution->SensorFullHeight =
        imgsensor_info.cap.grabwindow_height;

    sensor_resolution->SensorPreviewWidth =
        imgsensor_info.pre.grabwindow_width;
    sensor_resolution->SensorPreviewHeight =
        imgsensor_info.pre.grabwindow_height;

    sensor_resolution->SensorVideoWidth =
        imgsensor_info.normal_video.grabwindow_width;
    sensor_resolution->SensorVideoHeight =
        imgsensor_info.normal_video.grabwindow_height;

    sensor_resolution->SensorHighSpeedVideoWidth =
        imgsensor_info.hs_video.grabwindow_width;
    sensor_resolution->SensorHighSpeedVideoHeight =
        imgsensor_info.hs_video.grabwindow_height;

    sensor_resolution->SensorSlimVideoWidth =
        imgsensor_info.slim_video.grabwindow_width;
    sensor_resolution->SensorSlimVideoHeight =
        imgsensor_info.slim_video.grabwindow_height;

    sensor_resolution->SensorCustom1Width =
        imgsensor_info.custom1.grabwindow_width;
    sensor_resolution->SensorCustom1Height =
        imgsensor_info.custom1.grabwindow_height;

    sensor_resolution->SensorCustom2Width =
        imgsensor_info.custom2.grabwindow_width;
    sensor_resolution->SensorCustom2Height =
        imgsensor_info.custom2.grabwindow_height;

    return ERROR_NONE;
} /* get_resolution */

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
               MSDK_SENSOR_INFO_STRUCT *sensor_info,
               MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

    sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
    sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
    sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    sensor_info->SensorInterruptDelayLines = 4; /* not use */
    sensor_info->SensorResetActiveHigh = FALSE; /* not use */
    sensor_info->SensorResetDelayCount = 5; /* not use */

    sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
    sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
    sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
    sensor_info->SensorOutputDataFormat =
        imgsensor_info.sensor_output_dataformat;

    sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
    sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
    sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
    sensor_info->HighSpeedVideoDelayFrame =
        imgsensor_info.hs_video_delay_frame;
    sensor_info->SlimVideoDelayFrame =
        imgsensor_info.slim_video_delay_frame;
    sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
    sensor_info->Custom2DelayFrame = imgsensor_info.custom2_delay_frame;

    sensor_info->SensorMasterClockSwitch = 0; /* not use */
    sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

    sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;
    sensor_info->AESensorGainDelayFrame =
        imgsensor_info.ae_sensor_gain_delay_frame;
    sensor_info->AEISPGainDelayFrame =
        imgsensor_info.ae_ispGain_delay_frame;
    sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
    sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
    sensor_info->TEMPERATURE_SUPPORT = imgsensor_info.temperature_support;
    sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
    sensor_info->PDAF_Support = 0;
    sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
    sensor_info->SensorClockFreq = imgsensor_info.mclk;
    sensor_info->SensorClockDividCount = 3; /* not use */
    sensor_info->SensorClockRisingCount = 0;
    sensor_info->SensorClockFallingCount = 2; /* not use */
    sensor_info->SensorPixelClockCount = 3; /* not use */
    sensor_info->SensorDataLatchCount = 2; /* not use */

    sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
    sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
    sensor_info->SensorWidthSampling = 0; /* 0 is default 1x */
    sensor_info->SensorHightSampling = 0; /* 0 is default 1x */
    sensor_info->SensorPacketECCOrder = 1;

    switch (scenario_id) {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
        sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

        sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
            imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
        sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;

        sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
            imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

        break;
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

        sensor_info->SensorGrabStartX =
            imgsensor_info.normal_video.startx;
        sensor_info->SensorGrabStartY =
            imgsensor_info.normal_video.starty;

        sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
            imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

        break;
    case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
        sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
        sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;

        sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
            imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;

        break;
    case MSDK_SCENARIO_ID_SLIM_VIDEO:
        sensor_info->SensorGrabStartX =
            imgsensor_info.slim_video.startx;
        sensor_info->SensorGrabStartY =
            imgsensor_info.slim_video.starty;

        sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
            imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;

        break;

    case MSDK_SCENARIO_ID_CUSTOM1:
        sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx;
        sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;

        sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
            imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;
        break;

    case MSDK_SCENARIO_ID_CUSTOM2:
        sensor_info->SensorGrabStartX = imgsensor_info.custom2.startx;
        sensor_info->SensorGrabStartY = imgsensor_info.custom2.starty;

        sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
            imgsensor_info.custom2.mipi_data_lp2hs_settle_dc;
        break;

    default:
        sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
        sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

        sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
            imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
        break;
    }

    return ERROR_NONE;
}    /*    get_info  */


static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
            MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
            MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("scenario_id = %d\n", scenario_id);
    spin_lock(&imgsensor_drv_lock);
    imgsensor.current_scenario_id = scenario_id;
    spin_unlock(&imgsensor_drv_lock);
    switch (scenario_id) {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        preview(image_window, sensor_config_data);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        capture(image_window, sensor_config_data);
        break;
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        normal_video(image_window, sensor_config_data);
        break;
    case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
        hs_video(image_window, sensor_config_data);
        break;
    case MSDK_SCENARIO_ID_SLIM_VIDEO:
        slim_video(image_window, sensor_config_data);
        break;
    case MSDK_SCENARIO_ID_CUSTOM1:
        custom1(image_window, sensor_config_data);
        break;
    case MSDK_SCENARIO_ID_CUSTOM2:
        custom2(image_window, sensor_config_data);
        break;
    default:
        LOG_INF("Error ScenarioId setting");
        preview(image_window, sensor_config_data);
        return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
}    /* control() */


static kal_uint32 set_video_mode(UINT16 framerate)
{
    LOG_INF("framerate = %d\n ", framerate);
    /* SetVideoMode Function should fix framerate */
    if (framerate == 0)
        return ERROR_NONE;
    spin_lock(&imgsensor_drv_lock);
    if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
        imgsensor.current_fps = 296;
    else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
        imgsensor.current_fps = 146;
    else
        imgsensor.current_fps = framerate;
    spin_unlock(&imgsensor_drv_lock);
    set_max_framerate(imgsensor.current_fps, 1);

    return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
    LOG_INF("enable = %d, framerate = %d\n", enable, framerate);
    spin_lock(&imgsensor_drv_lock);
    if (enable) /*enable auto flicker*/
        imgsensor.autoflicker_en = KAL_TRUE;
    else /*Cancel Auto flick*/
        imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(
        enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate)
{
    kal_uint32 frame_length;

    LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

    switch (scenario_id) {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        frame_length = imgsensor_info.pre.pclk / framerate * 10
                / imgsensor_info.pre.linelength;
        spin_lock(&imgsensor_drv_lock);
        imgsensor.dummy_line =
            (frame_length > imgsensor_info.pre.framelength)
        ? (frame_length - imgsensor_info.pre.framelength) : 0;
        imgsensor.frame_length =
            imgsensor_info.pre.framelength
            + imgsensor.dummy_line;
        imgsensor.min_frame_length = imgsensor.frame_length;
        spin_unlock(&imgsensor_drv_lock);
        if (imgsensor.frame_length > imgsensor.shutter)
            set_dummy();
        break;
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        if (framerate == 0)
            return ERROR_NONE;
        frame_length = imgsensor_info.normal_video.pclk /
                framerate * 10 /
                imgsensor_info.normal_video.linelength;
        spin_lock(&imgsensor_drv_lock);
        imgsensor.dummy_line =
            (frame_length > imgsensor_info.normal_video.framelength)
        ? (frame_length - imgsensor_info.normal_video.framelength)
        : 0;
        imgsensor.frame_length =
            imgsensor_info.normal_video.framelength
            + imgsensor.dummy_line;
        imgsensor.min_frame_length = imgsensor.frame_length;
        spin_unlock(&imgsensor_drv_lock);
        if (imgsensor.frame_length > imgsensor.shutter)
            set_dummy();
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        if (imgsensor.current_fps != imgsensor_info.cap.max_framerate) {
            LOG_INF(
            "Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n"
            , framerate, imgsensor_info.cap.max_framerate/10);
        frame_length = imgsensor_info.cap.pclk / framerate * 10
                / imgsensor_info.cap.linelength;
        spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line =
            (frame_length > imgsensor_info.cap.framelength)
              ? (frame_length - imgsensor_info.cap.framelength) : 0;
            imgsensor.frame_length =
                imgsensor_info.cap.framelength
                + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
    }

        if (imgsensor.frame_length > imgsensor.shutter)
            set_dummy();
        break;
    case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
        frame_length = imgsensor_info.hs_video.pclk / framerate * 10
                / imgsensor_info.hs_video.linelength;
        spin_lock(&imgsensor_drv_lock);
        imgsensor.dummy_line =
            (frame_length > imgsensor_info.hs_video.framelength)
              ? (frame_length - imgsensor_info.hs_video.framelength)
              : 0;
        imgsensor.frame_length =
            imgsensor_info.hs_video.framelength
                + imgsensor.dummy_line;
        imgsensor.min_frame_length = imgsensor.frame_length;
        spin_unlock(&imgsensor_drv_lock);
        if (imgsensor.frame_length > imgsensor.shutter)
            set_dummy();
        break;
    case MSDK_SCENARIO_ID_SLIM_VIDEO:
        frame_length = imgsensor_info.slim_video.pclk / framerate * 10
            / imgsensor_info.slim_video.linelength;
        spin_lock(&imgsensor_drv_lock);
        imgsensor.dummy_line =
            (frame_length > imgsensor_info.slim_video.framelength)
            ? (frame_length - imgsensor_info.slim_video.framelength)
            : 0;
        imgsensor.frame_length =
            imgsensor_info.slim_video.framelength
            + imgsensor.dummy_line;
        imgsensor.min_frame_length = imgsensor.frame_length;
        spin_unlock(&imgsensor_drv_lock);
        if (imgsensor.frame_length > imgsensor.shutter)
            set_dummy();
        break;
    case MSDK_SCENARIO_ID_CUSTOM1:
        frame_length = imgsensor_info.custom1.pclk / framerate * 10
                / imgsensor_info.custom1.linelength;
        spin_lock(&imgsensor_drv_lock);
        imgsensor.dummy_line =
            (frame_length > imgsensor_info.custom1.framelength)
            ? (frame_length - imgsensor_info.custom1.framelength)
            : 0;
        imgsensor.frame_length =
            imgsensor_info.custom1.framelength
            + imgsensor.dummy_line;
        imgsensor.min_frame_length = imgsensor.frame_length;
        spin_unlock(&imgsensor_drv_lock);
        if (imgsensor.frame_length > imgsensor.shutter)
            set_dummy();
        break;
    case MSDK_SCENARIO_ID_CUSTOM2:
        frame_length = imgsensor_info.custom2.pclk / framerate * 10
                / imgsensor_info.custom2.linelength;
        spin_lock(&imgsensor_drv_lock);
        imgsensor.dummy_line =
            (frame_length > imgsensor_info.custom2.framelength)
            ? (frame_length - imgsensor_info.custom2.framelength)
            : 0;
        imgsensor.frame_length =
            imgsensor_info.custom2.framelength
            + imgsensor.dummy_line;
        imgsensor.min_frame_length = imgsensor.frame_length;
        spin_unlock(&imgsensor_drv_lock);
        if (imgsensor.frame_length > imgsensor.shutter)
            set_dummy();
        break;
    default:  /*coding with  preview scenario by default*/
        frame_length = imgsensor_info.pre.pclk / framerate * 10
            / imgsensor_info.pre.linelength;
        spin_lock(&imgsensor_drv_lock);
        imgsensor.dummy_line =
            (frame_length > imgsensor_info.pre.framelength)
            ? (frame_length - imgsensor_info.pre.framelength) : 0;
        imgsensor.frame_length =
            imgsensor_info.pre.framelength + imgsensor.dummy_line;
        imgsensor.min_frame_length = imgsensor.frame_length;
        spin_unlock(&imgsensor_drv_lock);
        if (imgsensor.frame_length > imgsensor.shutter)
            set_dummy();
        LOG_INF("error scenario_id = %d, we use preview scenario\n",
            scenario_id);
        break;
    }
    return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(
        enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate)
{

    switch (scenario_id) {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        *framerate = imgsensor_info.pre.max_framerate;
        break;
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        *framerate = imgsensor_info.normal_video.max_framerate;
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        *framerate = imgsensor_info.cap.max_framerate;
        break;
    case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
        *framerate = imgsensor_info.hs_video.max_framerate;
        break;
    case MSDK_SCENARIO_ID_SLIM_VIDEO:
        *framerate = imgsensor_info.slim_video.max_framerate;
        break;
    case MSDK_SCENARIO_ID_CUSTOM1:
        *framerate = imgsensor_info.custom1.max_framerate;
        break;
    case MSDK_SCENARIO_ID_CUSTOM2:
        *framerate = imgsensor_info.custom2.max_framerate;
        break;
    default:
        break;
    }

    return ERROR_NONE;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
                 UINT8 *feature_para, UINT32 *feature_para_len)
{
    UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
    UINT16 *feature_data_16 = (UINT16 *) feature_para;
    UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
    UINT32 *feature_data_32 = (UINT32 *) feature_para;
    unsigned long long *feature_data = (unsigned long long *) feature_para;
    /* unsigned long long *feature_return_para
     *  = (unsigned long long *) feature_para;
     */
    struct SET_PD_BLOCK_INFO_T *PDAFinfo;
    struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
    /* SET_SENSOR_AWB_GAIN *pSetSensorAWB
     *  = (SET_SENSOR_AWB_GAIN *)feature_para;
     */
    MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data
        = (MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

    /*LOG_INF("feature_id = %d\n", feature_id);*/
    switch (feature_id) {
    case SENSOR_FEATURE_SET_SENSOR_OTP:
        {
            enum IMGSENSOR_RETURN ret = IMGSENSOR_RETURN_SUCCESS;
            pr_debug("SENSOR_FEATURE_SET_SENSOR_OTP\n");
            ret = Eeprom_CallWriteService((ACDK_SENSOR_ENGMODE_STEREO_STRUCT *)(feature_para));
            if (ret == IMGSENSOR_RETURN_SUCCESS)
                return ERROR_NONE;
            else
                return ERROR_MSDK_IS_ACTIVATED;
        }
    case SENSOR_FEATURE_GET_GAIN_RANGE_BY_SCENARIO:
        *(feature_data + 1) = imgsensor_info.min_gain;
        *(feature_data + 2) = imgsensor_info.max_gain;
        break;
    case SENSOR_FEATURE_GET_BASE_GAIN_ISO_AND_STEP:
        *(feature_data + 0) = imgsensor_info.min_gain_iso;
        *(feature_data + 1) = imgsensor_info.gain_step;
        *(feature_data + 2) = imgsensor_info.gain_type;
        break;
    case SENSOR_FEATURE_GET_MIN_SHUTTER_BY_SCENARIO:
        *(feature_data + 1) = imgsensor_info.min_shutter;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
        switch (*feature_data) {
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.cap.pclk;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.normal_video.pclk;
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.hs_video.pclk;
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.slim_video.pclk;
            break;
        case MSDK_SCENARIO_ID_CUSTOM1:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.custom1.pclk;
            break;
        case MSDK_SCENARIO_ID_CUSTOM2:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.custom2.pclk;
            break;
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        default:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.pre.pclk;
            break;
        }
        break;
    case SENSOR_FEATURE_GET_PERIOD_BY_SCENARIO:
        switch (*feature_data) {
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
            = (imgsensor_info.cap.framelength << 16)
                + imgsensor_info.cap.linelength;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
            = (imgsensor_info.normal_video.framelength << 16)
                + imgsensor_info.normal_video.linelength;
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
            = (imgsensor_info.hs_video.framelength << 16)
                + imgsensor_info.hs_video.linelength;
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
            = (imgsensor_info.slim_video.framelength << 16)
                + imgsensor_info.slim_video.linelength;
            break;
        case MSDK_SCENARIO_ID_CUSTOM1:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
            = (imgsensor_info.custom1.framelength << 16)
                + imgsensor_info.custom1.linelength;
            break;
        case MSDK_SCENARIO_ID_CUSTOM2:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
            = (imgsensor_info.custom2.framelength << 16)
                + imgsensor_info.custom2.linelength;
            break;
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        default:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
            = (imgsensor_info.pre.framelength << 16)
                + imgsensor_info.pre.linelength;
            break;
        }
        break;
    case SENSOR_FEATURE_GET_OFFSET_TO_START_OF_EXPOSURE:
        *(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 3000000;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *feature_return_para_16++ = imgsensor.line_length;
        *feature_return_para_16 = imgsensor.frame_length;
        *feature_para_len = 4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *feature_return_para_32 = imgsensor.pclk;
        *feature_para_len = 4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
         set_shutter(*feature_data);
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
         /* night_mode((BOOL) *feature_data); */
        break;
    case SENSOR_FEATURE_CHECK_MODULE_ID:
        *feature_return_para_32 = imgsensor_info.module_id;
        break;
    case SENSOR_FEATURE_SET_GAIN:
        set_gain((UINT16) *feature_data);
        break;
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        write_cmos_sensor(sensor_reg_data->RegAddr,
                    sensor_reg_data->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        sensor_reg_data->RegData =
            read_cmos_sensor(sensor_reg_data->RegAddr);
        break;
    case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
        /*get the lens driver ID from EEPROM
         * or just return LENS_DRIVER_ID_DO_NOT_CARE
         * if EEPROM does not exist in camera module.
         */
        *feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
        *feature_para_len = 4;
        break;
    case SENSOR_FEATURE_SET_VIDEO_MODE:
        set_video_mode(*feature_data);
        break;
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
        get_imgsensor_id(feature_return_para_32);
        break;
    case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
        set_auto_flicker_mode((BOOL)*feature_data_16,
                      *(feature_data_16+1));
        break;
    case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
         set_max_framerate_by_scenario(
                (enum MSDK_SCENARIO_ID_ENUM)*feature_data,
                *(feature_data+1));
        break;
    case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
         get_default_framerate_by_scenario(
                (enum MSDK_SCENARIO_ID_ENUM)*(feature_data),
                (MUINT32 *)(uintptr_t)(*(feature_data+1)));
        break;
    case SENSOR_FEATURE_GET_PDAF_DATA:
        LOG_INF("SENSOR_FEATURE_GET_PDAF_DATA\n");
        read_imx319_DCC((kal_uint16) (*feature_data),
            (char *)(uintptr_t) (*(feature_data + 1)),
            (kal_uint32) (*(feature_data + 2)));
        break;
    case SENSOR_FEATURE_SET_TEST_PATTERN:
        set_test_pattern_mode((UINT8)*feature_data, (struct SET_SENSOR_PATTERN_SOLID_COLOR *)(feature_data+1));
        break;
    case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
        /* for factory mode auto testing */
        *feature_return_para_32 = imgsensor_info.checksum_value;
        *feature_para_len = 4;
        break;
    case SENSOR_FEATURE_SET_FRAMERATE:
        LOG_INF("current fps :%d\n", (UINT32)*feature_data_32);
        spin_lock(&imgsensor_drv_lock);
        imgsensor.current_fps = *feature_data_32;
        spin_unlock(&imgsensor_drv_lock);
        break;
    case SENSOR_FEATURE_SET_HDR:
        LOG_INF("ihdr enable :%d\n", (BOOL)*feature_data_32);
        spin_lock(&imgsensor_drv_lock);
        imgsensor.ihdr_mode = *feature_data_32;
        spin_unlock(&imgsensor_drv_lock);
        break;
    case SENSOR_FEATURE_GET_CROP_INFO:
        LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n",
            (UINT32)*feature_data);
        wininfo =
    (struct SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));

        switch (*feature_data_32) {
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            memcpy((void *)wininfo,
                (void *)&imgsensor_winsize_info[1],
                sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            memcpy((void *)wininfo,
                (void *)&imgsensor_winsize_info[2],
                sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            memcpy((void *)wininfo,
            (void *)&imgsensor_winsize_info[3],
            sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            memcpy((void *)wininfo,
            (void *)&imgsensor_winsize_info[4],
            sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
            break;
        case MSDK_SCENARIO_ID_CUSTOM1:
            memcpy((void *)wininfo,
            (void *)&imgsensor_winsize_info[5],
            sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
            break;
        case MSDK_SCENARIO_ID_CUSTOM2:
            memcpy((void *)wininfo,
            (void *)&imgsensor_winsize_info[6],
            sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
            break;
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        default:
            memcpy((void *)wininfo,
            (void *)&imgsensor_winsize_info[0],
            sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
            break;
        }
        break;
    case SENSOR_FEATURE_GET_PDAF_INFO:
        LOG_INF("SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%d\n",
            (UINT16) *feature_data);
        PDAFinfo =
          (struct SET_PD_BLOCK_INFO_T *)(uintptr_t)(*(feature_data+1));
        switch (*feature_data) {
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
        default:
            break;
        }
        break;
    case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
        LOG_INF(
        "SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY scenarioId:%d\n",
            (UINT16) *feature_data);
        switch (*feature_data) {
        default:
            *(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
            break;
        }
        break;
    case SENSOR_FEATURE_GET_TEMPERATURE_VALUE:
        *feature_return_para_32 = get_sensor_temperature();
        *feature_para_len = 4;
        break;
    case SENSOR_FEATURE_GET_PDAF_REG_SETTING:
        LOG_INF("SENSOR_FEATURE_GET_PDAF_REG_SETTING %d",
            (*feature_para_len));
        imx319_get_pdaf_reg_setting((*feature_para_len) / sizeof(UINT32)
            , feature_data_16);
        break;
    case SENSOR_FEATURE_SET_PDAF_REG_SETTING:
        LOG_INF("SENSOR_FEATURE_SET_PDAF_REG_SETTING %d",
             (*feature_para_len));
        imx319_set_pdaf_reg_setting((*feature_para_len) / sizeof(UINT32)
            , feature_data_16);
        break;
    case SENSOR_FEATURE_SET_PDAF:
        LOG_INF("PDAF mode :%d\n", *feature_data_16);
        imgsensor.pdaf_mode = *feature_data_16;
        break;
    case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
        LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",
            (UINT16)*feature_data,
            (UINT16)*(feature_data+1),
            (UINT16)*(feature_data+2));
        break;
    case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:
        set_shutter_frame_length((UINT16) (*feature_data),
                (UINT16) (*(feature_data + 1)), (UINT16) (*(feature_data + 2)));
        break;
    case SENSOR_FEATURE_SET_AWB_GAIN:
        break;
    case SENSOR_FEATURE_SET_HDR_SHUTTER:
        LOG_INF("SENSOR_FEATURE_SET_HDR_SHUTTER LE=%d, SE=%d\n",
            (UINT16)*feature_data, (UINT16)*(feature_data+1));
        #if 0
        ihdr_write_shutter((UINT16)*feature_data,
                   (UINT16)*(feature_data+1));
        #endif
        break;
    case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
        LOG_INF("SENSOR_FEATURE_SET_STREAMING_SUSPEND\n");
        streaming_control(KAL_FALSE);
        break;
    case SENSOR_FEATURE_SET_STREAMING_RESUME:
        LOG_INF("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%llu\n",
            *feature_data);
        if (*feature_data != 0)
            set_shutter(*feature_data);
        streaming_control(KAL_TRUE);
        break;
    case SENSOR_FEATURE_GET_BINNING_TYPE:
        switch (*(feature_data + 1)) {
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            *feature_return_para_32 = 1; /*BINNING_NONE*/
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            *feature_return_para_32 = 4; /*BINNING_SUMMED*/
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        default:
            *feature_return_para_32 = 1; /*BINNING_AVERAGED*/
            break;
        }
        pr_debug("SENSOR_FEATURE_GET_BINNING_TYPE AE_binning_type:%d,\n",
            *feature_return_para_32);
        *feature_para_len = 4;
        break;
    case SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE:
        *feature_return_para_32 = imgsensor.current_ae_effective_frame;
        break;
    case SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE:
        memcpy(feature_return_para_32,
        &imgsensor.ae_frm_mode, sizeof(struct IMGSENSOR_AE_FRM_MODE));
        break;
    case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
    {
        switch (*feature_data) {
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.cap.mipi_pixel_rate;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.normal_video.mipi_pixel_rate;
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.hs_video.mipi_pixel_rate;
            break;
        case MSDK_SCENARIO_ID_CUSTOM1:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.custom1.mipi_pixel_rate;
            break;
        case MSDK_SCENARIO_ID_CUSTOM2:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.custom2.mipi_pixel_rate;
            break;
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        default:
            *(MUINT32 *)(uintptr_t)(*(feature_data + 1))
                = imgsensor_info.pre.mipi_pixel_rate;
            break;
        }
    }
    break;
    case SENSOR_FEATURE_GET_FRAME_CTRL_INFO_BY_SCENARIO:
        *(feature_data + 1) = 1; /* margin info by scenario */
        *(feature_data + 2) = imgsensor_info.margin;
        break;
    case SENSOR_FEATURE_GET_VC_INFO:
    default:
        break;
    }

    return ERROR_NONE;
} /* feature_control() */

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
    open,
    get_info,
    get_resolution,
    feature_control,
    control,
    close
};


UINT32 IMX319_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc != NULL)
        *pfFunc = &sensor_func;
    return ERROR_NONE;
}
