/* SPDX-License-Identifier: GPL-2.0 */
/*
* Copyright (C) 2021 MediaTek Inc.
*/

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/irqdomain.h>
#include <linux/pm_runtime.h>
#include <linux/irq.h>
#include "../inc/mt6360_pmu.h"
#include "../../../../../power/oplus/oplus_chg_track.h"
#ifdef OPLUS_FEATURE_CHG_BASIC
extern bool mt6360_get_vbus_status(void);
extern int mt6360_chg_enable(bool en);
extern int mt6360_chg_enable_wdt(bool enable);
extern bool oplus_chg_wake_update_work(void);
extern void oplus_chg_check_break(int vbus_rising);
extern bool oplus_vooc_get_fastchg_started(void);
extern int oplus_vooc_get_adapter_update_status(void);
extern void oplus_vooc_reset_fastchg_after_usbout(void);
extern void oplus_chg_clear_chargerid_info(void);
extern void oplus_chg_set_chargerid_switch_val(int);
extern bool oplus_vooc_get_fastchg_to_normal(void);
extern bool oplus_vooc_get_fastchg_to_warm(void);
extern void oplus_chg_set_charger_type_unknown(void);
#ifndef CONFIG_OPLUS_CHARGER_MTK6873
extern bool oplus_otgctl_by_buckboost(void);
#endif
int __attribute__((weak)) oplus_chg_get_mmi_status(void)
{
	return 1;
}
int __attribute__((weak)) oplus_mtk_hv_flashled_plug(int plug)
{
	return 1;
}
#endif

#ifdef CONFIG_MT6360_PMU_DEBUG
#define PMU_ARRAY_SIZE 8
static unsigned long long duration_index[PMU_ARRAY_SIZE], pmu_irq_duration;
static int count_index[PMU_ARRAY_SIZE];
#endif
static irqreturn_t mt6360_pmu_irq_handler(int irq, void *data)
{
	struct mt6360_pmu_info *mpi = data;
	u8 irq_events[MT6360_PMU_IRQ_REGNUM] = {0};
	u8 irq_masks[MT6360_PMU_IRQ_REGNUM] = {0};
	int i, j, ret;
#ifdef OPLUS_FEATURE_CHG_BASIC
	bool vbus_status = false;
#endif

	mt_dbg(mpi->dev, "%s ++\n", __func__);
	pm_runtime_get_sync(mpi->dev);
	ret = mt6360_pmu_reg_block_read(mpi, MT6360_PMU_CHG_IRQ1,
					MT6360_PMU_IRQ_REGNUM, irq_events);
	if (ret < 0) {
		dev_err(mpi->dev, "fail to read irq events\n");
		goto out_irq_handler;
	}
	memcpy(irq_masks, mpi->cache_irq_masks, MT6360_PMU_IRQ_REGNUM);
	/* make faultb reg, it's not irq mask, 0xfb[7:0] & 0xfc[7] */
	irq_masks[11] = 0xff;
	irq_masks[12] |= 0x80;
	for (i = 0; i < MT6360_PMU_IRQ_REGNUM; i++) {
		irq_events[i] &= ~irq_masks[i];
		for (j = 0; j < 8; j++) {
			if (!(irq_events[i] & (1 << (u32)j)))
				continue;
			ret = irq_find_mapping(mpi->irq_domain, i * 8 + j);
			if (ret) {
				/* bypass adc donei & mivr irq */
				if ((i == 5 && j == 4) || (i == 0 && j == 6))
					mt_dbg(mpi->dev,
					       "handle_irq [%d,%d]\n", i, j);
#ifdef OPLUS_FEATURE_CHG_BASIC
				else {
					if (i == 7) {
						vbus_status = mt6360_get_vbus_status();
						oplus_chg_check_break(vbus_status);
						oplus_chg_track_check_wired_charging_break(vbus_status);
						printk(KERN_ERR "!!!!! mt6360_pmu_irq_handler: [%d]\n", vbus_status);
#if !defined CONFIG_OPLUS_CHARGER_MTK6873 && !defined CONFIG_OPLUS_CHARGER_MTK6833
						if (!oplus_otgctl_by_buckboost()) {
							mt6360_chg_enable_wdt(vbus_status);
						}
#else
						mt6360_chg_enable_wdt(vbus_status);
#endif
						if(vbus_status == 0) {
							oplus_mtk_hv_flashled_plug(0);
						}
						if (oplus_vooc_get_fastchg_started() == true
								&& oplus_vooc_get_adapter_update_status() != 1) {
							printk(KERN_ERR "[OPLUS_CHG] %s oplus_vooc_get_fastchg_started = true!\n", __func__);
							if (vbus_status) {
								/*vooc adapters MCU vbus reset time is about 800ms(default standard),
								 * but some adapters reset time is about 350ms, so when vbus plugin irq
								 * was trigger, fastchg_started is true(default standard is false).
								 */
								mt6360_chg_enable(false);
							}
						} else {
							if (!vbus_status) {
								oplus_vooc_reset_fastchg_after_usbout();
								if (oplus_vooc_get_fastchg_started() == false) {
									oplus_chg_set_chargerid_switch_val(0);
									oplus_chg_clear_chargerid_info();
								}
								oplus_chg_set_charger_type_unknown();
							} else {
								if ((oplus_vooc_get_fastchg_to_normal() == true)
										|| (oplus_vooc_get_fastchg_to_warm() == true)
										|| (oplus_chg_get_mmi_status() == 0)) {
									mt6360_chg_enable(false);
								}
							}
							oplus_chg_wake_update_work();
						}
					} else {
						dev_dbg(mpi->dev,
							"handle_irq [%d,%d]\n", i, j);
					}
				}
#else
				else
					dev_dbg(mpi->dev,
						"handle_irq [%d,%d]\n", i, j);
#endif
/* OPLUS_FEATURE_CHG_BASIC end */
				handle_nested_irq(ret);
			} else
				dev_err(mpi->dev, "unmapped [%d,%d]\n", i, j);
		}
	}
	ret = mt6360_pmu_reg_block_write(mpi, MT6360_PMU_CHG_IRQ1,
					 MT6360_PMU_IRQ_REGNUM, irq_events);
	if (ret < 0) {
		dev_err(mpi->dev, "fail to write clear irq events\n");
		goto out_irq_handler;
	}
	ret = mt6360_pmu_reg_set_bits(mpi,
				      MT6360_PMU_IRQ_SET, MT6360_IRQ_RETRIG);
	if (ret < 0)
		dev_err(mpi->dev, "fail to retrig interrupt\n");
out_irq_handler:
	pm_runtime_put(mpi->dev);
	mt_dbg(mpi->dev, "%s --\n", __func__);
	return IRQ_HANDLED;
}

static void mt6360_pmu_irq_bus_lock(struct irq_data *data)
{
	struct mt6360_pmu_info *mpi = data->chip_data;
	int ret;

	/* mask all irq indicator mask */
	ret = mt6360_pmu_reg_write(mpi, MT6360_PMU_IRQ_MASK, 0xff);
	if (ret < 0)
		dev_err(mpi->dev, "%s: fail to write irq ind_mask\n", __func__);
}

static void mt6360_pmu_irq_bus_sync_unlock(struct irq_data *data)
{
	struct mt6360_pmu_info *mpi = data->chip_data;
	int ret;
	unsigned int offset = data->hwirq;

	/* force clear current irq event */
	ret = mt6360_pmu_reg_write(mpi, MT6360_PMU_CHG_IRQ1 + offset / 8,
				   1 << (u32)(offset % 8));
	if (ret < 0)
		dev_err(mpi->dev, "%s: fail to write clr irq\n", __func__);
	/* unmask current irq */
	ret = mt6360_pmu_reg_write(mpi, MT6360_PMU_CHG_MASK1 + offset / 8,
				   mpi->cache_irq_masks[offset / 8]);
	if (ret < 0)
		dev_err(mpi->dev, "%s: fail to write irq mask\n", __func__);
	/* unmask all irq indicator mask */
	ret = mt6360_pmu_reg_write(mpi, MT6360_PMU_IRQ_MASK, 0);
	if (ret < 0)
		dev_err(mpi->dev, "%s: fail to write irq ind_mask\n", __func__);
}

static void mt6360_pmu_irq_enable(struct irq_data *data)
{
	struct mt6360_pmu_info *mpi = data->chip_data;

	mt_dbg(mpi->dev, "%s: hwirq[%d]\n", __func__, (int)data->hwirq);
	mpi->cache_irq_masks[data->hwirq / 8] &= ~(1 << (data->hwirq % 8));
}

static void mt6360_pmu_irq_disable(struct irq_data *data)
{
	struct mt6360_pmu_info *mpi = data->chip_data;

	mt_dbg(mpi->dev, "%s: hwirq[%d]\n", __func__, (int)data->hwirq);
	mpi->cache_irq_masks[data->hwirq / 8] |= (1 << (data->hwirq % 8));
}

static struct irq_chip mt6360_pmu_irq_chip = {
	.name = "mt6360_pmu_irqs",
	.irq_bus_lock = mt6360_pmu_irq_bus_lock,
	.irq_bus_sync_unlock = mt6360_pmu_irq_bus_sync_unlock,
	.irq_enable = mt6360_pmu_irq_enable,
	.irq_disable = mt6360_pmu_irq_disable,
};

static int mt6360_pmu_irq_domain_map(struct irq_domain *d,
				     unsigned int virq, irq_hw_number_t hw)
{
	struct mt6360_pmu_info *mpi = d->host_data;

	if (hw >= MT6360_PMU_IRQEVT_MAX)
		return -EINVAL;
	irq_set_chip_data(virq, mpi);
	irq_set_chip_and_handler(virq, &mt6360_pmu_irq_chip, handle_simple_irq);
	irq_set_nested_thread(virq, true);
	irq_set_parent(virq, mpi->irq);
	irq_set_noprobe(virq);
	return 0;
}

static void mt6360_pmu_irq_domain_unmap(struct irq_domain *d, unsigned int virq)
{
	irq_set_chip_and_handler(virq, NULL, NULL);
	irq_set_chip_data(virq, NULL);
}

static const struct irq_domain_ops mt6360_pmu_irq_domain_ops = {
	.map = mt6360_pmu_irq_domain_map,
	.unmap = mt6360_pmu_irq_domain_unmap,
	.xlate = irq_domain_xlate_onetwocell,
};

static int mt6360_pmu_gpio_irq_init(struct mt6360_pmu_info *mpi)
{
	struct mt6360_pmu_platform_data *pdata = dev_get_platdata(mpi->dev);
	int ret;

	ret = devm_gpio_request_one(mpi->dev, pdata->irq_gpio, GPIOF_IN,
				    devm_kasprintf(mpi->dev, GFP_KERNEL,
				    "%s.irq", dev_name(mpi->dev)));
	if (ret < 0) {
		dev_err(mpi->dev, "gpio reqeuest [%d] fail\n", pdata->irq_gpio);
		return ret;
	}
	mpi->irq = gpio_to_irq(pdata->irq_gpio);
	if (mpi->irq < 0) {
		dev_err(mpi->dev, "irq number [%d] fail\n", mpi->irq);
		return mpi->irq;
	}
	ret = devm_request_threaded_irq(mpi->dev, mpi->irq, NULL,
					mt6360_pmu_irq_handler,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					dev_name(mpi->dev), mpi);
	if (ret < 0) {
		dev_err(mpi->dev, "request irq [%d] fail\n", mpi->irq);
		return ret;
	}
	device_init_wakeup(mpi->dev, true);
	mpi->irq_domain = irq_domain_add_linear(mpi->dev->of_node,
						MT6360_PMU_IRQEVT_MAX,
						&mt6360_pmu_irq_domain_ops,
						mpi);
	if (!mpi->irq_domain) {
		dev_err(mpi->dev, "irq domain add fail\n");
		return -EINVAL;
	}
	return 0;
}

static int mt6360_pmu_irq_maskall(struct mt6360_pmu_info *mpi)
{
	u8 irq_events[MT6360_PMU_IRQ_REGNUM] = {0}, faultb_mask[2] = {0};
	int i, ret;

	/* init cache irq events*/
	memset(mpi->cache_irq_masks, 0xff, MT6360_PMU_IRQ_REGNUM);
	/* mask all indicator */
	ret = mt6360_pmu_reg_write(mpi, MT6360_PMU_IRQ_MASK, 0xff);
	if (ret < 0) {
		dev_err(mpi->dev, "write irq ind mask fail\n");
		return ret;
	}
	/* read faultb_mask */
	ret = mt6360_pmu_reg_block_read(mpi,
					MT6360_PMU_FAULTB_MASK, 2, faultb_mask);
	if (ret < 0) {
		dev_err(mpi->dev, "read faultb mask fail\n");
		return ret;
	}
	/* keep faultb default value */
	mpi->cache_irq_masks[11] = faultb_mask[0];
	mpi->cache_irq_masks[12] &= ~0x80;
	mpi->cache_irq_masks[12] |= (faultb_mask[1] & 0x80);
	ret = mt6360_pmu_reg_block_write(mpi, MT6360_PMU_CHG_MASK1,
					 MT6360_PMU_IRQ_REGNUM,
					 mpi->cache_irq_masks);
	if (ret < 0) {
		dev_err(mpi->dev, "write irq mask all to 0xff fail\n");
		return ret;
	}
	/* read booton status */
	ret = mt6360_pmu_reg_block_read(mpi, MT6360_PMU_CHG_IRQ1,
					MT6360_PMU_IRQ_REGNUM, irq_events);
	if (ret < 0) {
		dev_err(mpi->dev, "read booton irq status fail\n");
		return ret;
	}
	dev_info(mpi->dev, "booton irq status ++\n");
	for (i = 0; i < MT6360_PMU_IRQ_REGNUM; i++)
		dev_info(mpi->dev, "irq[%02x], %02x\n", i, irq_events[i]);
	dev_info(mpi->dev, "booton irq status --\n");
	/* write clear for booton events */
	ret = mt6360_pmu_reg_block_write(mpi, MT6360_PMU_CHG_IRQ1,
					 MT6360_PMU_IRQ_REGNUM, irq_events);
	if (ret < 0) {
		dev_err(mpi->dev, "write already irq events to clear fail\n");
		return ret;
	}
	/* config all indicator to unmask, but all channel events are masked */
	ret = mt6360_pmu_reg_write(mpi, MT6360_PMU_IRQ_MASK, 0x00);
	if (ret < 0) {
		dev_err(mpi->dev, "write irq ind mask fail\n");
		return ret;
	}
	return 0;
}

int mt6360_pmu_irq_suspend(struct mt6360_pmu_info *mpi)
{
	if (device_may_wakeup(mpi->dev))
		enable_irq_wake(mpi->irq);
	return 0;
}
EXPORT_SYMBOL_GPL(mt6360_pmu_irq_suspend);

int mt6360_pmu_irq_resume(struct mt6360_pmu_info *mpi)
{
	if (device_may_wakeup(mpi->dev))
		disable_irq_wake(mpi->irq);
	return 0;
}
EXPORT_SYMBOL_GPL(mt6360_pmu_irq_resume);

int mt6360_pmu_irq_register(struct mt6360_pmu_info *mpi)
{
	int ret;

	dev_dbg(mpi->dev, "%s ++\n", __func__);
	ret = mt6360_pmu_irq_maskall(mpi);
	if (ret < 0) {
		dev_err(mpi->dev, "irq mask all fail\n");
		return ret;
	}
	ret = mt6360_pmu_gpio_irq_init(mpi);
	if (ret < 0) {
		dev_err(mpi->dev, "gpio_irq_init fail\n");
		return ret;
	}
	dev_dbg(mpi->dev, "%s --\n", __func__);
	return 0;
}
EXPORT_SYMBOL_GPL(mt6360_pmu_irq_register);

void mt6360_pmu_irq_unregister(struct mt6360_pmu_info *mpi)
{
	irq_domain_remove(mpi->irq_domain);
	device_init_wakeup(mpi->dev, false);
	mt6360_pmu_irq_maskall(mpi);
}
EXPORT_SYMBOL_GPL(mt6360_pmu_irq_unregister);
