// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/cpufreq.h>
#if defined(OPLUS_FEATURE_SCHEDUTIL_USE_TL) && defined(CONFIG_SCHEDUTIL_USE_TL)
#include "tuning.h"
#define BUFFER_SIZE 1024
#else
#include "sched.h"
#endif

struct eas_data {
	struct attribute_group *attr_group;
	struct kobject *kobj;
	int init;
};

struct eas_data eas_info;

/* Try to get information of all CPUs */
int show_cpu_info(char *buf, int buf_size)
{
	int cpu;
	int len = 0;

	for_each_possible_cpu(cpu) {
		len += snprintf(buf+len, buf_size-len,
			"cpu=%d max=%4uMHz curr=%4uMHz ",
			cpu,

			/* Max frequency (limited by ceiling) */
			cpu_online(cpu)?cpufreq_quick_get_max(cpu):0,

			/* Current frequency */
			cpu_online(cpu)?cpufreq_quick_get(cpu):0
			);

		len += snprintf(buf+len, buf_size-len,
			"orig_cap=%4lu limit_cap=%4lu cur_cap=%4lu ",
			/* Original capacity */
			cpu_online(cpu)?capacity_orig_of(cpu):0,

			/* Ceiling limited capacity */
			cpu_online(cpu)?cpu_rq(cpu)->cpu_capacity:0,

			/* Current capacity */
			cpu_online(cpu)?cap_scale(capacity_orig_of(cpu),
					arch_scale_freq_capacity(cpu)):0
			);

		len += snprintf(buf+len, buf_size-len,
#ifdef CONFIG_SCHED_TUNE
			"boost=%4lu cfs=%4lu rt=%4lu (%s)\n",

			/* cpu boost utilization */
			cpu_online(cpu)?stune_util(cpu,
					cpu_util_rt(cpu_rq(cpu))):0,
#else
			"cfs=%4lu rt=%4lu (%s)\n",
#endif
			/* cpu cfs utilization */
			cpu_online(cpu)?cpu_util_cfs(cpu_rq(cpu)):0,

			/* cpu rt utilization */
			cpu_online(cpu)?cpu_util_rt(cpu_rq(cpu)):0,

			/* cpu on/off */
			cpu_online(cpu)?"on":"off"
			);
	}

	return len;
}

#if defined(CONFIG_ENERGY_MODEL) && defined(CONFIG_CPU_FREQ_GOV_SCHEDUTIL)
int show_perf_order_domain_info(char *buf, int buf_size)
{
	struct perf_order_domain *domain;
	int len = 0;

	len += snprintf(buf+len, buf_size-len,
		"Sort perf_domains from little to big:\n"
		);

	if (!pod_is_ready()) {
		len += snprintf(buf+len, buf_size-len,
		"Perf order domain is not ready!\n"
		);
		return len;
	}

	for_each_perf_domain_ascending(domain) {
		len += snprintf(buf+len, buf_size-len,
			"   cpumask: 0x%02lx\n",
			*cpumask_bits(&domain->possible_cpus)
			);
	}

#if defined(OPLUS_FEATURE_SCHEDUTIL_USE_TL) && defined(CONFIG_SCHEDUTIL_USE_TL)
	len += snprintf(buf+len,buf_size-len,
		"capacity_margin : %d\n",
		get_capacity_margin());

	len += snprintf(buf+len,buf_size-len,
		"get_capacity_margin_dvfs : %d\n",
		get_capacity_margin_dvfs());
#endif

	return len;
}
#endif

static ssize_t show_eas_info_attr(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	unsigned int len = 0;
	unsigned int max_len = 4096;

	len += show_cpu_info(buf+len, max_len - len);

#if defined(CONFIG_ENERGY_MODEL) && defined(CONFIG_CPU_FREQ_GOV_SCHEDUTIL)
	len += show_perf_order_domain_info(buf+len, max_len - len);
#endif

	return len;
}

#if defined(OPLUS_FEATURE_SCHEDUTIL_USE_TL) && defined(CONFIG_SCHEDUTIL_USE_TL)
static ssize_t store_set_capacity_margin(struct kobject *kobj,
                                        struct kobj_attribute *attr,
                                         const char *buf,size_t count)
{
	int val = 0 ;
	char acBuffer[BUFFER_SIZE];
	int arg;
	bool core_ctl_enable = false ;

	if((count > 0) && (count < BUFFER_SIZE))
	{
		if(scnprintf(acBuffer,BUFFER_SIZE,"%s",buf))
			if(kstrtoint(acBuffer,0,&arg) == 0)
				val = arg;
			else
				return count;
	}

	if(val == 0)
	{
		core_ctl_enable = false;
		set_capacity_margin(1024);
		set_capacity_margin_dvfs(1024);
		set_capacity_margin_dvfs_changed(true);
		pr_err("set capacity margin : 1024\n");
	}
	else if(val == 1)
	{
		core_ctl_enable = true;
		set_capacity_margin(1280);
		set_capacity_margin_dvfs(1280);
		set_capacity_margin_dvfs_changed(false);
		pr_err("set capacity margin : 1280\n");
	}
	else
	{
		set_capacity_margin(val);
		set_capacity_margin_dvfs(val);
		set_capacity_margin_dvfs_changed(true);
		pr_err("set capacity margin : %d\n",val);
	}
	return count;
}
#endif

static struct kobj_attribute eas_info_attr =
#if defined(OPLUS_FEATURE_SCHEDUTIL_USE_TL) && defined(CONFIG_SCHEDUTIL_USE_TL)
__ATTR(info, 0660, show_eas_info_attr, store_set_capacity_margin);
#else
__ATTR(info, 0400, show_eas_info_attr, NULL);
#endif

static struct attribute *eas_attrs[] = {
	&eas_info_attr.attr,
	NULL,
};

static struct attribute_group eas_attr_group = {
	.attrs = eas_attrs,
};

static int init_eas_attribs(void)
{
	int err;

	eas_info.attr_group = &eas_attr_group;

	/* Create /sys/devices/system/cpu/eas/... */
	eas_info.kobj = kobject_create_and_add("eas",
				&cpu_subsys.dev_root->kobj);
	if (!eas_info.kobj)
		return -ENOMEM;

	err = sysfs_create_group(eas_info.kobj, eas_info.attr_group);
	if (err)
		kobject_put(eas_info.kobj);
	else
		kobject_uevent(eas_info.kobj, KOBJ_ADD);

	return err;
}

static int __init eas_stats_init(void)
{
	int ret = 0;

	eas_info.init = 0;

	ret = init_eas_attribs();

	if (ret)
		eas_info.init = 1;

	return ret;
}
late_initcall(eas_stats_init);
