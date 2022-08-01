/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2014 MediaTek Inc.
 * Author: James Liao <jamesjj.liao@mediatek.com>
 */

#ifndef __DRV_CLK_MTK_H
#define __DRV_CLK_MTK_H

/*
 * This is a private header file. DO NOT include it except clk-*.c.
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

extern int mtk_is_mtcmos_enable(void);
extern spinlock_t *get_mtk_clk_lock(void);
extern spinlock_t *get_mtk_mtcmos_lock(void);

#define mtk_clk_lock(flags)	spin_lock_irqsave(get_mtk_clk_lock(), flags)
#define mtk_clk_unlock(flags)	\
	spin_unlock_irqrestore(get_mtk_clk_lock(), flags)
#define mtk_mtcmos_lock(flags)	spin_lock_irqsave(get_mtk_mtcmos_lock(), flags)
#define mtk_mtcmos_unlock(flags)	\
	spin_unlock_irqrestore(get_mtk_mtcmos_lock(), flags)

#define MAX_MUX_GATE_BIT	31
#define INVALID_MUX_GATE_BIT	(MAX_MUX_GATE_BIT + 1)

#endif /* __DRV_CLK_MTK_H */
