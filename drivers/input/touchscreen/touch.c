/***************************************************
 * File:touch.c
 * Copyright (c)  2008- 2030  oplus Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * TAG: BSP.TP.Init
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "oplus_touchscreen/Synaptics/S3508/synaptics_s3508.h"
#include "oplus_touchscreen/tp_devices.h"
#include "oplus_touchscreen/touchpanel_common.h"
#include <soc/oplus/system/oplus_project.h>
#include <soc/oplus/device_info.h>
#include "touch.h"

#define MAX_LIMIT_DATA_LENGTH         100
#define S3706_FW_NAME_18073 "tp/18073/18073_FW_S3706_SYNAPTICS.img"
#define S3706_BASELINE_TEST_LIMIT_NAME_18073 "tp/18073/18073_Limit_data.img"
#define S3706_FW_NAME_19301 "tp/19301/19301_FW_S3706_SYNAPTICS.img"
#define S3706_BASELINE_TEST_LIMIT_NAME_19301 "tp/19301/19301_Limit_data.img"
#define GT9886_FW_NAME_19551 "tp/19551/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19551 "tp/19551/LIMIT_GT9886_SAMSUNG.img"
#define GT9886_FW_NAME_19357 "tp/19357/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19357 "tp/19357/LIMIT_GT9886_SAMSUNG.img"
#define GT9886_FW_NAME_19537 "tp/19537/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19537 "tp/19537/LIMIT_GT9886_SAMSUNG.img"
#define TD4330_NF_CHIP_NAME "TD4330_NF"

extern char *saved_command_line;
int g_tp_dev_vendor = TP_UNKNOWN;
int g_tp_prj_id = 0;
int ret = 0;

/*if can not compile success, please update vendor/oppo_touchsreen*/
struct tp_dev_name tp_dev_names[] =
{
	{TP_OFILM, "OFILM"},
	{TP_BIEL, "BIEL"},
	{TP_TRULY, "TRULY"},
	{TP_BOE, "BOE"},
	{TP_G2Y, "G2Y"},
	{TP_TPK, "TPK"},
	{TP_JDI, "JDI"},
	{TP_TIANMA, "TIANMA"},
	{TP_SAMSUNG, "SAMSUNG"},
	{TP_DSJM, "DSJM"},
	{TP_BOE_B8, "BOEB8"},
	{TP_INNOLUX, "INNOLUX"},
	{TP_HIMAX_DPT, "DPT"},
	{TP_AUO, "AUO"},
	{TP_DEPUTE, "DEPUTE"},
	{TP_UNKNOWN, "UNKNOWN"},
};

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")


bool __init tp_judge_ic_match(char *tp_ic_name)
{
	int prj_id = 0;
	pr_err("[TP] tp_ic_name = %s \n", tp_ic_name);
	prj_id = get_project();

	switch(prj_id)
	{
	case 21251:
	case 21253:
	case 21254:
		if (strstr(tp_ic_name, "ili9882n")) {
			return true;
		}
		pr_err("[TP] ERROR! ic is not match driver\n");
		return false;
	case 132769:
		pr_info("[TP] tp judge ic forward for 206A1\n");
		if (strstr(tp_ic_name, "nf_nt36525b") && strstr(saved_command_line, "nt36525b_hlt"))
		{
			return true;
		}
		else if (strstr(tp_ic_name, "hx83102d_nf") && strstr(saved_command_line, "hx83102d"))
		{
			return true;
		}
		else if (strstr(tp_ic_name, "ili9881h") && strstr(saved_command_line, "ilt9881h"))
		{
			return true;
		}
		pr_err("[TP] ERROR! ic is not match driver\n");
		return false;
	case 20375: /*jelly*/
	case 20376:
	case 20377:
	case 20378:
	case 20379:
	case 131962:/*2037A*/
		pr_info("[TP] tp judge ic forward for %d.\n", get_project());
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt7807s")) {
			return true;
		} else if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt9882n_innolux")) {
			return true;
		} else if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt9882q_innolux")) {
			return true;
		} else if (strstr(tp_ic_name, "hx83102d_nf") && strstr(saved_command_line, "hx83102d_txd")) {
			return true;
		}
		pr_err("[TP] ERROR! jelly tp ic is not match driver\n");
		return false;
	case 21331:
	case 21332:
	case 21333:
	case 21334:
	case 21335:
	case 21336:
	case 21337:
	case 21338:
	case 21339:
	case 21361:
	case 21362:
	case 21363:
	case 21107:
                if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "td4160_inx")) {
                        return true;
                }
				else if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "td4160_truly")) {
                        return true;
                }
                else if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili9883c_hlt")) {
                        return true;
                }
                else if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili9883c_boe")) {
                        return true;
                }
		return false;
	default:
		break;
	}
	return true;
}

bool  tp_judge_ic_match_commandline(struct panel_info *panel_data)
{
	int prj_id = 0;
	int i = 0;
	prj_id = get_project();

	pr_err("[TP] get_project() = %d \n", prj_id);
	pr_err("[TP] boot_command_line = %s \n", saved_command_line);
	for(i = 0; i < panel_data->project_num; i++)
	{
		if(prj_id == panel_data->platform_support_project[i])
		{
			g_tp_prj_id = panel_data->platform_support_project_dir[i];
			if(strstr(saved_command_line, panel_data->platform_support_commandline[i]) || strstr("default_commandline", panel_data->platform_support_commandline[i]) )
			{
				pr_err("[TP] Driver match the project\n");
				return true;
			}
			else
			{
				break;
			}
		}
	}
	pr_err("[TP] Driver does not match the project\n");
	pr_err("Lcd module not found\n");
	return false;
}

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
	char *vendor;

	panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
	if (panel_data->test_limit_name == NULL)
	{
		pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
	}
	panel_data->extra = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
	if (panel_data->extra == NULL)
	{
		pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
	}
	//   panel_data->tp_type = g_tp_dev_vendor;
	if (panel_data->tp_type == TP_UNKNOWN)
	{
		pr_err("[TP]%s type is unknown\n", __func__);
		return 0;
	}

	vendor = GET_TP_DEV_NAME(panel_data->tp_type);

	strcpy(panel_data->manufacture_info.manufacture, vendor);

	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
	         "tp/%d/FW_%s_%s.img",
	         g_tp_prj_id, panel_data->chip_name, vendor);

	if (panel_data->test_limit_name)
	{
		snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
		         "tp/%d/LIMIT_%s_%s.img",
		         g_tp_prj_id, panel_data->chip_name, vendor);
	}

	if (strstr(saved_command_line, "oppo19551_samsung_ams644vk01_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xbd2860000", 11); //For 19550 19551 19553 19556 19597
	}
	if (strstr(saved_command_line, "oppo19357_samsung_ams644va04_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xfa1920000", 11); //For 19357 19358 19359 19354
	}
	if (strstr(saved_command_line, "oppo19537_samsung_ams643xf01_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xdd3400000", 11); //For 19536 19537 19538 19539
	}
	if (strstr(saved_command_line, "oppo20291_samsung_ams643xy01_1080p_dsi_vdo"))
	{
		memcpy(panel_data->manufacture_info.version, "0xFA270000", 11); //For 20291 20292 20293 20294 20295
	}
	if (strstr(saved_command_line, "nt36525b_hlt"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_NT36525B_HLT;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_NT36525B_HLT);
	}
	if (strstr(saved_command_line, "hx83102d"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_HX83102D_TRULY;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_HX83102D_TRULY);
	}
	if (strstr(saved_command_line, "ilt9881h"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_ILT9881H_LS;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_ILT9881H_LS);
	}

        if (strstr(saved_command_line, "oplus21331_td4160_inx")) {
                memcpy(panel_data->manufacture_info.version, "AA340IOA1", 9);
                panel_data->firmware_headfile.firmware_data = FW_21331_NF_ZHAOYUN;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_ZHAOYUN);
        	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                 	"tp/21331/FW_%s_%s.img",
                 	"TD4160", "INX");

        	if (panel_data->test_limit_name) {
                	snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                         	"tp/21331/LIMIT_%s_%s.img",
                         	"TD4160", "INX");
        	}
        }
        if (strstr(saved_command_line, "oplus21331_ili9883c_hlt")) {
                memcpy(panel_data->manufacture_info.version, "AA340HI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21331_NF_9883A;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_9883A);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/21331/FW_%s_%s.img",
                        "9883A", "HLT");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/21331/LIMIT_%s_%s.img",
                                "9883A", "HLT");
                }
        }
	if (strstr(saved_command_line, "oplus21331_ili9883c_boe")) {
                memcpy(panel_data->manufacture_info.version, "AA340BI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21331_NF_9883A;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_9883A);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/21331/FW_%s_%s.img",
                        "9883A", "BOE");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/21331/LIMIT_%s_%s.img",
                                "9883A", "BOE");
                }
        }
        if (strstr(saved_command_line, "oplus21361_td4160_inx")) {
                memcpy(panel_data->manufacture_info.version, "AA364IO01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21361_NF_INX;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21361_NF_INX);
        	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                 	"tp/21361/FW_%s_%s.img",
                 	"TD4160", "INX");

        	if (panel_data->test_limit_name) {
                	snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                         	"tp/21361/LIMIT_%s_%s.img",
                         	"TD4160", "INX");
        	}
        }
		if (strstr(saved_command_line, "oplus21361_td4160_truly")) {
                memcpy(panel_data->manufacture_info.version, "AA364TI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21361_NF_TRULY;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21361_NF_TRULY);
        	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                 	"tp/21361/FW_%s_%s.img",
                 	"TD4160", "TRULY");

        	if (panel_data->test_limit_name) {
                	snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                         	"tp/21361/LIMIT_%s_%s.img",
                         	"TD4160", "TRULY");
        	}
        }
        if (strstr(saved_command_line, "oplus21361_ili9883c_hlt")) {
                memcpy(panel_data->manufacture_info.version, "AA364HI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21361_NF_9883C;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21361_NF_9883C);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/21361/FW_%s_%s.img",
                        "9883C", "HLT");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/21361/LIMIT_%s_%s.img",
                                "9883C", "HLT");
                }
        }
	if (g_tp_prj_id == 21251) {
		if (strstr(saved_command_line, "oplus21251_boe_ili9882n")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI9882N_BOE.bin", g_tp_prj_id);
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI9882N_BOE.ini", g_tp_prj_id);
			strcpy(panel_data->manufacture_info.manufacture, "BOE");
			memcpy(panel_data->manufacture_info.version, "AA252CI", 7);
		} else if (strstr(saved_command_line, "oplus21251_csot_ili7807s")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI7807S_CSOT.bin", g_tp_prj_id);
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI7807S_CSOT.ini", g_tp_prj_id);
			strcpy(panel_data->manufacture_info.manufacture, "CSOT");
			memcpy(panel_data->manufacture_info.version, "AA252CI", 7);
			panel_data->firmware_headfile.firmware_data = FW_21251_NF_ILI7807S_CSOT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21251_NF_ILI7807S_CSOT);
		} else {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI9882Q_BOE.bin", g_tp_prj_id);
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI9882Q_BOE.ini", g_tp_prj_id);
			strcpy(panel_data->manufacture_info.manufacture, "BOE");
			memcpy(panel_data->manufacture_info.version, "AA252BI", 7);
			panel_data->firmware_headfile.firmware_data = FW_21251_NF_ILI9882Q_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21251_NF_ILI9882Q_BOE);
		}
	}

	if ((g_tp_prj_id == 20375) || (g_tp_prj_id == 20376) || (g_tp_prj_id == 20377)
		|| (g_tp_prj_id == 20378) || (g_tp_prj_id == 20379) || (g_tp_prj_id == 131962)) {
		pr_err("[TP] %s project is %d\n", __func__, g_tp_prj_id);
		if (strstr(saved_command_line, "ilt7807s_hlt")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20375/FW_NF_ILI7807S_HLT.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20375/LIMIT_NF_ILI7807S_HLT.ini");
			strcpy(panel_data->manufacture_info.manufacture, "HLT");
			memcpy(panel_data->manufacture_info.version, "AA070HI01", 9);
			panel_data->firmware_headfile.firmware_data = FW_20375_NF_ILI7807S_HLT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20375_NF_ILI7807S_HLT);
		}
		if (strstr(saved_command_line, "ilt9882n_innolux")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20375/FW_NF_ILI9882N_INX.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20375/LIMIT_NF_ILI9882N_INX.ini");
			strcpy(panel_data->manufacture_info.manufacture, "INX");
			memcpy(panel_data->manufacture_info.version, "AA070HI01", 9);
			panel_data->firmware_headfile.firmware_data = FW_20375_NF_ILI9882N_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20375_NF_ILI9882N_INX);
		}
		if (strstr(saved_command_line, "ilt9882q_innolux")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20375/FW_NF_ILI9882Q_INX.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20375/LIMIT_NF_ILI9882Q_INX.ini");
			strcpy(panel_data->manufacture_info.manufacture, "INX");
			memcpy(panel_data->manufacture_info.version, "AA070II01", 9);
			panel_data->firmware_headfile.firmware_data = FW_20375_NF_ILI9882Q_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20375_NF_ILI9882Q_INX);
		}
		if (strstr(saved_command_line, "hx83102d_txd")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20375/FW_NF_HX83102D_TXD.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20375/LIMIT_NF_HX83102D_TXD.img");
			strcpy(panel_data->manufacture_info.manufacture, "TXD");
			memcpy(panel_data->manufacture_info.version, "AA070tH01", 9);
			panel_data->firmware_headfile.firmware_data = FW_20375_NF_HX83102D_TXD;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20375_NF_HX83102D_TXD);
		}
	}

	panel_data->manufacture_info.fw_path = panel_data->fw_name;
	pr_info("[TP]vendor:%s fw:%s limit:%s\n",
	        vendor,
	        panel_data->fw_name,
	        panel_data->test_limit_name == NULL ? "NO Limit" : panel_data->test_limit_name);

	return 0;
}

