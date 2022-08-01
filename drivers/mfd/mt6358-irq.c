// SPDX-License-Identifier: GPL-2.0
//
// Copyright (c) 2019 MediaTek Inc.

#include <linux/interrupt.h>
#include <linux/mfd/mt6357/core.h>
#include <linux/mfd/mt6357/registers.h>
#include <linux/mfd/mt6358/core.h>
#include <linux/mfd/mt6358/registers.h>
#include <linux/mfd/mt6359/core.h>
#include <linux/mfd/mt6359/registers.h>
#include <linux/mfd/mt6397/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/wakeup_reason.h>

#define DEBUG_PMIC_IRQ	1

#ifdef OPLUS_FEATURE_POWERINFO_STANDBY
const char *wakeup_irq_name="NULL";
#define GET_IRQ_NAME(x)   case x: return(#x);
static inline const char *change_enum_to_string(enum mt6358_irq_numbers type){
     switch (type)
     {
		GET_IRQ_NAME(MT6358_IRQ_VPROC11_OC)
		GET_IRQ_NAME(MT6358_IRQ_VPROC12_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCORE_OC)
		GET_IRQ_NAME(MT6358_IRQ_VGPU_OC)
		GET_IRQ_NAME(MT6358_IRQ_VMODEM_OC)
		GET_IRQ_NAME(MT6358_IRQ_VDRAM1_OC)
		GET_IRQ_NAME(MT6358_IRQ_VS1_OC)
		GET_IRQ_NAME(MT6358_IRQ_VS2_OC)
		GET_IRQ_NAME(MT6358_IRQ_VPA_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCORE_PREOC)
		GET_IRQ_NAME(MT6358_IRQ_VFE28_OC)
		GET_IRQ_NAME(MT6358_IRQ_VXO22_OC)
		GET_IRQ_NAME(MT6358_IRQ_VRF18_OC)
		GET_IRQ_NAME(MT6358_IRQ_VRF12_OC)
		GET_IRQ_NAME(MT6358_IRQ_VEFUSE_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCN33_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCN28_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCN18_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCAMA1_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCAMA2_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCAMD_OC)
		GET_IRQ_NAME(MT6358_IRQ_VCAMIO_OC)
		GET_IRQ_NAME(MT6358_IRQ_VLDO28_OC)
		GET_IRQ_NAME(MT6358_IRQ_VA12_OC)
		GET_IRQ_NAME(MT6358_IRQ_VAUX18_OC)
		GET_IRQ_NAME(MT6358_IRQ_VAUD28_OC)
		GET_IRQ_NAME(MT6358_IRQ_VIO28_OC)
		GET_IRQ_NAME(MT6358_IRQ_VIO18_OC)
		GET_IRQ_NAME(MT6358_IRQ_VSRAM_PROC11_OC)
		GET_IRQ_NAME(MT6358_IRQ_VSRAM_PROC12_OC)
		GET_IRQ_NAME(MT6358_IRQ_VSRAM_OTHERS_OC)
		GET_IRQ_NAME(MT6358_IRQ_VSRAM_GPU_OC)
		GET_IRQ_NAME(MT6358_IRQ_VDRAM2_OC)
		GET_IRQ_NAME(MT6358_IRQ_VMC_OC)
		GET_IRQ_NAME(MT6358_IRQ_VMCH_OC)
		GET_IRQ_NAME(MT6358_IRQ_VEMC_OC)
		GET_IRQ_NAME(MT6358_IRQ_VSIM1_OC)
		GET_IRQ_NAME(MT6358_IRQ_VSIM2_OC)
		GET_IRQ_NAME(MT6358_IRQ_VIBR_OC)
		GET_IRQ_NAME(MT6358_IRQ_VUSB_OC)
		GET_IRQ_NAME(MT6358_IRQ_VBIF28_OC)
		GET_IRQ_NAME(MT6358_IRQ_PWRKEY)
		GET_IRQ_NAME(MT6358_IRQ_HOMEKEY)
		GET_IRQ_NAME(MT6358_IRQ_PWRKEY_R)
		GET_IRQ_NAME(MT6358_IRQ_HOMEKEY_R)
		GET_IRQ_NAME(MT6358_IRQ_NI_LBAT_INT)
		GET_IRQ_NAME(MT6358_IRQ_CHRDET)
		GET_IRQ_NAME(MT6358_IRQ_CHRDET_EDGE)
		GET_IRQ_NAME(MT6358_IRQ_VCDT_HV_DET)
		GET_IRQ_NAME(MT6358_IRQ_RTC)
		GET_IRQ_NAME(MT6358_IRQ_FG_BAT0_H)
		GET_IRQ_NAME(MT6358_IRQ_FG_BAT0_L)
		GET_IRQ_NAME(MT6358_IRQ_FG_CUR_H)
		GET_IRQ_NAME(MT6358_IRQ_FG_CUR_L)
		GET_IRQ_NAME(MT6358_IRQ_FG_ZCV)
		GET_IRQ_NAME(MT6358_IRQ_FG_BAT1_H)
		GET_IRQ_NAME(MT6358_IRQ_FG_BAT1_L)
		GET_IRQ_NAME(MT6358_IRQ_FG_N_CHARGE_L)
		GET_IRQ_NAME(MT6358_IRQ_FG_IAVG_H)
		GET_IRQ_NAME(MT6358_IRQ_FG_IAVG_L)
		GET_IRQ_NAME(MT6358_IRQ_FG_TIME_H)
		GET_IRQ_NAME(MT6358_IRQ_FG_DISCHARGE)
		GET_IRQ_NAME(MT6358_IRQ_FG_CHARGE)
		GET_IRQ_NAME(MT6358_IRQ_BATON_LV)
		GET_IRQ_NAME(MT6358_IRQ_BATON_HT)
		GET_IRQ_NAME(MT6358_IRQ_BATON_BAT_IN)
		GET_IRQ_NAME(MT6358_IRQ_BATON_BAT_OUT)
		GET_IRQ_NAME(MT6358_IRQ_BIF)
		GET_IRQ_NAME(MT6358_IRQ_BAT_H)
		GET_IRQ_NAME(MT6358_IRQ_BAT_L)
		GET_IRQ_NAME(MT6358_IRQ_BAT2_H)
		GET_IRQ_NAME(MT6358_IRQ_BAT2_L)
		GET_IRQ_NAME(MT6358_IRQ_BAT_TEMP_H)
		GET_IRQ_NAME(MT6358_IRQ_BAT_TEMP_L)
		GET_IRQ_NAME(MT6358_IRQ_AUXADC_IMP)
		GET_IRQ_NAME(MT6358_IRQ_NAG_C_DLTV)
		GET_IRQ_NAME(MT6358_IRQ_AUDIO)
		GET_IRQ_NAME(MT6358_IRQ_ACCDET)
		GET_IRQ_NAME(MT6358_IRQ_ACCDET_EINT0)
		GET_IRQ_NAME(MT6358_IRQ_ACCDET_EINT1)
		GET_IRQ_NAME(MT6358_IRQ_SPI_CMD_ALERT)
		GET_IRQ_NAME(MT6358_IRQ_NR)
     }
     return "Unsupported IRQ_NAME";
}
#endif/* OPLUS_FEATURE_POWERINFO_STANDBY */

struct irq_top_t {
	int hwirq_base;
	unsigned int num_int_regs;
	unsigned int en_reg;
	unsigned int en_reg_shift;
	unsigned int sta_reg;
	unsigned int sta_reg_shift;
	unsigned int top_offset;
};

#if DEBUG_PMIC_IRQ
struct irq_name_t {
	const char *name;
};

static struct irq_name_t mt6357_irq_name[] = MT6357_IRQ_NAME_GEN();
static struct irq_name_t mt6358_irq_name[] = MT6358_IRQ_NAME_GEN();
static struct irq_name_t mt6359_irq_name[] = MT6359_IRQ_NAME_GEN();
#endif

struct pmic_irq_data {
	unsigned int num_top;
	unsigned int num_pmic_irqs;
	unsigned int reg_width;
	unsigned short top_int_status_reg;
	bool *enable_hwirq;
	bool *cache_hwirq;
	struct irq_top_t *pmic_ints;
#if DEBUG_PMIC_IRQ
	struct irq_name_t *irq_name;
#endif
};

static struct irq_top_t mt6357_ints[] = {
	MT6357_TOP_GEN(BUCK),
	MT6357_TOP_GEN(LDO),
	MT6357_TOP_GEN(PSC),
	MT6357_TOP_GEN(SCK),
	MT6357_TOP_GEN(BM),
	MT6357_TOP_GEN(HK),
	MT6357_TOP_GEN(AUD),
	MT6357_TOP_GEN(MISC),
};

static struct irq_top_t mt6358_ints[] = {
	MT6358_TOP_GEN(BUCK),
	MT6358_TOP_GEN(LDO),
	MT6358_TOP_GEN(PSC),
	MT6358_TOP_GEN(SCK),
	MT6358_TOP_GEN(BM),
	MT6358_TOP_GEN(HK),
	MT6358_TOP_GEN(AUD),
	MT6358_TOP_GEN(MISC),
};

static struct irq_top_t mt6359_ints[] = {
	MT6359_TOP_GEN(BUCK),
	MT6359_TOP_GEN(LDO),
	MT6359_TOP_GEN(PSC),
	MT6359_TOP_GEN(SCK),
	MT6359_TOP_GEN(BM),
	MT6359_TOP_GEN(HK),
	MT6359_TOP_GEN(AUD),
	MT6359_TOP_GEN(MISC),
};

static void pmic_irq_enable(struct irq_data *data)
{
	unsigned int hwirq = irqd_to_hwirq(data);
	struct mt6397_chip *chip = irq_data_get_irq_chip_data(data);
	struct pmic_irq_data *irqd = chip->irq_data;

	irqd->enable_hwirq[hwirq] = true;
}

static void pmic_irq_disable(struct irq_data *data)
{
	unsigned int hwirq = irqd_to_hwirq(data);
	struct mt6397_chip *chip = irq_data_get_irq_chip_data(data);
	struct pmic_irq_data *irqd = chip->irq_data;

	irqd->enable_hwirq[hwirq] = false;
}

static void pmic_irq_lock(struct irq_data *data)
{
	struct mt6397_chip *chip = irq_data_get_irq_chip_data(data);

	mutex_lock(&chip->irqlock);
}

static void pmic_irq_sync_unlock(struct irq_data *data)
{
	unsigned int i, top_gp, en_reg, int_regs, shift;
	struct mt6397_chip *chip = irq_data_get_irq_chip_data(data);
	struct pmic_irq_data *irqd = chip->irq_data;

	for (i = 0; i < irqd->num_pmic_irqs; i++) {
		if (irqd->enable_hwirq[i] == irqd->cache_hwirq[i])
			continue;

		top_gp = 0;
		while ((top_gp + 1) < irqd->num_top &&
			i >= irqd->pmic_ints[top_gp + 1].hwirq_base)
			top_gp++;

		if (top_gp >= irqd->num_top) {
			mutex_unlock(&chip->irqlock);
			dev_err(chip->dev,
				"Failed to get top_group: %d\n", top_gp);
			return;
		}

		int_regs = (i - irqd->pmic_ints[top_gp].hwirq_base) /
			    irqd->reg_width;
		en_reg = irqd->pmic_ints[top_gp].en_reg +
			irqd->pmic_ints[top_gp].en_reg_shift * int_regs;
		shift = (i - irqd->pmic_ints[top_gp].hwirq_base) %
			irqd->reg_width;
		regmap_update_bits(chip->regmap, en_reg, BIT(shift),
				   irqd->enable_hwirq[i] << shift);
		irqd->cache_hwirq[i] = irqd->enable_hwirq[i];
	}
	mutex_unlock(&chip->irqlock);
}

static struct irq_chip mt6358_irq_chip = {
	.name = "mt6358-irq",
	.flags = IRQCHIP_SKIP_SET_WAKE,
	.irq_enable = pmic_irq_enable,
	.irq_disable = pmic_irq_disable,
	.irq_bus_lock = pmic_irq_lock,
	.irq_bus_sync_unlock = pmic_irq_sync_unlock,
};

static void mt6358_irq_sp_handler(struct mt6397_chip *chip,
				  unsigned int top_gp)
{
	unsigned int sta_reg, irq_status = 0;
	unsigned int hwirq, virq;
	int ret, i, j;
	
	#ifdef OPLUS_FEATURE_POWERINFO_STANDBY
	enum mt6358_irq_numbers irq_name;
	#endif/* OPLUS_FEATURE_POWERINFO_STANDBY */

	
	struct pmic_irq_data *irqd = chip->irq_data;

	for (i = 0; i < irqd->pmic_ints[top_gp].num_int_regs; i++) {
		sta_reg = irqd->pmic_ints[top_gp].sta_reg +
			irqd->pmic_ints[top_gp].sta_reg_shift * i;
		ret = regmap_read(chip->regmap, sta_reg, &irq_status);
		if (ret) {
			dev_err(chip->dev,
				"Failed to read irq status: %d\n", ret);
			return;
		}

		if (!irq_status)
			continue;

		for (j = 0; j < irqd->reg_width ; j++) {
			if ((irq_status & BIT(j)) == 0)
				continue;
			hwirq = irqd->pmic_ints[top_gp].hwirq_base +
				irqd->reg_width * i + j;
			virq = irq_find_mapping(chip->irq_domain, hwirq);
#if DEBUG_PMIC_IRQ
			dev_info(chip->dev,
				"Reg[0x%x]=0x%x,name=%s,hwirq=%d,type=%d\n",
				sta_reg, irq_status,
				irqd->irq_name[hwirq], hwirq,
				irq_get_trigger_type(virq));
			
			#ifdef OPLUS_FEATURE_POWERINFO_STANDBY
			irq_name=hwirq;
			/*irq name define in core.h -> mt6358_irq_numbers*/
			if((irq_name>=0)&&(irq_name<=145)){
				wakeup_irq_name=change_enum_to_string(irq_name);
				dev_info(chip->dev,"[PMIC_INT][%s]\n",wakeup_irq_name);
			}
			#endif/* OPLUS_FEATURE_POWERINFO_STANDBY */			

			log_irq_wakeup_reason(chip->irq);

			if (!strncmp(irqd->irq_name[hwirq].name, "chrdet_edge", 11)) {
				regmap_write(chip->regmap, sta_reg, BIT(j));
				irq_status &= ~BIT(j);
			}

			log_threaded_irq_wakeup_reason(virq, chip->irq);
#endif
			if (virq)
				handle_nested_irq(virq);
		}

		regmap_write(chip->regmap, sta_reg, irq_status);
	}
}

static irqreturn_t mt6358_irq_handler(int irq, void *data)
{
	struct mt6397_chip *chip = data;
	struct pmic_irq_data *irqd = chip->irq_data;
	unsigned int top_irq_status = 0;
	unsigned int i = 0;
	int ret = 0;

	pm_stay_awake(chip->dev);
	ret = regmap_read(chip->regmap,
			  irqd->top_int_status_reg,
			  &top_irq_status);
	if (ret) {
		pm_relax(chip->dev);
		dev_err(chip->dev, "Can't read TOP_INT_STATUS ret=%d\n", ret);
		return IRQ_NONE;
	}

	for (i = 0; i < irqd->num_top; i++) {
		if (top_irq_status & BIT(irqd->pmic_ints[i].top_offset))
			mt6358_irq_sp_handler(chip, i);
	}
	pm_relax(chip->dev);
	return IRQ_HANDLED;
}

static int pmic_irq_domain_map(struct irq_domain *d, unsigned int irq,
			       irq_hw_number_t hw)
{
	struct mt6397_chip *mt6397 = d->host_data;

	irq_set_chip_data(irq, mt6397);
	irq_set_chip_and_handler(irq, &mt6358_irq_chip, handle_level_irq);
	irq_set_nested_thread(irq, 1);
	irq_set_noprobe(irq);

	return 0;
}

static const struct irq_domain_ops mt6358_irq_domain_ops = {
	.map = pmic_irq_domain_map,
	.xlate = irq_domain_xlate_twocell,
};

int mt6358_irq_init(struct mt6397_chip *chip)
{
	int i, j, ret;
	struct pmic_irq_data *irqd;

	irqd = devm_kzalloc(chip->dev, sizeof(*irqd), GFP_KERNEL);
	if (!irqd)
		return -ENOMEM;

	chip->irq_data = irqd;

	mutex_init(&chip->irqlock);
	switch (chip->chip_id) {
	case MT6357_CHIP_ID:
		irqd->num_top = ARRAY_SIZE(mt6357_ints);
		irqd->num_pmic_irqs = MT6357_IRQ_NR;
		irqd->reg_width = MT6357_REG_WIDTH;
		irqd->top_int_status_reg = MT6357_TOP_INT_STATUS0;
		irqd->pmic_ints = mt6357_ints;
#if DEBUG_PMIC_IRQ
		irqd->irq_name = mt6357_irq_name;
#endif
		break;
	case MT6358_CHIP_ID:
		irqd->num_top = ARRAY_SIZE(mt6358_ints);
		irqd->num_pmic_irqs = MT6358_IRQ_NR;
		irqd->reg_width = MT6358_REG_WIDTH;
		irqd->top_int_status_reg = MT6358_TOP_INT_STATUS0;
		irqd->pmic_ints = mt6358_ints;
#if DEBUG_PMIC_IRQ
		irqd->irq_name = mt6358_irq_name;
#endif
		break;
	case MT6359_CHIP_ID:
		irqd->num_top = ARRAY_SIZE(mt6359_ints);
		irqd->num_pmic_irqs = MT6359_IRQ_NR;
		irqd->reg_width = MT6359_REG_WIDTH;
		irqd->top_int_status_reg = MT6359_TOP_INT_STATUS0;
		irqd->pmic_ints = mt6359_ints;
#if DEBUG_PMIC_IRQ
		irqd->irq_name = mt6359_irq_name;
#endif
		break;
	default:
		dev_err(chip->dev, "unsupported chip: 0x%x\n", chip->chip_id);
		ret = -ENODEV;
		break;
	}

	irqd->enable_hwirq = devm_kcalloc(chip->dev,
					  irqd->num_pmic_irqs,
					  sizeof(bool),
					  GFP_KERNEL);
	if (!irqd->enable_hwirq)
		return -ENOMEM;

	irqd->cache_hwirq = devm_kcalloc(chip->dev,
					 irqd->num_pmic_irqs,
					 sizeof(bool),
					 GFP_KERNEL);
	if (!irqd->cache_hwirq)
		return -ENOMEM;

	/* Disable all interrupt for initializing */
	for (i = 0; i < irqd->num_top; i++) {
		for (j = 0; j < irqd->pmic_ints[i].num_int_regs; j++)
			regmap_write(chip->regmap,
				     irqd->pmic_ints[i].en_reg +
				     irqd->pmic_ints[i].en_reg_shift * j, 0);
	}

	chip->irq_domain = irq_domain_add_linear(chip->dev->of_node,
						 irqd->num_pmic_irqs,
						 &mt6358_irq_domain_ops, chip);
	if (!chip->irq_domain) {
		dev_err(chip->dev, "could not create irq domain\n");
		return -ENODEV;
	}

	ret = devm_request_threaded_irq(chip->dev, chip->irq, NULL,
					mt6358_irq_handler,
					IRQF_ONESHOT,
					mt6358_irq_chip.name, chip);
	if (ret) {
		dev_err(chip->dev, "failed to register irq=%d; err: %d\n",
			chip->irq, ret);
		return ret;
	}

	enable_irq_wake(chip->irq);
	return ret;
}
