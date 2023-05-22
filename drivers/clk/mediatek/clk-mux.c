// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Owen Chen <owen.chen@mediatek.com>
 */

#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>

#include "clk-mtk.h"
#include "clk-mux.h"

#if defined(CONFIG_MACH_MT6768)
void mm_polling(struct clk_hw *hw);
#endif

static inline struct mtk_clk_mux *to_mtk_clk_mux(struct clk_hw *hw)
{
	return container_of(hw, struct mtk_clk_mux, hw);
}

static int mtk_clk_mux_enable(struct clk_hw *hw)
{
	struct mtk_clk_mux *mux = to_mtk_clk_mux(hw);
	u32 mask = 0;
	unsigned long flags = 0;
	int ret = 0;
	if (mux->data->gate_shift < 0)
		return 0;
	mask = BIT(mux->data->gate_shift);

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);
	else
		__acquire(mux->lock);
	ret = regmap_update_bits(mux->regmap, mux->data->mux_ofs,
			mask, ~mask);
	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);
	else
		__release(mux->lock);
	return ret;
}

static void mtk_clk_mux_disable(struct clk_hw *hw)
{
	struct mtk_clk_mux *mux = to_mtk_clk_mux(hw);
	u32 mask = 0;
	unsigned long flags = 0;

	if (mux->data->gate_shift < 0)
		return;
	mask = BIT(mux->data->gate_shift);

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);
	else
		__acquire(mux->lock);
	regmap_update_bits(mux->regmap, mux->data->mux_ofs, mask, mask);
	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);
	else
		__release(mux->lock);
}

static void mtk_clk_mux_disable_unused(struct clk_hw *hw)
{
	const char *c_n = clk_hw_get_name(hw);

	pr_notice("disable_unused - %s\n", c_n);

	mtk_clk_mux_disable(hw);
}

static int mtk_clk_mux_enable_setclr(struct clk_hw *hw)
{
	struct mtk_clk_mux *mux = to_mtk_clk_mux(hw);
	u32 val = 0;
	unsigned long flags = 0;

	if (mux->data->gate_shift < 0)
		return 0;

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);
	else
		__acquire(mux->lock);
	val = regmap_write(mux->regmap, mux->data->clr_ofs,
			BIT(mux->data->gate_shift));
	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);
	else
		__release(mux->lock);
	return val;
}

static void mtk_clk_mux_disable_setclr(struct clk_hw *hw)
{
	struct mtk_clk_mux *mux = to_mtk_clk_mux(hw);
	unsigned long flags = 0;

	if (mux->data->gate_shift < 0)
		return;

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);
	else
		__acquire(mux->lock);
	regmap_write(mux->regmap, mux->data->set_ofs,
			BIT(mux->data->gate_shift));
	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);
	else
		__release(mux->lock);
}

static void mtk_clk_mux_disable_setclr_unused(struct clk_hw *hw)
{
	const char *c_n = clk_hw_get_name(hw);

	pr_notice("disable_unused - %s\n", c_n);

	mtk_clk_mux_disable_setclr(hw);
}

static int mtk_clk_mux_is_enabled(struct clk_hw *hw)
{
	struct mtk_clk_mux *mux = to_mtk_clk_mux(hw);
	u32 val = 0;

	if (mux->data->gate_shift < 0)
		return true;
	regmap_read(mux->regmap, mux->data->mux_ofs, &val);

	return (val & BIT(mux->data->gate_shift)) == 0;
}

static u8 mtk_clk_mux_get_parent(struct clk_hw *hw)
{
	struct mtk_clk_mux *mux = to_mtk_clk_mux(hw);
	int num_parents = clk_hw_get_num_parents(hw);
	u32 mask = GENMASK(mux->data->mux_width - 1, 0);
	u32 val = 0;

	regmap_read(mux->regmap, mux->data->mux_ofs, &val);
	val = (val >> mux->data->mux_shift) & mask;

	if (val >= num_parents)
		return -EINVAL;
	return val;
}

static int mtk_clk_mux_set_parent_lock(struct clk_hw *hw, u8 index)
{
	struct mtk_clk_mux *mux = to_mtk_clk_mux(hw);
	u32 mask = GENMASK(mux->data->mux_width - 1, 0);
	unsigned long flags = 0;

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);
	else
		__acquire(mux->lock);

	regmap_update_bits(mux->regmap, mux->data->mux_ofs, mask,
		index << mux->data->mux_shift);

	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);
	else
		__release(mux->lock);

	return 0;
}

static int mtk_clk_mux_set_parent_setclr_lock(struct clk_hw *hw, u8 index)
{
	struct mtk_clk_mux *mux = to_mtk_clk_mux(hw);
	u32 mask = GENMASK(mux->data->mux_width - 1, 0);
	u32 val = 0, orig = 0;
	unsigned long flags = 0;
	const char *name;

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);
	else
		__acquire(mux->lock);

	regmap_read(mux->regmap, mux->data->mux_ofs, &orig);
	val = (orig & ~(mask << mux->data->mux_shift))
			| (index << mux->data->mux_shift);

	if (val != orig) {
		regmap_write(mux->regmap, mux->data->clr_ofs,
				mask << mux->data->mux_shift);
		regmap_write(mux->regmap, mux->data->set_ofs,
				index << mux->data->mux_shift);

#if defined(CONFIG_MACH_MT6768)
		/*
		 * Workaround for mm dvfs. Poll mm rdma reg before
		 * clkmux switching.
		 */
		if (hw && hw->clk) {
			name = __clk_get_name(hw->clk);
			if (name && !strcmp(name, "mm_sel"))
				mm_polling(hw);
		}
#endif

		if (mux->data->upd_shift >= 0)
			regmap_write(mux->regmap, mux->data->upd_ofs,
					BIT(mux->data->upd_shift));
	}

	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);
	else
		__release(mux->lock);

	return 0;
}

const struct clk_ops mtk_mux_ops = {
	.get_parent = mtk_clk_mux_get_parent,
	.set_parent = mtk_clk_mux_set_parent_lock,
};
EXPORT_SYMBOL(mtk_mux_ops);

const struct clk_ops mtk_mux_clr_set_upd_ops = {
	.get_parent = mtk_clk_mux_get_parent,
	.set_parent = mtk_clk_mux_set_parent_setclr_lock,
};
EXPORT_SYMBOL(mtk_mux_clr_set_upd_ops);

const struct clk_ops mtk_mux_gate_ops = {
	.enable = mtk_clk_mux_enable,
	.disable = mtk_clk_mux_disable,
	.is_enabled = mtk_clk_mux_is_enabled,
	.get_parent = mtk_clk_mux_get_parent,
	.set_parent = mtk_clk_mux_set_parent_lock,
	.disable_unused = mtk_clk_mux_disable_unused,
};
EXPORT_SYMBOL(mtk_mux_gate_ops);

const struct clk_ops mtk_mux_gate_clr_set_upd_ops = {
	.enable = mtk_clk_mux_enable_setclr,
	.disable = mtk_clk_mux_disable_setclr,
	.is_enabled = mtk_clk_mux_is_enabled,
	.get_parent = mtk_clk_mux_get_parent,
	.set_parent = mtk_clk_mux_set_parent_setclr_lock,
	.disable_unused = mtk_clk_mux_disable_setclr_unused,
};
EXPORT_SYMBOL(mtk_mux_gate_clr_set_upd_ops);

static struct clk *mtk_clk_register_mux(const struct mtk_mux *mux,
				 struct regmap *regmap,
				 spinlock_t *lock)
{
	struct mtk_clk_mux *clk_mux;
	struct clk_init_data init = {};
	struct clk *clk;

	clk_mux = kzalloc(sizeof(*clk_mux), GFP_KERNEL);
	if (!clk_mux)
		return ERR_PTR(-ENOMEM);

	init.name = mux->name;
	init.flags = mux->flags | CLK_SET_RATE_PARENT;
	init.parent_names = mux->parent_names;
	init.num_parents = mux->num_parents;
	init.ops = mux->ops;

	clk_mux->regmap = regmap;
	clk_mux->data = mux;
	clk_mux->lock = lock;
	clk_mux->hw.init = &init;

	clk = clk_register(NULL, &clk_mux->hw);
	if (IS_ERR(clk)) {
		kfree(clk_mux);
		return clk;
	}

	return clk;
}

int mtk_clk_register_muxes(const struct mtk_mux *muxes,
			   int num, struct device_node *node,
			   spinlock_t *lock,
			   struct clk_onecell_data *clk_data)
{
	struct regmap *regmap;
	struct clk *clk;
	int i;

	regmap = syscon_node_to_regmap(node);
	if (IS_ERR(regmap)) {
		pr_err("Cannot find regmap for %pOF: %ld\n", node,
		       PTR_ERR(regmap));
		return PTR_ERR(regmap);
	}

	for (i = 0; i < num; i++) {
		const struct mtk_mux *mux = &muxes[i];

		if (IS_ERR_OR_NULL(clk_data->clks[mux->id])) {
			clk = mtk_clk_register_mux(mux, regmap, lock);

			if (IS_ERR(clk)) {
				pr_err("Failed to register clk %s: %ld\n",
				       mux->name, PTR_ERR(clk));
				continue;
			}

			clk_data->clks[mux->id] = clk;
		}
	}

	return 0;
}
EXPORT_SYMBOL(mtk_clk_register_muxes);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek MUX");
MODULE_AUTHOR("MediaTek Inc.");
