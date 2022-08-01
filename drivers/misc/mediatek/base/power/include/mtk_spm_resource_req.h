/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

#ifndef __MTK_SPM_RESOURCE_REQ_H__
#define __MTK_SPM_RESOURCE_REQ_H__

#if defined(CONFIG_MACH_MT6757) || defined(CONFIG_MACH_KIBOPLUS)

#include "spm_v2/mtk_spm_resource_req.h"

#elif defined(CONFIG_MACH_MT6763)

#include "spm_v4/mtk_spm_resource_req.h"

#else
#include "spm/mtk_spm_resource_req.h"
#endif


#endif /* __MTK_SPM_RESOURCE_REQ_H__ */
