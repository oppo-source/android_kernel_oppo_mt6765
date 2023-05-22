// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef _MTK_VCODEC_PM_H_
#define _MTK_VCODEC_PM_H_

#include "ion_drv.h"
#include "mach/mt_iommu.h"

extern struct ion_device *g_ion_device;

#define MTK_PLATFORM_STR        "platform:mt6873"
#define MTK_VDEC_RACING_INFO_OFFSET  0x100
#define MTK_VDEC_RACING_INFO_SIZE 68

/**
 * struct mtk_vcodec_pm - Power management data structure
 */
struct mtk_vcodec_pm {
	struct clk      *vdec_bus_clk_src;
	struct clk      *vencpll;

	struct clk      *vcodecpll;
	struct clk      *univpll_d2;
	struct clk      *clk_cci400_sel;
	struct clk      *vdecpll;
	struct clk      *vdec_sel;
	struct clk      *vencpll_d2;
	struct clk      *venc_sel;
	struct clk      *univpll1_d2;
	struct clk      *venc_lt_sel;
	struct clk      *img_resz;
	struct device   *larbvdec;
	struct device   *larbvenc;
	struct device   *larbvenclt;
	struct device   *dev;
	struct device_node      *chip_node;
	struct mtk_vcodec_dev   *mtkdev;

	struct clk *clk_MT_CG_SOC;           /* VDEC SOC*/
	struct clk *clk_MT_CG_VDEC0;         /* VDEC core 0*/
	struct clk *clk_MT_CG_VENC0;         /* VENC core 0*/
	struct clk *clk_MT_CG_VDEC1;         /* VDEC core 1*/

	atomic_t dec_active_cnt;
	__u32 vdec_racing_info[MTK_VDEC_RACING_INFO_SIZE];
	struct mutex dec_racing_info_mutex;
};

enum mtk_dec_dump_addr_type {
	DUMP_VDEC_IN_BUF,
	DUMP_VDEC_OUT_BUF,
	DUMP_VDEC_REF_BUF,
	DUMP_VDEC_MV_BUF,
	DUMP_VDEC_UBE_BUF,
};

enum mtk_dec_dtsi_reg_idx {
	VDEC_SYS,
	VDEC_BASE,
	VDEC_VLD,
	VDEC_MC,
	VDEC_MV,
	VDEC_MISC,
	VDEC_LAT_MISC,
	VDEC_LAT_VLD,
	VDEC_RACING_CTRL,
	NUM_MAX_VDEC_REG_BASE,
};

enum mtk_enc_dtsi_reg_idx {
	VENC_SYS,
	VENC_C1_SYS,
	NUM_MAX_VENC_REG_BASE
};

#endif /* _MTK_VCODEC_PM_H_ */
