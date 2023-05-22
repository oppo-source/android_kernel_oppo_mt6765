// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/iio/consumer.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/atomic.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/regulator/driver.h>

#include "../inc/mt6360_pmu.h"
#include "../inc/mt6360_pmu_chg.h"
#include "../../../../power/oplus/oplus_chg_track.h"
#include <charger_class.h>

#include <mt-plat/upmu_common.h>
#include <mt-plat/mtk_boot.h>
#ifdef OPLUS_FEATURE_CHG_BASIC
#include <linux/delay.h>
#endif

#include <linux/phy/phy.h>
#include <mtk_charger.h>

#ifdef CONFIG_TCPC_CLASS
#include <tcpm.h>
#endif

#ifdef OPLUS_FEATURE_CHG_BASIC
extern bool oplus_chg_wake_update_work(void);
extern void oplus_chg_status_change(bool chg_status);
#endif

#ifdef OPLUS_FEATURE_CHG_BASIC
extern int oplus_mt6360_get_tchg(int *tchg_min,	int *tchg_max);
#endif

#define MT6360_PMU_CHG_DRV_VERSION	"1.0.8_MTK"

#define PHY_MODE_BC11_SET 1
#define PHY_MODE_BC11_CLR 2


void __attribute__ ((weak)) Charger_Detect_Init(void)
{
}

void __attribute__ ((weak)) Charger_Detect_Release(void)
{
}

struct tag_bootmode {
	u32 size;
	u32 tag;
	u32 bootmode;
	u32 boottype;
};

enum mt6360_adc_channel {
	MT6360_ADC_VBUSDIV5,
	MT6360_ADC_VSYS,
	MT6360_ADC_VBAT,
	MT6360_ADC_IBUS,
	MT6360_ADC_IBAT,
	MT6360_ADC_NODUMP,
	MT6360_ADC_TEMP_JC = MT6360_ADC_NODUMP,
	MT6360_ADC_USBID,
	MT6360_ADC_TS,
	MT6360_ADC_MAX,
};

static const char * const mt6360_adc_chan_list[] = {
	"VBUSDIV5", "VSYS", "VBAT", "IBUS", "IBAT", "TEMP_JC", "USBID", "TS",
};

struct mt6360_pmu_chg_info {
	struct device *dev;
	struct mt6360_pmu_info *mpi;
	struct iio_channel *channels[MT6360_ADC_MAX];
	struct charger_device *chg_dev;
	int hidden_mode_cnt;
	struct mutex hidden_mode_lock;
	struct mutex pe_lock;
	struct mutex aicr_lock;
	struct mutex tchg_lock;
	struct mutex ichg_lock;
	int tchg;
	u32 zcv;
	u32 ichg;
	u32 ichg_dis_chg;

	u32 bootmode;

	/* Charger type detection */
	struct mutex chgdet_lock;
	bool attach;
	bool pwr_rdy;
	bool bc12_en;
	int psy_usb_type;
#ifdef CONFIG_TCPC_CLASS
	bool tcpc_attach;

	/*dts:charger*/
//for bc1.2 power supply
	struct power_supply *chg_psy;

	/*type_c_port0*/
	struct tcpc_device *tcpc;
	struct notifier_block pd_nb;
	/*chg_det*/
	wait_queue_head_t attach_wq;
	atomic_t chrdet_start;
	struct task_struct *attach_task;
	struct mutex attach_lock;
	bool typec_attach;
	bool ignore_usb;
	bool bypass_chgdet;
	bool tcpc_kpoc;
#else
	struct work_struct chgdet_work;
#endif /* CONFIG_TCPC_CLASS */
	/*power supply*/
	struct power_supply_desc psy_desc;
	struct power_supply *psy;

	struct completion aicc_done;
	struct completion pumpx_done;
	atomic_t pe_complete;
	/* mivr */
	atomic_t mivr_cnt;
	wait_queue_head_t mivr_wq;
	struct task_struct *mivr_task;
	/* unfinish pe pattern */
	struct workqueue_struct *pe_wq;
	struct work_struct pe_work;
	u8 ctd_dischg_status;
	struct regulator_dev *otg_rdev;	//otg_vbus

	union power_supply_propval *old_propval;
#ifdef OPLUS_FEATURE_CHG_BASIC
	atomic_t suspended;
#endif
#ifdef CONFIG_OPLUS_HVDCP_SUPPORT
	struct delayed_work hvdcp_work;
	struct delayed_work hvdcp_result_check_work;
	enum power_supply_type hvdcp_type;
#endif
#ifdef OPLUS_FEATURE_CHG_BASIC
	bool support_hvdcp;
#endif
#ifdef OPLUS_FEATURE_CHG_BASIC
	unsigned int chgdet_mdelay;
#endif
#ifdef OPLUS_FEATURE_CHG_BASIC
	enum power_supply_type charger_type;
#endif
};

/* for recive bat oc notify */
struct mt6360_pmu_chg_info *g_mpci;

static const u32 mt6360_otg_oc_threshold[] = {
	500000, 700000, 1100000, 1300000, 1800000, 2100000, 2400000, 3000000,
}; /* uA */

enum mt6360_iinlmtsel {
	MT6360_IINLMTSEL_AICR_3250 = 0,
	MT6360_IINLMTSEL_CHG_TYPE,
	MT6360_IINLMTSEL_AICR,
	MT6360_IINLMTSEL_LOWER_LEVEL,
};

enum mt6360_charging_status {
	MT6360_CHG_STATUS_READY = 0,
	MT6360_CHG_STATUS_PROGRESS,
	MT6360_CHG_STATUS_DONE,
	MT6360_CHG_STATUS_FAULT,
	MT6360_CHG_STATUS_MAX,
};

enum mt6360_usbsw_state {
	MT6360_USBSW_CHG = 0,
	MT6360_USBSW_USB,
};

enum mt6360_pmu_chg_type {
	MT6360_CHG_TYPE_NOVBUS = 0,
	MT6360_CHG_TYPE_UNDER_GOING,
	MT6360_CHG_TYPE_SDP,
	MT6360_CHG_TYPE_SDPNSTD,
	MT6360_CHG_TYPE_DCP,
	MT6360_CHG_TYPE_CDP,
	MT6360_CHG_TYPE_MAX,
};

static enum power_supply_usb_type mt6360_pmu_chg_usb_types[] = {
	POWER_SUPPLY_USB_TYPE_UNKNOWN,
	POWER_SUPPLY_USB_TYPE_SDP,
	POWER_SUPPLY_USB_TYPE_DCP,
	POWER_SUPPLY_USB_TYPE_CDP,
	POWER_SUPPLY_USB_TYPE_C,
	POWER_SUPPLY_USB_TYPE_PD,
	POWER_SUPPLY_USB_TYPE_PD_DRP,
	POWER_SUPPLY_USB_TYPE_APPLE_BRICK_ID
};

static const char *mt6360_chg_status_name[MT6360_CHG_STATUS_MAX] = {
	"ready", "progress", "done", "fault",
};

static const struct mt6360_chg_platform_data def_platform_data = {
	.ichg = 2000000,		/* uA */
	.aicr = 500000,			/* uA */
	.mivr = 4400000,		/* uV */
	.cv = 4350000,			/* uA */
	.ieoc = 250000,			/* uA */
	.safety_timer = 12,		/* hour */
#ifdef CONFIG_MTK_BIF_SUPPORT
	.ircmp_resistor = 0,		/* uohm */
	.ircmp_vclamp = 0,		/* uV */
#else
	.ircmp_resistor = 25000,	/* uohm */
	.ircmp_vclamp = 32000,		/* uV */
#endif
	.en_te = true,
	.en_wdt = true,
	.aicc_once = true,
	.post_aicc = true,
	.batoc_notify = false,
	.bc12_sel = 0,
	.chg_name = "primary_chg",
};
#ifdef OPLUS_FEATURE_CHG_BASIC
static struct mt6360_pmu_chg_info *oplusmpci = NULL;
#endif
#ifdef OPLUS_FEATURE_CHG_BASIC
extern void oplus_chg_set_otg_online(bool online);
#endif

/* ================== */
/* Internal Functions */
/* ================== */
static inline u32 mt6360_trans_sel(u32 target, u32 min_val, u32 step,
				   u32 max_sel)
{
	u32 data = 0;

	if (target >= min_val)
		data = (target - min_val) / step;
	if (data > max_sel)
		data = max_sel;
	return data;
}

static u32 mt6360_trans_ichg_sel(u32 uA)
{
	return mt6360_trans_sel(uA, 100000, 100000, 0x31);
}

static u32 mt6360_trans_aicr_sel(u32 uA)
{
	return mt6360_trans_sel(uA, 100000, 50000, 0x3F);
}

static u32 mt6360_trans_mivr_sel(u32 uV)
{
	return mt6360_trans_sel(uV, 3900000, 100000, 0x5F);
}

static u32 mt6360_trans_cv_sel(u32 uV)
{
	return mt6360_trans_sel(uV, 3900000, 10000, 0x51);
}

static u32 mt6360_trans_ieoc_sel(u32 uA)
{
	return mt6360_trans_sel(uA, 100000, 50000, 0x0F);
}

static u32 mt6360_trans_safety_timer_sel(u32 hr)
{
	u32 mt6360_st_tbl[] = {4, 6, 8, 10, 12, 14, 16, 20};
	u32 cur_val, next_val;
	int i;

	if (hr < mt6360_st_tbl[0])
		return 0;
	for (i = 0; i < ARRAY_SIZE(mt6360_st_tbl) - 1; i++) {
		cur_val = mt6360_st_tbl[i];
		next_val = mt6360_st_tbl[i+1];
		if (hr >= cur_val && hr < next_val)
			return i;
	}
	return ARRAY_SIZE(mt6360_st_tbl) - 1;
}

static u32 mt6360_trans_ircmp_r_sel(u32 uohm)
{
	return mt6360_trans_sel(uohm, 0, 25000, 0x07);
}

static u32 mt6360_trans_ircmp_vclamp_sel(u32 uV)
{
	return mt6360_trans_sel(uV, 0, 32000, 0x07);
}

static const u32 mt6360_usbid_rup[] = {
	500000, 75000, 5000, 1000
};

static inline u32 mt6360_trans_usbid_rup(u32 rup)
{
	int i;
	int maxidx = ARRAY_SIZE(mt6360_usbid_rup) - 1;

	if (rup >= mt6360_usbid_rup[0])
		return 0;
	if (rup <= mt6360_usbid_rup[maxidx])
		return maxidx;

	for (i = 0; i < maxidx; i++) {
		if (rup == mt6360_usbid_rup[i])
			return i;
		if (rup < mt6360_usbid_rup[i] &&
		    rup > mt6360_usbid_rup[i + 1]) {
			if ((mt6360_usbid_rup[i] - rup) <=
			    (rup - mt6360_usbid_rup[i + 1]))
				return i;
			else
				return i + 1;
		}
	}
	return maxidx;
}

static const u32 mt6360_usbid_src_ton[] = {
	400, 1000, 4000, 10000, 40000, 100000, 400000,
};

static inline u32 mt6360_trans_usbid_src_ton(u32 src_ton)
{
	int i;
	int maxidx = ARRAY_SIZE(mt6360_usbid_src_ton) - 1;

	/* There is actually an option, always on, after 400000 */
	if (src_ton == 0)
		return maxidx + 1;
	if (src_ton < mt6360_usbid_src_ton[0])
		return 0;
	if (src_ton > mt6360_usbid_src_ton[maxidx])
		return maxidx;

	for (i = 0; i < maxidx; i++) {
		if (src_ton == mt6360_usbid_src_ton[i])
			return i;
		if (src_ton > mt6360_usbid_src_ton[i] &&
		    src_ton < mt6360_usbid_src_ton[i + 1]) {
			if ((src_ton - mt6360_usbid_src_ton[i]) <=
			    (mt6360_usbid_src_ton[i + 1] - src_ton))
				return i;
			else
				return i + 1;
		}
	}
	return maxidx;
}

static inline int mt6360_get_ieoc(struct mt6360_pmu_chg_info *mpci, u32 *uA)
{
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL9);
	if (ret < 0)
		return ret;
	ret = (ret & MT6360_MASK_IEOC) >> MT6360_SHFT_IEOC;
	*uA = 100000 + (ret * 50000);
	return ret;
}

static inline int mt6360_get_charging_status(
					struct mt6360_pmu_chg_info *mpci,
					enum mt6360_charging_status *chg_stat)
{
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT);
	if (ret < 0)
		return ret;
	*chg_stat = (ret & MT6360_MASK_CHG_STAT) >> MT6360_SHFT_CHG_STAT;
	return 0;
}

static inline int mt6360_is_charger_enabled(struct mt6360_pmu_chg_info *mpci,
					    bool *en)
{
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL2);
	if (ret < 0)
		return ret;
	*en = (ret & MT6360_MASK_CHG_EN) ? true : false;
	return 0;
}

static inline int mt6360_select_input_current_limit(
		struct mt6360_pmu_chg_info *mpci, enum mt6360_iinlmtsel sel)
{
	dev_dbg(mpci->dev,
		"%s: select input current limit = %d\n", __func__, sel);
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL2,
					  MT6360_MASK_IINLMTSEL,
					  sel << MT6360_SHFT_IINLMTSEL);
}

static int mt6360_enable_wdt(struct mt6360_pmu_chg_info *mpci, bool en)
{
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);

	dev_dbg(mpci->dev, "%s enable wdt, en = %d\n", __func__, en);
	if (!pdata->en_wdt)
		return 0;
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL13,
					  MT6360_MASK_CHG_WDT_EN,
					  en ? 0xff : 0);
}

static int mt6360_enable_otg_wdt(struct mt6360_pmu_chg_info *mpci, bool en)
{
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);

	dev_dbg(mpci->dev, "%s enable otg wdt, en = %d\n", __func__, en);
	if (!pdata->en_otg_wdt)
		return 0;
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL13,
					  MT6360_MASK_CHG_WDT_EN,
					  en ? 0xff : 0);
}

static inline int mt6360_get_chrdet_ext_stat(struct mt6360_pmu_chg_info *mpci,
					  bool *pwr_rdy)
{
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHRDET_STAT);
	if (ret < 0)
		return ret;
	*pwr_rdy = !!(ret & BIT(4));
	return 0;
}

static inline bool mt6360_is_meta_mode(struct mt6360_pmu_chg_info *mpci);
static void mt6360_power_supply_changed(struct mt6360_pmu_chg_info *mpci);

static int DPDM_Switch_TO_CHG_upstream(struct mt6360_pmu_chg_info *mpci,
						bool switch_to_chg)
{
	struct phy *phy;
	int mode = 0;
	int ret;

	mode = switch_to_chg ? PHY_MODE_BC11_SET : PHY_MODE_BC11_CLR;
	phy = phy_get(mpci->dev, "usb2-phy");
	if (IS_ERR_OR_NULL(phy)) {
		dev_info(mpci->dev, "phy_get fail\n");
		return -EINVAL;
	}

	ret = phy_set_mode_ext(phy, PHY_MODE_USB_DEVICE, mode);
	if (ret)
		dev_info(mpci->dev, "phy_set_mode_ext fail\n");

	phy_put(phy);

	return 0;
}

static int mt6360_set_usbsw_state(struct mt6360_pmu_chg_info *mpci, int state)
{
	dev_info(mpci->dev, "%s: state = %d\n", __func__, state);

	/* Switch D+D- to AP/MT6360 */
	if (state == MT6360_USBSW_CHG)
		DPDM_Switch_TO_CHG_upstream(mpci, true);
	else
		DPDM_Switch_TO_CHG_upstream(mpci, false);

	return 0;
}

#ifndef CONFIG_MT6360_DCDTOUT_SUPPORT
static int __maybe_unused mt6360_enable_dcd_tout(
				      struct mt6360_pmu_chg_info *mpci, bool en)
{
	dev_info(mpci->dev, "%s en = %d\n", __func__, en);
	return (en ? mt6360_pmu_reg_set_bits : mt6360_pmu_reg_clr_bits)
		(mpci->mpi, MT6360_PMU_DEVICE_TYPE, MT6360_MASK_DCDTOUTEN);
}

static int __maybe_unused mt6360_is_dcd_tout_enable(
				     struct mt6360_pmu_chg_info *mpci, bool *en)
{
	int ret;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_DEVICE_TYPE);
	if (ret < 0) {
		*en = false;
		return ret;
	}
	*en = (ret & MT6360_MASK_DCDTOUTEN ? true : false);
	return 0;
}
#endif

bool is_usb_rdy(struct device *dev)
{
	struct device_node *node;
	bool ready = false;

	node = of_parse_phandle(dev->of_node, "usb", 0);
	if (node) {
		ready = of_property_read_bool(node, "gadget-ready");
		dev_info(dev, "gadget-ready=%d\n", ready);
	} else
		dev_info(dev, "usb node missing or invalid\n");

	return ready;
}

static int __mt6360_enable_usbchgen(struct mt6360_pmu_chg_info *mpci, bool en)
{
	int i, ret = 0;
	const int max_wait_cnt = 200;
#ifndef CONFIG_TCPC_CLASS
	bool pwr_rdy = false;
#endif /* !CONFIG_TCPC_CLASS */
	enum mt6360_usbsw_state usbsw =
				       en ? MT6360_USBSW_CHG : MT6360_USBSW_USB;
#ifndef CONFIG_MT6360_DCDTOUT_SUPPORT
	bool dcd_en = false;
#endif /* CONFIG_MT6360_DCDTOUT_SUPPORT */

	dev_info(mpci->dev, "%s: en = %d\n", __func__, en);
	if (en) {
#ifndef CONFIG_MT6360_DCDTOUT_SUPPORT
		ret = mt6360_is_dcd_tout_enable(mpci, &dcd_en);
		if (!dcd_en)
			msleep(180);
#endif /* CONFIG_MT6360_DCDTOUT_SUPPORT */
		/* Workaround for CDP port */
		for (i = 0; i < max_wait_cnt; i++) {
			if (is_usb_rdy(mpci->dev)) {
				dev_info(mpci->dev, "%s: USB ready\n",
					__func__);
				break;
			}
			dev_info(mpci->dev, "%s: CDP block\n", __func__);
#ifndef CONFIG_TCPC_CLASS
			/* Check vbus */
			ret = mt6360_get_chrdet_ext_stat(mpci, &pwr_rdy);
			if (ret < 0) {
				dev_err(mpci->dev, "%s: fail, ret = %d\n",
					 __func__, ret);
				return ret;
			}
			dev_info(mpci->dev, "%s: pwr_rdy = %d\n", __func__,
				 pwr_rdy);
			if (!pwr_rdy) {
				dev_info(mpci->dev, "%s: plug out\n", __func__);
				return ret;
			}
#else
			if (!(mpci->tcpc_attach)) {
				dev_info(mpci->dev,
					 "%s: plug out\n", __func__);
				return 0;
			}
#endif /* !CONFIG_TCPC_CLASS */
			msleep(100);
		}
		if (i == max_wait_cnt)
			dev_err(mpci->dev, "%s: CDP timeout\n", __func__);
		else
			dev_info(mpci->dev, "%s: CDP free\n", __func__);
	}
	mt6360_set_usbsw_state(mpci, usbsw);
	ret = mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_DEVICE_TYPE,
					 MT6360_MASK_USBCHGEN, en ? 0xff : 0);
	if (ret >= 0)
		mpci->bc12_en = en;
	return ret;
}

static int mt6360_enable_usbchgen(struct mt6360_pmu_chg_info *mpci, bool en)
{
	int ret = 0;

	mutex_lock(&mpci->chgdet_lock);
	ret = __mt6360_enable_usbchgen(mpci, en);
	mutex_unlock(&mpci->chgdet_lock);
	return ret;
}

#ifdef CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT
static int mt6360_chgdet_pre_process(struct mt6360_pmu_chg_info *mpci)
{
#ifdef OPLUS_FEATURE_CHG_BASIC
	int ret = 0;
#endif
	bool attach = false;
#ifdef OPLUS_FEATURE_CHG_BASIC
	static struct power_supply *battery_psy = NULL;
	if (!battery_psy) {
		battery_psy = power_supply_get_by_name("battery");
	}
#endif /*OPLUS_FEATURE_CHG_BASIC*/

#ifdef CONFIG_TCPC_CLASS
	attach = mpci->tcpc_attach;
#else
	attach = mpci->pwr_rdy;
#endif /* CONFIG_TCPC_CLASS */

#ifdef CONFIG_OPLUS_HVDCP_SUPPORT
	if (mpci->support_hvdcp == true) {
		ret = mt6360_pmu_reg_write(mpci->mpi, HVDCP_DEVICE_TYPE, 0x0);
		if (ret < 0)
			dev_err(mpci->dev, "%s: fail to write hvdcp_device_type\n", __func__);

		ret = mt6360_pmu_reg_write(mpci->mpi, MT6360_PMU_DPDM_CTRL, 0x0);
	        if (ret < 0)
	                dev_err(mpci->dev, "%s: fail to write dpdm_ctrl\n", __func__);
	}
	if (!attach) {
		if (mpci->support_hvdcp == true) {
			cancel_delayed_work_sync(&mpci->hvdcp_work);
			cancel_delayed_work_sync(&mpci->hvdcp_result_check_work);
			mpci->psy_desc.type = POWER_SUPPLY_TYPE_UNKNOWN;
			mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;
			mpci->hvdcp_type = POWER_SUPPLY_TYPE_UNKNOWN;
			if (battery_psy)
				power_supply_changed(battery_psy);
			oplus_chg_wake_update_work();
		}
#ifdef OPLUS_FEATURE_CHG_BASIC
		mpci->charger_type = POWER_SUPPLY_TYPE_UNKNOWN;
#endif
	}
#endif

#ifdef FIXME
		if (attach && (mpci->bootmode == 5)) {
		/* Skip charger type detection to speed up meta boot.*/
		dev_notice(mpci->dev, "%s: force Standard USB Host in meta\n",
			   __func__);
		mpci->attach = attach;
		mpci->psy_desc.type = POWER_SUPPLY_TYPE_USB;
		mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_SDP;
		mt6360_power_supply_changed(mpci);
		power_supply_changed(mpci->psy);
#ifdef OPLUS_FEATURE_CHG_BASIC
		if (battery_psy)
			power_supply_changed(battery_psy);
		oplus_chg_wake_update_work();
#endif
		return 0;
	}
#endif
	return __mt6360_enable_usbchgen(mpci, attach);
}

static int mt6360_chgdet_post_process(struct mt6360_pmu_chg_info *mpci)
{
	int ret = 0;
	bool attach = false, inform_psy = true;
	u8 usb_status = MT6360_CHG_TYPE_NOVBUS;
#ifdef OPLUS_FEATURE_CHG_BASIC
	static struct power_supply *battery_psy = NULL;
	if (!battery_psy) {
		battery_psy = power_supply_get_by_name("battery");
	}
#endif /*OPLUS_FEATURE_CHG_BASIC*/

#ifdef CONFIG_TCPC_CLASS
	attach = mpci->tcpc_attach;
#else
	attach = mpci->pwr_rdy;
#endif /* CONFIG_TCPC_CLASS */
	if (mpci->attach == attach) {
		dev_info(mpci->dev, "%s: attach(%d) is the same\n",
				    __func__, attach);
		inform_psy = !attach;
		goto out;
	}
	mpci->attach = attach;
	dev_info(mpci->dev, "%s: attach = %d\n", __func__, attach);
#ifdef OPLUS_FEATURE_CHG_BASIC
		oplus_chg_status_change(attach);
#endif
	/* Plug out during BC12 */
	if (!attach) {
		dev_info(mpci->dev, "%s: Charger Type: UNKONWN\n", __func__);
		mpci->psy_desc.type = POWER_SUPPLY_TYPE_UNKNOWN;
		mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;
#ifdef OPLUS_FEATURE_CHG_BASIC
		mpci->charger_type = POWER_SUPPLY_TYPE_UNKNOWN;
#endif
		goto out;
	}
	/* Plug in */
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_USB_STATUS1);
	if (ret < 0)
		goto out;
	usb_status = (ret & MT6360_MASK_USB_STATUS) >> MT6360_SHFT_USB_STATUS;
	switch (usb_status) {
	case MT6360_CHG_TYPE_UNDER_GOING:
		dev_info(mpci->dev, "%s: under going...\n", __func__);
		return ret;
	case MT6360_CHG_TYPE_SDP:
		dev_info(mpci->dev,
			  "%s: Charger Type: STANDARD_HOST\n", __func__);
		mpci->psy_desc.type = POWER_SUPPLY_TYPE_USB;
		mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_SDP;
		break;
	case MT6360_CHG_TYPE_SDPNSTD:
		dev_info(mpci->dev,
			  "%s: Charger Type: NONSTANDARD_CHARGER\n", __func__);
#ifdef OPLUS_FEATURE_CHG_BASIC
		mpci->psy_desc.type = POWER_SUPPLY_TYPE_USB_DCP;
#else
		mpci->psy_desc.type = POWER_SUPPLY_TYPE_USB;
#endif
		mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
		break;
	case MT6360_CHG_TYPE_CDP:
		dev_info(mpci->dev,
			  "%s: Charger Type: CHARGING_HOST\n", __func__);
		mpci->psy_desc.type = POWER_SUPPLY_TYPE_USB_CDP;
		mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_CDP;
		break;
	case MT6360_CHG_TYPE_DCP:
		dev_info(mpci->dev,
			  "%s: Charger Type: STANDARD_CHARGER\n", __func__);
		mpci->psy_desc.type = POWER_SUPPLY_TYPE_USB_DCP;
		mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
#ifdef CONFIG_OPLUS_HVDCP_SUPPORT
		if (mpci->support_hvdcp == true) {
			dev_err(mpci->dev, "%s: enable hvdcp detect\n", __func__);
			ret = mt6360_pmu_reg_write(mpci->mpi, HVDCP_DEVICE_TYPE, 0x0);
			if (ret < 0)
				dev_err(mpci->dev, "%s: fail to write hvdcp_device_type\n", __func__);

			ret = mt6360_pmu_reg_write(mpci->mpi, MT6360_PMU_DPDM_CTRL, 0x0);
			if (ret < 0)
				dev_err(mpci->dev, "%s: fail to write dpdm_ctrl\n", __func__);
			ret = mt6360_pmu_reg_update_bits(mpci->mpi, HVDCP_DEVICE_TYPE, 0x80, 0x80);
	                if (ret < 0)
	                        dev_err(mpci->dev, "%s: fail to write dpdm_ctrl\n", __func__);
	        	schedule_delayed_work(&mpci->hvdcp_result_check_work, msecs_to_jiffies(3000));
		}
#endif
		break;
	}
out:
#ifndef OPLUS_FEATURE_CHG_BASIC
	if (!attach) {
		ret = __mt6360_enable_usbchgen(mpci, false);
		if (ret < 0)
			dev_notice(mpci->dev, "%s: disable chgdet fail\n",
				   __func__);
	} else if (mpci->psy_desc.type != POWER_SUPPLY_TYPE_USB_DCP)
		mt6360_set_usbsw_state(mpci, MT6360_USBSW_USB);
#else
	mpci->charger_type = mpci->psy_desc.type;
	if (mpci->support_hvdcp == true) {
		if (!attach) {
			cancel_delayed_work_sync(&mpci->hvdcp_work);
			cancel_delayed_work_sync(&mpci->hvdcp_result_check_work);
			mpci->hvdcp_type = POWER_SUPPLY_TYPE_UNKNOWN;

			ret = mt6360_pmu_reg_write(mpci->mpi, MT6360_PMU_DPDM_CTRL, 0x0);
			if (ret < 0)
				dev_err(mpci->dev, "%s: fail to write dpdm_ctrl\n", __func__);

			ret = __mt6360_enable_usbchgen(mpci, false);
			if (ret < 0)
				dev_notice(mpci->dev, "%s: disable chgdet fail\n",
				   	__func__);
		} else if (mpci->psy_usb_type != POWER_SUPPLY_USB_TYPE_DCP) {
			mt6360_set_usbsw_state(mpci, MT6360_USBSW_USB);
			printk(KERN_ERR "set MT6360_USBSW_USB\n");
		}
	} else {
			ret = __mt6360_enable_usbchgen(mpci, false);
			if (ret < 0)
				dev_notice(mpci->dev, "%s: disable chgdet fail\n",
						__func__);
	}
#endif /*OPLUS_FEATURE_CHG_BASIC*/
	if (!inform_psy)
		return ret;
	mt6360_power_supply_changed(mpci);
	power_supply_changed(mpci->psy);
#ifdef OPLUS_FEATURE_CHG_BASIC
	if (battery_psy)
		power_supply_changed(battery_psy);
	oplus_chg_wake_update_work();
#endif
	return ret;
}
#endif /* CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT */

static const u32 mt6360_vinovp_list[] = {
	5500000, 6500000, 10500000, 14500000,
};

static int mt6360_select_vinovp(struct mt6360_pmu_chg_info *mpci, u32 uV)
{
	int i;

	if (uV < mt6360_vinovp_list[0])
		return -EINVAL;
	for (i = 1; i < ARRAY_SIZE(mt6360_vinovp_list); i++) {
		if (uV < mt6360_vinovp_list[i])
			break;
	}
	i--;
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL19,
					  MT6360_MASK_CHG_VIN_OVP_VTHSEL,
					  i << MT6360_SHFT_CHG_VIN_OVP_VTHSEL);
}

static inline int mt6360_read_zcv(struct mt6360_pmu_chg_info *mpci)
{
	int ret = 0;
	u8 zcv_data[2] = {0};

	dev_dbg(mpci->dev, "%s\n", __func__);
	/* Read ZCV data */
	ret = mt6360_pmu_reg_block_read(mpci->mpi, MT6360_PMU_ADC_BAT_DATA_H,
					2, zcv_data);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: read zcv data fail\n", __func__);
		return ret;
	}
	mpci->zcv = 1250 * (zcv_data[0] * 256 + zcv_data[1]);
	dev_info(mpci->dev, "%s: zcv = (0x%02X, 0x%02X, %dmV)\n",
		 __func__, zcv_data[0], zcv_data[1], mpci->zcv/1000);
	/* Disable ZCV */
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_ADC_CONFIG,
				      MT6360_MASK_ZCV_EN);
	if (ret < 0)
		dev_err(mpci->dev, "%s: disable zcv fail\n", __func__);
	return ret;
}

/* ================== */
/* External Functions */
/* ================== */
static int __mt6360_set_ichg(struct mt6360_pmu_chg_info *mpci, u32 uA)
{
	int ret = 0;
	u32 data = 0;
#ifndef OPLUS_FEATURE_CHG_BASIC
	mt_dbg(mpci->dev, "%s\n", __func__);
#else
	printk(KERN_ERR "%s, ichg=%d ma\n", uA /1000);
#endif
	data = mt6360_trans_ichg_sel(uA);
	ret = mt6360_pmu_reg_update_bits(mpci->mpi,
					 MT6360_PMU_CHG_CTRL7,
					 MT6360_MASK_ICHG,
					 data << MT6360_SHFT_ICHG);
	if (ret < 0)
		dev_notice(mpci->dev, "%s: fail\n", __func__);
	else
		mpci->ichg = uA;
	return ret;
}

static int mt6360_set_ichg(struct charger_device *chg_dev, u32 uA)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	mutex_lock(&mpci->ichg_lock);
	ret = __mt6360_set_ichg(mpci, uA);
	mutex_unlock(&mpci->ichg_lock);
	return ret;
}

static int mt6360_get_ichg(struct charger_device *chg_dev, u32 *uA)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL7);
	if (ret < 0)
		return ret;
	ret = (ret & MT6360_MASK_ICHG) >> MT6360_SHFT_ICHG;
	*uA = 100000 + (ret * 100000);
	return 0;
}

static int mt6360_enable_hidden_mode(struct charger_device *chg_dev, bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	static const u8 pascode[] = { 0x69, 0x96, 0x63, 0x72, };
	int ret = 0;

	mutex_lock(&mpci->hidden_mode_lock);
	if (en) {
		if (mpci->hidden_mode_cnt == 0) {
			ret = mt6360_pmu_reg_block_write(mpci->mpi,
					   MT6360_PMU_TM_PAS_CODE1, 4, pascode);
			if (ret < 0)
				goto err;
		}
		mpci->hidden_mode_cnt++;
	} else {
		if (mpci->hidden_mode_cnt == 1)
			ret = mt6360_pmu_reg_write(mpci->mpi,
						 MT6360_PMU_TM_PAS_CODE1, 0x00);
		mpci->hidden_mode_cnt--;
		if (ret < 0)
			goto err;
	}
	mt_dbg(mpci->dev, "%s: en = %d\n", __func__, en);
	goto out;
err:
	dev_err(mpci->dev, "%s failed, en = %d\n", __func__, en);
out:
	mutex_unlock(&mpci->hidden_mode_lock);
	return ret;
}

static int mt6360_enable(struct charger_device *chg_dev, bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;
	u32 ichg_ramp_t = 0;

	mt_dbg(mpci->dev, "%s: en = %d\n", __func__, en);

	/* Workaround for vsys overshoot */
	mutex_lock(&mpci->ichg_lock);
	if (mpci->ichg < 500000) {
		dev_info(mpci->dev,
			 "%s: ichg < 500mA, bypass vsys wkard\n", __func__);
		goto out;
	}
	if (!en) {
		mpci->ichg_dis_chg = mpci->ichg;
		ichg_ramp_t = (mpci->ichg - 500000) / 50000 * 2;
		/* Set ichg to 500mA */
		ret = mt6360_pmu_reg_update_bits(mpci->mpi,
						 MT6360_PMU_CHG_CTRL7,
						 MT6360_MASK_ICHG,
						 0x04 << MT6360_SHFT_ICHG);
		if (ret < 0) {
			dev_notice(mpci->dev,
				   "%s: set ichg fail\n", __func__);
			goto vsys_wkard_fail;
		}
		mdelay(ichg_ramp_t);
	} else {
		if (mpci->ichg == mpci->ichg_dis_chg) {
			ret = __mt6360_set_ichg(mpci, mpci->ichg);
			if (ret < 0) {
				dev_notice(mpci->dev,
					   "%s: set ichg fail\n", __func__);
				goto out;
			}
		}
	}

out:
	ret = mt6360_pmu_reg_update_bits(mpci->mpi,
					 MT6360_PMU_CHG_CTRL2,
					 MT6360_MASK_CHG_EN, en ? 0xff : 0);
	if (ret < 0)
		dev_notice(mpci->dev, "%s: fail, en = %d\n", __func__, en);
vsys_wkard_fail:
	mutex_unlock(&mpci->ichg_lock);
	mt6360_power_supply_changed(mpci);
	return ret;
}

static int mt6360_get_min_ichg(struct charger_device *chg_dev, u32 *uA)
{
	*uA = 300000;
	return 0;
}

static int mt6360_set_cv(struct charger_device *chg_dev, u32 uV)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	u8 data = 0;

	dev_dbg(mpci->dev, "%s: cv = %d\n", __func__, uV);
	data = mt6360_trans_cv_sel(uV);
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL4,
					  MT6360_MASK_VOREG,
					  data << MT6360_SHFT_VOREG);
}

static int mt6360_get_cv(struct charger_device *chg_dev, u32 *uV)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL4);
	if (ret < 0)
		return ret;
	ret = (ret & MT6360_MASK_VOREG) >> MT6360_SHFT_VOREG;
	*uV = 3900000 + (ret * 10000);
	return 0;
}

static int mt6360_toggle_aicc(struct mt6360_pmu_chg_info *mpci)
{
	int ret = 0;
	u8 data = 0;

	mutex_lock(&mpci->mpi->io_lock);
	/* read aicc */
	ret = i2c_smbus_read_i2c_block_data(mpci->mpi->i2c,
					       MT6360_PMU_CHG_CTRL14, 1, &data);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: read aicc fail\n", __func__);
		goto out;
	}
	/* aicc off */
	data &= ~MT6360_MASK_RG_EN_AICC;
	ret = i2c_smbus_read_i2c_block_data(mpci->mpi->i2c,
					       MT6360_PMU_CHG_CTRL14, 1, &data);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: aicc off fail\n", __func__);
		goto out;
	}
	/* aicc on */
	data |= MT6360_MASK_RG_EN_AICC;
	ret = i2c_smbus_read_i2c_block_data(mpci->mpi->i2c,
					       MT6360_PMU_CHG_CTRL14, 1, &data);
	if (ret < 0)
		dev_err(mpci->dev, "%s: aicc on fail\n", __func__);
out:
	mutex_unlock(&mpci->mpi->io_lock);
	return ret;
}

static int mt6360_set_aicr(struct charger_device *chg_dev, u32 uA)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);
	int ret = 0;
	u8 data = 0;

	dev_dbg(mpci->dev, "%s\n", __func__);
	/* Toggle aicc for auto aicc mode */
	if (!pdata->aicc_once) {
		ret = mt6360_toggle_aicc(mpci);
		if (ret < 0) {
			dev_err(mpci->dev, "%s: toggle aicc fail\n", __func__);
			return ret;
		}
	}
	/* Disable sys drop improvement for download mode */
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_CHG_CTRL20,
				      MT6360_MASK_EN_SDI);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: disable en_sdi fail\n", __func__);
		return ret;
	}
	data = mt6360_trans_aicr_sel(uA);
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL3,
					  MT6360_MASK_AICR,
					  data << MT6360_SHFT_AICR);
}

static int mt6360_get_aicr(struct charger_device *chg_dev, u32 *uA)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL3);
	if (ret < 0)
		return ret;
	ret = (ret & MT6360_MASK_AICR) >> MT6360_SHFT_AICR;
	*uA = 100000 + (ret * 50000);
	return 0;
}

static int mt6360_get_min_aicr(struct charger_device *chg_dev, u32 *uA)
{
	*uA = 100000;
	return 0;
}

static int mt6360_set_ieoc(struct charger_device *chg_dev, u32 uA)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	u8 data = 0;

	dev_dbg(mpci->dev, "%s: ieoc = %d\n", __func__, uA);
	data = mt6360_trans_ieoc_sel(uA);
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL9,
					  MT6360_MASK_IEOC,
					  data << MT6360_SHFT_IEOC);
}

static int mt6360_set_mivr(struct charger_device *chg_dev, u32 uV)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	u32 aicc_vth = 0, data = 0;
	u8 aicc_vth_sel = 0;
	int ret = 0;

	mt_dbg(mpci->dev, "%s: mivr = %d\n", __func__, uV);
	if (uV < 3900000 || uV > 13400000) {
		dev_err(mpci->dev,
			"%s: unsuitable mivr val(%d)\n", __func__, uV);
		return -EINVAL;
	}
	/* Check if there's a suitable AICC_VTH */
	aicc_vth = uV + 200000;
	aicc_vth_sel = (aicc_vth - 3900000) / 100000;
	if (aicc_vth_sel > MT6360_AICC_VTH_MAXVAL) {
		dev_err(mpci->dev, "%s: can't match, aicc_vth_sel = %d\n",
			__func__, aicc_vth_sel);
		return -EINVAL;
	}
	/* Set AICC_VTH threshold */
	ret = mt6360_pmu_reg_update_bits(mpci->mpi,
					 MT6360_PMU_CHG_CTRL16,
					 MT6360_MASK_AICC_VTH,
					 aicc_vth_sel << MT6360_SHFT_AICC_VTH);
	if (ret < 0)
		return ret;
	/* Set MIVR */
	data = mt6360_trans_mivr_sel(uV);
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL6,
					  MT6360_MASK_MIVR,
					  data << MT6360_SHFT_MIVR);
}

static inline int mt6360_get_mivr(struct charger_device *chg_dev, u32 *uV)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL6);
	if (ret < 0)
		return ret;
	ret = (ret & MT6360_MASK_MIVR) >> MT6360_SHFT_MIVR;
	*uV = 3900000 + (ret * 100000);
	return 0;
}

static int mt6360_get_mivr_state(struct charger_device *chg_dev, bool *in_loop)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT1);
	if (ret < 0)
		return ret;
	*in_loop = (ret & MT6360_MASK_MIVR_EVT) >> MT6360_SHFT_MIVR_EVT;
	return 0;
}

static int mt6360_enable_te(struct charger_device *chg_dev, bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);

	dev_info(mpci->dev, "%s: en = %d\n", __func__, en);
	if (!pdata->en_te)
		return 0;
	return mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_CHG_CTRL2,
					  MT6360_MASK_TE_EN, en ? 0xff : 0);
}

static int mt6360_enable_pump_express(struct mt6360_pmu_chg_info *mpci,
				      bool pe20)
{
	long timeout, pe_timeout = pe20 ? 1400 : 2800;
	int ret = 0;

	dev_info(mpci->dev, "%s\n", __func__);
	ret = mt6360_set_aicr(mpci->chg_dev, 800000);
	if (ret < 0)
		return ret;
	ret = mt6360_set_ichg(mpci->chg_dev, 2000000);
	if (ret < 0)
		return ret;
	ret = mt6360_enable(mpci->chg_dev, true);
	if (ret < 0)
		return ret;
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_CHG_CTRL17,
				      MT6360_MASK_EN_PUMPX);
	if (ret < 0)
		return ret;
	ret = mt6360_pmu_reg_set_bits(mpci->mpi, MT6360_PMU_CHG_CTRL17,
				      MT6360_MASK_EN_PUMPX);
	if (ret < 0)
		return ret;
	reinit_completion(&mpci->pumpx_done);
	atomic_set(&mpci->pe_complete, 1);
	timeout = wait_for_completion_interruptible_timeout(
			       &mpci->pumpx_done, msecs_to_jiffies(pe_timeout));
	if (timeout == 0)
		ret = -ETIMEDOUT;
	else if (timeout < 0)
		ret = -EINTR;
	else
		ret = 0;
	if (ret < 0)
		dev_err(mpci->dev,
			"%s: wait pumpx timeout, ret = %d\n", __func__, ret);
	return ret;
}

static int mt6360_set_pep_current_pattern(struct charger_device *chg_dev,
					  bool is_inc)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	dev_dbg(mpci->dev, "%s: pe1.0 pump up = %d\n", __func__, is_inc);

	mutex_lock(&mpci->pe_lock);
	/* Set to PE1.0 */
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi,
				      MT6360_PMU_CHG_CTRL17,
				      MT6360_MASK_PUMPX_20_10);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: enable pumpx 10 fail\n", __func__);
		goto out;
	}

	/* Set Pump Up/Down */
	ret = mt6360_pmu_reg_update_bits(mpci->mpi,
					 MT6360_PMU_CHG_CTRL17,
					 MT6360_MASK_PUMPX_UP_DN,
					 is_inc ? 0xff : 0);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: set pumpx up/down fail\n", __func__);
		goto out;
	}
	ret = mt6360_enable_pump_express(mpci, false);
out:
	mutex_unlock(&mpci->pe_lock);
	return ret;
}

static int mt6360_set_pep20_efficiency_table(struct charger_device *chg_dev)
{
	return 0;
}

static int mt6360_set_pep20_current_pattern(struct charger_device *chg_dev,
					    u32 uV)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;
	u8 data = 0;

	dev_dbg(mpci->dev, "%s: pep2.0 = %d\n", __func__, uV);
	mutex_lock(&mpci->pe_lock);
	if (uV >= 5500000)
		data = (uV - 5500000) / 500000;
	if (data > MT6360_PUMPX_20_MAXVAL)
		data = MT6360_PUMPX_20_MAXVAL;
	/* Set to PE2.0 */
	ret = mt6360_pmu_reg_set_bits(mpci->mpi, MT6360_PMU_CHG_CTRL17,
				      MT6360_MASK_PUMPX_20_10);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: enable pumpx 20 fail\n", __func__);
		goto out;
	}
	/* Set Voltage */
	ret = mt6360_pmu_reg_update_bits(mpci->mpi,
					 MT6360_PMU_CHG_CTRL17,
					 MT6360_MASK_PUMPX_DEC,
					 data << MT6360_SHFT_PUMPX_DEC);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: set pumpx voltage fail\n", __func__);
		goto out;
	}
	ret = mt6360_enable_pump_express(mpci, true);
out:
	mutex_unlock(&mpci->pe_lock);
	return ret;
}

static int mt6360_reset_ta(struct charger_device *chg_dev)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	dev_dbg(mpci->dev, "%s\n", __func__);
	ret = mt6360_set_mivr(chg_dev, 4600000);
	if (ret < 0)
		return ret;
	ret = mt6360_select_input_current_limit(mpci, MT6360_IINLMTSEL_AICR);
	if (ret < 0)
		return ret;
	ret = mt6360_set_aicr(chg_dev, 100000);
	if (ret < 0)
		return ret;
	msleep(250);
	return mt6360_set_aicr(chg_dev, 500000);
}

static int mt6360_enable_cable_drop_comp(struct charger_device *chg_dev,
					 bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	dev_info(mpci->dev, "%s: en = %d\n", __func__, en);
	if (en)
		return ret;

	/* Set to PE2.0 */
	mutex_lock(&mpci->pe_lock);
	ret = mt6360_pmu_reg_set_bits(mpci->mpi, MT6360_PMU_CHG_CTRL17,
				      MT6360_MASK_PUMPX_20_10);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: enable pumpx 20 fail\n", __func__);
		goto out;
	}
	/* Disable cable drop compensation */
	ret = mt6360_pmu_reg_update_bits(mpci->mpi,
					 MT6360_PMU_CHG_CTRL17,
					 MT6360_MASK_PUMPX_DEC,
					 0x1F << MT6360_SHFT_PUMPX_DEC);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: set pumpx voltage fail\n", __func__);
		goto out;
	}
	ret = mt6360_enable_pump_express(mpci, true);
out:
	mutex_unlock(&mpci->pe_lock);
	return ret;
}

static inline int mt6360_get_aicc(struct mt6360_pmu_chg_info *mpci,
				  u32 *aicc_val)
{
	u8 aicc_sel = 0;
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_AICC_RESULT);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: read aicc result fail\n", __func__);
		return ret;
	}
	aicc_sel = (ret & MT6360_MASK_RG_AICC_RESULT) >>
						     MT6360_SHFT_RG_AICC_RESULT;
	*aicc_val = (aicc_sel * 50000) + 100000;
	return 0;
}

static inline int mt6360_post_aicc_measure(struct charger_device *chg_dev,
					   u32 start, u32 stop, u32 step,
					   u32 *measure)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int cur, ret;

	mt_dbg(mpci->dev,
		"%s: post_aicc = (%d, %d, %d)\n", __func__, start, stop, step);
	for (cur = start; cur < (stop + step); cur += step) {
		/* set_aicr to cur */
		ret = mt6360_set_aicr(chg_dev, cur + step);
		if (ret < 0)
			return ret;
		usleep_range(150, 200);
		ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT1);
		if (ret < 0)
			return ret;
		/* read mivr stat */
		if (ret & MT6360_MASK_MIVR_EVT)
			break;
	}
	*measure = cur;
	return 0;
}

static int mt6360_run_aicc(struct charger_device *chg_dev, u32 *uA)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);
	int ret = 0;
	u32 aicc_val = 0, aicr_val;
	long timeout;
	bool mivr_stat = false;

	mt_dbg(mpci->dev, "%s: aicc_once = %d\n", __func__, pdata->aicc_once);
	/* check MIVR stat is act */
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT1);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: read mivr stat fail\n", __func__);
		return ret;
	}
	mivr_stat = (ret & MT6360_MASK_MIVR_EVT) ? true : false;
	if (!mivr_stat) {
		dev_err(mpci->dev, "%s: mivr stat not act\n", __func__);
		return ret;
	}

	/* Auto run aicc */
	if (!pdata->aicc_once) {
		if (!try_wait_for_completion(&mpci->aicc_done)) {
			dev_info(mpci->dev, "%s: aicc is not act\n", __func__);
			return 0;
		}

		/* get aicc result */
		ret = mt6360_get_aicc(mpci, &aicc_val);
		if (ret < 0) {
			dev_err(mpci->dev,
				"%s: get aicc fail\n", __func__);
			return ret;
		}
		*uA = aicc_val;
		reinit_completion(&mpci->aicc_done);
		return ret;
	}

	/* Use aicc once method */
	/* Run AICC measure */
	mutex_lock(&mpci->pe_lock);
	ret = mt6360_pmu_reg_set_bits(mpci->mpi, MT6360_PMU_CHG_CTRL14,
				      MT6360_MASK_RG_EN_AICC);
	if (ret < 0)
		goto out;
	/* Clear AICC measurement IRQ */
	reinit_completion(&mpci->aicc_done);
	timeout = wait_for_completion_interruptible_timeout(
				   &mpci->aicc_done, msecs_to_jiffies(3000));
	if (timeout == 0)
		ret = -ETIMEDOUT;
	else if (timeout < 0)
		ret = -EINTR;
	else
		ret = 0;
	if (ret < 0) {
		dev_err(mpci->dev,
			"%s: wait AICC time out, ret = %d\n", __func__, ret);
		goto out;
	}
	/* get aicc_result */
	ret = mt6360_get_aicc(mpci, &aicc_val);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: get aicc result fail\n", __func__);
		goto out;
	}

	if (!pdata->post_aicc)
		goto skip_post_aicc;

	dev_info(mpci->dev, "%s: aicc pre val = %d\n", __func__, aicc_val);
	ret = mt6360_get_aicr(chg_dev, &aicr_val);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: get aicr fail\n", __func__);
		goto out;
	}
	ret = mt6360_set_aicr(chg_dev, aicc_val);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: set aicr fail\n", __func__);
		goto out;
	}
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_CHG_CTRL14,
				      MT6360_MASK_RG_EN_AICC);
	if (ret < 0)
		goto out;
	/* always start/end aicc_val/aicc_val+200mA */
	ret = mt6360_post_aicc_measure(chg_dev, aicc_val,
				       aicc_val + 200000, 50000, &aicc_val);
	if (ret < 0)
		goto out;
	ret = mt6360_set_aicr(chg_dev, aicr_val);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: set aicr fail\n", __func__);
		goto out;
	}
	dev_info(mpci->dev, "%s: aicc post val = %d\n", __func__, aicc_val);
skip_post_aicc:
	*uA = aicc_val;
out:
	/* Clear EN_AICC */
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_CHG_CTRL14,
				      MT6360_MASK_RG_EN_AICC);
	mutex_unlock(&mpci->pe_lock);
	return ret;
}

static int mt6360_enable_power_path(struct charger_device *chg_dev,
					    bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	dev_dbg(mpci->dev, "%s: en = %d\n", __func__, en);
	return mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_CHG_CTRL1,
					MT6360_MASK_FORCE_SLEEP, en ? 0 : 0xff);
}

static int mt6360_is_power_path_enabled(struct charger_device *chg_dev,
						bool *en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL1);
	if (ret < 0)
		return ret;
	*en = (ret & MT6360_MASK_FORCE_SLEEP) ? false : true;
	return 0;
}

static int mt6360_enable_safety_timer(struct charger_device *chg_dev,
					      bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	dev_dbg(mpci->dev, "%s: en = %d\n", __func__, en);
	return mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_CHG_CTRL12,
					  MT6360_MASK_TMR_EN, en ? 0xff : 0);
}

static int mt6360_is_safety_timer_enabled(
				struct charger_device *chg_dev, bool *en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL12);
	if (ret < 0)
		return ret;
	*en = (ret & MT6360_MASK_TMR_EN) ? true : false;
	return 0;
}
#ifdef DELECHECK
// for hz
static int mt6360_enable_hz(struct charger_device *chg_dev, bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	dev_info(mpci->dev, "%s: en = %d\n", __func__, en);

	ret = (en ? mt6360_pmu_reg_set_bits : mt6360_pmu_reg_clr_bits)
		(mpci->mpi, MT6360_PMU_CHG_CTRL1, MT6360_MASK_HZ_EN);

	return ret;
}
#endif
static const u32 otg_oc_table[] = {
	500000, 700000, 1100000, 1300000, 1800000, 2100000, 2400000, 3000000
};

static int mt6360_set_otg_current_limit(struct charger_device *chg_dev,
						u32 uA)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int i;

	/* Set higher OC threshold protect */
	for (i = 0; i < ARRAY_SIZE(otg_oc_table); i++) {
		if (uA <= otg_oc_table[i])
			break;
	}
	if (i == ARRAY_SIZE(otg_oc_table))
		i = MT6360_OTG_OC_MAXVAL;
	dev_dbg(mpci->dev,
		"%s: select oc threshold = %d\n", __func__, otg_oc_table[i]);

	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  MT6360_PMU_CHG_CTRL10,
					  MT6360_MASK_OTG_OC,
					  i << MT6360_SHFT_OTG_OC);
}

static int mt6360_enable_otg(struct charger_device *chg_dev, bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;
#ifdef OPLUS_FEATURE_CHG_BASIC
	static struct power_supply *usb_psy = NULL;
	if (!usb_psy) {
		usb_psy = power_supply_get_by_name("usb");
		//dev_err(mpci->dev, "%s: battery_psy null\n", __func__);
	}
#endif /*OPLUS_FEATURE_CHG_BASIC*/

	dev_dbg(mpci->dev, "%s: en = %d\n", __func__, en);
	ret = mt6360_enable_otg_wdt(mpci, en ? true : false);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: set wdt fail, en = %d\n", __func__, en);
		return ret;
	}
#ifdef OPLUS_FEATURE_CHG_BASIC
	oplus_chg_set_otg_online(en ? true : false);
	if (usb_psy)
		power_supply_changed(usb_psy);
#endif
	return mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_CHG_CTRL1,
					  MT6360_MASK_OPA_MODE, en ? 0xff : 0);
}

static int mt6360_enable_discharge(struct charger_device *chg_dev,
					   bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int i, ret = 0;
	const int dischg_retry_cnt = 3;
	bool is_dischg;

	dev_dbg(mpci->dev, "%s: en = %d\n", __func__, en);
	ret = mt6360_enable_hidden_mode(mpci->chg_dev, true);
	if (ret < 0)
		return ret;
	/* Set bit2 of reg[0x31] to 1/0 to enable/disable discharging */
	ret = mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_CHG_HIDDEN_CTRL2,
					 MT6360_MASK_DISCHG, en ? 0xff : 0);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: fail, en = %d\n", __func__, en);
		goto out;
	}

	if (!en) {
		for (i = 0; i < dischg_retry_cnt; i++) {
			ret = mt6360_pmu_reg_read(mpci->mpi,
						  MT6360_PMU_CHG_HIDDEN_CTRL2);
			is_dischg = (ret & MT6360_MASK_DISCHG) ? true : false;
			if (!is_dischg)
				break;
			ret = mt6360_pmu_reg_clr_bits(mpci->mpi,
						MT6360_PMU_CHG_HIDDEN_CTRL2,
						MT6360_MASK_DISCHG);
			if (ret < 0) {
				dev_err(mpci->dev,
					"%s: disable dischg failed\n",
					__func__);
				goto out;
			}
		}
		if (i == dischg_retry_cnt) {
			dev_err(mpci->dev, "%s: dischg failed\n", __func__);
			ret = -EINVAL;
		}
	}
out:
	mt6360_enable_hidden_mode(mpci->chg_dev, false);
	return ret;
}

static int mt6360_enable_chg_type_det(struct charger_device *chg_dev, bool en)
{
	int ret = 0;
#if defined(CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT) && defined(CONFIG_TCPC_CLASS)
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);

	dev_info(mpci->dev, "%s: en = %d\n", __func__, en);

	if (pdata->bc12_sel != 0)
		return ret;

	mutex_lock(&mpci->chgdet_lock);
	if (mpci->tcpc_attach == en) {
		dev_info(mpci->dev, "%s attach(%d) is the same\n",
			 __func__, mpci->tcpc_attach);
		goto out;
	}
	mpci->tcpc_attach = en;
	ret = (en ? mt6360_chgdet_pre_process :
		    mt6360_chgdet_post_process)(mpci);
out:
	mutex_unlock(&mpci->chgdet_lock);
#endif /* CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT && CONFIG_TCPC_CLASS */
	return ret;
}

static int mt6360_get_adc(struct charger_device *chg_dev, enum adc_channel chan,
			  int *min, int *max)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	enum mt6360_adc_channel channel;
	int ret = 0;

	switch (chan) {
	case ADC_CHANNEL_VBUS:
		channel = MT6360_ADC_VBUSDIV5;
		break;
	case ADC_CHANNEL_VSYS:
		channel = MT6360_ADC_VSYS;
		break;
	case ADC_CHANNEL_VBAT:
		channel = MT6360_ADC_VBAT;
		break;
	case ADC_CHANNEL_IBUS:
		channel = MT6360_ADC_IBUS;
		break;
	case ADC_CHANNEL_IBAT:
		channel = MT6360_ADC_IBAT;
		break;
	case ADC_CHANNEL_TEMP_JC:
		channel = MT6360_ADC_TEMP_JC;
		break;
	case ADC_CHANNEL_USBID:
		channel = MT6360_ADC_USBID;
		break;
	case ADC_CHANNEL_TS:
		channel = MT6360_ADC_TS;
		break;
	default:
		return -ENOTSUPP;
	}
	ret = iio_read_channel_processed(mpci->channels[channel], min);
	if (ret < 0) {
		dev_info(mpci->dev, "%s: fail(%d)\n", __func__, ret);
		return ret;
	}
	*max = *min;
	return 0;
}

static int mt6360_get_vbus(struct charger_device *chg_dev, u32 *vbus)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

#ifdef OPLUS_FEATURE_CHG_BASIC
	if (oplusmpci) {
		if (atomic_read(&oplusmpci->suspended) == 1)
			return -1;
	}
#endif
	mt_dbg(mpci->dev, "%s\n", __func__);
	return mt6360_get_adc(chg_dev, ADC_CHANNEL_VBUS, vbus, vbus);
}

static int mt6360_get_ibus(struct charger_device *chg_dev, u32 *ibus)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	mt_dbg(mpci->dev, "%s\n", __func__);
	return mt6360_get_adc(chg_dev, ADC_CHANNEL_IBUS, ibus, ibus);
}

static int mt6360_get_ibat(struct charger_device *chg_dev, u32 *ibat)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	mt_dbg(mpci->dev, "%s\n", __func__);
	return mt6360_get_adc(chg_dev, ADC_CHANNEL_IBAT, ibat, ibat);
}

static int mt6360_get_tchg(struct charger_device *chg_dev,
				   int *tchg_min, int *tchg_max)
{
#ifdef OPLUS_FEATURE_CHG_BASIC
	oplus_mt6360_get_tchg(tchg_min,tchg_max);
	printk("%s: --tchg = %d\n", __func__, tchg_min);
#else
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int temp_jc = 0, ret = 0, retry_cnt = 3;

	mt_dbg(mpci->dev, "%s\n", __func__);
	/* temp abnormal Workaround */
	do {
		ret = mt6360_get_adc(chg_dev, ADC_CHANNEL_TEMP_JC,
				     &temp_jc, &temp_jc);
		if (ret < 0) {
			dev_err(mpci->dev,
				"%s: failed, ret = %d\n", __func__, ret);
			return ret;
		}
	} while (temp_jc >= 120 && (retry_cnt--) > 0);
	mutex_lock(&mpci->tchg_lock);
	if (temp_jc >= 120)
		temp_jc = mpci->tchg;
	else
		mpci->tchg = temp_jc;
	mutex_unlock(&mpci->tchg_lock);
	*tchg_min = *tchg_max = temp_jc;
	dev_err(mpci->dev, "%s: tchg = %d\n", __func__, temp_jc);
#endif
	return 0;
}

static int mt6360_kick_wdt(struct charger_device *chg_dev)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

#ifdef OPLUS_FEATURE_CHG_BASIC
	if (oplusmpci) {
		if (atomic_read(&oplusmpci->suspended) == 1)
			return -1;
	}
#endif
	dev_dbg(mpci->dev, "%s\n", __func__);
	return mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL1);
}

static int mt6360_safety_check(struct charger_device *chg_dev, u32 polling_ieoc)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret, ibat = 0;
	static int eoc_cnt;

	mt_dbg(mpci->dev, "%s\n", __func__);
	ret = iio_read_channel_processed(mpci->channels[MT6360_ADC_IBAT],
					 &ibat);
	if (ret < 0)
		dev_err(mpci->dev, "%s: failed, ret = %d\n", __func__, ret);

	if (ibat <= polling_ieoc)
		eoc_cnt++;
	else
		eoc_cnt = 0;
	/* If ibat is less than polling_ieoc for 3 times, trigger EOC event */
	if (eoc_cnt == 3) {
		dev_info(mpci->dev, "%s: polling_ieoc = %d, ibat = %d\n",
			 __func__, polling_ieoc, ibat);
		charger_dev_notify(mpci->chg_dev, CHARGER_DEV_NOTIFY_EOC);
		eoc_cnt = 0;
	}
	return ret;
}

static int mt6360_reset_eoc_state(struct charger_device *chg_dev)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	dev_dbg(mpci->dev, "%s\n", __func__);

	ret = mt6360_enable_hidden_mode(mpci->chg_dev, true);
	if (ret < 0)
		return ret;
	ret = mt6360_pmu_reg_set_bits(mpci->mpi, MT6360_PMU_CHG_HIDDEN_CTRL1,
				      MT6360_MASK_EOC_RST);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: set failed, ret = %d\n", __func__, ret);
		goto out;
	}
	udelay(100);
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_CHG_HIDDEN_CTRL1,
				      MT6360_MASK_EOC_RST);
	if (ret < 0) {
		dev_err(mpci->dev,
			"%s: clear failed, ret = %d\n", __func__, ret);
		goto out;
	}
out:
	mt6360_enable_hidden_mode(mpci->chg_dev, false);
	return ret;
}

static int mt6360_is_charging_done(struct charger_device *chg_dev,
					   bool *done)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	enum mt6360_charging_status chg_stat;
	int ret = 0;

	mt_dbg(mpci->dev, "%s\n", __func__);
	ret = mt6360_get_charging_status(mpci, &chg_stat);
	if (ret < 0)
		return ret;
	*done = (chg_stat == MT6360_CHG_STATUS_DONE) ? true : false;
	return 0;
}

static int mt6360_get_zcv(struct charger_device *chg_dev, u32 *uV)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	dev_info(mpci->dev, "%s: zcv = %dmV\n", __func__, mpci->zcv / 1000);
	*uV = mpci->zcv;
	return 0;
}

static int mt6360_dump_registers(struct charger_device *chg_dev)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int i, ret = 0;
	int adc_vals[MT6360_ADC_MAX];
	u32 ichg = 0, aicr = 0, mivr = 0, cv = 0, ieoc = 0;
	enum mt6360_charging_status chg_stat = MT6360_CHG_STATUS_READY;
	bool chg_en = false;
	u8 chg_stat1 = 0, chg_ctrl[2] = {0};

	dev_dbg(mpci->dev, "%s\n", __func__);
	ret = mt6360_get_ichg(chg_dev, &ichg);
	ret |= mt6360_get_aicr(chg_dev, &aicr);
	ret |= mt6360_get_mivr(chg_dev, &mivr);
	ret |= mt6360_get_cv(chg_dev, &cv);
	ret |= mt6360_get_ieoc(mpci, &ieoc);
	ret |= mt6360_get_charging_status(mpci, &chg_stat);
	ret |= mt6360_is_charger_enabled(mpci, &chg_en);
	if (ret < 0) {
		dev_notice(mpci->dev, "%s: parse chg setting fail\n", __func__);
		return ret;
	}
	for (i = 0; i < MT6360_ADC_MAX; i++) {
		/* Skip unnecessary channel */
		if (i >= MT6360_ADC_NODUMP)
			break;
		ret = iio_read_channel_processed(mpci->channels[i],
						 &adc_vals[i]);
		if (ret < 0) {
			dev_err(mpci->dev,
				"%s: read [%s] adc fail(%d)\n",
				__func__, mt6360_adc_chan_list[i], ret);
			return ret;
		}
	}
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT1);
	if (ret < 0)
		return ret;
	chg_stat1 = ret;

	ret = mt6360_pmu_reg_block_read(mpci->mpi, MT6360_PMU_CHG_CTRL1,
					2, chg_ctrl);
	if (ret < 0)
		return ret;
	dev_info(mpci->dev,
		 "%s: ICHG = %dmA, AICR = %dmA, MIVR = %dmV, IEOC = %dmA, CV = %dmV\n",
		 __func__, ichg / 1000, aicr / 1000, mivr / 1000, ieoc / 1000,
		 cv / 1000);
	dev_info(mpci->dev,
		 "%s: VBUS = %dmV, IBUS = %dmA, VSYS = %dmV, VBAT = %dmV, IBAT = %dmA\n",
		 __func__,
		 adc_vals[MT6360_ADC_VBUSDIV5] / 1000,
		 adc_vals[MT6360_ADC_IBUS] / 1000,
		 adc_vals[MT6360_ADC_VSYS] / 1000,
		 adc_vals[MT6360_ADC_VBAT] / 1000,
		 adc_vals[MT6360_ADC_IBAT] / 1000);
	dev_info(mpci->dev, "%s: CHG_EN = %d, CHG_STATUS = %s, CHG_STAT1 = 0x%02X\n",
		 __func__, chg_en, mt6360_chg_status_name[chg_stat], chg_stat1);
	dev_info(mpci->dev, "%s: CHG_CTRL1 = 0x%02X, CHG_CTRL2 = 0x%02X\n",
		 __func__, chg_ctrl[0], chg_ctrl[1]);
	return 0;
}

static int mt6360_do_event(struct charger_device *chg_dev, u32 event,
				   u32 args)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	mt_dbg(mpci->dev, "%s\n", __func__);

	if (!mpci->psy) {
		dev_notice(mpci->dev, "%s: cannot get psy\n", __func__);
		return -ENODEV;
	}

	switch (event) {
	case EVENT_FULL:
	case EVENT_RECHARGE:
	case EVENT_DISCHARGE:
		mt6360_power_supply_changed(mpci);
		break;
	default:
		break;
	}

	return 0;
}

static int mt6360_plug_in(struct charger_device *chg_dev)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	dev_dbg(mpci->dev, "%s\n", __func__);

	ret = mt6360_enable_wdt(mpci, true);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: en wdt failed\n", __func__);
		return ret;
	}
	/* Replace CHG_EN by TE for avoid CV level too low trigger ieoc */
	/* TODO: First select cv, then chg_en, no need ? */
	ret = mt6360_enable_te(chg_dev, true);
	if (ret < 0)
		dev_err(mpci->dev, "%s: en te failed\n", __func__);

	return ret;
}

static int mt6360_plug_out(struct charger_device *chg_dev)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	int ret = 0;

	dev_dbg(mpci->dev, "%s\n", __func__);

	ret = mt6360_enable_wdt(mpci, false);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: disable wdt failed\n", __func__);
		return ret;
	}
	ret = mt6360_enable_te(chg_dev, false);
	if (ret < 0)
		dev_err(mpci->dev, "%s: disable te failed\n", __func__);

	return ret;
}

static int mt6360_enable_usbid(struct charger_device *chg_dev, bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	return (en ? mt6360_pmu_reg_set_bits : mt6360_pmu_reg_clr_bits)
		(mpci->mpi, MT6360_PMU_USBID_CTRL1, MT6360_MASK_USBID_EN);
}

static int mt6360_set_usbid_rup(struct charger_device *chg_dev, u32 rup)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	u32 data = mt6360_trans_usbid_rup(rup);

	return mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_USBID_CTRL1,
					  MT6360_MASK_ID_RPULSEL,
					  data << MT6360_SHFT_ID_RPULSEL);
}

static int mt6360_set_usbid_src_ton(struct charger_device *chg_dev, u32 src_ton)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);
	u32 data = mt6360_trans_usbid_src_ton(src_ton);

	return mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_USBID_CTRL1,
					  MT6360_MASK_ISTDET,
					  data << MT6360_SHFT_ISTDET);
}

static int mt6360_enable_usbid_floating(struct charger_device *chg_dev, bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	return (en ? mt6360_pmu_reg_set_bits : mt6360_pmu_reg_clr_bits)
		(mpci->mpi, MT6360_PMU_USBID_CTRL2, MT6360_MASK_USBID_FLOAT);
}

static int mt6360_enable_force_typec_otp(struct charger_device *chg_dev,
					 bool en)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	return (en ? mt6360_pmu_reg_set_bits : mt6360_pmu_reg_clr_bits)
		(mpci->mpi, MT6360_PMU_TYPEC_OTP_CTRL,
		 MT6360_MASK_TYPEC_OTP_SWEN);
}

static int mt6360_get_ctd_dischg_status(struct charger_device *chg_dev,
					u8 *status)
{
	struct mt6360_pmu_chg_info *mpci = charger_get_data(chg_dev);

	*status = mpci->ctd_dischg_status;
	return 0;
}

static const struct charger_ops mt6360_chg_ops = {
	/* cable plug in/out */
	.plug_in = mt6360_plug_in,
	.plug_out = mt6360_plug_out,
	/* enable */
	.enable = mt6360_enable,
	/* charging current */
	.set_charging_current = mt6360_set_ichg,
	.get_charging_current = mt6360_get_ichg,
	.get_min_charging_current = mt6360_get_min_ichg,
	/* charging voltage */
	.set_constant_voltage = mt6360_set_cv,
	.get_constant_voltage = mt6360_get_cv,
	/* charging input current */
	.set_input_current = mt6360_set_aicr,
	.get_input_current = mt6360_get_aicr,
	.get_min_input_current = mt6360_get_min_aicr,
	/* set termination current */
	.set_eoc_current = mt6360_set_ieoc,
	/* charging mivr */
	.set_mivr = mt6360_set_mivr,
	.get_mivr = mt6360_get_mivr,
	.get_mivr_state = mt6360_get_mivr_state,
	/* charing termination */
	.enable_termination = mt6360_enable_te,
	/* PE+/PE+20 */
	.send_ta_current_pattern = mt6360_set_pep_current_pattern,
	.set_pe20_efficiency_table = mt6360_set_pep20_efficiency_table,
	.send_ta20_current_pattern = mt6360_set_pep20_current_pattern,
	.reset_ta = mt6360_reset_ta,
	.enable_cable_drop_comp = mt6360_enable_cable_drop_comp,
	.run_aicl = mt6360_run_aicc,
	/* Power path */
	.enable_powerpath = mt6360_enable_power_path,
	.is_powerpath_enabled = mt6360_is_power_path_enabled,
	/* safety timer */
	.enable_safety_timer = mt6360_enable_safety_timer,
	.is_safety_timer_enabled = mt6360_is_safety_timer_enabled,
	/* OTG */
	.enable_otg = mt6360_enable_otg,
	.set_boost_current_limit = mt6360_set_otg_current_limit,
	.enable_discharge = mt6360_enable_discharge,
	/* Charger type detection */
	.enable_chg_type_det = mt6360_enable_chg_type_det,
	/* ADC */
	.get_adc = mt6360_get_adc,
	.get_vbus_adc = mt6360_get_vbus,
	.get_ibus_adc = mt6360_get_ibus,
	.get_ibat_adc = mt6360_get_ibat,
	.get_tchg_adc = mt6360_get_tchg,
	/* kick wdt */
	.kick_wdt = mt6360_kick_wdt,
	/* misc */
	.safety_check = mt6360_safety_check,
	.reset_eoc_state = mt6360_reset_eoc_state,
	.is_charging_done = mt6360_is_charging_done,
	.get_zcv = mt6360_get_zcv,
	.dump_registers = mt6360_dump_registers,
	/* event */
	.event = mt6360_do_event,
	/* TypeC */
	.enable_usbid = mt6360_enable_usbid,
	.set_usbid_rup = mt6360_set_usbid_rup,
	.set_usbid_src_ton = mt6360_set_usbid_src_ton,
	.enable_usbid_floating = mt6360_enable_usbid_floating,
	.enable_force_typec_otp = mt6360_enable_force_typec_otp,
	.get_ctd_dischg_status = mt6360_get_ctd_dischg_status,
	.enable_hidden_mode = mt6360_enable_hidden_mode,
	//.enable_hz = mt6360_enable_hz,
};

static const struct charger_properties mt6360_chg_props = {
	.alias_name = "mt6360_chg",
};

static irqreturn_t mt6360_pmu_chg_treg_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	int ret = 0;

	dev_err(mpci->dev, "%s\n", __func__);
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT1);
	if (ret < 0)
		return ret;
	if ((ret & MT6360_MASK_CHG_TREG) >> MT6360_SHFT_CHG_TREG)
		dev_err(mpci->dev,
			"%s: thermal regulation loop is active\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_aicr_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static void mt6360_pmu_chg_irq_enable(const char *name, int en);
static irqreturn_t mt6360_pmu_chg_mivr_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	mt_dbg(mpci->dev, "%s\n", __func__);
	mt6360_pmu_chg_irq_enable("chg_mivr_evt", 0);
	atomic_inc(&mpci->mivr_cnt);
	wake_up(&mpci->mivr_wq);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_pwr_rdy_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	bool pwr_rdy = false;
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT1);
	if (ret < 0)
		return ret;
	pwr_rdy = (ret & MT6360_MASK_PWR_RDY_EVT);
	dev_info(mpci->dev, "%s: pwr_rdy = %d\n", __func__, pwr_rdy);

	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_batsysuv_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_vsysuv_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_vsysov_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_vbatov_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_vbusov_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	struct chgdev_notify *noti = &(mpci->chg_dev->noti);
	bool vbusov_stat = false;
	int ret = 0;

	dev_warn(mpci->dev, "%s\n", __func__);
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT2);
	if (ret < 0)
		goto out;
	vbusov_stat = !!(ret & BIT(7));
	noti->vbusov_stat = vbusov_stat;
	dev_info(mpci->dev, "%s: stat = %d\n", __func__, vbusov_stat);
out:
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_wd_pmu_det_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_info(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_wd_pmu_done_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_info(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_tmri_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	int ret = 0;

	dev_warn(mpci->dev, "%s\n", __func__);
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT4);
	if (ret < 0)
		return IRQ_HANDLED;
	dev_info(mpci->dev, "%s: chg_stat4 = 0x%02x\n", __func__, ret);
	if (!(ret & MT6360_MASK_CHG_TMRI))
		return IRQ_HANDLED;
	charger_dev_notify(mpci->chg_dev, CHARGER_DEV_NOTIFY_SAFETY_TIMEOUT);

	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_adpbadi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_rvpi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_otpi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_aiccmeasl_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
	complete(&mpci->aicc_done);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chgdet_donei_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_wdtmri_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	int ret;

	dev_warn(mpci->dev, "%s\n", __func__);
	/* Any I2C R/W can kick watchdog timer */
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_CTRL1);
	if (ret < 0)
		dev_err(mpci->dev, "%s: kick wdt failed\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_ssfinishi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_rechgi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_termi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chg_ieoci_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	bool ieoc_stat = false;
	int ret = 0;

	dev_dbg(mpci->dev, "%s\n", __func__);
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT5);
	if (ret < 0)
		goto out;
	ieoc_stat = !!(ret & BIT(7));
	if (!ieoc_stat)
		goto out;
out:
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_pumpx_donei_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_info(mpci->dev, "%s\n", __func__);
	atomic_set(&mpci->pe_complete, 0);
	complete(&mpci->pumpx_done);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_bst_batuvi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_bst_vbusovi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_bst_olpi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_warn(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_attachi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
#ifdef CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT
	mutex_lock(&mpci->chgdet_lock);
	if (!mpci->bc12_en) {
		dev_err(mpci->dev, "%s: bc12 disabled, ignore irq\n",
			__func__);
		goto out;
	}
	mt6360_chgdet_post_process(mpci);
out:
	mutex_unlock(&mpci->chgdet_lock);
#endif /* CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT */
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_detachi_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

#ifdef CONFIG_OPLUS_HVDCP_SUPPORT
static void mt6360_hvdcp_result_check_work(struct work_struct *work)
{
        int ret = 0;
        struct mt6360_pmu_chg_info *mpci =
                (struct mt6360_pmu_chg_info *)container_of(work,
                struct mt6360_pmu_chg_info, hvdcp_result_check_work.work);

        dev_err(mpci->dev, "%s\n", __func__);

        ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_DEVICE_TYPE);
        if (ret < 0) {
                dev_err(mpci->dev, "%s: fail to read device_type\n", __func__);
        }

        dev_err(mpci->dev, "%s: device type: %d\n", __func__, ret);

        if (ret & BIT(3)) {
                dev_err(mpci->dev, "%s: HVDCP detect\n", __func__);
                mpci->hvdcp_type = POWER_SUPPLY_TYPE_USB_HVDCP;
                ret = mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_DPDM_CTRL, 0x1F, 0x15);
#ifdef OPLUS_FEATURE_CHG_BASIC
		oplus_chg_track_record_chg_type_info();
#endif
                if (ret < 0)
                        dev_err(mpci->dev, "%s: fail to write dpdm_ctrl\n", __func__);
        } else {
                mpci->hvdcp_type = POWER_SUPPLY_TYPE_USB_DCP;
                dev_err(mpci->dev, "%s: HVDCP not detect\n", __func__);
	}

}

static void mt6360_hvdcp_work(struct work_struct *work)
{
        int ret = 0;
        struct mt6360_pmu_chg_info *mpci =
                (struct mt6360_pmu_chg_info *)container_of(work,
                struct mt6360_pmu_chg_info, hvdcp_work.work);

	dev_err(mpci->dev, "%s\n", __func__);

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_DEVICE_TYPE);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: fail to read device_type\n", __func__);
	}

	dev_err(mpci->dev, "%s: device type: %d\n", __func__, ret);

	if (ret & BIT(3)) {
		dev_err(mpci->dev, "%s: HVDCP detect\n", __func__);
		mpci->hvdcp_type = POWER_SUPPLY_TYPE_USB_HVDCP;
		ret = mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_DPDM_CTRL, 0x1F, 0x15);
#ifdef OPLUS_FEATURE_CHG_BASIC
		oplus_chg_track_record_chg_type_info();
#endif
		if (ret < 0)
			dev_err(mpci->dev, "%s: fail to write dpdm_ctrl\n", __func__);
	} else {
		mpci->hvdcp_type = POWER_SUPPLY_TYPE_USB_DCP;
		dev_err(mpci->dev, "%s: HVDCP not detect\n", __func__);
	}

}
#endif
static irqreturn_t mt6360_pmu_hvdcp_det_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
#ifdef CONFIG_OPLUS_HVDCP_SUPPORT
	if (mpci->support_hvdcp == true) {
		cancel_delayed_work_sync(&mpci->hvdcp_result_check_work);
		schedule_delayed_work(&mpci->hvdcp_work, msecs_to_jiffies(150));
	}
#endif
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chgdeti_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_dcdti_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;

	dev_dbg(mpci->dev, "%s\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t mt6360_pmu_chrdet_ext_evt_handler(int irq, void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	int ret = 0;
	bool pwr_rdy = false;

	ret = mt6360_get_chrdet_ext_stat(mpci, &pwr_rdy);
	dev_info(mpci->dev, "%s: pwr_rdy = %d\n", __func__, pwr_rdy);
	if (ret < 0)
		goto out;
	if (mpci->pwr_rdy == pwr_rdy)
		goto out;
	mpci->pwr_rdy = pwr_rdy;
#ifdef CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT
#ifndef CONFIG_TCPC_CLASS
	mutex_lock(&mpci->chgdet_lock);
	(pwr_rdy ? mt6360_chgdet_pre_process :
		   mt6360_chgdet_post_process)(mpci);
	mutex_unlock(&mpci->chgdet_lock);
#endif /* !CONFIG_TCPC_CLASS */
#endif /* CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT */
	if (atomic_read(&mpci->pe_complete) && pwr_rdy == true &&
	    mpci->mpi->chip_rev <= 0x02) {
		dev_info(mpci->dev, "%s: re-trigger pe20 pattern\n", __func__);
		queue_work(mpci->pe_wq, &mpci->pe_work);
	}
out:
	return IRQ_HANDLED;
}

static struct mt6360_pmu_irq_desc mt6360_pmu_chg_irq_desc[] = {
	MT6360_PMU_IRQDESC(chg_treg_evt),
	MT6360_PMU_IRQDESC(chg_aicr_evt),
	MT6360_PMU_IRQDESC(chg_mivr_evt),
	MT6360_PMU_IRQDESC(pwr_rdy_evt),
	MT6360_PMU_IRQDESC(chg_batsysuv_evt),
	MT6360_PMU_IRQDESC(chg_vsysuv_evt),
	MT6360_PMU_IRQDESC(chg_vsysov_evt),
	MT6360_PMU_IRQDESC(chg_vbatov_evt),
	MT6360_PMU_IRQDESC(chg_vbusov_evt),
	MT6360_PMU_IRQDESC(wd_pmu_det),
	MT6360_PMU_IRQDESC(wd_pmu_done),
	MT6360_PMU_IRQDESC(chg_tmri),
	MT6360_PMU_IRQDESC(chg_adpbadi),
	MT6360_PMU_IRQDESC(chg_rvpi),
	MT6360_PMU_IRQDESC(otpi),
	MT6360_PMU_IRQDESC(chg_aiccmeasl),
	MT6360_PMU_IRQDESC(chgdet_donei),
	MT6360_PMU_IRQDESC(wdtmri),
	MT6360_PMU_IRQDESC(ssfinishi),
	MT6360_PMU_IRQDESC(chg_rechgi),
	MT6360_PMU_IRQDESC(chg_termi),
	MT6360_PMU_IRQDESC(chg_ieoci),
	MT6360_PMU_IRQDESC(pumpx_donei),
	MT6360_PMU_IRQDESC(bst_batuvi),
	MT6360_PMU_IRQDESC(bst_vbusovi),
	MT6360_PMU_IRQDESC(bst_olpi),
	MT6360_PMU_IRQDESC(attachi),
	MT6360_PMU_IRQDESC(detachi),
	MT6360_PMU_IRQDESC(hvdcp_det),
	MT6360_PMU_IRQDESC(chgdeti),
	MT6360_PMU_IRQDESC(dcdti),
	MT6360_PMU_IRQDESC(chrdet_ext_evt),
};

static void mt6360_pmu_chg_irq_enable(const char *name, int en)
{
	struct mt6360_pmu_irq_desc *irq_desc;
	int i = 0;

	if (unlikely(!name))
		return;
	for (i = 0; i < ARRAY_SIZE(mt6360_pmu_chg_irq_desc); i++) {
		irq_desc = mt6360_pmu_chg_irq_desc + i;
		if (unlikely(!irq_desc->name))
			continue;
		if (!strcmp(irq_desc->name, name)) {
			if (en)
				enable_irq(irq_desc->irq);
			else
				disable_irq_nosync(irq_desc->irq);
			break;
		}
	}
}

static void mt6360_pmu_chg_irq_register(struct platform_device *pdev)
{
	struct mt6360_pmu_irq_desc *irq_desc;
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(mt6360_pmu_chg_irq_desc); i++) {
		irq_desc = mt6360_pmu_chg_irq_desc + i;
		if (unlikely(!irq_desc->name))
			continue;
		ret = platform_get_irq_byname(pdev, irq_desc->name);
		if (ret < 0)
			continue;
		irq_desc->irq = ret;
		ret = devm_request_threaded_irq(&pdev->dev, irq_desc->irq, NULL,
						irq_desc->irq_handler,
						IRQF_TRIGGER_FALLING,
						irq_desc->name,
						platform_get_drvdata(pdev));
		if (ret < 0)
			dev_err(&pdev->dev,
				"request %s irq fail\n", irq_desc->name);
	}
}

static int mt6360_toggle_cfo(struct mt6360_pmu_chg_info *mpci)
{
	int ret = 0;
	u8 data = 0;

	mutex_lock(&mpci->mpi->io_lock);
	/* check if strobe mode */
	ret = i2c_smbus_read_i2c_block_data(mpci->mpi->i2c,
						  MT6360_PMU_FLED_EN, 1, &data);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: read cfo fail\n", __func__);
		goto out;
	}
	if (data & MT6360_MASK_STROBE_EN) {
		dev_err(mpci->dev, "%s: fled in strobe mode\n", __func__);
		goto out;
	}
	/* read data */
	ret = i2c_smbus_read_i2c_block_data(mpci->mpi->i2c,
						MT6360_PMU_CHG_CTRL2, 1, &data);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: read cfo fail\n", __func__);
		goto out;
	}
	/* cfo off */
	data &= ~MT6360_MASK_CFO_EN;
	ret = i2c_smbus_write_i2c_block_data(mpci->mpi->i2c,
						MT6360_PMU_CHG_CTRL2, 1, &data);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: clear cfo fail\n", __func__);
		goto out;
	}
	/* cfo on */
	data |= MT6360_MASK_CFO_EN;
	ret = i2c_smbus_write_i2c_block_data(mpci->mpi->i2c,
						MT6360_PMU_CHG_CTRL2, 1, &data);
	if (ret < 0)
		dev_err(mpci->dev, "%s: set cfo fail\n", __func__);
out:
	mutex_unlock(&mpci->mpi->io_lock);
	return ret;
}

static int mt6360_chg_mivr_task_threadfn(void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	u32 ibus;
	int ret;

	dev_info(mpci->dev, "%s ++\n", __func__);
	while (!kthread_should_stop()) {
		wait_event(mpci->mivr_wq, atomic_read(&mpci->mivr_cnt) > 0 ||
							 kthread_should_stop());
		mt_dbg(mpci->dev, "%s: enter mivr thread\n", __func__);
		if (kthread_should_stop())
			break;
		pm_stay_awake(mpci->dev);
		/* check real mivr stat or not */
		ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT1);
		if (ret < 0)
			goto loop_cont;
		if (!(ret & MT6360_MASK_MIVR_EVT)) {
			mt_dbg(mpci->dev, "%s: mivr stat not act\n", __func__);
			goto loop_cont;
		}
		/* read ibus adc */
		ret = mt6360_get_ibus(mpci->chg_dev, &ibus);
		if (ret < 0) {
			dev_err(mpci->dev, "%s: get ibus adc fail\n", __func__);
			goto loop_cont;
		}
		/* if ibus adc value < 100mA), toggle cfo */
		if (ibus < 100000) {
			dev_dbg(mpci->dev, "%s: enter toggle cfo\n", __func__);
			ret = mt6360_toggle_cfo(mpci);
			if (ret < 0)
				dev_err(mpci->dev,
					"%s: toggle cfo fail\n", __func__);
		}
loop_cont:
		pm_relax(mpci->dev);
		atomic_set(&mpci->mivr_cnt, 0);
		mt6360_pmu_chg_irq_enable("chg_mivr_evt", 1);
		msleep(200);
	}
	dev_info(mpci->dev, "%s --\n", __func__);
	return 0;
}

static void mt6360_trigger_pep_work_handler(struct work_struct *work)
{
	struct mt6360_pmu_chg_info *mpci =
		(struct mt6360_pmu_chg_info *)container_of(work,
		struct mt6360_pmu_chg_info, pe_work);
	int ret = 0;

	ret = mt6360_set_pep20_current_pattern(mpci->chg_dev, 5000000);
	if (ret < 0)
		dev_err(mpci->dev, "%s: trigger pe20 pattern fail\n",
			__func__);
}

#if defined(CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT)\
&& !defined(CONFIG_TCPC_CLASS)
static void mt6360_chgdet_work_handler(struct work_struct *work)
{
	int ret = 0;
	bool pwr_rdy = false;
	struct mt6360_pmu_chg_info *mpci =
		(struct mt6360_pmu_chg_info *)container_of(work,
		struct mt6360_pmu_chg_info, chgdet_work);

	mutex_lock(&mpci->chgdet_lock);
	/* Check PWR_RDY_STAT */
	ret = mt6360_get_chrdet_ext_stat(mpci, &pwr_rdy);
	if (ret < 0)
		goto out;
	/* power not good */
	if (!pwr_rdy)
		goto out;
	/* power good */
	mpci->pwr_rdy = pwr_rdy;
	/* Turn on USB charger detection */
	ret = __mt6360_enable_usbchgen(mpci, true);
	if (ret < 0)
		dev_err(mpci->dev, "%s: en bc12 fail\n", __func__);
out:
	mutex_unlock(&mpci->chgdet_lock);
}
#endif /* CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT && !CONFIG_TCPC_CLASS */

static const struct mt6360_pdata_prop mt6360_pdata_props[] = {
	MT6360_PDATA_VALPROP(ichg, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL7, 2, 0xFC,
			     mt6360_trans_ichg_sel, 0),
	MT6360_PDATA_VALPROP(aicr, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL3, 2, 0xFC,
			     mt6360_trans_aicr_sel, 0),
	MT6360_PDATA_VALPROP(mivr, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL6, 1, 0xFE,
			     mt6360_trans_mivr_sel, 0),
	MT6360_PDATA_VALPROP(cv, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL4, 1, 0xFE,
			     mt6360_trans_cv_sel, 0),
	MT6360_PDATA_VALPROP(ieoc, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL9, 4, 0xF0,
			     mt6360_trans_ieoc_sel, 0),
	MT6360_PDATA_VALPROP(safety_timer, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL12, 5, 0xE0,
			     mt6360_trans_safety_timer_sel, 0),
	MT6360_PDATA_VALPROP(ircmp_resistor, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL18, 3, 0x38,
			     mt6360_trans_ircmp_r_sel, 0),
	MT6360_PDATA_VALPROP(ircmp_vclamp, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL18, 0, 0x07,
			     mt6360_trans_ircmp_vclamp_sel, 0),
	MT6360_PDATA_VALPROP(aicc_once, struct mt6360_chg_platform_data,
			     MT6360_PMU_CHG_CTRL14, 0, 0x04, NULL, 0),
};

static int mt6360_chg_apply_pdata(struct mt6360_pmu_chg_info *mpci,
				  struct mt6360_chg_platform_data *pdata)
{
	int ret;

	dev_dbg(mpci->dev, "%s ++\n", __func__);
	ret = mt6360_pdata_apply_helper(mpci->mpi, pdata, mt6360_pdata_props,
					ARRAY_SIZE(mt6360_pdata_props));
	if (ret < 0)
		return ret;
	dev_dbg(mpci->dev, "%s ++\n", __func__);
	return 0;
}

static const struct mt6360_val_prop mt6360_val_props[] = {
	MT6360_DT_VALPROP(ichg, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(aicr, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(mivr, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(cv, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(ieoc, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(safety_timer, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(ircmp_resistor, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(ircmp_vclamp, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(en_te, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(en_wdt, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(en_otg_wdt, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(aicc_once, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(post_aicc, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(batoc_notify, struct mt6360_chg_platform_data),
	MT6360_DT_VALPROP(bc12_sel, struct mt6360_chg_platform_data),
};

static int mt6360_chg_parse_dt_data(struct device *dev,
				    struct mt6360_chg_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	dev_dbg(dev, "%s ++\n", __func__);
	memcpy(pdata, &def_platform_data, sizeof(*pdata));
	mt6360_dt_parser_helper(np, (void *)pdata,
				mt6360_val_props, ARRAY_SIZE(mt6360_val_props));
	dev_dbg(dev, "%s --\n", __func__);
	return 0;
}

static int mt6360_enable_ilim(struct mt6360_pmu_chg_info *mpci, bool en)
{
	return (en ? mt6360_pmu_reg_set_bits : mt6360_pmu_reg_clr_bits)
		(mpci->mpi, MT6360_PMU_CHG_CTRL3, MT6360_MASK_ILIM_EN);
}

static inline bool mt6360_is_meta_mode(struct mt6360_pmu_chg_info *mpci)
{
	return (mpci->bootmode == META_BOOT || mpci->bootmode == ADVMETA_BOOT);
}

static void mt6360_get_bootmode(struct mt6360_pmu_chg_info *mpci)
{
	struct device_node *boot_node = NULL;
	const struct tag_bootmode *tag = NULL;

	boot_node = of_parse_phandle(mpci->dev->of_node, "bootmode", 0);
	if (!boot_node) {
		dev_notice(mpci->dev, "%s: get bootmode phandle fail\n",
				      __func__);
		return;
	}

	tag = of_get_property(boot_node, "atag,boot", NULL);
	if (!tag) {
		dev_notice(mpci->dev, "%s: get property atag,boot fail\n",
				      __func__);
		return;
	}

	dev_info(mpci->dev, "%s: size:%d tag:0x%X mode:%d type:%d\n",
			    __func__, tag->size, tag->tag,
			    tag->bootmode, tag->boottype);
	mpci->bootmode = tag->bootmode;
}

static int mt6360_chg_init_setting(struct mt6360_pmu_chg_info *mpci)
{
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);
	int ret = 0;

	dev_info(mpci->dev, "%s\n", __func__);

	mt6360_get_bootmode(mpci);

	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHRDET_STAT);
	if (ret >= 0)
		mpci->ctd_dischg_status = ret & 0xE3;
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_CTD_CTRL, 0x40);
	if (ret < 0)
		dev_err(mpci->dev, "%s: disable ctd ctrl fail\n", __func__);
	ret = mt6360_select_input_current_limit(mpci, MT6360_IINLMTSEL_AICR);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: select iinlmtsel by aicr fail\n",
			__func__);
		return ret;
	}
	usleep_range(5000, 6000);
	ret = mt6360_enable_ilim(mpci, false);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: disable ilim fail\n", __func__);
		return ret;
	}
	if (mt6360_is_meta_mode(mpci)) {
		ret = mt6360_pmu_reg_update_bits(mpci->mpi,
						 MT6360_PMU_CHG_CTRL3,
						 MT6360_MASK_AICR,
						 0x02 << MT6360_SHFT_AICR);
		dev_info(mpci->dev, "%s: set aicr to 200mA in meta mode\n",
			__func__);
	}

	/* disable wdt reduce 1mA power consumption */
	ret = mt6360_enable_wdt(mpci, false);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: disable wdt fail\n", __func__);
		return ret;
	}
	/* Disable USB charger type detect, no matter use it or not */
	ret = mt6360_enable_usbchgen(mpci, false);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: disable chg type detect fail\n",
			__func__);
		return ret;
	}
	/* unlock ovp limit for pump express, can be replaced by option */
	ret = mt6360_select_vinovp(mpci, 14500000);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: unlimit vin for pump express\n",
			__func__);
		return ret;
	}
	/* Disable TE, set TE when plug in/out */
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_CHG_CTRL2,
				      MT6360_MASK_TE_EN);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: disable te fail\n", __func__);
		return ret;
	}
	/* Read ZCV */
	ret = mt6360_read_zcv(mpci);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: read zcv fail\n", __func__);
		return ret;
	}
	/* enable AICC_EN if aicc_once = 0 */
	if (!pdata->aicc_once) {
		ret = mt6360_pmu_reg_set_bits(mpci->mpi, MT6360_PMU_CHG_CTRL14,
					      MT6360_MASK_RG_EN_AICC);
		if (ret < 0) {
			dev_err(mpci->dev,
				"%s: enable en_aicc fail\n", __func__);
			return ret;
		}
	}
#ifndef CONFIG_MT6360_DCDTOUT_SUPPORT
	/* Disable DCD */
	ret = mt6360_enable_dcd_tout(mpci, false);
	if (ret < 0)
		dev_notice(mpci->dev, "%s disable dcd fail\n", __func__);
#endif
	/* Check BATSYSUV occurred last time boot-on */
	ret = mt6360_pmu_reg_read(mpci->mpi, MT6360_PMU_CHG_STAT);
	if (ret < 0) {
		dev_err(mpci->dev, "%s: read BATSYSUV fail\n", __func__);
		return ret;
	}
	if (!(ret & MT6360_MASK_CHG_BATSYSUV)) {
		dev_warn(mpci->dev, "%s: BATSYSUV occurred\n", __func__);
		ret = mt6360_pmu_reg_set_bits(mpci->mpi, MT6360_PMU_CHG_STAT,
					      MT6360_MASK_CHG_BATSYSUV);
		if (ret < 0)
			dev_err(mpci->dev,
				"%s: clear BATSYSUV fail\n", __func__);
	}

	/* USBID ID_TD = 32T */
	ret = mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_USBID_CTRL2,
					 MT6360_MASK_IDTD |
					 MT6360_MASK_USBID_FLOAT, 0x62);
#ifdef OPLUS_FEATURE_CHG_BASIC
	if (ret < 0) {
		dev_err(mpci->dev, "%s: set USBID_TD fail\n", __func__);
		return ret;
	}
#endif /*OPLUS_FEATURE_CHG_BASIC*/

	/* Disable TypeC OTP for check EVB version by TS pin */
	ret = mt6360_pmu_reg_clr_bits(mpci->mpi, MT6360_PMU_TYPEC_OTP_CTRL,
				      MT6360_MASK_TYPEC_OTP_EN);
#ifdef OPLUS_FEATURE_CHG_BASIC
	/* DCD Timeout: 300ms */
	ret = mt6360_pmu_reg_update_bits(mpci->mpi, MT6360_PMU_DEVICE_TYPE,
			MT6360_MASK_DCD_TIMEOUT, 0x00);
#endif /*OPLUS_FEATURE_CHG_BASIC*/

	return ret;
}

static int mt6360_set_shipping_mode(struct mt6360_pmu_chg_info *mpci)
{
	struct mt6360_pmu_info *mpi = mpci->mpi;
	int ret;
	u8 data = 0;

	dev_info(mpci->dev, "%s\n", __func__);
	mutex_lock(&mpi->io_lock);
	/* disable shipping mode rst */
	ret = i2c_smbus_read_i2c_block_data(mpi->i2c,
					    MT6360_PMU_CORE_CTRL2, 1, &data);
	if (ret < 0)
		goto out;
	data |= MT6360_MASK_SHIP_RST_DIS;
	dev_info(mpci->dev, "%s: reg[0x06] = 0x%02x\n", __func__, data);
	ret = i2c_smbus_write_i2c_block_data(mpi->i2c,
					     MT6360_PMU_CORE_CTRL2, 1, &data);
	if (ret < 0) {
		dev_err(mpci->dev,
			"%s: fail to disable shipping mode rst\n", __func__);
		goto out;
	}

#ifdef OPLUS_FEATURE_CHG_BASIC
	data = 0xC0;
#else
	data = 0x80;
#endif
	/* enter shipping mode and disable cfo_en/chg_en */
	ret = i2c_smbus_write_i2c_block_data(mpi->i2c,
					     MT6360_PMU_CHG_CTRL2, 1, &data);
	if (ret < 0)
		dev_err(mpci->dev,
			"%s: fail to enter shipping mode\n", __func__);
			
#ifdef OPLUS_FEATURE_CHG_BASIC
	printk("mt6360_set_shipping_mode\n");
#endif
out:
	mutex_unlock(&mpi->io_lock);
	return ret;
}

static ssize_t shipping_mode_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct mt6360_pmu_chg_info *mpci = dev_get_drvdata(dev);
	int32_t tmp = 0;
	int ret = 0;

	if (kstrtoint(buf, 10, &tmp) < 0) {
		dev_notice(dev, "parsing number fail\n");
		return -EINVAL;
	}
	if (tmp != 5526789)
		return -EINVAL;
	ret = mt6360_set_shipping_mode(mpci);
	if (ret < 0)
		return ret;
	return count;
}
static const DEVICE_ATTR_WO(shipping_mode);

#ifdef FIXME /* TODO: wait mtk_charger_intf.h */
void mt6360_recv_batoc_callback(BATTERY_OC_LEVEL tag)
{
	int ret, cnt = 0;

	if (tag != BATTERY_OC_LEVEL_1)
		return;
	while (!pmic_get_register_value(PMIC_RG_INT_STATUS_FG_CUR_H)) {
		if (cnt >= 1) {
			ret = mt6360_set_shipping_mode(g_mpci);
			if (ret < 0)
				dev_err(g_mpci->dev,
					"%s: set shipping mode fail\n",
					__func__);
			else
				dev_info(g_mpci->dev,
					 "%s: set shipping mode done\n",
					 __func__);
		}
		mdelay(8);
		cnt++;
	}
	dev_info(g_mpci->dev, "%s exit, cnt = %d, FG_CUR_H = %d\n",
		 __func__, cnt,
		 pmic_get_register_value(PMIC_RG_INT_STATUS_FG_CUR_H));
}
#endif

#ifdef OPLUS_FEATURE_CHG_BASIC
bool mt6360_get_vbus_status(void)
{
	bool vbus_rising = false;

	if (oplusmpci) {
		if (atomic_read(&oplusmpci->suspended) == 0)
			mt6360_get_chrdet_ext_stat(oplusmpci, &vbus_rising);
	} else {
		printk(KERN_ERR "%s NULL\n", __func__);
	}
	return vbus_rising;
}
EXPORT_SYMBOL(mt6360_get_vbus_status);

int mt6360_get_vbus_rising(void)
{
	bool vbus_rising = false;
	int ret = 0;

	if (oplusmpci) {
		if (atomic_read(&oplusmpci->suspended) == 0)
			ret = mt6360_get_chrdet_ext_stat(oplusmpci, &vbus_rising);
		else
			ret = -1;
	} else {
		printk(KERN_ERR "%s NULL\n", __func__);
		return 0;
	}
	return (ret < 0) ? ret : (vbus_rising ? 1 : 0);
}
EXPORT_SYMBOL(mt6360_get_vbus_rising);

int mt6360_chg_enable(bool en)
{
	int rc = 0;

	if (oplusmpci) {
		if (atomic_read(&oplusmpci->suspended) == 0)
			rc = mt6360_pmu_reg_update_bits(oplusmpci->mpi, MT6360_PMU_CHG_CTRL2,
					MT6360_MASK_CHG_EN, en ? 0xff : 0);
		else
			printk(KERN_ERR "%s in suspended\n", __func__);
	} else {
		printk(KERN_ERR "%s NULL\n", __func__);
	}
	return rc;
}
EXPORT_SYMBOL(mt6360_chg_enable);

int mt6360_check_charging_enable(void)
{
	bool chg_enable = false;

	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return 0;
	}
	if (atomic_read(&oplusmpci->suspended) == 1)
		return 0;
	mt6360_is_charger_enabled(oplusmpci, &chg_enable);
	return chg_enable ? 1 : 0;
}
EXPORT_SYMBOL(mt6360_check_charging_enable);

int mt6360_suspend_charger(bool suspend)
{
	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return -1;
	}
	if (atomic_read(&oplusmpci->suspended) == 1)
		return -1;
	return mt6360_pmu_reg_update_bits(oplusmpci->mpi, MT6360_PMU_CHG_CTRL1,
			MT6360_MASK_FORCE_SLEEP, suspend ? 0xff : 0);
}
EXPORT_SYMBOL(mt6360_suspend_charger);

int mt6360_set_rechg_voltage(int rechg_mv)
{
	unsigned char reg = 0;

	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return -1;
	}
	if (rechg_mv < 150) {
		reg = 0x0;//100mV
	} else if (rechg_mv < 200) {
		reg = 0x1;//150mV
	} else if (rechg_mv < 250) {
		reg = 0x2;//200mV
	} else {
		reg = 0x3;//250mV
	}
	if (atomic_read(&oplusmpci->suspended) == 1)
		return -1;
	return mt6360_pmu_reg_update_bits(oplusmpci->mpi,
			MT6360_PMU_CHG_CTRL11, 0x03, reg);
}
EXPORT_SYMBOL(mt6360_set_rechg_voltage);

int mt6360_reset_charger(void)
{
	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return -1;
	}
	if (atomic_read(&oplusmpci->suspended) == 1)
		return -1;
	return mt6360_pmu_reg_update_bits(oplusmpci->mpi,
			MT6360_PMU_RST1, 0x40, 0x40);
}
EXPORT_SYMBOL(mt6360_reset_charger);

int mt6360_set_chging_term_disable(bool disable)
{
	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return -1;
	}
	if (atomic_read(&oplusmpci->suspended) == 1)
		return -1;
	return mt6360_pmu_reg_update_bits(oplusmpci->mpi,
			MT6360_PMU_CHG_CTRL9, 0x08, disable ? 0x0 : 0x08);
}
EXPORT_SYMBOL(mt6360_set_chging_term_disable);

int mt6360_aicl_enable(bool enable)
{
	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return -1;
	}
	if (atomic_read(&oplusmpci->suspended) == 1)
		return -1;
	return mt6360_pmu_reg_update_bits(oplusmpci->mpi,
			MT6360_PMU_CHG_CTRL6, 0x1, enable ? 1 : 0);
}
EXPORT_SYMBOL(mt6360_aicl_enable);

int mt6360_chg_enable_wdt(bool enable)
{
	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return -1;
	}
	if (atomic_read(&oplusmpci->suspended) == 1)
		return -1;
	return mt6360_enable_wdt(oplusmpci, enable);
}
EXPORT_SYMBOL(mt6360_chg_enable_wdt);

int mt6360_set_register(unsigned char addr, unsigned char mask, unsigned char data)
{
	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return -1;
	}
	if (atomic_read(&oplusmpci->suspended) == 1)
		return -1;
	return mt6360_pmu_reg_update_bits(oplusmpci->mpi, addr, mask, data);
}
EXPORT_SYMBOL(mt6360_set_register);

#ifdef OPLUS_FEATURE_CHG_BASIC
int mt6360_enter_shipmode(void)
{
	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return -1;
	}
	return mt6360_set_shipping_mode(oplusmpci);
}
EXPORT_SYMBOL(mt6360_enter_shipmode);
#endif

#ifdef CONFIG_OPLUS_HVDCP_SUPPORT
enum power_supply_type mt6360_get_hvdcp_type(void)
{
	if (!oplusmpci) {
		printk(KERN_ERR "%s NULL\n", __func__);
		return POWER_SUPPLY_TYPE_USB_DCP;
        }

	return oplusmpci->hvdcp_type;
}

void mt6360_enable_hvdcp_detect(void)
{
        int ret = 0;

	if (!oplusmpci) {
                printk(KERN_ERR "%s oplusmpci NULL\n", __func__);
                return ;
        }

        dev_err(oplusmpci->dev, "%s\n", __func__);

        dev_err(oplusmpci->dev, "%s: enable hvdcp detect\n", __func__);
        
        ret = mt6360_pmu_reg_write(oplusmpci->mpi, HVDCP_DEVICE_TYPE, 0x0);
        if (ret < 0)
                dev_err(oplusmpci->dev, "%s: fail to write hvdcp_device_type\n", __func__);
        
        ret = mt6360_pmu_reg_write(oplusmpci->mpi, MT6360_PMU_DPDM_CTRL, 0x0);
        if (ret < 0)
                dev_err(oplusmpci->dev, "%s: fail to write dpdm_ctrl\n", __func__);
        
        ret = mt6360_pmu_reg_update_bits(oplusmpci->mpi, HVDCP_DEVICE_TYPE, 0x80, 0x80);
        if (ret < 0)
                dev_err(oplusmpci->dev, "%s: fail to write dpdm_ctrl\n", __func__);
        
        schedule_delayed_work(&oplusmpci->hvdcp_result_check_work, msecs_to_jiffies(3000));
}
EXPORT_SYMBOL(mt6360_enable_hvdcp_detect);
#endif
#endif /* OPLUS_FEATURE_CHG_BASIC */

#ifdef OPLUS_FEATURE_CHG_BASIC
enum power_supply_type mt_get_charger_type(void) 
{
	if (!oplusmpci) {
		return POWER_SUPPLY_TYPE_UNKNOWN;
	}

	if (!oplusmpci->typec_attach)
		return POWER_SUPPLY_TYPE_UNKNOWN;

	return oplusmpci->charger_type;
}
#endif 
/* ======================= */
/* MT6360 Power Supply Ops */
/* ======================= */
static int mt6360_pmu_chg_get_online(struct mt6360_pmu_chg_info *mpci,
				     union power_supply_propval *val)
{
	bool online = false;

#ifdef CONFIG_TCPC_CLASS
	mutex_lock(&mpci->attach_lock);
	online = mpci->typec_attach;
	mutex_unlock(&mpci->attach_lock);
#else
	int ret = 0;

	ret = mt6360_get_chrdet_ext_stat(mpci, &online);
	if (ret < 0) {
		dev_notice(mpci->dev,
			   "%s: read chrdet_ext_stat fail\n", __func__);
		return ret;
	}
#endif
#ifndef OPLUS_FEATURE_CHG_BASIC
	dev_info(mpci->dev, "%s: online = %d\n", __func__, online);
#endif
	val->intval = online;

	return 0;
}

static int mt6360_pmu_chg_set_online(struct mt6360_pmu_chg_info *mpci,
				     const union power_supply_propval *val)
{
	return mt6360_enable_chg_type_det(mpci->chg_dev, val->intval);
}

static int mt6360_pmu_chg_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	struct mt6360_pmu_chg_info *mpci = power_supply_get_drvdata(psy);
	enum mt6360_charging_status chg_stat = MT6360_CHG_STATUS_READY;
	int ret = 0;

	val->intval = 0;

#ifndef OPLUS_FEATURE_CHG_BASIC
	dev_dbg(mpci->dev, "%s: prop = %d\n", __func__, psp);
#endif

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		ret = mt6360_pmu_chg_get_online(mpci, val);
		break;
	case POWER_SUPPLY_PROP_AUTHENTIC:
		val->intval = mpci->ignore_usb;
		break;
	case POWER_SUPPLY_PROP_TYPE:
		val->intval = mpci->psy_desc.type;
		break;
	case POWER_SUPPLY_PROP_USB_TYPE:
		val->intval = mpci->psy_usb_type;
		break;
	case POWER_SUPPLY_PROP_STATUS:
		ret = mt6360_pmu_chg_get_online(mpci, val);
		if (!val->intval) {
			val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
			break;
		}
		ret = mt6360_get_charging_status(mpci, &chg_stat);
		switch (chg_stat) {
		case MT6360_CHG_STATUS_READY:
		case MT6360_CHG_STATUS_FAULT:
			val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
			break;
		case MT6360_CHG_STATUS_PROGRESS:
			val->intval = POWER_SUPPLY_STATUS_CHARGING;
			break;
		case MT6360_CHG_STATUS_DONE:
			val->intval = POWER_SUPPLY_STATUS_FULL;
			break;
		default:
			val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
			break;
		}
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		if (mpci->psy_desc.type == POWER_SUPPLY_TYPE_USB)
			val->intval = 500000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		if (mpci->psy_desc.type == POWER_SUPPLY_TYPE_USB)
			val->intval = 5000000;
		break;
	default:
		ret = -ENODATA;
		break;
	}

	return ret;
}

static int mt6360_pmu_chg_set_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       const union power_supply_propval *val)
{
	struct mt6360_pmu_chg_info *mpci = power_supply_get_drvdata(psy);
	int ret = 0;

	dev_dbg(mpci->dev, "%s: prop = %d\n", __func__, psp);
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		ret = mt6360_pmu_chg_set_online(mpci, val);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int mt6360_pmu_chg_property_is_writeable(struct power_supply *psy,
						enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		return 1;
	default:
		return 0;
	}
}

static enum power_supply_property mt6360_pmu_chg_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_AUTHENTIC,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_USB_TYPE,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
};

static const struct power_supply_desc mt6360_pmu_chg_desc = {
	.type			= POWER_SUPPLY_TYPE_USB,
	.properties		= mt6360_pmu_chg_properties,
	.num_properties		= ARRAY_SIZE(mt6360_pmu_chg_properties),
	.get_property		= mt6360_pmu_chg_get_property,
	.set_property		= mt6360_pmu_chg_set_property,
	.property_is_writeable	= mt6360_pmu_chg_property_is_writeable,
	.usb_types		= mt6360_pmu_chg_usb_types,
	.num_usb_types		= ARRAY_SIZE(mt6360_pmu_chg_usb_types),
};

static char *mt6360_pmu_chg_supplied_to[] = {
	"battery",
	"mtk-master-charger"
};

static void mt6360_power_supply_changed(struct mt6360_pmu_chg_info *mpci)
{
	int ret = 0, i = 0;
	union power_supply_propval propval[mt6360_pmu_chg_desc.num_properties];

	memset(propval, 0, sizeof(propval));

	for (i = 0; i < ARRAY_SIZE(propval); i++) {
		ret = mt6360_pmu_chg_get_property(mpci->psy,
						  mt6360_pmu_chg_properties[i],
						  &propval[i]);
		if (ret < 0)
			dev_notice(mpci->dev,
				   "%s: get prop fail(%d), i = %d\n",
				   __func__, ret, i);
	}

	if (memcmp(mpci->old_propval, propval, sizeof(propval))) {
		memcpy(mpci->old_propval, propval, sizeof(propval));
		power_supply_changed(mpci->psy);
	}
}

/*otg_vbus*/
static int mt6360_boost_enable(struct regulator_dev *rdev)
{
	struct mt6360_pmu_chg_info *mpci = rdev_get_drvdata(rdev);

	return mt6360_enable_otg(mpci->chg_dev, true);
}

static int mt6360_boost_disable(struct regulator_dev *rdev)
{
	struct mt6360_pmu_chg_info *mpci = rdev_get_drvdata(rdev);

	return mt6360_enable_otg(mpci->chg_dev, false);
}

static int mt6360_boost_is_enabled(struct regulator_dev *rdev)
{
	struct mt6360_pmu_chg_info *mpci = rdev_get_drvdata(rdev);
	const struct regulator_desc *desc = rdev->desc;
	int ret = 0;

	ret = mt6360_pmu_reg_read(mpci->mpi, desc->enable_reg);

	if (ret < 0)
		return ret;
	return ret & desc->enable_mask ? true : false;
}

static int mt6360_boost_set_voltage_sel(struct regulator_dev *rdev,
					unsigned int sel)
{
	/*sel = min-min_uv/vsetp*/
	struct mt6360_pmu_chg_info *mpci = rdev_get_drvdata(rdev);
	const struct regulator_desc *desc = rdev->desc;
	int shift = ffs(desc->vsel_mask) - 1;
	u8 data = sel + 0x11;//0x11 = 4.85V

	return mt6360_pmu_reg_update_bits(mpci->mpi, desc->enable_reg,
				desc->enable_mask, data<<shift);
}

static int mt6360_boost_get_voltage_sel(struct regulator_dev *rdev)
{
	struct mt6360_pmu_chg_info *mpci = rdev_get_drvdata(rdev);
	const struct regulator_desc *desc = rdev->desc;
	int shift = ffs(desc->vsel_mask) - 1, ret;

	ret = mt6360_pmu_reg_read(mpci->mpi, desc->vsel_reg);
	if (ret < 0)
		return ret;
	return (((ret & desc->vsel_mask) >> shift)-0x11);
}

static int mt6360_boost_set_current_limit(struct regulator_dev *rdev,
					  int min_uA, int max_uA)
{
	struct mt6360_pmu_chg_info *mpci = rdev_get_drvdata(rdev);
	const struct regulator_desc *desc = rdev->desc;
	int i, shift = ffs(desc->csel_mask) - 1;

	for (i = 0; i < ARRAY_SIZE(mt6360_otg_oc_threshold); i++) {
		if (min_uA <= mt6360_otg_oc_threshold[i])
			break;
	}
	if (i == ARRAY_SIZE(mt6360_otg_oc_threshold) ||
		mt6360_otg_oc_threshold[i] > max_uA) {
		dev_notice(mpci->dev,
			"%s: out of current range\n", __func__);
		return -EINVAL;
	}
	dev_info(mpci->dev, "%s: select otg_oc = %d\n",
		 __func__, mt6360_otg_oc_threshold[i]);
	return mt6360_pmu_reg_update_bits(mpci->mpi,
					  desc->csel_reg,
					  desc->csel_mask,
					  i << shift);
}

static int mt6360_boost_get_current_limit(struct regulator_dev *rdev)
{
	struct mt6360_pmu_chg_info *mpci = rdev_get_drvdata(rdev);
	const struct regulator_desc *desc = rdev->desc;
	int shift = ffs(desc->csel_mask) - 1, ret;

	ret = mt6360_pmu_reg_read(mpci->mpi, desc->csel_reg);
	if (ret < 0)
		return ret;
	ret = (ret & desc->csel_mask) >> shift;
	if (ret >= ARRAY_SIZE(mt6360_otg_oc_threshold))
		return -EINVAL;
	return mt6360_otg_oc_threshold[ret];
}

static const struct regulator_ops mt6360_chg_otg_ops = {
	.list_voltage = regulator_list_voltage_linear,
	.enable = mt6360_boost_enable,
	.disable = mt6360_boost_disable,
	.is_enabled = mt6360_boost_is_enabled,
	.set_voltage_sel = mt6360_boost_set_voltage_sel,
	.get_voltage_sel = mt6360_boost_get_voltage_sel,
	.set_current_limit = mt6360_boost_set_current_limit,
	.get_current_limit = mt6360_boost_get_current_limit,
};

static const struct regulator_desc mt6360_otg_rdesc = {
	.of_match = "usb-otg-vbus",
	.name = "usb-otg-vbus",
	.ops = &mt6360_chg_otg_ops,
	.owner = THIS_MODULE,
	.type = REGULATOR_VOLTAGE,
	.min_uV = 4850000,
	.uV_step = 25000, /* 25mV per step */
	.n_voltages = 40, /* 4850mV to 5825mV */
	.vsel_reg = MT6360_PMU_CHG_CTRL5,
	.vsel_mask = 0xFC,
	.enable_reg = MT6360_PMU_CHG_CTRL1,
	.enable_mask = MT6360_MASK_OPA_MODE,
	.csel_reg = MT6360_PMU_CHG_CTRL10,
	.csel_mask = MT6360_MASK_OTG_OC,
};
//====pd_notifier_start===
#ifdef CONFIG_TCPC_CLASS
static int mt6360_get_charger_type(struct mt6360_pmu_chg_info *mpci,
	bool attach)
{
	static struct power_supply *chg_psy;
	int ret = 0;
	union power_supply_propval val = {.intval = 0};
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);

	if (!chg_psy) {
		if (pdata->bc12_sel == 1)
			chg_psy = power_supply_get_by_name("mtk_charger_type");
		else if (pdata->bc12_sel == 2)
			chg_psy = power_supply_get_by_name("ext_charger_type");
	}

	if (!chg_psy) {
		pr_notice("%s Couldn't get chg_psy\n", __func__);
		mpci->psy_desc.type = attach ? POWER_SUPPLY_TYPE_USB :
					       POWER_SUPPLY_TYPE_UNKNOWN;
		mpci->psy_usb_type = attach ? POWER_SUPPLY_USB_TYPE_DCP :
					      POWER_SUPPLY_USB_TYPE_UNKNOWN;
		goto out;
	}

	if (attach) {
		val.intval = true;
		ret = power_supply_set_property(chg_psy,
						POWER_SUPPLY_PROP_ONLINE, &val);
		ret = power_supply_get_property(chg_psy,
						POWER_SUPPLY_PROP_TYPE, &val);
		mpci->psy_desc.type = val.intval;
		pr_notice("%s type:%d\n", __func__, val.intval);
		ret = power_supply_get_property(chg_psy,
						POWER_SUPPLY_PROP_USB_TYPE,
						&val);
		mpci->psy_usb_type = val.intval;
		pr_notice("%s usb_type:%d\n", __func__, val.intval);
	} else {
		mpci->psy_desc.type = POWER_SUPPLY_TYPE_UNKNOWN;
		mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;
	}
out:
	mt6360_power_supply_changed(mpci);

	return mpci->psy_usb_type;
}

static int typec_attach_thread(void *data)
{
	struct mt6360_pmu_chg_info *mpci = data;
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(mpci->dev);
	int ret = 0;
	bool typec_attach = false, ignore_usb = false;
	union power_supply_propval val = {.intval = 0};

#ifdef OPLUS_FEATURE_CHG_BASIC
		unsigned int ms = 0;
#endif
	pr_info("%s: ++\n", __func__);
	while (!kthread_should_stop()) {
		wait_event(mpci->attach_wq,
			   atomic_read(&mpci->chrdet_start) > 0 ||
							 kthread_should_stop());
		if (kthread_should_stop())
			break;
#ifdef OPLUS_FEATURE_CHG_BASIC
		ms = mpci->chgdet_mdelay;
		mdelay(ms);
#endif
		mutex_lock(&mpci->attach_lock);
		typec_attach = mpci->typec_attach;
		ignore_usb = mpci->ignore_usb;
		atomic_set(&mpci->chrdet_start, 0);
		mutex_unlock(&mpci->attach_lock);

		pr_notice("%s bc12_sel:%d typec_attach:%d ignore_usb:%d\n",
			  __func__, pdata->bc12_sel, typec_attach, ignore_usb);

		if (typec_attach && ignore_usb) {
			mpci->bypass_chgdet = true;
			goto bypass_chgdet;
		} else if (!typec_attach && mpci->bypass_chgdet) {
			mpci->bypass_chgdet = false;
			goto bypass_chgdet;
		}

		val.intval = typec_attach;
		if (pdata->bc12_sel == 0) {
			ret = power_supply_set_property(mpci->chg_psy,
						POWER_SUPPLY_PROP_ONLINE, &val);
			if (ret < 0)
				dev_info(mpci->dev, "%s: set online fail(%d)\n",
					__func__, ret);
		} else
			mt6360_get_charger_type(mpci, typec_attach);
		continue;
bypass_chgdet:
		mutex_lock(&mpci->chgdet_lock);
		if (typec_attach) {
			mpci->psy_desc.type = POWER_SUPPLY_TYPE_USB;
			if (tcpm_inquire_typec_attach_state(mpci->tcpc) ==
						   TYPEC_ATTACHED_AUDIO)
				mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
			else
				mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_SDP;
		} else {
			mpci->psy_desc.type = POWER_SUPPLY_TYPE_UNKNOWN;
			mpci->psy_usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;
		}
		mutex_unlock(&mpci->chgdet_lock);
		mt6360_power_supply_changed(mpci);
	}
	return ret;
}

static void handle_typec_attach(struct mt6360_pmu_chg_info *mpci,
				bool attach, bool ignore)
{
	mutex_lock(&mpci->attach_lock);
	if (mpci->typec_attach == attach) {
		mutex_unlock(&mpci->attach_lock);
		return;
	}
	mpci->typec_attach = attach;
	mpci->ignore_usb = ignore;
	atomic_inc(&mpci->chrdet_start);
	mutex_unlock(&mpci->attach_lock);
	wake_up(&mpci->attach_wq);
}

#ifndef OPLUS_FEATURE_CHG_BASIC
void fg_charger_in_handler(void)
{
	static int chr_type;
	int current_chr_type;

	current_chr_type = mt_get_charger_type();

	bm_debug("[%s] notify daemon %d %d\n",
		__func__,
		chr_type, current_chr_type);

	if (current_chr_type != POWER_SUPPLY_TYPE_UNKNOWN) {
		if (chr_type == POWER_SUPPLY_TYPE_UNKNOWN)
			wakeup_fg_algo(FG_INTR_CHARGER_IN);
	}

	if (current_chr_type == POWER_SUPPLY_TYPE_UNKNOWN) {
		if (chr_type != POWER_SUPPLY_TYPE_UNKNOWN)
			wakeup_fg_algo(FG_INTR_CHARGER_OUT);
	}

	chr_type = current_chr_type;
}
#endif
static int pd_tcp_notifier_call(struct notifier_block *nb,
				unsigned long event, void *data)
{
	struct tcp_notify *noti = data;
	struct mt6360_pmu_chg_info *mpci =
		container_of(nb, struct mt6360_pmu_chg_info, pd_nb);
	uint8_t old_state = TYPEC_UNATTACHED, new_state = TYPEC_UNATTACHED;

	switch (event) {
	case TCP_NOTIFY_SINK_VBUS:
		if (tcpm_inquire_typec_attach_state(mpci->tcpc) ==
						   TYPEC_ATTACHED_AUDIO)
			handle_typec_attach(mpci, !!noti->vbus_state.mv, true);
		break;
	case TCP_NOTIFY_TYPEC_STATE:
		old_state = noti->typec_state.old_state;
		new_state = noti->typec_state.new_state;
		if (old_state == TYPEC_UNATTACHED &&
		    (new_state == TYPEC_ATTACHED_SNK ||
		     new_state == TYPEC_ATTACHED_NORP_SRC ||
		     new_state == TYPEC_ATTACHED_CUSTOM_SRC ||
		     new_state == TYPEC_ATTACHED_DBGACC_SNK)) {
			dev_info(mpci->dev,
				 "%s Charger plug in, polarity = %d\n",
				 __func__, noti->typec_state.polarity);
#ifdef OPLUS_FEATURE_CHG_BASIC
			pr_info("%s call switch_usb_state = 1 \n", __func__);
			//switch_usb_state(1);
#endif
#ifdef OPLUS_FEATURE_CHG_BASIC
			mpci->chgdet_mdelay = 450;
#endif
			handle_typec_attach(mpci, true, false);
		} else if ((old_state == TYPEC_ATTACHED_SNK ||
			    old_state == TYPEC_ATTACHED_NORP_SRC ||
			    old_state == TYPEC_ATTACHED_CUSTOM_SRC ||
			    old_state == TYPEC_ATTACHED_DBGACC_SNK ||
			    old_state == TYPEC_ATTACHED_AUDIO) &&
			    new_state == TYPEC_UNATTACHED) {
			dev_info(mpci->dev, "%s Charger plug out\n", __func__);
			handle_typec_attach(mpci, false, false);
		} else if (old_state == TYPEC_ATTACHED_SRC &&
			   new_state == TYPEC_ATTACHED_SNK) {
			dev_info(mpci->dev, "%s Source_to_Sink\n", __func__);
			handle_typec_attach(mpci, true, true);
		}  else if (old_state == TYPEC_ATTACHED_SNK &&
			    new_state == TYPEC_ATTACHED_SRC) {
			dev_info(mpci->dev, "%s Sink_to_Source\n", __func__);
			pr_info("%s USB Plug out\n", __func__);
			if (mpci->tcpc_kpoc) {
				pr_info("%s: typec unattached, power off\n",
					__func__);
#ifndef OPLUS_FEATURE_CHG_BASIC
#ifdef FIXME
				kernel_power_off();
#endif
#endif /*OPLUS_FEATURE_CHG_BASIC*/
			}
#ifdef OPLUS_FEATURE_CHG_BASIC
			pr_info("%s call switch_usb_state = 0 \n", __func__);
			//switch_usb_state(0);
#endif
#ifdef OPLUS_FEATURE_CHG_BASIC
			mpci->chgdet_mdelay = 0;
#endif
			handle_typec_attach(mpci, false, false);
		} else if (old_state == TYPEC_ATTACHED_SRC &&
			   new_state == TYPEC_ATTACHED_SNK) {
			pr_info("%s Source_to_Sink\n", __func__);
#ifdef OPLUS_FEATURE_CHG_BASIC
			mpci->chgdet_mdelay = 0;
#endif
			handle_typec_attach(mpci, true, true);
		}  else if (old_state == TYPEC_ATTACHED_SNK &&
			    new_state == TYPEC_ATTACHED_SRC) {
			pr_info("%s Sink_to_Source\n", __func__);
#ifdef OPLUS_FEATURE_CHG_BASIC
                     mpci->chgdet_mdelay = 0;
#endif			
			handle_typec_attach(mpci, false, true);
		}
		break;
	default:
		break;
	};

	return NOTIFY_OK;
}
#endif
//====pd_notifier_end=====

#ifdef OPLUS_FEATURE_CHG_BASIC
int mt6360_get_flashlight_temperature(int *temp)
{
	int ret;
	if (oplusmpci) {
		if (atomic_read(&oplusmpci->suspended) == 1)
			return -1;
	} else {
		pr_err("%s oplusmpci NULL\n", __func__);
		return -1;
	}
	ret = iio_read_channel_processed(oplusmpci->channels[MT6360_ADC_TS], temp);
	//printk(KERN_DEBUG "%s: raw flashlight temp=%d\n", __func__, *temp);
	if (ret < 0) {
		dev_info(oplusmpci->dev, "%s: fail(%d)\n", __func__, ret);
		return ret;
	}
	return 0;
}

EXPORT_SYMBOL(mt6360_get_flashlight_temperature);
#endif

static int mt6360_pmu_chg_probe(struct platform_device *pdev)
{
	struct mt6360_chg_platform_data *pdata = dev_get_platdata(&pdev->dev);
	struct mt6360_pmu_chg_info *mpci;
	struct iio_channel *channel;
	struct power_supply_config charger_cfg = {};
	struct regulator_config config = { };
	bool use_dt = pdev->dev.of_node;
	int i, ret = 0;

	dev_info(&pdev->dev, "%s\n", __func__);
	if (use_dt) {
		pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;
		ret = mt6360_chg_parse_dt_data(&pdev->dev, pdata);
		if (ret < 0) {
			dev_err(&pdev->dev, "parse dt fail\n");
			return ret;
		}
		pdev->dev.platform_data = pdata;
	}
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data specified\n");
		return -EINVAL;
	}
	mpci = devm_kzalloc(&pdev->dev, sizeof(*mpci), GFP_KERNEL);
	if (!mpci)
		return -ENOMEM;
	mpci->dev = &pdev->dev;
	mpci->mpi = dev_get_drvdata(pdev->dev.parent);
	mpci->hidden_mode_cnt = 0;
	mutex_init(&mpci->hidden_mode_lock);
	mutex_init(&mpci->pe_lock);
	mutex_init(&mpci->aicr_lock);
	mutex_init(&mpci->chgdet_lock);
	mutex_init(&mpci->tchg_lock);
	mutex_init(&mpci->ichg_lock);
	mpci->tchg = 0;
	mpci->ichg = 2000000;
	mpci->ichg_dis_chg = 2000000;
	mpci->attach = false;
	g_mpci = mpci;
#ifdef OPLUS_FEATURE_CHG_BASIC
	mpci->support_hvdcp = of_property_read_bool(pdev->dev.of_node, "support_hvdcp");
	printk(KERN_ERR "%s: support_hvdcp=%d\n", __func__, mpci->support_hvdcp);
#endif

#if defined(CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT)\
&& !defined(CONFIG_TCPC_CLASS)
	INIT_WORK(&mpci->chgdet_work, mt6360_chgdet_work_handler);
#endif /* CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT && !CONFIG_TCPC_CLASS */
#ifdef CONFIG_OPLUS_HVDCP_SUPPORT
	INIT_DELAYED_WORK(&mpci->hvdcp_work, mt6360_hvdcp_work);
	INIT_DELAYED_WORK(&mpci->hvdcp_result_check_work, mt6360_hvdcp_result_check_work);
	mpci->hvdcp_type = POWER_SUPPLY_TYPE_UNKNOWN;
#endif
	init_completion(&mpci->aicc_done);
	init_completion(&mpci->pumpx_done);
	atomic_set(&mpci->pe_complete, 0);
	atomic_set(&mpci->mivr_cnt, 0);
	init_waitqueue_head(&mpci->mivr_wq);
#ifdef CONFIG_TCPC_CLASS
	init_waitqueue_head(&mpci->attach_wq);
	atomic_set(&mpci->chrdet_start, 0);
	mutex_init(&mpci->attach_lock);
#endif
	mpci->old_propval =
		devm_kcalloc(mpci->dev, mt6360_pmu_chg_desc.num_properties,
			     sizeof(*mpci->old_propval), GFP_KERNEL);
	if (!mpci->old_propval)
		return -ENOMEM;
	platform_set_drvdata(pdev, mpci);

	/* apply platform data */
	ret = mt6360_chg_apply_pdata(mpci, pdata);
	if (ret < 0) {
		dev_err(&pdev->dev, "apply pdata fail\n");
		goto err_mutex_init;
	}
	/* Initial Setting */
	ret = mt6360_chg_init_setting(mpci);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s: init setting fail\n", __func__);
		goto err_mutex_init;
	}

	/* Get ADC iio channels */
	for (i = 0; i < MT6360_ADC_MAX; i++) {
		channel = devm_iio_channel_get(&pdev->dev,
					       mt6360_adc_chan_list[i]);
		if (IS_ERR(channel)) {
			ret = PTR_ERR(channel);
			goto err_mutex_init;
		}
		mpci->channels[i] = channel;
	}

	/* charger class register */
	mpci->chg_dev = charger_device_register(pdata->chg_name, mpci->dev,
						mpci, &mt6360_chg_ops,
						&mt6360_chg_props);
	if (IS_ERR(mpci->chg_dev)) {
		dev_err(mpci->dev, "charger device register fail\n");
		ret = PTR_ERR(mpci->chg_dev);
		goto err_mutex_init;
	}

	/* irq register */
	mt6360_pmu_chg_irq_register(pdev);
	device_init_wakeup(&pdev->dev, true);
	/* mivr task */
	mpci->mivr_task = kthread_run(mt6360_chg_mivr_task_threadfn, mpci,
				      "mivr_thread.%s", dev_name(mpci->dev));
	ret = PTR_ERR_OR_ZERO(mpci->mivr_task);
	if (ret < 0) {
		dev_err(mpci->dev, "create mivr handling thread fail\n");
		goto err_register_chg_dev;
	}
	ret = device_create_file(mpci->dev, &dev_attr_shipping_mode);
	if (ret < 0) {
		dev_notice(&pdev->dev, "create shipping attr fail\n");
		goto err_create_mivr_thread_run;
	}
	/* for trigger unfinish pe pattern */
	mpci->pe_wq = create_singlethread_workqueue("pe_pattern");
	if (!mpci->pe_wq) {
		dev_err(mpci->dev, "%s: create pe_pattern work queue fail\n",
			__func__);
		goto err_shipping_mode_attr;
	}
	INIT_WORK(&mpci->pe_work, mt6360_trigger_pep_work_handler);

	/* register fg bat oc notify */
#ifdef FIXME	//without mtk_charger_intf  BATTERY_OC_LEVEL
	if (pdata->batoc_notify)
		register_battery_oc_notify(&mt6360_recv_batoc_callback,
					   BATTERY_OC_PRIO_CHARGER);
#endif

/*new framework*/
	/* otg regulator */
	config.dev = &pdev->dev;
	config.driver_data = mpci;
	mpci->otg_rdev = devm_regulator_register(&pdev->dev,
						&mt6360_otg_rdesc, &config);
	if (IS_ERR(mpci->otg_rdev)) {
		ret = PTR_ERR(mpci->otg_rdev);
		goto err_register_otg;
	}


	/* power supply register */
	memcpy(&mpci->psy_desc, &mt6360_pmu_chg_desc, sizeof(mpci->psy_desc));
	mpci->psy_desc.name = dev_name(&pdev->dev);

	charger_cfg.drv_data = mpci;
	charger_cfg.of_node = pdev->dev.of_node;
	charger_cfg.supplied_to = mt6360_pmu_chg_supplied_to;
	charger_cfg.num_supplicants = ARRAY_SIZE(mt6360_pmu_chg_supplied_to);
	mpci->psy = devm_power_supply_register(&pdev->dev,
					&mpci->psy_desc, &charger_cfg);
	if (IS_ERR(mpci->psy)) {
		dev_notice(&pdev->dev, "Fail to register power supply dev\n");
		ret = PTR_ERR(mpci->psy);
		goto err_register_psy;
	}

#ifdef CONFIG_TCPC_CLASS
	/*get bc1.2 power supply:chg_psy*/
	mpci->chg_psy = devm_power_supply_get_by_phandle(&pdev->dev,
							     "charger");
	if (IS_ERR(mpci->chg_psy)) {
		dev_notice(&pdev->dev, "Failed to get charger psy\n");
		ret = PTR_ERR(mpci->chg_psy);
		goto err_psy_get_phandle;
	}

	mpci->attach_task = kthread_run(typec_attach_thread, mpci,
					"attach_thread");
	if (IS_ERR(mpci->attach_task)) {
		ret = PTR_ERR(mpci->attach_task);
		goto err_attach_task;
	}

	mpci->tcpc = tcpc_dev_get_by_name("type_c_port0");
	if (!mpci->tcpc) {
		pr_notice("%s get tcpc device type_c_port0 fail\n", __func__);
		ret = -ENODEV;
		goto err_get_tcpcdev;
	}

	mpci->pd_nb.notifier_call = pd_tcp_notifier_call;
	ret = register_tcp_dev_notifier(mpci->tcpc, &mpci->pd_nb,
					TCP_NOTIFY_TYPE_ALL);
	if (ret < 0) {
		pr_notice("%s: register tcpc notifer fail\n", __func__);
		ret = -EINVAL;
		goto err_register_tcp_notifier;
	}
#endif

	/* Schedule work for microB's BC1.2 */
#if defined(CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT)\
&& !defined(CONFIG_TCPC_CLASS)
	schedule_work(&mpci->chgdet_work);
#endif /* CONFIG_MT6360_PMU_CHARGER_TYPE_DETECT && !CONFIG_TCPC_CLASS */
	dev_info(&pdev->dev, "%s: successfully probed\n", __func__);
#ifdef OPLUS_FEATURE_CHG_BASIC
	oplusmpci = mpci;
	oplusmpci->charger_type = POWER_SUPPLY_TYPE_UNKNOWN;
	atomic_set(&oplusmpci->suspended, 0);
//	INIT_DELAYED_WORK(&mt6360_bst_olpi_work, mt6360_otg_ocp_work);
#endif
	return 0;
#ifdef CONFIG_TCPC_CLASS
err_register_tcp_notifier:
err_get_tcpcdev:
	if (mpci->attach_task)
		kthread_stop(mpci->attach_task);
err_attach_task:
err_psy_get_phandle:
#endif
err_register_psy:
err_register_otg:
	destroy_workqueue(mpci->pe_wq);
err_shipping_mode_attr:
	device_remove_file(mpci->dev, &dev_attr_shipping_mode);
err_create_mivr_thread_run:
	if (mpci->mivr_task)
		kthread_stop(mpci->mivr_task);
err_register_chg_dev:
	charger_device_unregister(mpci->chg_dev);
err_mutex_init:
	mutex_destroy(&mpci->tchg_lock);
	mutex_destroy(&mpci->chgdet_lock);
	mutex_destroy(&mpci->aicr_lock);
	mutex_destroy(&mpci->pe_lock);
	mutex_destroy(&mpci->hidden_mode_lock);
	mutex_destroy(&mpci->attach_lock);
#ifdef OPLUS_FEATURE_CHG_BASIC
	oplusmpci = NULL;
#endif
	return -EPROBE_DEFER;
}

static int mt6360_pmu_chg_remove(struct platform_device *pdev)
{
	struct mt6360_pmu_chg_info *mpci = platform_get_drvdata(pdev);

	dev_dbg(mpci->dev, "%s\n", __func__);
	flush_workqueue(mpci->pe_wq);
	destroy_workqueue(mpci->pe_wq);
	if (mpci->mivr_task) {
		atomic_inc(&mpci->mivr_cnt);
		wake_up(&mpci->mivr_wq);
		kthread_stop(mpci->mivr_task);
	}
	if (mpci->attach_task)
		kthread_stop(mpci->attach_task);
	device_remove_file(mpci->dev, &dev_attr_shipping_mode);
	charger_device_unregister(mpci->chg_dev);
	mutex_destroy(&mpci->tchg_lock);
	mutex_destroy(&mpci->chgdet_lock);
	mutex_destroy(&mpci->aicr_lock);
	mutex_destroy(&mpci->pe_lock);
	mutex_destroy(&mpci->hidden_mode_lock);
	mutex_destroy(&mpci->attach_lock);
#ifdef OPLUS_FEATURE_CHG_BASIC
	oplusmpci = NULL;
#endif
	return 0;
}

static int __maybe_unused mt6360_pmu_chg_suspend(struct device *dev)
{
#ifdef OPLUS_FEATURE_CHG_BASIC
	if (oplusmpci)
		atomic_set(&oplusmpci->suspended, 1);
#endif
	return 0;
}

static int __maybe_unused mt6360_pmu_chg_resume(struct device *dev)
{
#ifdef OPLUS_FEATURE_CHG_BASIC
	if (oplusmpci)
		atomic_set(&oplusmpci->suspended, 0);
#endif
	return 0;
}

static SIMPLE_DEV_PM_OPS(mt6360_pmu_chg_pm_ops,
			 mt6360_pmu_chg_suspend, mt6360_pmu_chg_resume);

static const struct of_device_id __maybe_unused mt6360_pmu_chg_of_id[] = {
	{ .compatible = "mediatek,mt6360_pmu_chg", },
	{},
};
MODULE_DEVICE_TABLE(of, mt6360_pmu_chg_of_id);

static const struct platform_device_id mt6360_pmu_chg_id[] = {
	{ "mt6360_pmu_chg", 0 },
	{},
};
MODULE_DEVICE_TABLE(platform, mt6360_pmu_chg_id);

static struct platform_driver mt6360_pmu_chg_driver = {
	.driver = {
		.name = "mt6360_pmu_chg",
		.owner = THIS_MODULE,
		.pm = &mt6360_pmu_chg_pm_ops,
		.of_match_table = of_match_ptr(mt6360_pmu_chg_of_id),
	},
	.probe = mt6360_pmu_chg_probe,
	.remove = mt6360_pmu_chg_remove,
	.id_table = mt6360_pmu_chg_id,
};

static int __init mt6360_pmu_chg_init(void)
{
	return platform_driver_register(&mt6360_pmu_chg_driver);
}
device_initcall_sync(mt6360_pmu_chg_init);

static void __exit mt6360_pmu_chg_exit(void)
{
	platform_driver_unregister(&mt6360_pmu_chg_driver);
}
module_exit(mt6360_pmu_chg_exit);

MODULE_AUTHOR("CY_Huang <cy_huang@richtek.com>");
MODULE_DESCRIPTION("MT6360 PMU CHG Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(MT6360_PMU_CHG_DRV_VERSION);

/*
 * Version Note
 * 1.0.8_MTK
 * (1) Fix no charging icon when power role swap to sink on kernel-4.19
 * (2) Fix mt6360_pmu_chg_ieoci_handler()
 *
 * 1.0.7_MTK
 * (1) Fix Unbalanced enable for MIVR IRQ
 * (2) Sleep 200ms before do another iteration in mt6360_chg_mivr_task_threadfn
 *
 * 1.0.6_MTK
 * (1) Fix the usages of charger power supply
 *
 * 1.0.5_MTK
 * (1) Prevent charger type infromed repeatedly
 *
 * 1.0.4_MTK
 * (1) Mask mivr irq until mivr task has run an iteration
 *
 * 1.0.3_MTK
 * (1) fix zcv adc from 5mV to 1.25mV per step
 * (2) add BC12 initial setting dcd timeout disable when unuse dcd
 *
 * 1.0.2_MTK
 * (1) remove eoc, rechg, te irq for evb with phone load
 * (2) report power supply online with chg type detect done
 * (3) remove unused irq event and status
 * (4) add chg termination irq notifier when safety timer timeout
 *
 * 1.0.1_MTK
 * (1) fix dtsi parse attribute about en_te, en_wdt, aicc_once
 * (2) add charger class get vbus adc interface
 * (3) add initial setting about disable en_sdi, and check batsysuv.
 *
 * 1.0.0_MTK
 * (1) Initial Release
 */
