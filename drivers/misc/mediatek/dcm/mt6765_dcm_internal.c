// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/io.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>
/* #include <mt-plat/mtk_io.h> */
/* #include <mt-plat/sync_write.h> */
/* include <mt-plat/mtk_secure_api.h> */
#include "mt6765_dcm_internal.h"
#include "mtk_dcm.h"

#define DEBUGLINE dcm_pr_info("%s %d\n", __func__, __LINE__)

static short dcm_cpu_cluster_stat;
static short dcm_debug_stat;


unsigned int all_dcm_type =
		(ARMCORE_DCM_TYPE | MCUSYS_DCM_TYPE | MCSI_DCM_TYPE
		| STALL_DCM_TYPE | RGU_DCM_TYPE
		| GIC_SYNC_DCM_TYPE | INFRA_DCM_TYPE
		| DDRPHY_DCM_TYPE | EMI_DCM_TYPE | DRAMC_DCM_TYPE
		);
unsigned int init_dcm_type =
		(ARMCORE_DCM_TYPE | MCUSYS_DCM_TYPE | MCSI_DCM_TYPE
		| STALL_DCM_TYPE
		| GIC_SYNC_DCM_TYPE | INFRA_DCM_TYPE
		);

#if defined(__KERNEL__) && defined(CONFIG_OF)
unsigned long dcm_infracfg_ao_base;
unsigned long dcm_mcucfg_base;
unsigned long dcm_mp0_cpucfg_base;
unsigned long dcm_mp1_cpucfg_base;
unsigned long dcm_mcu_misccfg_base;
unsigned long dcm_chn0_emi_base;
unsigned long dcm_chn1_emi_base;
unsigned long dcm_gce_base;
unsigned long dcm_audio_base;
unsigned long dcm_efusec_base;
unsigned long dcm_mfgcfg_base;
unsigned long dcm_mmsys_config_base;
unsigned long dcm_smi_common_base;
unsigned long dcm_smi_larb0_base;
unsigned long dcm_smi_larb2_base;
unsigned long dcm_smi_larb1_base;
unsigned long dcm_venc_base;
unsigned long dcm_jpgenc_base;

#define DCM_NODE "mediatek,mt6765-dcm"

#endif /* #if defined(__KERNEL__) && defined(CONFIG_OF) */

short is_dcm_bringup(void)
{
#ifdef DCM_BRINGUP
	dcm_pr_info("%s: skipped for bring up\n", __func__);
	return 1;
#else
	return 0;
#endif
}

/*****************************************
 * following is implementation per DCM module.
 * 1. per-DCM function is 1-argu with ON/OFF/MODE option.
 *****************************************/
int dcm_topckg(int on)
{
	return 0;
}

void dcm_infracfg_ao_emi_indiv(int on)
{
}

int dcm_infra_preset(int on)
{
	return 0;
}

int dcm_infra(int on)
{
	dcm_infracfg_ao_audio(on);
	dcm_infracfg_ao_icusb(on);
	dcm_infracfg_ao_infra_mem(on);
	dcm_infracfg_ao_infra_peri(on);
	dcm_infracfg_ao_p2p_dsi_csi(on);
	dcm_infracfg_ao_ssusb(on);

	return 0;
}

int dcm_peri(int on)
{
	return 0;
}

int dcm_armcore(int on)
{
	dcm_mcu_misccfg_bus_arm_pll_divider_dcm(on);
	dcm_mcu_misccfg_mp0_arm_pll_divider_dcm(on);
	dcm_mcu_misccfg_mp1_arm_pll_divider_dcm(on);

	return 0;
}

int dcm_mcusys(int on)
{
	dcm_mcu_misccfg_adb400_dcm(on);
	dcm_mcu_misccfg_bus_sync_dcm(on);
	dcm_mcu_misccfg_bus_clock_dcm(on);
	dcm_mcu_misccfg_bus_fabric_dcm(on);
	dcm_mcu_misccfg_l2_shared_dcm(on);
	dcm_mcu_misccfg_mp0_sync_dcm_enable(on);
	dcm_mcu_misccfg_mp1_sync_dcm_enable(on);
	dcm_mcu_misccfg_mcu_misc_dcm(on);

	return 0;
}

int dcm_mcusys_preset(int on)
{
	return 0;
}

int dcm_big_core_preset(void)
{
	return 0;
}

int dcm_big_core(int on)
{
	return 0;
}

int dcm_stall_preset(int on)
{
	reg_write(SYNC_DCM_CLUSTER_CONFIG, 0x063f0000);

	return 0;
}

int dcm_stall(int on)
{
	dcm_mcu_misccfg_mp0_stall_dcm(on);
	dcm_mcu_misccfg_mp1_stall_dcm(on);

	return 0;
}

int dcm_gic_sync(int on)
{
	dcm_mcu_misccfg_gic_sync_dcm(on);

	return 0;
}

int dcm_last_core(int on)
{
	return 0;
}

int dcm_rgu(int on)
{
	dcm_mp0_cpucfg_mp0_rgu_dcm(on);
	dcm_mp1_cpucfg_mp1_rgu_dcm(on);

	return 0;
}

int dcm_dramc_ao(int on)
{

	return 0;
}

int dcm_ddrphy(int on)
{

	return 0;
}

int dcm_emi(int on)
{
	dcm_chn0_emi_dcm_emi_group(on);
	dcm_chn1_emi_dcm_emi_group(on);

	return 0;
}

int dcm_lpdma(int on)
{
	return 0;
}

int dcm_pwrap(int on)
{
	return 0;
}

int dcm_mcsi_preset(int on)
{
	return 0;
}

int dcm_mcsi(int on)
{
	dcm_mcucfg_mcsi_dcm(on);

	return 0;
}

void dcm_dump_regs(void)
{
	dcm_pr_info("\n******** dcm dump register *********\n");
	/* MCUSYS reg */

	/* ARMCORE reg */

	/* STALL reg */

	/* INFRA_AO reg */
	REG_DUMP(PERI_BUS_DCM_CTRL);
	REG_DUMP(DFS_MEM_DCM_CTRL);
	REG_DUMP(PERI_BUS_DCM_CTRL);
	REG_DUMP(INFRA_MISC);
	REG_DUMP(MEM_DCM_CTRL);
	REG_DUMP(INFRA_BUS_DCM_CTRL);
	REG_DUMP(PERI_BUS_DCM_CTRL);
	REG_DUMP(P2P_RX_CLK_ON);
	REG_DUMP(PERI_BUS_DCM_CTRL);
	REG_DUMP(PERI_BUS_DCM_CTRL);
	REG_DUMP(PERI_BUS_DCM_CTRL);

	/* INFRA_AO_MEM reg */

	/* SUB INFRA reg */

	/* INFRA EMI reg */

	/* emi */
	REG_DUMP(CHN0_EMI_CHN_EMI_CONB);
	REG_DUMP(CHN1_EMI_CHN_EMI_CONB);

	/* dramc */

	/* ddrphy */
}

void get_default(unsigned int *type, int *state)
{
#ifndef DCM_DEFAULT_ALL_OFF
	/** enable all dcm **/
	*type = init_dcm_type;
	*state = DCM_DEFAULT;
#else /* DCM_DEFAULT_ALL_OFF */
	*type = all_dcm_type;
	*state = DCM_OFF;
#endif /* #ifndef DCM_DEFAULT_ALL_OFF */
}

void get_init_type(unsigned int *type)
{
	*type = init_dcm_type;
}
void get_all_type(unsigned int *type)
{
	*type = all_dcm_type;
}
void get_init_by_k_type(unsigned int *type)
{
#ifdef ENABLE_DCM_IN_LK
	*type = INIT_DCM_TYPE_BY_K;
#else
	*type = init_dcm_type;
#endif
}

void set_debug_mode(unsigned int mode)
{
	dcm_debug_stat = mode;
}

struct DCM_OPS dcm_ops = {
	.dump_regs = (DCM_FUNC_VOID_VOID) dcm_dump_regs,
	.get_default = (DCM_FUNC_VOID_UINTR_INTR) get_default,
	.get_init_type = (DCM_FUNC_VOID_UINTR) get_init_type,
	.get_all_type = (DCM_FUNC_VOID_UINTR) get_all_type,
	.get_init_by_k_type = (DCM_FUNC_VOID_UINTR) get_init_by_k_type,
	.set_debug_mode = (DCM_FUNC_VOID_UINT) set_debug_mode,
};

struct DCM_BASE dcm_base_array[] = {
	DCM_BASE_INFO(dcm_infracfg_ao_base),
	DCM_BASE_INFO(dcm_mcucfg_base),
	DCM_BASE_INFO(dcm_mp0_cpucfg_base),
	DCM_BASE_INFO(dcm_mp1_cpucfg_base),
	DCM_BASE_INFO(dcm_mcu_misccfg_base),
	DCM_BASE_INFO(dcm_chn0_emi_base),
	DCM_BASE_INFO(dcm_chn1_emi_base),
	DCM_BASE_INFO(dcm_gce_base),
	DCM_BASE_INFO(dcm_audio_base),
	DCM_BASE_INFO(dcm_efusec_base),
	DCM_BASE_INFO(dcm_mfgcfg_base),
	DCM_BASE_INFO(dcm_mmsys_config_base),
	DCM_BASE_INFO(dcm_smi_common_base),
	DCM_BASE_INFO(dcm_smi_larb0_base),
	DCM_BASE_INFO(dcm_smi_larb2_base),
	DCM_BASE_INFO(dcm_smi_larb1_base),
	DCM_BASE_INFO(dcm_venc_base),
	DCM_BASE_INFO(dcm_jpgenc_base),
};

struct DCM dcm_array[NR_DCM_TYPE] = {
	{
	 .typeid = ARMCORE_DCM_TYPE,
	 .name = "ARMCORE_DCM",
	 .func = (DCM_FUNC) dcm_armcore,
	 .current_state = ARMCORE_DCM_MODE1,
	 .default_state = ARMCORE_DCM_MODE1,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = MCUSYS_DCM_TYPE,
	 .name = "MCUSYS_DCM",
	 .func = (DCM_FUNC) dcm_mcusys,
	 .current_state = MCUSYS_DCM_ON,
	 .default_state = MCUSYS_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = INFRA_DCM_TYPE,
	 .name = "INFRA_DCM",
	 .func = (DCM_FUNC) dcm_infra,
	 .current_state = INFRA_DCM_ON,
	 .default_state = INFRA_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = PERI_DCM_TYPE,
	 .name = "PERI_DCM",
	 .func = (DCM_FUNC) dcm_peri,
	 .current_state = PERI_DCM_ON,
	 .default_state = PERI_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = EMI_DCM_TYPE,
	 .name = "EMI_DCM",
	 .func = (DCM_FUNC) dcm_emi,
	 .current_state = EMI_DCM_ON,
	 .default_state = EMI_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = DRAMC_DCM_TYPE,
	 .name = "DRAMC_DCM",
	 .func = (DCM_FUNC) dcm_dramc_ao,
	 .current_state = DRAMC_AO_DCM_ON,
	 .default_state = DRAMC_AO_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = DDRPHY_DCM_TYPE,
	 .name = "DDRPHY_DCM",
	 .func = (DCM_FUNC) dcm_ddrphy,
	 .current_state = DDRPHY_DCM_ON,
	 .default_state = DDRPHY_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = STALL_DCM_TYPE,
	 .name = "STALL_DCM",
	 .func = (DCM_FUNC) dcm_stall,
	 .current_state = STALL_DCM_ON,
	 .default_state = STALL_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = BIG_CORE_DCM_TYPE,
	 .name = "BIG_CORE_DCM",
	 .func = (DCM_FUNC) dcm_big_core,
	 .current_state = BIG_CORE_DCM_ON,
	 .default_state = BIG_CORE_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = GIC_SYNC_DCM_TYPE,
	 .name = "GIC_SYNC_DCM",
	 .func = (DCM_FUNC) dcm_gic_sync,
	 .current_state = GIC_SYNC_DCM_ON,
	 .default_state = GIC_SYNC_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = LAST_CORE_DCM_TYPE,
	 .name = "LAST_CORE_DCM",
	 .func = (DCM_FUNC) dcm_last_core,
	 .current_state = LAST_CORE_DCM_ON,
	 .default_state = LAST_CORE_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = RGU_DCM_TYPE,
	 .name = "RGU_CORE_DCM",
	 .func = (DCM_FUNC) dcm_rgu,
	 .current_state = RGU_DCM_ON,
	 .default_state = RGU_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = TOPCKG_DCM_TYPE,
	 .name = "TOPCKG_DCM",
	 .func = (DCM_FUNC) dcm_topckg,
	 .current_state = TOPCKG_DCM_ON,
	 .default_state = TOPCKG_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = LPDMA_DCM_TYPE,
	 .name = "LPDMA_DCM",
	 .func = (DCM_FUNC) dcm_lpdma,
	 .current_state = LPDMA_DCM_ON,
	 .default_state = LPDMA_DCM_ON,
	 .disable_refcnt = 0,
	 },
	{
	 .typeid = MCSI_DCM_TYPE,
	 .name = "MCSI_DCM",
	 .func = (DCM_FUNC) dcm_mcsi,
	 .current_state = MCSI_DCM_ON,
	 .default_state = MCSI_DCM_ON,
	 .disable_refcnt = 0,
	 },
};

void dcm_set_hotplug_nb(void) {}

short dcm_get_cpu_cluster_stat(void)
{
	return dcm_cpu_cluster_stat;
}

/**/
void dcm_array_register(void)
{
	mt_dcm_array_register(dcm_array, &dcm_ops);
}

/*From DCM COMMON*/

#ifdef CONFIG_OF
int mt_dcm_dts_map(void)
{
	struct device_node *node;
	unsigned int i;
	/* dcm */
	node = of_find_compatible_node(NULL, NULL, DCM_NODE);
	if (!node) {
		dcm_pr_info("error: cannot find node %s\n", DCM_NODE);
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(dcm_base_array); i++) {
		//*dcm_base_array[i].base= (unsigned long)of_iomap(node, i);
		*(dcm_base_array[i].base) = (unsigned long)of_iomap(node, i);

		if (!*(dcm_base_array[i].base)) {
			dcm_pr_info("error: cannot iomap base %s\n",
				dcm_base_array[i].name);
			return -1;
		}
	}
	/* infracfg_ao */
	return 0;
}
#else
int mt_dcm_dts_map(void)
{
	return 0;
}
#endif /* #ifdef CONFIG_PM */


void dcm_pre_init(void)
{
	dcm_pr_info("weak function of %s\n", __func__);
}

static int __init mt6765_dcm_init(void)
{
	int ret = 0;

	if (is_dcm_bringup())
		return 0;

	if (is_dcm_initialized())
		return 0;

	if (mt_dcm_dts_map()) {
		dcm_pr_notice("%s: failed due to DTS failed\n", __func__);
		return -1;
	}

	dcm_array_register();

	ret = mt_dcm_common_init();

	return ret;
}

static void __exit mt6765_dcm_exit(void)
{
}
MODULE_SOFTDEP("pre:mtk_dcm.ko");
module_init(mt6765_dcm_init);
module_exit(mt6765_dcm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MediaTek DCM driver");
