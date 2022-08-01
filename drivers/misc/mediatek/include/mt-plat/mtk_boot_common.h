/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 MediaTek Inc.
 */

#ifndef __MTK_BOOT_COMMON_H__
#define __MTK_BOOT_COMMON_H__

/* boot type definitions */
enum boot_mode_t {
	NORMAL_BOOT = 0,
	META_BOOT = 1,
	RECOVERY_BOOT = 2,
	SW_REBOOT = 3,
	FACTORY_BOOT = 4,
	ADVMETA_BOOT = 5,
	ATE_FACTORY_BOOT = 6,
	ALARM_BOOT = 7,
	KERNEL_POWER_OFF_CHARGING_BOOT = 8,
	LOW_POWER_OFF_CHARGING_BOOT = 9,
	DONGLE_BOOT = 10,
#ifdef VENDOR_EDIT
	OPPO_SAU_BOOT = 11,
	SILENCE_BOOT = 12,
	AGING_BOOT = 998,
	SAFE_BOOT = 999,
#endif /* VENDOR_EDIT */
	UNKNOWN_BOOT
};

#ifdef VENDOR_EDIT
typedef enum
{
	OPPO_NORMAL_BOOT = 0,
	OPPO_SILENCE_BOOT = 1,
	OPPO_SAFE_BOOT = 2,
	OPPO_AGING_BOOT = 3,
	OPPO_UNKNOWN_BOOT
}OPPO_BOOTMODE;

extern OPPO_BOOTMODE oppo_boot_mode;
#endif /* VENDOR_EDIT */

#ifdef VENDOR_EDIT
extern OPPO_BOOTMODE oppo_boot_mode;
#endif /* VENDOR_EDIT */

/* for boot type usage */
#define BOOTDEV_NAND            (0)
#define BOOTDEV_SDMMC           (1)
#define BOOTDEV_UFS             (2)

#define BOOT_DEV_NAME           "BOOT"
#define BOOT_SYSFS              "boot"
#define BOOT_MODE_SYSFS_ATTR    "boot_mode"
#define BOOT_TYPE_SYSFS_ATTR    "boot_type"

extern enum boot_mode_t get_boot_mode(void);
extern unsigned int get_boot_type(void);
extern bool is_meta_mode(void);
extern bool is_advanced_meta_mode(void);
extern void set_boot_mode(unsigned int bm);

#endif
