// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/rtc.h>
#include <linux/wakeup_reason.h>
#include <linux/syscore_ops.h>
#include <mt6877_cond.h>
#include <mtk_lpm_module.h>

#include <aee.h>
#include <mtk_lpm.h>

#include <mt6877_spm_comm.h>
#include <mt6877_spm_reg.h>
#include <mt6877_pwr_ctrl.h>
#include <mt6877_pcm_def.h>
#include <mtk_dbg_common_v1.h>
#include <mt-plat/mtk_ccci_common.h>
#include <mtk_lpm_call.h>
#include <mtk_lpm_call_type.h>
#include <mtk_lpm_timer.h>
#include <mtk_lpm_sysfs.h>
#include <gs/v1/mtk_power_gs.h>

#ifdef OPLUS_FEATURE_CONN_POWER_MONITOR
//add for mtk connectivity power monitor
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#endif /* OPLUS_FEATURE_CONN_POWER_MONITOR */

#define MT6877_LOG_MONITOR_STATE_NAME	"mcusysoff"
#define MT6877_LOG_DEFAULT_MS		5000

#define PCM_32K_TICKS_PER_SEC		(32768)
#define PCM_TICK_TO_SEC(TICK)	(TICK / PCM_32K_TICKS_PER_SEC)

#ifdef CONFIG_MTK_LPM_GS_DUMP_SUPPORT
struct MT6886_LOGGER_NODE mt6877_log_gs_idle;
struct MTK_LPM_GS_IDLE_INFO {
	unsigned short limit;
	unsigned short limit_set;
	unsigned int dump_type;
};
struct MTK_LPM_GS_IDLE_INFO mt6877_log_gs_info;
struct cpumask mtk_lpm_gs_idle_cpumask;

#define IS_MTK_LPM_GS_UPDATE(x)	\
	((cpumask_weight(&mtk_lpm_gs_idle_cpumask)\
	== num_online_cpus()) &&\
	(x.limit != x.limit_set))
#endif

static struct mt6877_spm_wake_status mt6877_wake;
void __iomem *mt6877_spm_base;

struct mt6877_log_helper {
	short cur;
	short prev;
	struct mt6877_spm_wake_status *wakesrc;
};
struct mt6877_log_helper mt6877_logger_help = {
	.wakesrc = &mt6877_wake,
	.cur = 0,
	.prev = 0,
};

static char *mt6877_spm_cond_cg_str[PLAT_SPM_COND_MAX] = {
	[PLAT_SPM_COND_MTCMOS_1]	= "MTCMOS_1",
	[PLAT_SPM_COND_MTCMOS_2]	= "MTCMOS_2",
	[PLAT_SPM_COND_CG_INFRA_0]	= "INFRA_0",
	[PLAT_SPM_COND_CG_INFRA_1]	= "INFRA_1",
	[PLAT_SPM_COND_CG_INFRA_2]	= "INFRA_2",
	[PLAT_SPM_COND_CG_INFRA_3]	= "INFRA_3",
	[PLAT_SPM_COND_CG_INFRA_4]	= "INFRA_4",
	[PLAT_SPM_COND_CG_INFRA_5]	= "INFRA_5",
	[PLAT_SPM_COND_CG_MMSYS_0]	= "MMSYS_0",
	[PLAT_SPM_COND_CG_MMSYS_1]	= "MMSYS_1",
	[PLAT_SPM_COND_CG_MMSYS_2]	= "MMSYS_2",
	[PLAT_SPM_COND_CG_MMSYS_3]      = "MMSYS_3",
	[PLAT_SPM_COND_CG_MMSYS_4]	= "MMSYS_4",
	[PLAT_SPM_COND_CG_MMSYS_5]	= "MMSYS_5",
	[PLAT_SPM_COND_CG_MMSYS_6]	= "MMSYS_6",
	[PLAT_SPM_COND_CG_MMSYS_7]	= "MMSYS_7",
};

const char *mt6877_wakesrc_str[32] = {
	[0] = " R12_PCM_TIMER",
	[1] = " R12_SPM_DEBUG_B",
	[2] = " R12_KP_IRQ_B",
	[3] = " R12_APWDT_EVENT_B",
	[4] = " R12_APXGPT_EVENT_B",
	[5] = " R12_CONN2AP_WAKEUP_B",
	[6] = " R12_EINT_EVENT_B",
	[7] = " R12_CONN_WDT_IRQ_B",
	[8] = " R12_CCIF0_EVENT_B",
	[9] = " R12_LOWBATTERY_IRQ_B",
	[10] = " R12_SSPM2SPM_WAKEUP_B",
	[11] = " R12_SCP2SPM_WAKEUP_B",
	[12] = " R12_ADSP2SPM_WAKEUP_B",
	[13] = " R12_PCM_WDT_WAKEUP_B",
	[14] = " R12_USB0_CDSC_B",
	[15] = " R12_USB0_POWERDWN_B",
	[16] = " R12_SYS_TIMER_EVENT_B",
	[17] = " R12_EINT_EVENT_SECURE_B",
	[18] = " R12_CCIF1_EVENT_B",
	[19] = " R12_UART0_IRQ_B",
	[20] = " R12_AFE_IRQ_MCU_B",
	[21] = " R12_THERM_CTRL_EVENT_B",
	[22] = " R12_SYS_CIRQ_IRQ_B",
	[23] = " R12_MD2AP_PEER_EVENT_B",
	[24] = " R12_CSYSPWREQ_B",
	[25] = " R12_MD1_WDT_B",
	[26] = " R12_AP2AP_PEER_WAKEUP_B",
	[27] = " R12_SEJ_EVENT_B",
	[28] = " R12_CPU_WAKEUPEVENT_B",
	[29] = " R12_APUSYS",
	[30] = " R12_CPU_IRQOUT_B",
	[31] = " R12_MCUSYS_IDLE_B",
};

struct spm_wakesrc_irq_list mt6877_spm_wakesrc_irqs[] = {
	/* mtk-kpd */
	{ WAKE_SRC_STA1_KP_IRQ_B, "mediatek,kp", 0, 0},
	/* mt_wdt */
	{ WAKE_SRC_STA1_APWDT_EVENT_B, "mediatek,toprgu", 0, 0},
	/* bt_cvsd_int */
	{ WAKE_SRC_STA1_CONN2AP_SPM_WAKEUP_B, "mediatek,mtk-btcvsd-snd", 0, 0},
	/* wf_hif_int */
	{ WAKE_SRC_STA1_CONN2AP_SPM_WAKEUP_B, "mediatek,wifi", 0, 0},
	/* conn2ap_btif_wakeup_out */
	{ WAKE_SRC_STA1_CONN2AP_SPM_WAKEUP_B, "mediatek,bt", 0, 0},
	/* conn2ap_sw_irq */
	{ WAKE_SRC_STA1_CONN2AP_SPM_WAKEUP_B, "mediatek,bt", 1, 0},
	/* CCIF_AP_DATA */
	{ WAKE_SRC_STA1_CCIF0_EVENT_B, "mediatek,ap_ccif0", 0, 0},
	/* SCP IPC0 */
	{ WAKE_SRC_STA1_SC_SCP2SPM_WAKEUP_B, "mediatek,scp", 0, 0},
	/* SCP IPC1 */
	{ WAKE_SRC_STA1_SC_SCP2SPM_WAKEUP_B, "mediatek,scp", 1, 0},
	/* MBOX_ISR0 */
	{ WAKE_SRC_STA1_SC_SCP2SPM_WAKEUP_B, "mediatek,scp", 2, 0},
	/* MBOX_ISR1 */
	{ WAKE_SRC_STA1_SC_SCP2SPM_WAKEUP_B, "mediatek,scp", 3, 0},
	/* MBOX_ISR2 */
	{ WAKE_SRC_STA1_SC_SCP2SPM_WAKEUP_B, "mediatek,scp", 4, 0},
	/* MBOX_ISR3 */
	{ WAKE_SRC_STA1_SC_SCP2SPM_WAKEUP_B, "mediatek,scp", 5, 0},
	/* MBOX_ISR4 */
	{ WAKE_SRC_STA1_SC_SCP2SPM_WAKEUP_B, "mediatek,scp", 6, 0},
	/* ADSP_A_AUD */
	{ WAKE_SRC_STA1_SC_ADSP2SPM_WAKEUP_B, "mediatek,adsp_core_0", 2, 0},
	/* CCIF0_AP */
	{ WAKE_SRC_STA1_MD1_WDT_B, "mediatek,mddriver", 2, 0},
	/* DPMAIF_AP */
	{ WAKE_SRC_STA1_AP2AP_PEER_WAKEUPEVENT_B, "mediatek,dpmaif", 0, 0},
};

#define plat_mmio_read(offset)	__raw_readl(mt6877_spm_base + offset)
u64 ap_pd_count;
u64 ap_slp_duration;
u64 spm_26M_off_count;
u64 spm_26M_off_duration;
u32 before_ap_slp_duration;

#ifdef OPLUS_FEATURE_CONN_POWER_MONITOR
//add for mtk connectivity power monitor
#define RET_ERR  1
#define RET_OK  0
#define OPLUS_26M_PCT_GAP	50
#define OPLUS_TIME_GAP	PCM_32K_TICKS_PER_SEC

static struct miscdevice lpm_object;
static struct workqueue_struct *mQueue = NULL;
static struct delayed_work mWork;
static char mUevent[256] = {'\0'};
static unsigned long mWakeupLast = OPLUS_TIME_GAP;

static void oplusWorkHandler(struct work_struct *data)
{
	char *envp[2];

	if (mUevent[0] == '\0')
		return;

	envp[0] = mUevent;
	envp[1] = NULL;
	kobject_uevent_env(
		&lpm_object.this_device->kobj,
		KOBJ_CHANGE, envp);
}

int oplusLpmUeventInit(void)
{
        u_int8_t ret = RET_OK;

        if (lpm_object.this_device != NULL) {
                return RET_OK;
        }
        lpm_object.name = "lpm";
        lpm_object.minor = MISC_DYNAMIC_MINOR;
        ret = misc_register(&lpm_object);
        if (ret == RET_OK) {
                ret = kobject_uevent(&lpm_object.this_device->kobj, KOBJ_ADD);
                if (ret != RET_OK) {
                        misc_deregister(&lpm_object);
                        return RET_ERR;
                }
        }
	if (mQueue == NULL) {
		mQueue = create_singlethread_workqueue("oplus_conn_uevent");
		INIT_DELAYED_WORK(&mWork, oplusWorkHandler);
	}
        return ret;
}

int oplusLpmSendUevent(const char *src)
{
        if (lpm_object.this_device == NULL) {
                return RET_ERR;
        }
	if (src == NULL) {
		return RET_ERR;
	}

	strlcpy(mUevent, src, sizeof(mUevent));
	if (mQueue) {
		queue_delayed_work(mQueue, &mWork, msecs_to_jiffies(200));
	} else {
		schedule_delayed_work(&mWork, msecs_to_jiffies(200));
	}

	return RET_OK;
}
#endif /* OPLUS_FEATURE_CONN_POWER_MONITOR */

#define IRQ_NUMBER	\
(sizeof(mt6877_spm_wakesrc_irqs)/sizeof(struct spm_wakesrc_irq_list))
static void mt6877_get_spm_wakesrc_irq(void)
{
	int i;
	struct device_node *node = NULL;

	for (i = 0; i < IRQ_NUMBER; i++) {
		if (mt6877_spm_wakesrc_irqs[i].name == NULL)
			continue;

		node = of_find_compatible_node(NULL, NULL,
			mt6877_spm_wakesrc_irqs[i].name);
		if (!node) {
			pr_info("[name:spm&][SPM] find '%s' node failed\n",
				mt6877_spm_wakesrc_irqs[i].name);
			continue;
		}

		mt6877_spm_wakesrc_irqs[i].irq_no =
			irq_of_parse_and_map(node,
				mt6877_spm_wakesrc_irqs[i].order);

		if (!mt6877_spm_wakesrc_irqs[i].irq_no) {
			pr_info("[name:spm&][SPM] get '%s' failed\n",
				mt6877_spm_wakesrc_irqs[i].name);
		}
	}
}

struct mt6877_logger_timer {
	struct mt_lpm_timer tm;
	unsigned int fired;
};
struct mt6877_logger_fired_info {
	unsigned int fired;
	int state_index;
};

static struct mt6877_logger_timer mt6877_log_timer;
static struct mt6877_logger_fired_info mt6877_logger_fired;

int mt6877_get_wakeup_status(struct mt6877_log_helper *help)
{
	if (!help->wakesrc || !mt6877_spm_base)
		return -EINVAL;

	help->wakesrc->r12 = plat_mmio_read(SPM_BK_WAKE_EVENT);
	help->wakesrc->r12_ext = plat_mmio_read(SPM_WAKEUP_STA);
	help->wakesrc->raw_sta = plat_mmio_read(SPM_WAKEUP_STA);
	help->wakesrc->raw_ext_sta = plat_mmio_read(SPM_WAKEUP_EXT_STA);
	help->wakesrc->md32pcm_wakeup_sta = plat_mmio_read(MD32PCM_WAKEUP_STA);
	help->wakesrc->md32pcm_event_sta = plat_mmio_read(MD32PCM_EVENT_STA);

	help->wakesrc->src_req = plat_mmio_read(SPM_SRC_REQ);

	/* backup of SPM_WAKEUP_MISC */
	help->wakesrc->wake_misc = plat_mmio_read(SPM_BK_WAKE_MISC);

	/* get sleep time */
	/* backup of PCM_TIMER_OUT */
	help->wakesrc->timer_out = plat_mmio_read(SPM_BK_PCM_TIMER);

	/* get other SYS and co-clock status */
	help->wakesrc->r13 = plat_mmio_read(MD32PCM_SCU_STA0);
	help->wakesrc->idle_sta = plat_mmio_read(SUBSYS_IDLE_STA);
	help->wakesrc->req_sta0 = plat_mmio_read(SPM_REQ_STA_0);
	help->wakesrc->req_sta1 = plat_mmio_read(SPM_REQ_STA_1);
	help->wakesrc->req_sta2 = plat_mmio_read(SPM_REQ_STA_2);
	help->wakesrc->req_sta3 = plat_mmio_read(SPM_REQ_STA_3);
	help->wakesrc->req_sta4 = plat_mmio_read(SPM_REQ_STA_4);
	help->wakesrc->req_sta5 = plat_mmio_read(SPM_REQ_STA_5);

	/* get HW CG check status */
	help->wakesrc->cg_check_sta = (plat_mmio_read(SPM_REQ_STA_2) & 0x1F);

	/* get debug flag for PCM execution check */
	help->wakesrc->debug_flag = plat_mmio_read(PCM_WDT_LATCH_SPARE_0);
	help->wakesrc->debug_flag1 = plat_mmio_read(PCM_WDT_LATCH_SPARE_1);

	/* get backup SW flag status */
	help->wakesrc->b_sw_flag0 = plat_mmio_read(SPM_SW_RSV_7);
	help->wakesrc->b_sw_flag1 = plat_mmio_read(SPM_SW_RSV_8);

	help->wakesrc->rt_req_sta0 = plat_mmio_read(SPM_SW_RSV_2);
	help->wakesrc->rt_req_sta1 = plat_mmio_read(SPM_SW_RSV_3);
	help->wakesrc->rt_req_sta2 = plat_mmio_read(SPM_SW_RSV_4);
	help->wakesrc->rt_req_sta3 = plat_mmio_read(SPM_SW_RSV_5);
	help->wakesrc->rt_req_sta4 = plat_mmio_read(SPM_SW_RSV_6);
	/* get ISR status */
	help->wakesrc->isr = plat_mmio_read(SPM_IRQ_STA);

	/* get SW flag status */
	help->wakesrc->sw_flag0 = plat_mmio_read(SPM_SW_FLAG_0);
	help->wakesrc->sw_flag1 = plat_mmio_read(SPM_SW_FLAG_1);

	/* get CLK SETTLE */
	help->wakesrc->clk_settle = plat_mmio_read(SPM_CLK_SETTLE);
	/* check abort */
	help->wakesrc->is_abort =
		help->wakesrc->debug_flag & DEBUG_ABORT_MASK;
	help->wakesrc->is_abort |=
		help->wakesrc->debug_flag1 & DEBUG_ABORT_MASK_1;

	help->cur += 1;
	return 0;
}

static void mt6877_save_sleep_info(void)
{
#define AVOID_OVERFLOW (0xFFFFFFFF00000000)
	u32 off_26M_duration;
	u32 slp_duration;

	slp_duration = plat_mmio_read(SPM_BK_PCM_TIMER);
	if (slp_duration == before_ap_slp_duration)
		return;

	/* Save ap off counter and duration */
	if (ap_pd_count >= AVOID_OVERFLOW)
		ap_pd_count = 0;
	else
		ap_pd_count++;

	if (ap_slp_duration >= AVOID_OVERFLOW)
		ap_slp_duration = 0;
	else {
		ap_slp_duration = ap_slp_duration + slp_duration;
		before_ap_slp_duration = slp_duration;
	}

	/* Save 26M's off counter and duration */
	if (spm_26M_off_duration >= AVOID_OVERFLOW)
		spm_26M_off_duration = 0;
	else {
		off_26M_duration = plat_mmio_read(SPM_BK_VTCXO_DUR);
		if (off_26M_duration == 0)
			return;

		spm_26M_off_duration = spm_26M_off_duration +
			off_26M_duration;
	}

	if (spm_26M_off_count >= AVOID_OVERFLOW)
		spm_26M_off_count = 0;
	else
		spm_26M_off_count = (plat_mmio_read(SPM_SRCCLKENA_EVENT_COUNT_STA)
					& 0xffff)
			+ spm_26M_off_count;
}

static void mt6877_suspend_show_detailed_wakeup_reason
	(struct mt6877_spm_wake_status *wakesta)
{
	int i, ret;
	unsigned int irq_no;
	bool state;

#if !defined(CONFIG_FPGA_EARLY_PORTING)
#ifdef CONFIG_MTK_CCCI_DEVICES
	exec_ccci_kern_func_by_md_id(0, ID_DUMP_MD_SLEEP_MODE,
		NULL, 0);
#endif

#ifdef CONFIG_MTK_ECCCI_DRIVER
	if (wakesta->r12 & R12_AP2AP_PEER_WAKEUP_B)
		exec_ccci_kern_func_by_md_id(0, ID_GET_MD_WAKEUP_SRC,
		NULL, 0);
	if (wakesta->r12 & R12_MD2AP_PEER_EVENT_B)
		exec_ccci_kern_func_by_md_id(0, ID_GET_MD_WAKEUP_SRC,
		NULL, 0);
	if (wakesta->r12 & R12_CCIF0_EVENT_B)
		exec_ccci_kern_func_by_md_id(0, ID_GET_MD_WAKEUP_SRC,
		NULL, 0);
	if (wakesta->r12 & R12_CCIF1_EVENT_B)
		exec_ccci_kern_func_by_md_id(2, ID_GET_MD_WAKEUP_SRC,
		NULL, 0);
#endif
#endif
	/* Check if pending irq happen and log them */
	for (i = 0; i < IRQ_NUMBER; i++) {
		if (mt6877_spm_wakesrc_irqs[i].name == NULL ||
			!mt6877_spm_wakesrc_irqs[i].irq_no)
			continue;
		if (mt6877_spm_wakesrc_irqs[i].wakesrc & wakesta->r12) {
			irq_no = mt6877_spm_wakesrc_irqs[i].irq_no;
			ret = irq_get_irqchip_state(irq_no, IRQCHIP_STATE_PENDING, &state);

			if (ret < 0) {
				printk_deferred("failed to read irq pending\n");
				continue;
			}
			if (state)
				log_irq_wakeup_reason(irq_no);
		}
	}
}

static void dump_lp_cond(void)
{
#define MT6877_DBG_SMC(_id, _act, _rc, _param) ({\
	(u32) mtk_lpm_smc_spm_dbg(_id, _act, _rc, _param); })

	int i;
	u32 blkcg;

	for (i = 1 ; i < PLAT_SPM_COND_MAX ; i++) {
		blkcg = MT6877_DBG_SMC(MT_SPM_DBG_SMC_UID_BLOCK_DETAIL, MT_LPM_SMC_ACT_GET, 0, i);
		if (blkcg != 0)
			printk_deferred("suspend warning: CG: %6s = 0x%08lx\n"
				, mt6877_spm_cond_cg_str[i], blkcg);

	}
}

static void mt6877_suspend_spm_rsc_req_check
	(struct mt6877_spm_wake_status *wakesta)
{
#define LOG_BUF_SIZE		        256
#define IS_BLOCKED_OVER_TIMES		10
#undef AVOID_OVERFLOW
#define AVOID_OVERFLOW (0xF0000000)
static u32 is_blocked_cnt;
	char log_buf[LOG_BUF_SIZE] = { 0 };
	int log_size = 0;
	u32 is_no_blocked = 0;
	u32 req_sta_0, req_sta_1, req_sat_ccif, req_sta_2, req_sta_3, req_sta_4, req_sta_5;
	u32 src_req;

	if (is_blocked_cnt >= AVOID_OVERFLOW)
		is_blocked_cnt = 0;

	/* Check if ever enter deepest System LPM */
	is_no_blocked = wakesta->debug_flag & 0x2;

	/* Check if System LPM ever is blocked over 10 times */
	if (!is_no_blocked)
		is_blocked_cnt++;
	else
		is_blocked_cnt = 0;

	if (is_blocked_cnt < IS_BLOCKED_OVER_TIMES)
		return;

	/* Show who is blocking system LPM */
	log_size += scnprintf(log_buf + log_size,
		LOG_BUF_SIZE - log_size,
		"suspend warning:(OneShot) System LPM is blocked by ");

	req_sta_0 = plat_mmio_read(SPM_REQ_STA_0);
	req_sta_1 = plat_mmio_read(SPM_REQ_STA_1);
	req_sta_2 = plat_mmio_read(SPM_REQ_STA_2);
	req_sta_3 = plat_mmio_read(SPM_REQ_STA_3);
	req_sta_4 = plat_mmio_read(SPM_REQ_STA_4);
	req_sta_5 = plat_mmio_read(SPM_REQ_STA_5);

	if (req_sta_0 & (0x1F << 5))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "apu ");

	if (req_sta_0 & (0x1F << 10))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "adsp ");

	req_sat_ccif = req_sta_1 & 0xFFF;
	if (req_sat_ccif < 0xFFF)
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "ccif ");

	if (req_sta_2 & (0x3F << 5))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "conn ");

	if (req_sta_2 & (0xF << 11))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "disp ");

	if (req_sta_2 & (0x1F << 15))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "dpmaif ");

	if (req_sta_2 & (0xF << 27))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "gce ");

	if ((req_sta_3 & (0x1 << 31)) || (req_sta_4 & 0x1F))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "md ");

	if (req_sta_4 & (0x7FFF << 8))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "msdc ");

	if ((req_sta_4 & (0xF << 28)) || (req_sta_5 & 0x1))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "scp ");

	if (req_sta_5 & (0x3 << 18))
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "ufs ");

	src_req = plat_mmio_read(SPM_SRC_REQ);
	if (src_req & 0x436) {
		dump_lp_cond();
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_SIZE - log_size, "spm ");
	}

	WARN_ON(strlen(log_buf) >= LOG_BUF_SIZE);

	printk_deferred("[name:spm&][SPM] %s", log_buf);
}

static int mt6877_show_message(struct mt6877_spm_wake_status *wakesrc, int type,
					const char *prefix, void *data)
{
#undef LOG_BUF_SIZE
	#define LOG_BUF_SIZE		256
	#define LOG_BUF_OUT_SZ		768
	#define IS_WAKE_MISC(x)	(wakesrc->wake_misc & x)
	#define IS_LOGBUF(ptr, newstr) \
		((strlen(ptr) + strlen(newstr)) < LOG_BUF_SIZE)

	int i;
	unsigned int spm_26M_off_pct = 0;
	char buf[LOG_BUF_SIZE] = { 0 };
	char log_buf[LOG_BUF_OUT_SZ] = { 0 };
	char *local_ptr = NULL;
	int log_size = 0;
	unsigned int wr = WR_UNKNOWN;
	const char *scenario = prefix ?: "UNKNOWN";

	/* Disable rcu lock checking */
	if (type == MT_LPM_ISSUER_SUSPEND)
		rcu_irq_enter_irqson();

	if (wakesrc->is_abort != 0) {
		/* add size check for vcoredvfs */
		aee_sram_printk("SPM ABORT (%s), r13 = 0x%x, ",
			scenario, wakesrc->r13);
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_OUT_SZ - log_size,
			"[SPM] ABORT (%s), r13 = 0x%x, ",
			scenario, wakesrc->r13);

		aee_sram_printk(" debug_flag = 0x%x 0x%x",
			wakesrc->debug_flag, wakesrc->debug_flag1);
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_OUT_SZ - log_size,
			" debug_flag = 0x%x 0x%x",
			wakesrc->debug_flag, wakesrc->debug_flag1);

		aee_sram_printk(" sw_flag = 0x%x 0x%x",
			wakesrc->sw_flag0, wakesrc->sw_flag1);
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_OUT_SZ - log_size,
			" sw_flag = 0x%x 0x%x\n",
			wakesrc->sw_flag0, wakesrc->sw_flag1);

		aee_sram_printk(" b_sw_flag = 0x%x 0x%x",
			wakesrc->b_sw_flag0, wakesrc->b_sw_flag1);
		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_OUT_SZ - log_size,
			" b_sw_flag = 0x%x 0x%x",
			wakesrc->b_sw_flag0, wakesrc->b_sw_flag1);

		wr =  WR_ABORT;
	} else {
		if (wakesrc->r12 & R12_PCM_TIMER_B) {
			if (wakesrc->wake_misc & WAKE_MISC_PCM_TIMER_EVENT) {
				local_ptr = " PCM_TIMER";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_PCM_TIMER;
			}
		}

		if (wakesrc->r12 & R12_SPM_DEBUG_B) {
			if (IS_WAKE_MISC(WAKE_MISC_DVFSRC_IRQ)) {
				local_ptr = " DVFSRC";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_DVFSRC;
			}
			if (IS_WAKE_MISC(WAKE_MISC_TWAM_IRQ_B)) {
				local_ptr = " TWAM";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_TWAM;
			}
			if (IS_WAKE_MISC(WAKE_MISC_PMSR_IRQ_B_SET0)) {
				local_ptr = " PMSR";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_PMSR;
			}
			if (IS_WAKE_MISC(WAKE_MISC_PMSR_IRQ_B_SET1)) {
				local_ptr = " PMSR";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_PMSR;
			}
			if (IS_WAKE_MISC(WAKE_MISC_SPM_ACK_CHK_WAKEUP_0)) {
				local_ptr = " SPM_ACK_CHK";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_SPM_ACK_CHK;
			}
			if (IS_WAKE_MISC(WAKE_MISC_SPM_ACK_CHK_WAKEUP_1)) {
				local_ptr = " SPM_ACK_CHK";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_SPM_ACK_CHK;
			}
			if (IS_WAKE_MISC(WAKE_MISC_SPM_ACK_CHK_WAKEUP_2)) {
				local_ptr = " SPM_ACK_CHK";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_SPM_ACK_CHK;
			}
			if (IS_WAKE_MISC(WAKE_MISC_SPM_ACK_CHK_WAKEUP_3)) {
				local_ptr = " SPM_ACK_CHK";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_SPM_ACK_CHK;
			}
			if (IS_WAKE_MISC(WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL)) {
				local_ptr = " SPM_ACK_CHK";
				if (IS_LOGBUF(buf, local_ptr))
					strncat(buf, local_ptr,
						strlen(local_ptr));
				wr = WR_SPM_ACK_CHK;
			}
		}
		for (i = 1; i < 32; i++) {
			if (wakesrc->r12 & (1U << i)) {
				if (IS_LOGBUF(buf, mt6877_wakesrc_str[i]))
					strncat(buf, mt6877_wakesrc_str[i],
						strlen(mt6877_wakesrc_str[i]));

				wr = WR_WAKE_SRC;
			}
		}
		WARN_ON(strlen(buf) >= LOG_BUF_SIZE);

		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_OUT_SZ - log_size,
			"%s wake up by %s, timer_out = %u, r13 = 0x%x, debug_flag = 0x%x 0x%x, ",
			scenario, buf, wakesrc->timer_out, wakesrc->r13,
			wakesrc->debug_flag, wakesrc->debug_flag1);

		log_size += scnprintf(log_buf + log_size,
			LOG_BUF_OUT_SZ - log_size,
			"r12 = 0x%x, r12_ext = 0x%x, raw_sta = 0x%x 0x%x 0x%x, idle_sta = 0x%x, ",
			wakesrc->r12, wakesrc->r12_ext,
			wakesrc->raw_sta,
			wakesrc->md32pcm_wakeup_sta,
			wakesrc->md32pcm_event_sta,
			wakesrc->idle_sta);

		log_size += scnprintf(log_buf + log_size,
			  LOG_BUF_OUT_SZ - log_size,
			  "req_sta =  0x%x 0x%x 0x%x 0x%x 0x%x 0x%x, cg_check_sta =0x%x, isr = 0x%x, rt_req_sta0 = 0x%x rt_req_sta1 = 0x%x rt_req_sta2 = 0x%x rt_req_sta3 = 0x%x dram_sw_con_3 = 0x%x, ",
			  wakesrc->req_sta0, wakesrc->req_sta1,
			  wakesrc->req_sta2, wakesrc->req_sta3,
			  wakesrc->req_sta4, wakesrc->req_sta5, wakesrc->cg_check_sta,
			  wakesrc->isr, wakesrc->rt_req_sta0,
			  wakesrc->rt_req_sta1, wakesrc->rt_req_sta2,
			  wakesrc->rt_req_sta3, wakesrc->rt_req_sta4);

		log_size += scnprintf(log_buf + log_size,
				LOG_BUF_OUT_SZ - log_size,
				"raw_ext_sta = 0x%x, wake_misc = 0x%x, pcm_flag = 0x%x 0x%x 0x%x 0x%x, req = 0x%x, ",
				wakesrc->raw_ext_sta,
				wakesrc->wake_misc,
				wakesrc->sw_flag0,
				wakesrc->sw_flag1, wakesrc->b_sw_flag0,
				wakesrc->b_sw_flag0,
				wakesrc->src_req);

		log_size += scnprintf(log_buf + log_size,
				LOG_BUF_OUT_SZ - log_size,
				" clk_settle = 0x%x, ", wakesrc->clk_settle);

		if (type == MT_LPM_ISSUER_SUSPEND) {
			/* calculate 26M off percentage in suspend period */
			if (wakesrc->timer_out != 0) {
				spm_26M_off_pct =
					(100 * plat_mmio_read(SPM_BK_VTCXO_DUR))
							/ wakesrc->timer_out;
			}
			log_size += scnprintf(log_buf + log_size,
				LOG_BUF_OUT_SZ - log_size,
				"wlk_cntcv_l = 0x%x, wlk_cntcv_h = 0x%x, 26M_off_pct = %d\n",
				plat_mmio_read(SYS_TIMER_VALUE_L),
				plat_mmio_read(SYS_TIMER_VALUE_H),
				spm_26M_off_pct);

#ifdef OPLUS_FEATURE_CONN_POWER_MONITOR
			//add for mtk connectivity power monitor
			if ((wakesrc->r13 & (R13_CONN_DDR_EN|R13_CONN_STATE|R13_CONN_SRCCKENA|R13_CONN_APSRC_REQ|R13_CONN_SRCCLKENB))
				&& (spm_26M_off_pct < OPLUS_26M_PCT_GAP)) {
				mWakeupLast += wakesrc->timer_out;
				if (mWakeupLast >= OPLUS_TIME_GAP) {
					char uevent[100] = {'\0'};
					snprintf(uevent, sizeof(uevent), "lpm=src:%s;r13:%d;timer_out:%d;26M_off_pct:%d;", buf, wakesrc->r13, wakesrc->timer_out, spm_26M_off_pct);
					oplusLpmSendUevent(uevent);
					mWakeupLast = 0;
				}
			}
#endif /* OPLUS_FEATURE_CONN_POWER_MONITOR */
		}
	}
	WARN_ON(log_size >= LOG_BUF_OUT_SZ);

	if (type == MT_LPM_ISSUER_SUSPEND) {
		printk_deferred("[name:spm&][SPM] %s", log_buf);
		mt6877_suspend_show_detailed_wakeup_reason(wakesrc);
		mt6877_suspend_spm_rsc_req_check(wakesrc);

		printk_deferred("[name:spm&][SPM] Suspended for %d.%03d seconds",
			PCM_TICK_TO_SEC(wakesrc->timer_out),
			PCM_TICK_TO_SEC((wakesrc->timer_out %
				PCM_32K_TICKS_PER_SEC)
			* 1000));

		/* Eable rcu lock checking */
		rcu_irq_exit_irqson();
	} else
		pr_info("[name:spm&][SPM] %s", log_buf);

	return wr;
}

int mt6877_issuer_func(int type, const char *prefix, void *data)
{
	mt6877_get_wakeup_status(&mt6877_logger_help);
	return mt6877_show_message(mt6877_logger_help.wakesrc,
					type, prefix, data);
}

struct mtk_lpm_issuer mt6877_issuer = {
	.log = mt6877_issuer_func,
};

static int mt6877_idle_save_sleep_info_nb_func(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct mtk_lpm_nb_data *nb_data = (struct mtk_lpm_nb_data *)data;

	if (nb_data && (action == MTK_LPM_NB_BEFORE_REFLECT))
		mt6877_save_sleep_info();

	return NOTIFY_OK;
}

struct notifier_block mt6877_idle_save_sleep_info_nb = {
	.notifier_call = mt6877_idle_save_sleep_info_nb_func,
};

static void mt6877_suspend_save_sleep_info_func(void)
{
	mt6877_save_sleep_info();
}

static struct syscore_ops mt6877_suspend_save_sleep_info_syscore_ops = {
	.resume = mt6877_suspend_save_sleep_info_func,
};
static int mt6877_log_timer_func(unsigned long long dur, void *priv)
{
	struct mt6877_logger_timer *timer =
			(struct mt6877_logger_timer *)priv;
	struct mt6877_logger_fired_info *info = &mt6877_logger_fired;

	if (timer->fired != info->fired) {
		/* if the wake src had beed update before
		 * then won't do wake src update
		 */
		if (mt6877_logger_help.prev == mt6877_logger_help.cur)
			mt6877_get_wakeup_status(&mt6877_logger_help);
		mt6877_show_message(mt6877_logger_help.wakesrc,
					MT_LPM_ISSUER_CPUIDLE,
					"MCUSYSOFF", NULL);
		mt6877_logger_help.prev = mt6877_logger_help.cur;
	} else
		pr_info("[name:spm&][SPM] MCUSYSOFF Didn't enter low power scenario\n");

	timer->fired = info->fired;
	return 0;
}

#ifdef CONFIG_MTK_LPM_GS_DUMP_SUPPORT
struct mtk_lpm_gs_idleinfo {
	unsigned int type[2];
};
#endif

static int mt6877_logger_nb_func(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct mtk_lpm_nb_data *nb_data = (struct mtk_lpm_nb_data *)data;
	struct mt6877_logger_fired_info *info = &mt6877_logger_fired;

	if (nb_data && (action == MTK_LPM_NB_BEFORE_REFLECT)
	    && (nb_data->index == info->state_index)) {
		info->fired++;
#ifdef CONFIG_MTK_LPM_GS_DUMP_SUPPORT
		cpumask_clear_cpu(nb_data->cpu, &mtk_lpm_gs_idle_cpumask);
#endif
	}

#ifdef CONFIG_MTK_LPM_GS_DUMP_SUPPORT
	if (nb_data && (action == MTK_LPM_NB_AFTER_PROMPT)
	    && (nb_data->index == info->state_index)) {
		cpumask_set_cpu(nb_data->cpu, &mtk_lpm_gs_idle_cpumask);

		if (IS_MTK_LPM_GS_UPDATE(mt6877_log_gs_info)) {
			struct mtk_lpm_callee_simple *callee = NULL;
			struct mtk_lpm_data val;

			val.d.u32 = mt6877_log_gs_info.dump_type;
			if (!mtk_lpm_callee_get(MTK_LPM_CALLEE_PWR_GS, &callee))
				callee->set(MTK_LPM_PWR_GS_TYPE_VCORELP_26M, &val);

			mt6877_log_gs_info.limit =
				mt6877_log_gs_info.limit_set;
		}
	}
#endif
	return NOTIFY_OK;
}

struct notifier_block mt6877_logger_nb = {
	.notifier_call = mt6877_logger_nb_func,
};

static ssize_t mt6877_logger_debugfs_read(char *ToUserBuf,
					size_t sz, void *priv)
{
	char *p = ToUserBuf;
	int len;

	if (priv == ((void *)&mt6877_log_timer)) {
		len = scnprintf(p, sz, "%lu\n",
			mtk_lpm_timer_interval(&mt6877_log_timer.tm));
		p += len;
	}
#ifdef CONFIG_MTK_LPM_GS_DUMP_SUPPORT
	else if (priv == ((void *)&mt6877_log_gs_info)) {
		len = scnprintf(p, sz, "golden_type: %u\n",
				mt6877_log_gs_info.dump_type);
		p += len;
	}
#endif
	return (p - ToUserBuf);
}

static ssize_t mt6877_logger_debugfs_write(char *FromUserBuf,
				   size_t sz, void *priv)
{
	if (priv == ((void *)&mt6877_log_timer)) {
		unsigned int val = 0;

		if (!kstrtouint(FromUserBuf, 10, &val)) {
			if (val == 0)
				mtk_lpm_timer_stop(&mt6877_log_timer.tm);
			else
				mtk_lpm_timer_interval_update(
						&mt6877_log_timer.tm, val);
		}
	}
#ifdef CONFIG_MTK_LPM_GS_DUMP_SUPPORT
	else if (priv == ((void *)&mt6877_log_gs_info)) {
		char cmd[64];
		unsigned int param;

		memset(cmd, 0, sizeof(cmd));
		if (sscanf(FromUserBuf, "%20s %u", cmd, &param) == 2) {
			if (!strcmp(cmd, "golden_dump")) {
				if (param)
					mt6877_log_gs_info.limit_set += 1;
			} else if (!strcmp(cmd, "golden_type"))
				mt6877_log_gs_info.dump_type = param;
		}
	}
#endif
	return sz;
}

struct MT6886_LOGGER_NODE {
	struct mtk_lp_sysfs_handle handle;
	struct mtk_lp_sysfs_op op;
};
#define MT6877_LOGGER_NODE_INIT(_n, _priv) ({\
	_n.op.fs_read = mt6877_logger_debugfs_read;\
	_n.op.fs_write = mt6877_logger_debugfs_write;\
	_n.op.priv = _priv; })\


struct mtk_lp_sysfs_handle mt6877_log_tm_node;
struct MT6886_LOGGER_NODE mt6877_log_tm_interval;

int mt6877_logger_timer_debugfs_init(void)
{
	mtk_lpm_sysfs_sub_entry_add("logger", 0644,
				NULL, &mt6877_log_tm_node);

	MT6877_LOGGER_NODE_INIT(mt6877_log_tm_interval,
				&mt6877_log_timer);
	mtk_lpm_sysfs_sub_entry_node_add("interval", 0644,
				&mt6877_log_tm_interval.op,
				&mt6877_log_tm_node,
				&mt6877_log_tm_interval.handle);
	return 0;
}

int __init mt6877_logger_init(void)
{
	struct device_node *node = NULL;
	struct cpuidle_driver *drv = NULL;
	struct cpuidle_device *dev = NULL;
#ifdef OPLUS_FEATURE_CONN_POWER_MONITOR
	//add for mtk connectivity power monitor
	int ret = 0;
#endif /* OPLUS_FEATURE_CONN_POWER_MONITOR */

	node = of_find_compatible_node(NULL, NULL, "mediatek,sleep");

	if (node) {
		mt6877_spm_base = of_iomap(node, 0);
		of_node_put(node);
	}

	if (mt6877_spm_base)
		mtk_lp_issuer_register(&mt6877_issuer);
	else
		pr_info("[name:mtk_lpm][P] - Don't register the issue by error! (%s:%d)\n",
			__func__, __LINE__);

#ifdef OPLUS_FEATURE_CONN_POWER_MONITOR
	//add for mtk connectivity power monitor
	ret = oplusLpmUeventInit();
	if (ret != 0) {
		pr_info("lpmUventInit error");
	}
#endif /* OPLUS_FEATURE_CONN_POWER_MONITOR */

	mt6877_get_spm_wakesrc_irq();
	mtk_lpm_sysfs_root_entry_create();
	mt6877_logger_timer_debugfs_init();

	dev = cpuidle_get_device();
	drv = cpuidle_get_cpu_driver(dev);
	mt6877_logger_fired.state_index = -1;

	if (drv) {
		int idx;

		for (idx = 0; idx < drv->state_count; ++idx) {
			if (!strcmp(MT6877_LOG_MONITOR_STATE_NAME,
					drv->states[idx].name)) {
				mt6877_logger_fired.state_index = idx;
				break;
			}
		}
	}

	mtk_lpm_notifier_register(&mt6877_logger_nb);
	mtk_lpm_notifier_register(&mt6877_idle_save_sleep_info_nb);

	mt6877_log_timer.tm.timeout = mt6877_log_timer_func;
	mt6877_log_timer.tm.priv = &mt6877_log_timer;
	mtk_lpm_timer_init(&mt6877_log_timer.tm, MTK_LPM_TIMER_REPEAT);
	mtk_lpm_timer_interval_update(&mt6877_log_timer.tm,
					MT6877_LOG_DEFAULT_MS);
	mtk_lpm_timer_start(&mt6877_log_timer.tm);

	register_syscore_ops(&mt6877_suspend_save_sleep_info_syscore_ops);

	return 0;
}
late_initcall_sync(mt6877_logger_init);

