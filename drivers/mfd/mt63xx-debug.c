// SPDX-License-Identifier: GPL-2.0
//
// Copyright (c) 2019 MediaTek Inc.

#include <linux/mfd/mt6397/core.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#if !defined(CONFIG_MTK_ECCCI_DRIVER)
#include <mtk_boot_common.h>
#endif

struct mt63xx_consumer_data {
	struct mutex lock;
	struct regmap *regmap;
	unsigned int reg_value;
};

#if !defined(CONFIG_MTK_ECCCI_DRIVER)
struct tag_bootmode {
	u32 size;
	u32 tag;
	u32 bootmode;
	u32 boottype;
};

static int pmic_get_boot_mode(void)
{
	struct device_node *np_chosen = NULL;
	struct tag_bootmode *tag = NULL;
	static int boot_mode = -1;

	if (boot_mode >= 0)
		return boot_mode;

	np_chosen = of_find_node_by_path("/chosen");
	if (!np_chosen)
		np_chosen = of_find_node_by_path("/chosen@0");

	tag = (struct tag_bootmode *)of_get_property(np_chosen, "atag,boot", NULL);
	if (!tag) {
		pr_notice("%s: fail to get atag,boot\n", __func__);
	} else {
		pr_debug("%s: boot_mode: 0x%x\n", __func__, tag->bootmode);
		boot_mode = tag->bootmode;
	}

	return boot_mode;
}
#endif

static ssize_t pmic_access_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct mt63xx_consumer_data *data = dev_get_drvdata(dev);

	pr_info("[%s] 0x%x\n", __func__, data->reg_value);
	return sprintf(buf, "0x%x\n", data->reg_value);
}

static ssize_t pmic_access_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf,
				 size_t size)
{
	struct mt63xx_consumer_data *data;
	int ret = 0;
	char *pvalue = NULL, *addr, *val;
	unsigned int reg_val = 0;
	unsigned int reg_adr = 0;

	if (dev) {
		data = dev_get_drvdata(dev);
		if (!data)
			return -ENODEV;
	} else
		return -ENODEV;

	if (buf != NULL && size != 0) {
		pr_info("[%s] size is %d, buf is %s\n", __func__,
			(int)size, buf);

		pvalue = (char *)buf;
		addr = strsep(&pvalue, " ");
		val = strsep(&pvalue, " ");
		if (addr)
			ret = kstrtou32(addr, 16, (unsigned int *)&reg_adr);
		mutex_lock(&data->lock);
		if (val) {
			ret = kstrtou32(val, 16, (unsigned int *)&reg_val);
			ret = regmap_write(data->regmap, reg_adr, reg_val);
		} else {
			ret = regmap_read(data->regmap,
					  reg_adr, &data->reg_value);
		}
		mutex_unlock(&data->lock);
		pr_info("%s PMIC Reg[0x%x]=0x%x!\n",
			(val ? "write" : "read"), reg_adr,
			(val ? reg_val : data->reg_value));
	}
	return size;
}
static DEVICE_ATTR_RW(pmic_access);

static int mt63xx_debug_probe(struct platform_device *pdev)
{
	struct mt6397_chip *chip = dev_get_drvdata(pdev->dev.parent);
	struct mt63xx_consumer_data *drvdata;
	int ret = 0;

	drvdata = devm_kzalloc(&pdev->dev, sizeof(struct mt63xx_consumer_data),
			       GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	mutex_init(&drvdata->lock);
	drvdata->regmap = chip->regmap;
	platform_set_drvdata(pdev, drvdata);

#if !defined(CONFIG_MTK_ECCCI_DRIVER)
	/* for wifi only projects */
	if (pmic_get_boot_mode() == META_BOOT) {
		/* Create sysfs entry only in meta mode*/
		ret = device_create_file(&pdev->dev, &dev_attr_pmic_access);
		if (ret < 0)
			pr_info("%s failed to create sysfs file\n", __func__);
	}
#else
	/* Create sysfs entry */
	ret = device_create_file(&pdev->dev, &dev_attr_pmic_access);
	if (ret < 0)
		pr_info("%s failed to create sysfs file\n", __func__);
#endif

	return ret;
}

static int mt63xx_debug_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_pmic_access);

	return 0;
}

static const struct of_device_id mt63xx_debug_of_match[] = {
	{
		.compatible = "mediatek,mt63xx-debug",
	}, {
		/* sentinel */
	}
};
MODULE_DEVICE_TABLE(of, mt63xx_debug_of_match);

static struct platform_driver mt63xx_debug_driver = {
	.driver = {
		.name = "mt63xx-debug",
		.of_match_table = mt63xx_debug_of_match,
	},
	.probe = mt63xx_debug_probe,
	.remove = mt63xx_debug_remove,
};

module_platform_driver(mt63xx_debug_driver);

MODULE_AUTHOR("Wen Su <wen.su@mediatek.com>");
MODULE_DESCRIPTION("Debug driver for MediaTek MT63xx PMIC");
MODULE_LICENSE("GPL");
