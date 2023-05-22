/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 MediaTek Inc.
 */


#include <linux/string.h>

#include "mmdvfs_plat.h"
#include "mtk_qos_sram.h"
#include "smi_pmqos.h"
#if IS_ENABLED(CONFIG_MACH_MT6877)
#if defined(USE_MEDIATEK_EMI)
#include <memory/mediatek/dramc.h>
#endif
#include <dt-bindings/memory/mt6877-larb-port.h>
#else
#include <dt-bindings/memory/mt6853-larb-port.h>
#endif

#undef pr_fmt
#define pr_fmt(fmt) "[mmdvfs][plat]" fmt


#define LARB_MDP_ID 2
#define LARB_VENC_ID 7

#define LARB_IMG_ID 9
#define LARB_IMG2_ID 11
#define LARB_CAM_ID 13
#define LARB_CAM2_ID 14
#define LARB_CAM3_ID 16
#define LARB_CAM4_ID 17



static const u32 mdp_comm_port_shift = (0 * SMI_COMM_MASTER_NUM + 4);
static const u32 venc_comm_port_shift = (0 * SMI_COMM_MASTER_NUM + 3);
static const u32 img_comm_port_shift = (0 * SMI_COMM_MASTER_NUM + 5);
static const u32 cam_comm_port_shift = (0 * SMI_COMM_MASTER_NUM + 6);
static const u32 cam_comm_port2_shift = (0 * SMI_COMM_MASTER_NUM + 7);

#ifdef QOS_BOUND_DETECT
void mmdvfs_update_qos_sram(struct mm_larb_request larb_req[], u32 larb_update)
{

	u32 bw;

	if (larb_update & (1 << mdp_comm_port_shift)) {
		bw = larb_req[LARB_MDP_ID].total_bw_data;
		qos_sram_write(MM_SMI_MDP, bw);
	}

	if (larb_update & (1 << venc_comm_port_shift)) {
		bw = larb_req[LARB_VENC_ID].total_bw_data;
		//bw += larb_req[LARB_VENC2_ID].total_bw_data;
		qos_sram_write(MM_SMI_VENC, bw);
	}

	if (larb_update & (1 << img_comm_port_shift)) {
		bw = larb_req[LARB_IMG_ID].total_bw_data;
		qos_sram_write(MM_SMI_IMG, bw);
	}

	if (larb_update & ((1 << cam_comm_port_shift)
				| (1 << cam_comm_port2_shift))) {
		bw = larb_req[LARB_CAM_ID].total_bw_data;
		bw += larb_req[LARB_CAM2_ID].total_bw_data;
		bw += larb_req[LARB_CAM3_ID].total_bw_data;
		bw += larb_req[LARB_CAM4_ID].total_bw_data;
		qos_sram_write(MM_SMI_CAM, bw);
	}
}
#endif
static u32 log_common_port_ids;
static u32 log_larb_ids;
bool mmdvfs_log_larb_mmp(s32 common_port_id, s32 larb_id)
{
	if (common_port_id >= 0)
		return (1 << common_port_id) & log_common_port_ids;
	if (larb_id >= 0)
		return (1 << larb_id) & log_larb_ids;
	return false;
}

/* Return port number of CCU on SMI common */
inline u32 mmdvfs_get_ccu_smi_common_port(u32 master_id)
{
	if (master_id == get_virtual_port(VIRTUAL_CCU_COMMON))
		return 7;
	else
		return 6;
}

s32 get_ccu_hrt_bw(struct mm_larb_request larb_req[])
{
	struct mm_qos_request *enum_req = NULL;
	s32 bw = larb_req[MTK_IOMMU_TO_LARB(get_virtual_port(
				VIRTUAL_CCU_COMMON))].total_hrt_data;
	bw += larb_req[MTK_IOMMU_TO_LARB(get_virtual_port(
				VIRTUAL_CCU_COMMON2))].total_hrt_data;

	list_for_each_entry(enum_req,
		&larb_req[LARB_CAM_ID].larb_list, larb_node) {
		if (enum_req->master_id == M4U_PORT_L13_CAM_CCUI
			|| enum_req->master_id == M4U_PORT_L13_CAM_CCUO)
			bw += enum_req->hrt_value;
	}
	list_for_each_entry(enum_req,
		&larb_req[LARB_CAM2_ID].larb_list, larb_node) {
		if (enum_req->master_id == M4U_PORT_L14_CAM_CCUI
			|| enum_req->master_id == M4U_PORT_L14_CAM_CCUO)
			bw += enum_req->hrt_value;
	}
	return bw;
}

s32 get_md_hrt_bw(void)
{
#if IS_ENABLED(CONFIG_MACH_MT6853)
	return 3344;
#elif IS_ENABLED(CONFIG_MACH_MT6877)
    /*
     * Reduce MD worst HRT BW for DISP layers .
     * If there is disp underrun issue, we need to confirm
     * whether to add MD var: 1700.
     */
    return (3427*100/65);
#elif IS_ENABLED(CONFIG_MACH_MT6781)
	return 1843;
#else
	return 3888;
#endif
}

s32 dram_write_weight(s32 val)
{
	return val;
}

s32 emi_occ_ratio(void)
{
#if IS_ENABLED(CONFIG_MACH_MT6877)
#if defined(USE_MEDIATEK_EMI)
	if (mtk_dramc_get_ddr_type() == TYPE_LPDDR5)
		return 680;
#endif
	return 740;
#else
	return 500;
#endif
}

s32 emi_occ_ui_only(void)
{
#if IS_ENABLED(CONFIG_MACH_MT6877)
#if defined(USE_MEDIATEK_EMI)
	if (mtk_dramc_get_ddr_type() == TYPE_LPDDR5)
		return 680;
#endif
	return 745;
#else
	return 500;
#endif
}


s32 cam_occ_ratio(void)
{
#if IS_ENABLED(CONFIG_MACH_MT6877)
#if defined(USE_MEDIATEK_EMI)
	if (mtk_dramc_get_ddr_type() == TYPE_LPDDR5)
		return 860;
#endif
	return 880;
#else
	return 1000;
#endif
}

s32 disp_occ_ratio(void)
{
#if IS_ENABLED(CONFIG_MACH_MT6877)
#if defined(USE_MEDIATEK_EMI)
	if (mtk_dramc_get_ddr_type() == TYPE_LPDDR5)
		return 880;
#endif
	return 900;
#else
	return 1000;
#endif
}

