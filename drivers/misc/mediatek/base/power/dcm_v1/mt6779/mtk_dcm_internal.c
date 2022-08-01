// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 MediaTek Inc.
 */

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>
#include <mt-plat/mtk_io.h>
#include <mt-plat/sync_write.h>
//#include <mt-plat/mtk_secure_api.h>

#include <mtk_dcm_internal.h>

#define DEBUGLINE dcm_pr_info("%s %d\n", __func__, __LINE__)

static short dcm_cpu_cluster_stat;

unsigned int all_dcm_type =
		(ARMCORE_DCM_TYPE | MCUSYS_DCM_TYPE | STALL_DCM_TYPE |
		INFRA_DCM_TYPE | DDRPHY_DCM_TYPE | EMI_DCM_TYPE
		| DRAMC_DCM_TYPE);
unsigned int init_dcm_type =
		(ARMCORE_DCM_TYPE | MCUSYS_DCM_TYPE | STALL_DCM_TYPE |
		INFRA_DCM_TYPE);

#if defined(__KERNEL__) && defined(CONFIG_OF)
unsigned long dcm_infracfg_ao_base;
unsigned long dcm_pwrap_base;
unsigned long dcm_mcucfg_base;
unsigned long dcm_cpccfg_rg_base;

/* infra_cfg_ao_mem : can't change on-the-fly */
unsigned long dcm_infracfg_ao_mem_base;

/* dramc */
unsigned long dcm_dramc_ch0_top0_ao_base;
unsigned long dcm_dramc_ch1_top0_ao_base;

/* ddrphy */
unsigned long dcm_dramc_ch0_top5_ao_base;
unsigned long dcm_dramc_ch1_top5_ao_base;

/* emi */
unsigned long dcm_ch0_emi_base;
unsigned long dcm_emi_base;

/* the DCMs that not used actually in MT6779 */
unsigned long dcm_mm_iommu_base;
unsigned long dcm_vpu_iommu_base;
unsigned long dcm_sspm_base;
unsigned long dcm_audio_base;
unsigned long dcm_msdc1_base;


struct DCM_BASE {
	char *name;
	unsigned long *base;
};

#define DCM_BASE_INFO(_name) \
{ \
	.name = #_name, \
	.base = &_name, \
}

struct DCM_BASE dcm_base_array[] = {
	DCM_BASE_INFO(dcm_infracfg_ao_base),
	DCM_BASE_INFO(dcm_infracfg_ao_mem_base),
	DCM_BASE_INFO(dcm_mcucfg_base),
	DCM_BASE_INFO(dcm_cpccfg_rg_base),
	DCM_BASE_INFO(dcm_dramc_ch0_top0_ao_base),
	DCM_BASE_INFO(dcm_dramc_ch1_top0_ao_base),
	DCM_BASE_INFO(dcm_dramc_ch0_top5_ao_base),
	DCM_BASE_INFO(dcm_dramc_ch1_top5_ao_base),
	DCM_BASE_INFO(dcm_ch0_emi_base),
	DCM_BASE_INFO(dcm_emi_base),
};

#define EMI_NODE		"mediatek,emi"
#define DCM_NODE		"mediatek,mt6779-dcm"

#endif /* #if defined(__KERNEL__) && defined(CONFIG_OF) */

#define TOTAL_CORE_NUM  (CORE_NUM_L+CORE_NUM_B)
#define CORE_NUM_L      (6)
#define CORE_NUM_B      (2)

static inline void arch_get_cluster_cpus(struct cpumask *cpus, int cluster_id)
{
	int cpu = 0;

	cpumask_clear(cpus);

	if (cluster_id == 0) {
		cpu = 0;

		while (cpu < CORE_NUM_L) {
			cpumask_set_cpu(cpu, cpus);
			cpu++;
		}
	} else {
		cpu = CORE_NUM_L;

		while (cpu < TOTAL_CORE_NUM) {
			cpumask_set_cpu(cpu, cpus);
			cpu++;
		}
	}
}

static inline int arch_get_cluster_id(unsigned int cpu)
{
		return cpu < 4 ? 0:1;
}

short is_dcm_bringup(void)
{
#ifdef DCM_BRINGUP
	dcm_pr_info("%s: skipped for bring up\n", __func__);
	return 1;
#else
	return 0;
#endif
}

#ifdef CONFIG_OF
int mt_dcm_dts_map(void)
{
	struct device_node *node;
	/* dcm */
	node = of_find_compatible_node(NULL, NULL, DCM_NODE);
	if (!node) {
		dcm_pr_info("error: cannot find node %s\n", DCM_NODE);
		return -1;
	}
	dcm_infracfg_ao_base = (unsigned long)of_iomap(node, 0);
	if (!dcm_infracfg_ao_base) {
		dcm_pr_info("error: cannot iomap %s\n", dcm_base_array[0].name);
		return -1;
	}
	dcm_infracfg_ao_mem_base = (unsigned long)of_iomap(node, 1);
	if (!dcm_infracfg_ao_base) {
		dcm_pr_info("error: cannot iomap %s\n", dcm_base_array[1].name);
		return -1;
	}
	dcm_mcucfg_base = (unsigned long)of_iomap(node, 2);
	if (!dcm_mcucfg_base) {
		dcm_pr_info("error: cannot iomap %s\n", dcm_base_array[2].name);
		return -1;
	}
	dcm_cpccfg_rg_base = (unsigned long)of_iomap(node, 3);
	if (!dcm_cpccfg_rg_base) {
		dcm_pr_info("error: cannot iomap %s\n", dcm_base_array[3].name);
		return -1;
	}
	/* dramc ch0*/
	dcm_dramc_ch0_top0_ao_base = (unsigned long)of_iomap(node, 4);
	if (!dcm_dramc_ch0_top0_ao_base) {
		dcm_pr_info("error: cannot iomap dramc ch0\n");
		return -1;
	}
	/* dramc ch1*/
	dcm_dramc_ch1_top0_ao_base = (unsigned long)of_iomap(node, 5);
	if (!dcm_dramc_ch1_top0_ao_base) {
		dcm_pr_info("error: cannot iomap dramc ch1\n");
		return -1;
	}
	/* ddrphy ch0*/
	dcm_dramc_ch0_top5_ao_base = (unsigned long)of_iomap(node, 6);
	if (!dcm_dramc_ch0_top5_ao_base) {
		dcm_pr_info("error: cannot iomap ddrphy ch0\n");
		return -1;
	}
	/* ddrphy ch1*/
	dcm_dramc_ch1_top5_ao_base = (unsigned long)of_iomap(node, 7);
	if (!dcm_dramc_ch1_top5_ao_base) {
		dcm_pr_info("error: cannot iomap ddrphy ch1\n");
		return -1;
	}
	/* emi ch0 */
	dcm_ch0_emi_base = (unsigned long)of_iomap(node, 8);
	if (!dcm_ch0_emi_base) {
		dcm_pr_info("error: cannot iomap CH0 EMI\n");
		return -1;
	}
	/* cen emi */
	dcm_emi_base = (unsigned long)of_iomap(node, 9);
	if (!dcm_emi_base) {
		dcm_pr_info("error: cannot iomap CEN EMI\n");
		return -1;
	}

	return 0;
}
#else
int mt_dcm_dts_map(void)
{
	return 0;
}
#endif /* #ifdef CONFIG_PM */




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
	dcm_infracfg_ao_infra_bus_dcm(on);

	/* MT6779: Debounce setting, and not DCM really. */
	/* dcm_infracfg_ao_infra_emi_local_dcm(on); */

	dcm_infracfg_ao_infra_rx_p2p_dcm(on);
	dcm_infracfg_ao_peri_bus_dcm(on);
	dcm_infracfg_ao_peri_module_dcm(on);

	/* MT6779: INFRACFG_AO_MEM. It has been enabled in preloader
	 *         and can't not be turned off.
	 */
	/* dcm_infracfg_ao_mem_dcm_emi_group(on) */


	return 0;
}

int dcm_peri(int on)
{
	return 0;
}

int dcm_armcore(int mode)
{
	dcm_mp_cpusys_top_bus_pll_div_dcm(mode);
	dcm_mp_cpusys_top_cpu_pll_div_0_dcm(mode);
	dcm_mp_cpusys_top_cpu_pll_div_1_dcm(mode);
	dcm_mp_cpusys_top_cpu_pll_div_2_dcm(mode);

	return 0;
}

int dcm_mcusys(int on)
{
	dcm_mp_cpusys_top_adb_dcm(on);
	dcm_mp_cpusys_top_apb_dcm(on);
	dcm_mp_cpusys_top_cpubiu_dbg_cg(on);
	dcm_mp_cpusys_top_cpubiu_dcm(on);
	dcm_mp_cpusys_top_misc_dcm(on);
	dcm_mp_cpusys_top_mp0_qdcm(on);

	dcm_cpccfg_reg_emi_wfifo(on);

	/* for MT6779 */
	dcm_mp_cpusys_top_last_cor_idle_dcm(on);

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
	return 0;
}

int dcm_stall(int on)
{
	dcm_mp_cpusys_top_core_stall_dcm(on);
	dcm_mp_cpusys_top_fcm_stall_dcm(on);
	dcm_cpccfg_reg_mp_stall_dcm(on);

	return 0;
}

int dcm_gic_sync(int on)
{
	return 0;
}

int dcm_last_core(int on)
{
	return 0;
}

int dcm_rgu(int on)
{
	return 0;
}

int dcm_dramc_ao(int on)
{
	dcm_dramc_ch1_top0_ddrphy(on);
	dcm_dramc_ch0_top0_ddrphy(on);
	return 0;
}

int dcm_ddrphy(int on)
{
	dcm_dramc_ch1_top5_ddrphy(on);
	dcm_dramc_ch0_top5_ddrphy(on);

	return 0;
}

int dcm_emi(int on)
{
	dcm_chn0_emi_chn_emi_dcm(on);
	dcm_emi_emi_dcm(on);

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
	return 0;
}

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

void dcm_dump_regs(void)
{
	dcm_pr_info("\n******** dcm dump register *********\n");
	/* mcusys */
	REG_DUMP(MP0_DCM_CFG0);
	REG_DUMP(MP0_DCM_CFG7);
	REG_DUMP(MP_MISC_DCM_CFG0);

	REG_DUMP(MP_ADB_DCM_CFG0);
	REG_DUMP(MP_ADB_DCM_CFG2);
	REG_DUMP(MP_ADB_DCM_CFG4);

	REG_DUMP(MCUSYS_DCM_CFG0);

	REG_DUMP(CPU_PLLDIV_CFG0);
	REG_DUMP(CPU_PLLDIV_CFG1);
	REG_DUMP(CPU_PLLDIV_CFG2);
	REG_DUMP(BUS_PLLDIV_CFG);

	REG_DUMP(MCSI_CFG2);
	REG_DUMP(MCSI_DCM0);
	REG_DUMP(EMI_WFIFO);
	REG_DUMP(SLOW_CK_CFG);

	/* infra_ao */
	REG_DUMP(INFRA_BUS_DCM_CTRL);
	REG_DUMP(PERI_BUS_DCM_CTRL);
	REG_DUMP(MEM_DCM_CTRL);
	REG_DUMP(P2P_RX_CLK_ON);
	REG_DUMP(INFRA_AXIMEM_IDLE_BIT_EN_0);

	/* infra_ao_mem */
	REG_DUMP(INFRA_EMI_DCM_CFG0);
	REG_DUMP(INFRA_EMI_DCM_CFG3);

	/* emi */
	REG_DUMP(EMI_CONM);
	REG_DUMP(EMI_CONN);
	REG_DUMP(EMI_THRO_CTRL0);
	REG_DUMP(CHN0_EMI_CHN_EMI_CONB);

	/* dramc */
	REG_DUMP(DRAMC_CH0_TOP0_DRAMC_PD_CTRL);
	REG_DUMP(DRAMC_CH0_TOP0_CLKAR);
	REG_DUMP(DRAMC_CH1_TOP0_DRAMC_PD_CTRL);
	REG_DUMP(DRAMC_CH1_TOP0_CLKAR);

	/* ddrphy */
	REG_DUMP(DRAMC_CH0_TOP5_MISC_CG_CTRL0);
	REG_DUMP(DRAMC_CH0_TOP5_MISC_CG_CTRL2);
	REG_DUMP(DRAMC_CH0_TOP5_MISC_CTRL2);
	REG_DUMP(DRAMC_CH1_TOP5_MISC_CG_CTRL0);
	REG_DUMP(DRAMC_CH1_TOP5_MISC_CG_CTRL2);
	REG_DUMP(DRAMC_CH1_TOP5_MISC_CTRL2);
}

#ifdef CONFIG_HOTPLUG_CPU
/* Disable DCM before cpu up/cpu down */
static int dcm_cpuhp_notify_enter(unsigned int hcpu)
{
	unsigned int cpu = (long)hcpu;
	struct cpumask cpuhp_cpumask;
	struct cpumask cpu_online_cpumask;

	arch_get_cluster_cpus(&cpuhp_cpumask, arch_get_cluster_id(cpu));
	cpumask_and(&cpu_online_cpumask, &cpuhp_cpumask, cpu_online_mask);
	if (cpumask_weight(&cpu_online_cpumask) == 1) {
		switch (cpu / 4) {
		case 0:
			dcm_pr_dbg(
			"%s: cpu=%u, LL CPU_ONLINE\n",
			__func__, cpu);
			dcm_cpu_cluster_stat |= DCM_CPU_CLUSTER_LL;
			break;
		case 1:
			dcm_pr_dbg(
			"%s: cpu=%u, L CPU_ONLINE\n",
			__func__, cpu);
			dcm_cpu_cluster_stat |= DCM_CPU_CLUSTER_L;
			break;
		case 2:
			dcm_pr_dbg(
			"%s: cpu=%u, B CPU_ONLINE\n",
			__func__, cpu);
			dcm_cpu_cluster_stat |= DCM_CPU_CLUSTER_B;
			break;
		default:
			break;
		}
	}

	return 0;
}

/* Enable DCM after cpu up/cpu down */
static int dcm_cpuhp_notify_leave(unsigned int hcpu)
{
	unsigned int cpu = (long)hcpu;
	struct cpumask cpuhp_cpumask;
	struct cpumask cpu_online_cpumask;
		arch_get_cluster_cpus(&cpuhp_cpumask, arch_get_cluster_id(cpu));
		cpumask_and(&cpu_online_cpumask, &cpuhp_cpumask,
						cpu_online_mask);
		if (cpumask_weight(&cpu_online_cpumask) == 1) {
			switch (cpu / 4) {
			case 0:
				dcm_pr_dbg(
				"%s: cpu=%u, LL CPU_DOWN_PREPARE\n",
				__func__, cpu);
				dcm_cpu_cluster_stat &= ~DCM_CPU_CLUSTER_LL;
				break;
			case 1:
				dcm_pr_dbg(
				"%s: cpu=%u, L CPU_DOWN_PREPARE\n",
				__func__, cpu);
				dcm_cpu_cluster_stat &= ~DCM_CPU_CLUSTER_L;
				break;
			case 2:
				dcm_pr_dbg(
				"%s: cpu=%u, B CPU_DOWN_PREPARE\n",
				__func__, cpu);
				dcm_cpu_cluster_stat &= ~DCM_CPU_CLUSTER_B;
				break;
			default:
				break;
			}
		}

	return 0;
}

#endif /* #ifdef CONFIG_HOTPLUG_CPU */

void dcm_set_hotplug_nb(void)
{
#ifdef CONFIG_HOTPLUG_CPU

	int ret = 0;

	ret = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "dcm_cb",
				dcm_cpuhp_notify_enter,
				dcm_cpuhp_notify_leave);
	if (ret < 0)
		dcm_pr_info("[%s]: fail to register_cpu_notifier\n",
				__func__);
#endif /* #ifdef CONFIG_HOTPLUG_CPU */
}

short dcm_get_cpu_cluster_stat(void)
{
	return dcm_cpu_cluster_stat;
}
