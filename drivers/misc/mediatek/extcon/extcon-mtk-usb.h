/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 */

struct mtk_extcon_info {
	struct device *dev;
	struct extcon_dev *edev;
	struct usb_role_switch *role_sw;
	unsigned int c_role; /* current data role */
	struct workqueue_struct *extcon_wq;
	bool support_u3;
	struct regulator *vbus;
	struct gpio_desc *id_gpiod;
	unsigned int vbus_vol;
	unsigned int vbus_cur;
	unsigned int id_irq;
	bool vbus_on;
	struct device_connection dev_conn;
	struct power_supply *usb_psy;
	struct notifier_block psy_nb;
	struct delayed_work wq_detcable;
#ifdef CONFIG_TCPC_CLASS
	struct tcpc_device *tcpc_dev;
	struct notifier_block tcpc_nb;
#endif
	bool bypss_typec_sink;
};

struct usb_role_info {
	struct mtk_extcon_info *extcon;
	struct delayed_work dwork;
	unsigned int d_role; /* desire data role */
};

enum {
	DUAL_PROP_MODE_UFP = 0,
	DUAL_PROP_MODE_DFP,
	DUAL_PROP_MODE_NONE,
};

enum {
	DUAL_PROP_PR_SRC = 0,
	DUAL_PROP_PR_SNK,
	DUAL_PROP_PR_NONE,
};

enum {
	DUAL_PROP_DR_HOST = 0,
	DUAL_PROP_DR_DEVICE,
	DUAL_PROP_DR_NONE,
};

#if defined ADAPT_PSY_V1
extern void mt_usb_connect_v1(void);
extern void mt_usb_disconnect_v1(void);
#endif //ADAPT_PSY_V1
#ifndef CONFIG_TCPC_CLASS
extern void oplus_usb_extcon_set_role(unsigned int role);
#endif
