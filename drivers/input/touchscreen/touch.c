/***************************************************
 * File:touch.c
 * VENDOR_EDIT
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
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
#include "oppo_touchscreen/tp_devices.h"
#include "oppo_touchscreen/touchpanel_common.h"
#include <soc/oppo/oppo_project.h>
#include <soc/oppo/device_info.h>
#include <soc/oplus/system/oplus_project.h>
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
struct hw_resource *g_hw_res;
static bool is_tp_type_got_in_match = false;

/*if can not compile success, please update vendor/oppo_touchsreen*/
struct tp_dev_name tp_dev_names[] = {
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
	{TP_INX, "INX"},
	{TP_TXD, "TXD"},
	{TP_HLT, "HLT"},
	{TP_HLT_TWO, "HLT_TWO"},
	{TP_SKY, "SKY"},
	{TP_UNKNOWN, "UNKNOWN"},
};

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")

typedef enum {
	ili9881_inx,
	nt36525b_txd,
	ili9882n_huaxian,
	nt36525b_hlt,
	nt36525b_hlt_two,/*5 supply touch panel cost reduction*/
	ft8006s_txd,
	ili9882n_skyworth,
	ili9882n_boe,
	ili9882n_hlt,
	ili7807s_csot,
	ili9882n_boe_90hz,
	ili9882n_boe_b8,
	ili9881h,
	nt36525b,
	ili9882n,
	ft8006s,
	TP_INDEX_NULL,
} TP_USED_INDEX;
TP_USED_INDEX tp_used_index  = TP_INDEX_NULL;

bool __init tp_judge_ic_match(char *tp_ic_name)
{
	pr_err("[TP] tp_ic_name = %s \n", tp_ic_name);

	switch(get_project()) {
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
		if (strstr(tp_ic_name, "nf_nt36525b") && strstr(saved_command_line, "nt36525b")) {
		return true;
		} else if (strstr(tp_ic_name, "hx83102d_nf") && strstr(saved_command_line, "hx83102d")) {
		return true;
		} else if (strstr(tp_ic_name, "ili9881h") && strstr(saved_command_line, "ilt9881h")) {
		return true;
		} else if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt9882n")) {
		return true;
		}
		pr_err("[TP] ERROR! ic is not match driver\n");
		return false;
	case 20701:
		pr_info("[TP] tp judge ic forward for 20701\n");
		if (strstr(tp_ic_name, "nf_nt36525b") && strstr(saved_command_line, "nt36525b")) {
			return true;
		}
		if (strstr(tp_ic_name, "hx83102d_nf") && strstr(saved_command_line, "hx83102d")) {
			return true;
		}
		if (strstr(tp_ic_name, "hx83112a_nf") && strstr(saved_command_line, "hx83112a")) {
			return true;
		}
		if (strstr(tp_ic_name, "focaltech,fts") && strstr(saved_command_line, "ft8006s")) {
			return true;
		}
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ili9882n")) {
			return true;
		}
		pr_err("[TP] ERROR! ic is not match driver\n");
		return false;
    case 20375: //jelly
    case 20376:
    case 20377:
    case 20378:
    case 20379:
    case 131962://2037A
		pr_info("[TP] tp judge ic forward for %d.\n", get_project());
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt7807s")) {
			g_tp_dev_vendor = TP_ILI7807S_HLT;
			pr_info("[TP]touchpanel: g_tp_dev_vendor = %d.\n", g_tp_dev_vendor);
			return true;
		} else if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt9882n_innolux")) {
			g_tp_dev_vendor = TP_ILI9882N_INX;
			pr_info("[TP]touchpanel: g_tp_dev_vendor = %d.\n", g_tp_dev_vendor);
			return true;
		} else if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt9882q_innolux")) {
			g_tp_dev_vendor = TP_ILI9882Q_INX;
			pr_info("[TP]touchpanel: g_tp_dev_vendor = %d.\n", g_tp_dev_vendor);
			return true;
		} else if (strstr(tp_ic_name, "hx83102d_nf") && strstr(saved_command_line, "hx83102d_txd")) {
			g_tp_dev_vendor = TP_HX83102D_TXD;
			pr_info("[TP]touchpanel: g_tp_dev_vendor = %d.\n", g_tp_dev_vendor);
			return true;
		}
		pr_err("[TP] ERROR! jelly tp ic is not match driver\n");
		return false;
	case 20361:
	case 20362:
	case 20363:
	case 20364:
	case 20365:
	case 20366:
		pr_info("[TP] tp judge ic forward for 20361\n");
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ili9882n")) {
			return true;
		}
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "htl9882n")) {
			return true;
		}
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "csot7807s")) {
			return true;
		}
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "soda9882n")) {
			return true;
		}
		pr_err("20361 [TP] ERROR! ic is not match driver\n");
		return false;
	case 20271:
	case 20272:
	case 20273:
	case 20274:
	case 20091:
	case 131745:
	case 131746:
	case 131747:
	case 131706:
	case 131707:
	case 131708:
	case 131709:
	case 21281:
	case 21282:
	case 21283:
	case 21285:
		pr_err("[TP] tp judge ic forward for 20271 || 21281\n");
		if (strstr(tp_ic_name, "ili9881h") && strstr(saved_command_line, "ilt9881h")) {
			return true;
		}
		if (strstr(tp_ic_name, "nt36525b") && strstr(saved_command_line, "nt36525b")) {
			return true;
		}
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt9882n")) {
			return true;
		}
		if (strstr(tp_ic_name, "focaltech") && strstr(saved_command_line, "ft8006s")) {
			return true;
		}
		pr_err("20271 || 21281 [TP] ERROR! ic is not match driver\n");
		return false;

	default:
		pr_err("[TP] ERROR! is not match any project.\n");
		break;
	}
	return true;
}

bool  tp_judge_ic_match_commandline(struct panel_info *panel_data)
{
	int prj_id = 0;
	int i = 0;
	bool ic_matched = false;
	prj_id = get_project();
	pr_err("[TP] get_project() = %d \n", prj_id);

	for(i = 0; i<panel_data->project_num; i++){
		if(prj_id == panel_data->platform_support_project[i]){
			g_tp_prj_id = panel_data->platform_support_project_dir[i];
			if(strstr(saved_command_line, panel_data->platform_support_commandline[i])||strstr("default_commandline", panel_data->platform_support_commandline[i]) ){
				pr_err("[TP] Driver match the project\n");
				ic_matched = true;
			}
			else{
				ic_matched = false;
				break;
			}
		}
	}
	if(!ic_matched) {
		pr_err("[TP] Driver does not match the project\n");
		pr_err("Lcd module not found\n");
		return false;
	}

	switch(prj_id) {
	case 20361:
	case 20362:
	case 20363:
	case 20364:
	case 20365:
		pr_info("[TP] case 20361\n");
		is_tp_type_got_in_match = true;

		if (strstr(saved_command_line, "ili9882n")) {
			pr_err("[TP] touch ic = ili9882n \n");
			tp_used_index = ili9882n_boe;
			g_tp_dev_vendor = TP_BOE;
		}

		break;
	case 20271:
	case 20272:
	case 20273:
	case 20274:
	case 20091:
	case 131745:
	case 131746:
	case 131747:
	case 131706:
	case 131707:
	case 131708:
	case 131709:
		pr_info("[TP] case 20271\n");
		is_tp_type_got_in_match = true;

		if (strstr(saved_command_line, "ilt9881h_inx_yogurt_hdp_dsi_vdo")) {
			pr_err("[TP] touch ic = ili9882h inx \n");
			tp_used_index = ili9881h;
			g_tp_dev_vendor = TP_INX;
		}
		if (strstr(saved_command_line, "nt36525b_txd_yogurt_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = nt36525b txd\n");
			tp_used_index = nt36525b;
			g_tp_dev_vendor = TP_TXD;
		}
		if (strstr(saved_command_line, "ilt9882n_huaxian_yogurt_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = ilt9882n huaxian \n");
			tp_used_index = ili9882n;
			g_tp_dev_vendor = TP_BOE;
		}
		if (strstr(saved_command_line, "nt36525b_hlt_yogurt_hdp_dsi_vdo")) {
			pr_err("[TP] touch ic = nt36525b \n");
			tp_used_index = nt36525b;
			g_tp_dev_vendor = TP_HLT;
		}
		if (strstr(saved_command_line, "nt36525b_hlt_two_yogurt_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = nt36525b hlt \n");
			tp_used_index = nt36525b;
			g_tp_dev_vendor = TP_HLT_TWO;
		}
		if (strstr(saved_command_line, "ft8006s_txd_yogurt_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = ft8006s \n");
			tp_used_index = ft8006s;
			g_tp_dev_vendor = TP_TXD;
		}
		if (strstr(saved_command_line, "ilt9882n_skyworth_yogurt_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = ili9882n  skyworth\n");
			tp_used_index = ili9882n;
			g_tp_dev_vendor = TP_SKY;
		}
		if (strstr(saved_command_line, "ilt9882n_txd_yogurt_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = ili9882n txd\n");
			tp_used_index = ili9882n;
			g_tp_dev_vendor = TP_TXD;
		}

		break;
	case 21281:
	case 21282:
	case 21283:
	case 21285:
		pr_info("[TP] case 21281\n");
		is_tp_type_got_in_match = true;

		if (strstr(saved_command_line, "ilt9881h_inx_yogurta_hdp_dsi_vdo")) {
			pr_err("[TP] touch ic = ili9881h inx \n");
			tp_used_index = ili9881h;
			g_tp_dev_vendor = TP_INX;
		}
		if (strstr(saved_command_line, "nt36525b_txd_yogurta_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = nt36525b txd\n");
			tp_used_index = nt36525b;
			g_tp_dev_vendor = TP_TXD;
		}
		if (strstr(saved_command_line, "ilt9882n_huaxian_yogurta_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = ilt9882n huaxian \n");
			tp_used_index = ili9882n;
			g_tp_dev_vendor = TP_BOE;
		}
		if (strstr(saved_command_line, "nt36525b_hlt_yogurta_hdp_dsi_vdo")) {
			pr_err("[TP] touch ic = nt36525b \n");
			tp_used_index = nt36525b;
			g_tp_dev_vendor = TP_HLT;
		}
		if (strstr(saved_command_line, "nt36525b_hlt_two_yogurta_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = nt36525b hlt \n");
			tp_used_index = nt36525b;
			g_tp_dev_vendor = TP_HLT_TWO;
		}
		if (strstr(saved_command_line, "ft8006s_txd_yogurta_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = ft8006s \n");
			tp_used_index = ft8006s;
			g_tp_dev_vendor = TP_TXD;
		}
		if (strstr(saved_command_line, "ilt9882n_skyworth_yogurta_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = ili9882n  skyworth\n");
			tp_used_index = ili9882n;
			g_tp_dev_vendor = TP_SKY;
		}
		if (strstr(saved_command_line, "ilt9882n_txd_yogurta_hdp_dsi_vdo_lcm")) {
			pr_err("[TP] touch ic = ili9882n txd\n");
			tp_used_index = ili9882n;
			g_tp_dev_vendor = TP_TXD;
		}

		break;

	default:
		pr_info("other project, no need process here!\n");
		break;
	}
	pr_info("[TP]ic:%d, vendor:%d\n", tp_used_index, g_tp_dev_vendor);
	return true;
}

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
    char* vendor;
	int prj_id = 0;
	prj_id = get_project();
	g_hw_res = hw_res;

    panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
    if (panel_data->test_limit_name == NULL) {
        pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
    }
        panel_data->extra= kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
        if (panel_data->extra == NULL) {
            pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
        }
//   panel_data->tp_type = g_tp_dev_vendor;
    if (panel_data->tp_type == TP_UNKNOWN) {
        pr_err("[TP]%s type is unknown\n", __func__);
        return 0;
    }

    vendor = GET_TP_DEV_NAME(panel_data->tp_type);

	if (strstr(saved_command_line, "hx83112a_txd_boe_fives")) {
		hw_res->TX_NUM = 18;
		hw_res->RX_NUM = 36;
		vendor = "TXD";
	}

    strcpy(panel_data->manufacture_info.manufacture, vendor);

    snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
            "tp/%d/FW_%s_%s.img",
            g_tp_prj_id, panel_data->chip_name, vendor);

    if (panel_data->test_limit_name) {
        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
            "tp/%d/LIMIT_%s_%s.img",
            g_tp_prj_id, panel_data->chip_name, vendor);
    }

    if (strstr(saved_command_line, "oppo19551_samsung_ams644vk01_1080p_dsi_cmd")) {
        memcpy(panel_data->manufacture_info.version, "0xbd2860000", 11); //For 19550 19551 19553 19556 19597
    }
    if (strstr(saved_command_line, "oppo19357_samsung_ams644va04_1080p_dsi_cmd")) {
        memcpy(panel_data->manufacture_info.version, "0xfa1920000", 11); //For 19357 19358 19359 19354
    }
    if (strstr(saved_command_line, "oppo19537_samsung_ams643xf01_1080p_dsi_cmd")) {
        memcpy(panel_data->manufacture_info.version, "0xdd3400000", 11); //For 19536 19537 19538 19539
    }
    if (strstr(saved_command_line, "oppo20291_samsung_ams643xy01_1080p_dsi_vdo")) {
        memcpy(panel_data->manufacture_info.version, "0xFA270000", 11); //For 20291 20292 20293 20294 20295
    }
    if (g_tp_prj_id == 132769){
        pr_err("[TP]project is 0x206A1\n", __func__);
        if (strstr(saved_command_line, "nt36525b_hlt_psc_boe_hdp_dsi_vdo_lcm")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_NF_NT36525B_HLTB3.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                     "tp/206A1/LIMIT_NF_NT36525B_HLTB3.img");

            strcpy(panel_data->manufacture_info.manufacture, "HLT");
            memcpy(panel_data->manufacture_info.version, "HLT_B3_NT25_", 12);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_NT36525B_HLTB3;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_NT36525B_HLTB3);
        }
        if (strstr(saved_command_line, "nt36525b_hlt_psc_hdp_dsi_vdo_lcm")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_NF_NT36525B_HLT.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                     "tp/206A1/LIMIT_NF_NT36525B_HLT.img");

            strcpy(panel_data->manufacture_info.manufacture, "HLT");
            memcpy(panel_data->manufacture_info.version, "HLT_NT25_",9);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_NT36525B_HLT;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_NT36525B_HLT);
        }
        if (strstr(saved_command_line, "nt36525b_txd")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_NF_NT36525B_TXD.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                     "tp/206A1/LIMIT_NF_NT36525B_TXD.img");

            strcpy(panel_data->manufacture_info.manufacture, "TXD");
            memcpy(panel_data->manufacture_info.version, "TXD_NT25_",9);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_NT36525B_TXD;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_NT36525B_TXD);
        }
        if (strstr(saved_command_line, "hx83102d_truly")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_HX83102D_NF_TRULY.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                    "tp/206A1/LIMIT_HX83102D_NF_TRULY.img");

            strcpy(panel_data->manufacture_info.manufacture, "TRULY");
            memcpy(panel_data->manufacture_info.version, "XL_HX83102D_",12);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_HX83102D_TRULY;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_HX83102D_TRULY);
        }
        if (strstr(saved_command_line, "hx83102d_hlt")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_HX83102D_NF_HLTB8.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                    "tp/206A1/LIMIT_HX83102D_NF_HLTB8.img");

            strcpy(panel_data->manufacture_info.manufacture, "HLTB8");
	    memcpy(panel_data->manufacture_info.version, "HLT_B8_HX02D", 12);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_HX83102D_HLTB8;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_HX83102D_HLTB8);
        }
        if (strstr(saved_command_line, "ilt9881h_ls")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_NF_ILT9881H_LS.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                     "tp/206A1/LIMIT_NF_ILT9881H_LS.ini");

            strcpy(panel_data->manufacture_info.manufacture, "LS");
            memcpy(panel_data->manufacture_info.version, "LS_ILI9881H_",12);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_ILT9881H_LS;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_ILT9881H_LS);
        }
        if (strstr(saved_command_line, "ilt9881h_txd")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_NF_ILT9881H_TXD.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                    "tp/206A1/LIMIT_NF_ILT9881H_TXD.ini");

            strcpy(panel_data->manufacture_info.manufacture, "TXD");
	        memcpy(panel_data->manufacture_info.version, "TXD_ILI881H_", 12);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_ILT9881H_TXD;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_ILT9881H_TXD);
        }
        if (strstr(saved_command_line, "ilt9882n_truly")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_NF_ILT9882N_TRULY.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                    "tp/206A1/LIMIT_NF_ILT9882N_TRULY.ini");

            strcpy(panel_data->manufacture_info.manufacture, "TRULY");
	        memcpy(panel_data->manufacture_info.version, "TRULY_ILI2N_", 12);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_ILT9882N_TRULY;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_ILT9882N_TRULY);
        }
        if (strstr(saved_command_line, "ilt9882n_ls")) {

            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                    "tp/206A1/FW_NF_ILT9882N_LS.bin");
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                    "tp/206A1/LIMIT_NF_ILT9882N_LS.ini");

            strcpy(panel_data->manufacture_info.manufacture, "LS");
            memcpy(panel_data->manufacture_info.version, "LS_ILI9882N_",12);
            panel_data->firmware_headfile.firmware_data = FW_206A1_NF_ILT9882N_LS;
            panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_ILT9882N_LS);
        }
    }
	if (g_tp_prj_id == 20701) {
		pr_err("[TP]project is 20701\n", __func__);
		if (strstr(saved_command_line, "nt36525b_hlt_boe")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20701/FW_NT36525B_NF_90HZ_HLT.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20701/LIMIT_NT36525B_NF_90HZ_HLT.img");

			strcpy(panel_data->manufacture_info.manufacture, "HLT");
			memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
			panel_data->firmware_headfile.firmware_data = FW_NT36525B_NF_90HZ_HLT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_NT36525B_NF_90HZ_HLT);
		}
		if (strstr(saved_command_line, "hx83102d_txd_inx")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20701/FW_HX83102D_NF_90HZ_TXD.img");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20701/LIMIT_HX83102D_NF_90HZ_TXD.img");

			strcpy(panel_data->manufacture_info.manufacture, "TXD");
			memcpy(panel_data->manufacture_info.version, "TXD_HX102D__", 12);
			panel_data->firmware_headfile.firmware_data = FW_HX83102D_NF_90HZ_TXD;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_HX83102D_NF_90HZ_TXD);
		}
                if (strstr(saved_command_line, "hx83102d_truly_truly")) {
                        snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                                "tp/20701/FW_HX83102D_NF_90HZ_TRULY.img");
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/20701/LIMIT_HX83102D_NF_90HZ_TRULY.img");

                        strcpy(panel_data->manufacture_info.manufacture, "TRULY");
                        memcpy(panel_data->manufacture_info.version, "TRULY_HX102D", 12);
                        panel_data->firmware_headfile.firmware_data = FW_20701_HX83102D_NF_90HZ_TRULY;
                        panel_data->firmware_headfile.firmware_size = sizeof(FW_20701_HX83102D_NF_90HZ_TRULY);
                }
		if (strstr(saved_command_line, "hx83112a_txd_boe_fives")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20701/FW_HX83112A_NF_90HZ_TXD.img");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20701/LIMIT_HX83112A_NF_90HZ_TXD.img");

			strcpy(panel_data->manufacture_info.manufacture, "TXD");
			memcpy(panel_data->manufacture_info.version, "TXD_HX112A__", 12);
			panel_data->firmware_headfile.firmware_data = FW_HX83112A_NF_90HZ_TXD;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_HX83112A_NF_90HZ_TXD);
		}
		if (strstr(saved_command_line, "ili9882n_truly_truly")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20701/FW_ILI9882N_NF_90HZ_TRULY.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20701/LIMIT_ILI9882N_NF_90HZ_TRULY.ini");

			strcpy(panel_data->manufacture_info.manufacture, "TRULY");
			memcpy(panel_data->manufacture_info.version, "TRULY_ILI2N_", 12);
			panel_data->firmware_headfile.firmware_data = FW_ILI9882N_NF_90HZ_TRULY;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_ILI9882N_NF_90HZ_TRULY);
		}
		if (strstr(saved_command_line, "ili9882n_mn228_txd_inx")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20701/FW_ILI9882N_NF_MN228_TXD.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20701/LIMIT_ILI9882N_NF_MN228_TXD.ini");

			strcpy(panel_data->manufacture_info.manufacture, "TXD");
			memcpy(panel_data->manufacture_info.version, "TXD_MN_ILI2N", 12);
			panel_data->firmware_headfile.firmware_data = FW_ILI9882N_NF_MN228_TXD;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_ILI9882N_NF_MN228_TXD);
		}
		if (strstr(saved_command_line, "ili9882n_txd_inx")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20701/FW_ILI9882N_NF_90HZ_TXD.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20701/LIMIT_ILI9882N_NF_90HZ_TXD.ini");

			strcpy(panel_data->manufacture_info.manufacture, "TXD");
			memcpy(panel_data->manufacture_info.version, "TXD_ILI2N", 9);
			panel_data->firmware_headfile.firmware_data = FW_ILI9882N_NF_90HZ_TXD;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_ILI9882N_NF_90HZ_TXD);
		}
		if (strstr(saved_command_line, "ft8006s_hlt_boe")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20701/FW_FT8006S_NF_90HZ_HLT.img");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20701/LIMIT_FT8006S_NF_90HZ_HLT.img");

			strcpy(panel_data->manufacture_info.manufacture, "HLT");
			memcpy(panel_data->manufacture_info.version, "HLT_FT8006S_", 12);
			panel_data->firmware_headfile.firmware_data = FW_FT8006S_NF_90HZ_HLT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_FT8006S_NF_90HZ_HLT);
		}
	}
        //jelly tp fw path init
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
	if (g_tp_prj_id == 20361 || g_tp_prj_id == 20363) {
		pr_err("[TP]project is 20361\n", __func__);
		if ((strstr(saved_command_line, "boe_ili9882n")) || (tp_used_index == ili9882n_boe)) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20361/FW_NF_ILI9882N_BOE.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20361/LIMIT_NF_ILI9882N_BOE.ini");
			pr_err("[TP]project is 20361 boe_ili9882n\n", __func__);
			strcpy(panel_data->manufacture_info.manufacture, "BOE");
			memcpy(panel_data->manufacture_info.version, "AA036BI03", 9);
			panel_data->firmware_headfile.firmware_data = FW_20361_NF_ILI9882N_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20361_NF_ILI9882N_BOE);
		}
		if (strstr(saved_command_line, "boe_htl9882n")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
			"tp/20361/FW_NF_ILI9882N_HLT.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
			"tp/20361/LIMIT_NF_ILI9882N_HLT.ini");

			strcpy(panel_data->manufacture_info.manufacture, "HLT");
			memcpy(panel_data->manufacture_info.version, "AA036HI03", 9);
			panel_data->firmware_headfile.firmware_data = FW_20361_NF_ILI9882N_HLT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20361_NF_ILI9882N_HLT);
		}
		if (strstr(saved_command_line, "csot7807s")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
			"tp/20361/FW_NF_ILI7807S_CSOT.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
			    "tp/20361/LIMIT_NF_ILI7807S_CSOT.ini");

			strcpy(panel_data->manufacture_info.manufacture, "CSOT");
			memcpy(panel_data->manufacture_info.version, "AA036CI05", 9);
			panel_data->firmware_headfile.firmware_data = FW_20361_NF_ILI7807S_CSOT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20361_NF_ILI7807S_CSOT);
		}
		if (strstr(saved_command_line, "boe_soda9882n")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
			    "tp/20361/FW_NF_ILI9882N_BOE_B8.bin");
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
			    "tp/20361/LIMIT_NF_ILI9882N_BOE_B8.ini");

			strcpy(panel_data->manufacture_info.manufacture, "BOE");
			memcpy(panel_data->manufacture_info.version, "AA036B904", 10);
			panel_data->firmware_headfile.firmware_data = FW_20361_NF_ILI9882N_BOE_90HZ;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20361_NF_ILI9882N_BOE_90HZ);
		}

	}
	if (g_tp_prj_id == 20271 || g_tp_prj_id == 21281 || g_tp_prj_id == 21285) {
		pr_err("[TP]project is 20271\n", __func__);
		if (tp_used_index == nt36525b) {
			if (g_tp_dev_vendor == TP_HLT) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_NT36525B_HLT.bin", g_tp_prj_id);
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_NT36525B_HLT.img", g_tp_prj_id);

				strcpy(panel_data->manufacture_info.manufacture, "HLT");
				memcpy(panel_data->manufacture_info.version, "FA361HN00", 9);
				panel_data->firmware_headfile.firmware_data = FW_20271_NF_NT36525B_HLT;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_20271_NF_NT36525B_HLT);
			}else if (g_tp_dev_vendor == TP_TXD) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_NT36525B_TXD.bin", g_tp_prj_id);
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_NT36525B_TXD.img", g_tp_prj_id);

				strcpy(panel_data->manufacture_info.manufacture, "TXD");
				memcpy(panel_data->manufacture_info.version, "FA361TN00", 9);
				panel_data->firmware_headfile.firmware_data = FW_20271_NF_NT36525B_TXD;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_20271_NF_NT36525B_TXD);
			}else if (g_tp_dev_vendor == TP_HLT_TWO) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_NT36525B_HLT_TWO.bin", g_tp_prj_id);
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_NT36525B_HLT_TWO.img", g_tp_prj_id);

				strcpy(panel_data->manufacture_info.manufacture, "HLT_TWO");
				memcpy(panel_data->manufacture_info.version, "FA361TN00", 9);
				panel_data->firmware_headfile.firmware_data = FW_20271_NF_NT36525B_HLT_TWO;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_20271_NF_NT36525B_HLT_TWO);
			}
		}else if(tp_used_index == ili9881h) {
			if (g_tp_dev_vendor == TP_INX) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI9881H_INNOLUX.bin", g_tp_prj_id);
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI9881H_INNOLUX.ini", g_tp_prj_id);

				strcpy(panel_data->manufacture_info.manufacture, "INNOLUX");
				memcpy(panel_data->manufacture_info.version, "FA361ll00", 9);
				panel_data->firmware_headfile.firmware_data = FW_20271_NF_ILI9881H_INX;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_20271_NF_ILI9881H_INX);
			}
		}else if(tp_used_index == ili9882n) {
			if (g_tp_dev_vendor == TP_BOE) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI9882N_HUAXIAN.bin", g_tp_prj_id);
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI9882N_HUAXIAN.ini", g_tp_prj_id);

				strcpy(panel_data->manufacture_info.manufacture, "BOE");
				memcpy(panel_data->manufacture_info.version, "FA361CI00", 9);
				panel_data->firmware_headfile.firmware_data = FW_20271_NF_ILI9882N_HUAXIAN;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_20271_NF_ILI9882N_HUAXIAN);
			}else if (g_tp_dev_vendor == TP_SKY) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI9882N_SKYWORTH.bin", g_tp_prj_id);
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI9882N_SKYWORTH.ini", g_tp_prj_id);

				strcpy(panel_data->manufacture_info.manufacture, "SKYWORTH");
				memcpy(panel_data->manufacture_info.version, "FA361SI00", 9);
				panel_data->firmware_headfile.firmware_data = FW_20271_NF_ILI9882N_SKYWORTH;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_20271_NF_ILI9882N_SKYWORTH);
			}else if (g_tp_dev_vendor == TP_TXD) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI9882N_BOE_B8.bin", g_tp_prj_id);
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI9882N_BOE_B8.ini", g_tp_prj_id);

				strcpy(panel_data->manufacture_info.manufacture, "TXD");
				memcpy(panel_data->manufacture_info.version, "FA361BI00", 9);
				panel_data->firmware_headfile.firmware_data = FW_20271_NF_ILI9882N_BOE_B8;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_20271_NF_ILI9882N_BOE_B8);
			}
		}else if(tp_used_index == ft8006s) {
			if (g_tp_dev_vendor == TP_TXD) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_FT8006S_TXD_FT.bin", g_tp_prj_id);
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_FT8006S_TXD_FT.img", g_tp_prj_id);

				strcpy(panel_data->manufacture_info.manufacture, "TXD");
				memcpy(panel_data->manufacture_info.version, "FA361TF00", 9);
				panel_data->firmware_headfile.firmware_data = FW_20271_NF_FT8006S_TXD;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_20271_NF_FT8006S_TXD);
			}
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
	panel_data->manufacture_info.fw_path = panel_data->fw_name;

    pr_info("[TP]vendor:%s fw:%s limit:%s\n",
        vendor,
        panel_data->fw_name,
        panel_data->test_limit_name == NULL ? "NO Limit" : panel_data->test_limit_name);

    return 0;
}

