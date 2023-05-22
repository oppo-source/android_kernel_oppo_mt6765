/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

#ifndef __MTK_SPM_IDLE_COMMON_H__
#define __MTK_SPM_IDLE_COMMON_H__

#if defined(CONFIG_MACH_MT6757) || defined(CONFIG_MACH_KIBOPLUS)

#include "spm_v2/mtk_spm_idle.h"

#elif defined(CONFIG_MACH_MT6763) || defined(CONFIG_MACH_MT6739) || defined(CONFIG_MACH_MT6771)

#include "spm_v4/mtk_spm_idle.h"

#elif defined(CONFIG_MACH_MT6768) || defined(CONFIG_MACH_MT6785)

#include "spm_v1/mtk_spm_idle.h"

#endif

#endif /* __MTK_SPM_IDLE_COMMON_H__ */

