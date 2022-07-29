// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017 MediaTek Inc.
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <asm/setup.h>
#include <mtk_spm_internal.h>
#include <mtk_power_gs_api.h>

#ifdef OPLUS_FEATURE_POWERINFO_STANDBY
#include "../../../../../../base/power/owakelock/oppo_wakelock_profiler_mtk.h"
#include "mtk_spm_suspend_internal.h"
#include "pcm_def.h"
static int wakeup_state;
#endif/*OPLUS_FEATURE_POWERINFO_STANDBY*/


#define WORLD_CLK_CNTCV_L        (0x10017008)
#define WORLD_CLK_CNTCV_H        (0x1001700C)
static u32 pcm_timer_ramp_max_sec_loop = 1;
u64 ap_pd_count;
u64 ap_slp_duration;

u64 spm_26M_off_count;
u64 spm_26M_off_duration;

char *wakesrc_str[32] = {
	[0] = " R12_PCM_TIMER_EVENT",
	[1] = " R12_SPM_TWAM_IRQ_B",
	[2] = " R12_KP_IRQ_B",
	[3] = " R12_APWDT_EVENT_B",
	[4] = " R12_APXGPT1_EVENT_B",
	[5] = " R12_CONN2AP_SPM_WAKEUP_B",
	[6] = " R12_EINT_EVENT_B",
	[7] = " R12_CONN_WDT_IRQ_B",
	[8] = " R12_CCIF0_EVENT_B",
	[9] = " R12_LOWBATTERY_IRQ_B",
	[10] = " R12_SC_SSPM2SPM_WAKEUP",
	[11] = " R12_SC_SCP2SPM_WAKEUP",
	[12] = " R12_SC_ADSP2SPM_WAKEUP",
	[13] = " R12_PCM_WDT_EVENT_B",
	[14] = " R12_USBX_CDSC_B",
	[15] = " R12_USBX_POWERDWN_B",
	[16] = " R12_SYS_TIMER_EVENT_B",
	[17] = " R12_EINT_EVENT_SECURE_B",
	[18] = " R12_CCIF1_EVENT_B",
	[19] = " R12_UART0_IRQ_B",
	[20] = " R12_AFE_IRQ_MCU_B",
	[21] = " R12_THERMAL_CTRL_EVENT_B",
	[22] = " R12_SYS_CIRQ_IRQ_B",
	[23] = " R12_MD2AP_PEER_WAKEUP_EVENT",
	[24] = " R12_CSYSPWREQ_B",
	[25] = " R12_MD1_WDT_B",
	[26] = " R12_AP2AP_PEER_WAKEUP_EVENT",
	[27] = " R12_SEJ_EVENT_B",
	[28] = " R12_SPM_CPU_WAKEUP_EVENT",
	[29] = " R12_CPU_IRQOUT",
	[30] = " R12_CPU_WFI",
	[31] = " R12_MCUSYS_IDLE_TO_EMI_ALL",
};

#ifdef OPLUS_FEATURE_POWERINFO_STANDBY
const u32 R13_CLK_STABLE[32] = {
 	[0] = R13_SRCCLKENI0,                 
 	[1] = R13_SRCCLKENI1,                 
 	[2] = R13_MD_SRCCLKENA_0,             
 	[3] = R13_MD_APSRC_REQ_0,             
 	[4] = R13_CONN_DDR_EN,                
 	[5] = R13_MD_SRCCLKENA_1,             
 	[6] = R13_SSPM_SRCCLKENA,             
 	[7] = R13_SSPM_APSRC_REQ,             
 	[8] = R13_MD1_STATE,                  
 	[9] = R13_EMI_CLK_OFF_2_ACK,          
 	[10] = R13_MM_STATE,                   
 	[11] = R13_SSPM_STATE,                
 	[12] = R13_MD_DDR_EN_0,                
 	[13] = R13_CONN_STATE,                 
 	[14] = R13_CONN_SRCCLKENA,             
 	[15] = R13_CONN_APSRC_REQ,             
 	[16] = R13_BIT16,                      
 	[17] = R13_BIT17,                      
 	[18] = R13_SCP_STATE,                  
 	[19] = R13_CSYSPWRUPREQ,               
 	[20] = R13_PWRAP_SLEEP_ACK,            
 	[21] = R13_EMI_CLK_OFF_ACK_ALL,        
 	[22] = R13_AUDIO_DSP_STATE,            
 	[23] = R13_SW_DMDRAMCSHU_ACK_ALL,      
 	[24] = R13_CONN_SRCCLKENB,             
 	[25] = R13_SC_DR_SRAM_LOAD_ACK_ALL,    
 	[26] = R13_INFRA_AUX_IDLE,             
 	[27] = R13_DVFS_STATE,                 
 	[28] = R13_SC_DR_SRAM_PLL_LOAD_ACK_ALL,
 	[29] = R13_SC_DR_SRAM_RESTORE_ACK_ALL, 
 	[30] = R13_MD_VRF18_REQ_0,             
 	[31] = R13_DDR_EN_STATE, 
};
#endif/*OPLUS_FEATURE_POWERINFO_STANDBY*/

/**************************************
 * Function and API
 **************************************/
int __spm_get_pcm_timer_val(const struct pwr_ctrl *pwrctrl)
{
	u32 val;

	/* set PCM timer (set to max when disable) */
	if (pwrctrl->timer_val_ramp_en != 0) {
		u32 index;

		get_random_bytes(&index, sizeof(index));

		val = PCM_TIMER_RAMP_BASE_DPIDLE + (index & 0x000000FF);
	} else if (pwrctrl->timer_val_ramp_en_sec != 0) {
		u32 index;

		get_random_bytes(&index, sizeof(index));

		pcm_timer_ramp_max_sec_loop++;
		if (pcm_timer_ramp_max_sec_loop >= 50) {
			pcm_timer_ramp_max_sec_loop = 0;
			/* range 5min to 10min */
			val = PCM_TIMER_RAMP_BASE_SUSPEND_LONG +
				index % PCM_TIMER_RAMP_BASE_SUSPEND_LONG;
		} else {
			/* range 50ms to 16sec50ms */
			val = PCM_TIMER_RAMP_BASE_SUSPEND_50MS +
				index % PCM_TIMER_RAMP_BASE_SUSPEND_SHORT;
		}
	} else {
		if (pwrctrl->timer_val_cust == 0)
			val = pwrctrl->timer_val ? : PCM_TIMER_MAX;
		else
			val = pwrctrl->timer_val_cust;
	}

	return val;
}

void __spm_set_pwrctrl_pcm_flags(struct pwr_ctrl *pwrctrl, u32 flags)
{
	if (pwrctrl->pcm_flags_cust == 0)
		pwrctrl->pcm_flags = flags;
	else
		pwrctrl->pcm_flags = pwrctrl->pcm_flags_cust;
}

void __spm_set_pwrctrl_pcm_flags1(struct pwr_ctrl *pwrctrl, u32 flags)
{
	if (pwrctrl->pcm_flags1_cust == 0)
		pwrctrl->pcm_flags1 = flags;
	else
		pwrctrl->pcm_flags1 = pwrctrl->pcm_flags1_cust;
}

void __spm_sync_pcm_flags(struct pwr_ctrl *pwrctrl)
{
	/* set PCM flags and data */
	if (pwrctrl->pcm_flags_cust_clr != 0)
		pwrctrl->pcm_flags &= ~pwrctrl->pcm_flags_cust_clr;
	if (pwrctrl->pcm_flags_cust_set != 0)
		pwrctrl->pcm_flags |= pwrctrl->pcm_flags_cust_set;
	if (pwrctrl->pcm_flags1_cust_clr != 0)
		pwrctrl->pcm_flags1 &= ~pwrctrl->pcm_flags1_cust_clr;
	if (pwrctrl->pcm_flags1_cust_set != 0)
		pwrctrl->pcm_flags1 |= pwrctrl->pcm_flags1_cust_set;
}

void __spm_get_wakeup_status(struct wake_status *wakesta)
{
	/* get wakeup event */
	/* backup of PCM_REG12_DATA */
	wakesta->r12 = spm_read(SPM_SW_RSV_0);
	wakesta->r12_ext = spm_read(SPM_WAKEUP_STA);
	wakesta->raw_sta = spm_read(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = spm_read(SPM_WAKEUP_EXT_STA);
	wakesta->md32pcm_wakeup_sta = spm_read(MD32PCM_WAKEUP_STA);
	wakesta->md32pcm_event_sta = spm_read(MD32PCM_EVENT_STA);
	/* backup of SPM_WAKEUP_MISC */
	wakesta->wake_misc = spm_read(SPM_SW_RSV_5);

	/* get sleep time */
	/* backup of PCM_TIMER_OUT */
	wakesta->timer_out = spm_read(SPM_SW_RSV_6);

	/* get other SYS and co-clock status */
	wakesta->r13 = spm_read(PCM_REG13_DATA);
	wakesta->idle_sta = spm_read(SUBSYS_IDLE_STA);
	wakesta->req_sta0 = spm_read(SRC_REQ_STA_0);
	wakesta->req_sta1 = spm_read(SRC_REQ_STA_1);
	wakesta->req_sta2 = spm_read(SRC_REQ_STA_2);
	wakesta->req_sta3 = spm_read(SRC_REQ_STA_3);
	wakesta->req_sta4 = spm_read(SRC_REQ_STA_4);

	/* get debug flag for PCM execution check */
	wakesta->debug_flag = spm_read(PCM_WDT_LATCH_SPARE_0);
	wakesta->debug_flag1 = spm_read(PCM_WDT_LATCH_SPARE_1);

	/* get backup SW flag status */
	wakesta->b_sw_flag0 = spm_read(SPM_SW_RSV_7);   /* SPM_SW_RSV_7 */
	wakesta->b_sw_flag1 = spm_read(SPM_SW_RSV_8);   /* SPM_SW_RSV_8 */

	/* get ISR status */
	wakesta->isr = spm_read(SPM_IRQ_STA);

	/* get SW flag status */
	wakesta->sw_flag0 = spm_read(SPM_SW_FLAG_0);
	wakesta->sw_flag1 = spm_read(SPM_SW_FLAG_1);

	/* get CLK SETTLE */
	wakesta->clk_settle = spm_read(SPM_CLK_SETTLE); /* SPM_CLK_SETTLE */
	/* check abort */
	wakesta->is_abort = wakesta->debug_flag & DEBUG_ABORT_MASK;
	wakesta->is_abort |= wakesta->debug_flag1 & DEBUG_ABORT_MASK_1;
}

#define AVOID_OVERFLOW (0xFFFFFFFF00000000)
void __spm_save_ap_sleep_info(struct wake_status *wakesta)
{
	if (ap_pd_count >= AVOID_OVERFLOW)
		ap_pd_count = 0;
	else
		ap_pd_count++;

	if (ap_slp_duration >= AVOID_OVERFLOW)
		ap_slp_duration = 0;
	else
		ap_slp_duration = ap_slp_duration + wakesta->timer_out;
}

void __spm_save_26m_sleep_info(void)
{
	if (spm_26M_off_count >= AVOID_OVERFLOW)
		spm_26M_off_count = 0;
	else
		spm_26M_off_count = (spm_read(SPM_26M_COUNT) & 0xffff)
			+ spm_26M_off_count;

	if (spm_26M_off_duration >= AVOID_OVERFLOW)
		spm_26M_off_duration = 0;
	else
		spm_26M_off_duration = spm_26M_off_duration +
			spm_read(SPM_SW_RSV_4);
}

void rekick_vcorefs_scenario(void)
{
/* FIXME: */
}

#ifdef OPLUS_FEATURE_POWERINFO_STANDBY
static void mt6779_suspend_CLK_check(const struct wake_status *wakesta,const u64 deep_sleep_time)
{
	#define LOG_BUF_SIZE		        256
	#define IS_BLOCKED_OVER_TIMES		10
	char log_buf[LOG_BUF_SIZE] = { 0 };
	int log_size = 0;
	u32 is_no_blocked = 0;
	//static u32 is_blocked_cnt;
	is_no_blocked = wakesta->debug_flag & 0x2;
	/*
	if(!is_no_blocked)
		is_blocked_cnt++;
	else
		is_blocked_cnt=0;
	*/
	/* Check if ever enter deepest System LPM */
	if (is_no_blocked){ //ok:debug_flag = 0xd0fff3ff   NG:debug_flag = 0xd0eef300
		clk_state_statics(wakesta->debug_flag,wakesta->timer_out,deep_sleep_time,"static_clk_sleep_count");
		return;
	}
	/* Check if System LPM ever is blocked over 10 times,and ap suspend time less than 1s */
	//if ((is_blocked_cnt < IS_BLOCKED_OVER_TIMES)&&(wakesta->timer_out<32768))
	//	return;
	clk_state_statics(wakesta->r13,wakesta->timer_out,deep_sleep_time,"static_clk_no_sleep");
	/* Show who is blocking system LPM */
	log_size += scnprintf(log_buf + log_size,
		LOG_BUF_SIZE - log_size,
		"suspend warning:System LPM is blocked by ");//NG:r13 = 0xc400712c  OK:r13 = 0x4000000
	/*R13 defined in pcm_def.h*/
	if ((wakesta->r13 &(R13_CLK_STABLE[2]|R13_CLK_STABLE[3]|R13_CLK_STABLE[5]|R13_CLK_STABLE[8]|R13_CLK_STABLE[12]|R13_CLK_STABLE[30]))){//0x4000112C
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "MD_clk(2/3/5/8/12/30) ");	
	}

	if ((wakesta->r13 &(R13_CLK_STABLE[4]|R13_CLK_STABLE[13]|R13_CLK_STABLE[14]|R13_CLK_STABLE[15]|R13_CLK_STABLE[24])))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "CONN_clk(4/13/14/15/24/) ");

	if ((wakesta->r13 &(R13_CLK_STABLE[25]|R13_CLK_STABLE[28]|R13_CLK_STABLE[29])))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "SC_DR_SRAM_clk(25/28/29) ");

	if (wakesta->r13 & (R13_CLK_STABLE[10]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "MM_disp_clk(10) ");

	if (wakesta->r13 & (R13_CLK_STABLE[18]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "SCP_clk(18) ");

	if (wakesta->r13 & (R13_CLK_STABLE[22]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "AUDIO_clk(22) ");

	if (wakesta->r13 & (R13_CLK_STABLE[31]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "DDR_clk(31) ");

	if (wakesta->r13 & (R13_CLK_STABLE[6]|R13_CLK_STABLE[7]|R13_CLK_STABLE[11]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "SSPM_clk(6/7/11) ");

	if (wakesta->r13 & (R13_CLK_STABLE[0]|R13_CLK_STABLE[1]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "SRC_clk(0/1) ");

	if (wakesta->r13 & (R13_CLK_STABLE[16]|R13_CLK_STABLE[17]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "BIT(16/17) ");

	if (wakesta->r13 & (R13_CLK_STABLE[9]|R13_CLK_STABLE[21]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "EMI_clk(9/21) ");
	
	if (wakesta->r13 & (R13_CLK_STABLE[19]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "CSYSPWRUPREQ_clk(19) ");
	
	if (wakesta->r13 & (R13_CLK_STABLE[20]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "PWRAP_clk(20) ");
	
	if (wakesta->r13 & (R13_CLK_STABLE[23]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "SW_DMDRAMCSHU_clk(23) ");
	
	if (wakesta->r13 & (R13_CLK_STABLE[27]))
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "DVFS_clk(27) ");
	
	if (wakesta->r13==0x4000000)
		log_size += scnprintf(log_buf + log_size,LOG_BUF_SIZE - log_size, "unknow_clk ");

	WARN_ON(strlen(log_buf) >= LOG_BUF_SIZE);

	printk_deferred("[name:spm&][SPM] %s", log_buf);
}
#endif /*OPLUS_FEATURE_POWERINFO_STANDBY*/

unsigned int __spm_output_wake_reason(
	const struct wake_status *wakesta, bool suspend, const char *scenario)
{
	int i;
	char buf[LOG_BUF_SIZE] = { 0 };
	char log_buf[1024] = { 0 };
	char *local_ptr;
	int log_size = 0;
	unsigned int wr = WR_UNKNOWN;
	unsigned int spm_26M_off_pct = 0;

	if (wakesta->is_abort != 0) {
		/* add size check for vcoredvfs */
		aee_sram_printk("SPM ABORT (%s), r13 = 0x%x, ",
			scenario, wakesta->r13);
		printk_deferred("[name:spm&][SPM] ABORT (%s), r13 = 0x%x, ",
			scenario, wakesta->r13);

		aee_sram_printk(" debug_flag = 0x%x 0x%x\n",
			wakesta->debug_flag, wakesta->debug_flag1);
		printk_deferred(" debug_flag = 0x%x 0x%x\n",
			wakesta->debug_flag, wakesta->debug_flag1);

		aee_sram_printk(" sw_flag = 0x%x 0x%x\n",
			wakesta->sw_flag0, wakesta->sw_flag1);
		printk_deferred(" sw_flag = 0x%x 0x%x\n",
			wakesta->sw_flag0, wakesta->sw_flag1);

		aee_sram_printk(" b_sw_flag = 0x%x 0x%x\n",
			wakesta->b_sw_flag0, wakesta->b_sw_flag1);
		printk_deferred(" b_sw_flag = 0x%x 0x%x\n",
			wakesta->b_sw_flag0, wakesta->b_sw_flag1);

		wr =  WR_ABORT;
	}


	if (wakesta->r12 & R12_PCM_TIMER_EVENT) {

		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER_EVENT) {
			local_ptr = wakesrc_str[0];
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_PCM_TIMER;
		}
	}

	if (wakesta->r12 & R12_SPM_TWAM_IRQ_B) {

		if (wakesta->wake_misc & WAKE_MISC_DVFSRC_IRQ) {
			local_ptr = " DVFSRC";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_DVFSRC;
		}

		if (wakesta->wake_misc & WAKE_MISC_TWAM_IRQ_B) {
			local_ptr = " TWAM";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_TWAM;
		}

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET0) {
			local_ptr = " PMSR";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_PMSR;
		}

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET1) {
			local_ptr = " PMSR";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_PMSR;
		}

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET2) {
			local_ptr = " PMSR";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_PMSR;
		}

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_0) {
			local_ptr = " SPM_ACK_CHK";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_SPM_ACK_CHK;
		}

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_1) {
			local_ptr = " SPM_ACK_CHK";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_SPM_ACK_CHK;
		}

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_2) {
			local_ptr = " SPM_ACK_CHK";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_SPM_ACK_CHK;
		}

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_3) {
			local_ptr = " SPM_ACK_CHK";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_SPM_ACK_CHK;
		}

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL) {
			local_ptr = " SPM_ACK_CHK";
			if ((strlen(buf) + strlen(local_ptr)) < LOG_BUF_SIZE)
				strncat(buf, local_ptr, strlen(local_ptr));
			wr = WR_SPM_ACK_CHK;
		}
	}

	for (i = 2; i < 32; i++) {
		if (wakesta->r12 & (1U << i)) {
			if ((strlen(buf) + strlen(wakesrc_str[i])) <
				LOG_BUF_SIZE)
				strncat(buf, wakesrc_str[i],
					strlen(wakesrc_str[i]));

			wr = WR_WAKE_SRC;
		}
	}
	WARN_ON(strlen(buf) >= LOG_BUF_SIZE);

	log_size += sprintf(log_buf,
		"%s wake up by %s, timer_out = %u, r13 = 0x%x, debug_flag = 0x%x 0x%x, ",
		scenario, buf, wakesta->timer_out, wakesta->r13,
		wakesta->debug_flag, wakesta->debug_flag1);

	log_size += sprintf(log_buf + log_size,
		  "r12 = 0x%x, r12_ext = 0x%x, raw_sta = 0x%x, 0x%x, 0x%x 0x%x, ",
		  wakesta->r12, wakesta->r12_ext,
		  wakesta->raw_sta, wakesta->idle_sta,
		  wakesta->md32pcm_wakeup_sta,
		  wakesta->md32pcm_event_sta);

	log_size += sprintf(log_buf + log_size,
		  "req_sta =  0x%x 0x%x 0x%x 0x%x 0x%x, isr = 0x%x, ",
		  wakesta->req_sta0, wakesta->req_sta1, wakesta->req_sta2,
		  wakesta->req_sta3, wakesta->req_sta4, wakesta->isr);

	log_size += sprintf(log_buf + log_size,
		"raw_ext_sta = 0x%x, wake_misc = 0x%x, sw_flag = 0x%x 0x%x 0x%x 0x%x, req = 0x%x,",
		wakesta->raw_ext_sta,
		wakesta->wake_misc,
		wakesta->sw_flag0,
		wakesta->sw_flag1,
		wakesta->b_sw_flag0,
		wakesta->b_sw_flag1,
		spm_read(SPM_SRC_REQ));

	if (!strcmp(scenario, "suspend")) {
		/* calculate 26M off percentage in suspend period */
		if (wakesta->timer_out != 0) {
			spm_26M_off_pct = 100 * spm_read(SPM_SW_RSV_4)
						/ wakesta->timer_out;
		}

		log_size += sprintf(log_buf + log_size,
			" clk_settle = 0x%x, ",
			wakesta->clk_settle);

		log_size += sprintf(log_buf + log_size,
			"wlk_cntcv_l = 0x%x, wlk_cntcv_h = 0x%x, 26M_off_pct = %d\n",
			_golden_read_reg(WORLD_CLK_CNTCV_L),
			_golden_read_reg(WORLD_CLK_CNTCV_H),
			spm_26M_off_pct);
		#ifdef OPLUS_FEATURE_POWERINFO_STANDBY
		mt6779_suspend_CLK_check(wakesta,spm_read(SPM_SW_RSV_4));
		printk_deferred("[name:spm&][SPM] Suspended for %d.%03d seconds",
			PCM_TICK_TO_SEC(wakesta->timer_out),
			PCM_TICK_TO_SEC((wakesta->timer_out % PCM_32K_TICKS_PER_SEC)* 1000));
		#endif /* OPLUS_FEATURE_POWERINFO_STANDBY */
	} else
		log_size += sprintf(log_buf + log_size,
			" clk_settle = 0x%x\n",
			wakesta->clk_settle);
	
	#ifdef OPLUS_FEATURE_POWERINFO_STANDBY
	//R12_EINT_EVENT_B wakeup statistics in other place
	if(suspend==true && (!(wakesta->r12 & R12_EINT_EVENT_B))){
		pr_info("%s:wakeup_reson=%d scenario=%s wakeupby(buf)=%s",__func__,wr,scenario,buf);
		wakeup_state=false;
		wakeup_state=wakeup_reasons_statics(buf, WS_CNT_WLAN|WS_CNT_ADSP|WS_CNT_SENSOR|WS_CNT_MODEM);
		if((wakeup_state==false)&&(strlen(buf)!=0)){
			wakeup_reasons_statics("other",WS_CNT_OTHER);	
		}
	}
	#endif /* OPLUS_FEATURE_POWERINFO_STANDBY */
	
	WARN_ON(log_size >= 1024);

	if (!suspend)
		printk_deferred("[name:spm&][SPM] %s", log_buf);
	else {
		aee_sram_printk("%s", log_buf);
		printk_deferred("[name:spm&][SPM] %s", log_buf);
	}
	return wr;
}

long spm_get_current_time_ms(void)
{
	struct timeval t;

	do_gettimeofday(&t);
	return ((t.tv_sec & 0xFFF) * 1000000 + t.tv_usec) / 1000;
}

void spm_set_dummy_read_addr(int debug)
{
// FIXME
}

int __attribute__ ((weak)) get_dynamic_period(
	int first_use, int first_wakeup_time, int battery_capacity_level)
{
	/* pr_info("NO %s !!!\n", __func__); */
	return 5401;
}

u32 __spm_get_wake_period(int pwake_time, unsigned int last_wr)
{
	int period = SPM_WAKE_PERIOD;

	if (pwake_time < 0) {
		/* use FG to get the period of 1% battery decrease */
		period = get_dynamic_period(last_wr != WR_PCM_TIMER
				? 1 : 0, SPM_WAKE_PERIOD, 1);
		if (period <= 0) {
			printk_deferred("[name:spm&][SPM] CANNOT GET PERIOD FROM FUEL GAUGE\n");
			period = SPM_WAKE_PERIOD;
		}
	} else {
		period = pwake_time;
		aee_sram_printk("pwake = %d\n", pwake_time);
		printk_deferred("[name:spm&][SPM] pwake = %d\n", pwake_time);
	}

	if (period > 36 * 3600)	/* max period is 36.4 hours */
		period = 36 * 3600;

	return period;
}

MODULE_DESCRIPTION("SPM-Internal Driver v0.1");
