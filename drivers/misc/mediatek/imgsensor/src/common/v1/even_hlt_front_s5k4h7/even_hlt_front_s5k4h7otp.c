#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
//#include <asm/atomic.h>
#include <linux/slab.h>


//#include "kd_camera_hw.h"
#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "cam_cal.h"
#include "cam_cal_define.h"

#include "even_hlt_front_s5k4h7otp.h"
#include "even_hlt_front_s5k4h7mipiraw_Sensor.h"



static struct  even_hlt_front_s5k4h7_otp_struct *EVEN_HLT_FRONT_S5K4H7_OTP;
static void even_hlt_front_s5k4h7_read_OTP(struct even_hlt_front_s5k4h7_otp_struct *EVEN_HLT_FRONT_S5K4H7_OTP)
{
	unsigned char val=0;
	int R_temp =0 ,Gr_temp = 0,Gb_temp = 0,B_temp = 0;
	unsigned int year_temp = 0,month_temp = 0,day_temp = 0;

	even_hlt_front_s5k4h7_write_cmos_sensor(0x0100,0x01);
	mdelay(5);
	even_hlt_front_s5k4h7_write_cmos_sensor(0x0A02,0x15);
	even_hlt_front_s5k4h7_write_cmos_sensor(0x3B41,0x01);
	even_hlt_front_s5k4h7_write_cmos_sensor(0x3B42,0x03);
	even_hlt_front_s5k4h7_write_cmos_sensor(0x3B40,0x01);
	even_hlt_front_s5k4h7_write_cmos_sensor(0x0A00,0x01);
	mdelay(5);

	val=even_hlt_front_s5k4h7_read_cmos_sensor(0x0A10);//flag of info and awb
	printk("even_hlt_front_s5k4h7_read_OTP group1 0x0A10 val=%x\n",val);

	if(val==0x01)
	{
		EVEN_HLT_FRONT_S5K4H7_OTP->flag = 0x01;
		EVEN_HLT_FRONT_S5K4H7_OTP->MID = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A04);
		EVEN_HLT_FRONT_S5K4H7_OTP->LID = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A0A);
		R_temp = (even_hlt_front_s5k4h7_read_cmos_sensor(0x0A12)|(even_hlt_front_s5k4h7_read_cmos_sensor(0x0A13)<<8));
		Gr_temp = (even_hlt_front_s5k4h7_read_cmos_sensor(0x0A14)|(even_hlt_front_s5k4h7_read_cmos_sensor(0x0A15)<<8));
		Gb_temp = (even_hlt_front_s5k4h7_read_cmos_sensor(0x0A16)|(even_hlt_front_s5k4h7_read_cmos_sensor(0x0A17)<<8));
		B_temp = (even_hlt_front_s5k4h7_read_cmos_sensor(0x0A18)|(even_hlt_front_s5k4h7_read_cmos_sensor(0x0A19)<<8));
		EVEN_HLT_FRONT_S5K4H7_OTP->RGr_ratio = R_temp*512/Gr_temp;
		EVEN_HLT_FRONT_S5K4H7_OTP->BGr_ratio = B_temp*512/Gr_temp;
		EVEN_HLT_FRONT_S5K4H7_OTP->GbGr_ratio = Gb_temp*512/Gr_temp;
		year_temp = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A08);
		month_temp = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A07);
		day_temp = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A06);
	}else{

		even_hlt_front_s5k4h7_write_cmos_sensor(0x0A02,0x17);
		even_hlt_front_s5k4h7_write_cmos_sensor(0x3B41,0x01);
		even_hlt_front_s5k4h7_write_cmos_sensor(0x3B42,0x03);
		even_hlt_front_s5k4h7_write_cmos_sensor(0x3B40,0x01);
		even_hlt_front_s5k4h7_write_cmos_sensor(0x0A00,0x01);
		mdelay(5);
		val=even_hlt_front_s5k4h7_read_cmos_sensor(0x0A10);//flag of info and awb
		printk("even_hlt_front_s5k4h7_read_OTP group2 0x0A10 val=%x\n",val);

		if(val==0x01){

			EVEN_HLT_FRONT_S5K4H7_OTP->flag = 0x01;
			EVEN_HLT_FRONT_S5K4H7_OTP->MID = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A04);
			EVEN_HLT_FRONT_S5K4H7_OTP->LID = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A0A);
			R_temp = (even_hlt_front_s5k4h7_read_cmos_sensor(0x0A12)|(even_hlt_front_s5k4h7_read_cmos_sensor(0x0A13)<<8));
			Gr_temp = (even_hlt_front_s5k4h7_read_cmos_sensor(0x0A14)|(even_hlt_front_s5k4h7_read_cmos_sensor(0x0A15)<<8));
			Gb_temp = (even_hlt_front_s5k4h7_read_cmos_sensor(0x0A16)|(even_hlt_front_s5k4h7_read_cmos_sensor(0x0A17)<<8));
			B_temp = (even_hlt_front_s5k4h7_read_cmos_sensor(0x0A18)|(even_hlt_front_s5k4h7_read_cmos_sensor(0x0A19)<<8));
			EVEN_HLT_FRONT_S5K4H7_OTP->RGr_ratio = R_temp*512/Gr_temp;
			EVEN_HLT_FRONT_S5K4H7_OTP->BGr_ratio = B_temp*512/Gr_temp;
			EVEN_HLT_FRONT_S5K4H7_OTP->GbGr_ratio = Gb_temp*512/Gr_temp;
			year_temp = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A08);
			month_temp = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A07);
			day_temp = even_hlt_front_s5k4h7_read_cmos_sensor(0x0A06);
		}else{
			EVEN_HLT_FRONT_S5K4H7_OTP->flag=0x00;
			EVEN_HLT_FRONT_S5K4H7_OTP->MID =0x00;
			EVEN_HLT_FRONT_S5K4H7_OTP->LID = 0x00;
			EVEN_HLT_FRONT_S5K4H7_OTP->RGr_ratio = 0x00;
			EVEN_HLT_FRONT_S5K4H7_OTP->BGr_ratio = 0x00;
			EVEN_HLT_FRONT_S5K4H7_OTP->GbGr_ratio = 0x00;
		}
	}

	printk("even_hlt_front_s5k4h7_read_OTP EVEN_HLT_FRONT_S5K4H7_OTP->MID=0x%x,EVEN_HLT_FRONT_S5K4H7_OTP->LID=0x%x\r\n",EVEN_HLT_FRONT_S5K4H7_OTP->MID,EVEN_HLT_FRONT_S5K4H7_OTP->LID);
	printk("even_hlt_front_s5k4h7_read_OTP R_temp=%d,Gr_temp=%d,Gb_temp=%d,B_temp=%d\r\n",R_temp,Gr_temp,Gb_temp,B_temp);
	printk("even_hlt_front_s5k4h7_read_OTP EVEN_HLT_FRONT_S5K4H7_OTP->RGr_ratio=%d,EVEN_HLT_FRONT_S5K4H7_OTP->BGr_ratio=%d EVEN_HLT_FRONT_S5K4H7_OTP->GbGr_ratio=%d\r\n",EVEN_HLT_FRONT_S5K4H7_OTP->RGr_ratio,EVEN_HLT_FRONT_S5K4H7_OTP->BGr_ratio,EVEN_HLT_FRONT_S5K4H7_OTP->GbGr_ratio);

	even_hlt_front_s5k4h7_write_cmos_sensor(0x0A00,0x00);
	even_hlt_front_s5k4h7_write_cmos_sensor(0x0100,0x00);
}
static void even_hlt_front_s5k4h7_apply_OTP(struct even_hlt_front_s5k4h7_otp_struct *EVEN_HLT_FRONT_S5K4H7_OTP)
{
	int R_gain,B_gain,Gb_gain,Gr_gain,Base_gain;
	if(((EVEN_HLT_FRONT_S5K4H7_OTP->flag)&0x03)!=0x01)
		return;

	R_gain = (RGr_ratio_Typical*1000)/EVEN_HLT_FRONT_S5K4H7_OTP->RGr_ratio;
	B_gain = (BGr_ratio_Typical*1000)/EVEN_HLT_FRONT_S5K4H7_OTP->BGr_ratio;
	Gb_gain = (GbGr_ratio_Typical*1000)/EVEN_HLT_FRONT_S5K4H7_OTP->GbGr_ratio;
	Gr_gain = 1000;
    Base_gain = R_gain;
	if(Base_gain>B_gain) Base_gain=B_gain;
	if(Base_gain>Gb_gain) Base_gain=Gb_gain;
	if(Base_gain>Gr_gain) Base_gain=Gr_gain;
	R_gain = 0x100 * R_gain /Base_gain;
	B_gain = 0x100 * B_gain /Base_gain;
	Gb_gain = 0x100 * Gb_gain /Base_gain;
	Gr_gain = 0x100 * Gr_gain /Base_gain;
	printk("HLT_S5K5e8_OTP R_gain=%x,B_gain=%x Gb_gain=%x Gr_gain=%x\n",R_gain,B_gain,Gb_gain,Gr_gain);
	if(Gr_gain>0x100)
		{
			even_hlt_front_s5k4h7_write_cmos_sensor(0x020e,Gr_gain>>8);
			even_hlt_front_s5k4h7_write_cmos_sensor(0x020f,Gr_gain&0xff);
		}
	if(R_gain>0x100)
		{
			even_hlt_front_s5k4h7_write_cmos_sensor(0x0210,R_gain>>8);
			even_hlt_front_s5k4h7_write_cmos_sensor(0x0211,R_gain&0xff);
		}
	if(B_gain>0x100)
		{
			even_hlt_front_s5k4h7_write_cmos_sensor(0x0212,B_gain>>8);
			even_hlt_front_s5k4h7_write_cmos_sensor(0x0213,B_gain&0xff);
		}
	if(Gb_gain>0x100)
		{
			even_hlt_front_s5k4h7_write_cmos_sensor(0x0214,Gb_gain>>8);
			even_hlt_front_s5k4h7_write_cmos_sensor(0x0215,Gb_gain&0xff);
		}

	//LSC on
	even_hlt_front_s5k4h7_write_cmos_sensor(0x3400,0x00);
	even_hlt_front_s5k4h7_write_cmos_sensor(0x0b00,0x01);
}
char EVEN_HLT_FRONT_S5K4H7_OTP_CheckID(void)
{
	EVEN_HLT_FRONT_S5K4H7_OTP = kzalloc(sizeof(struct even_hlt_front_s5k4h7_otp_struct), GFP_KERNEL);
	even_hlt_front_s5k4h7_read_OTP(EVEN_HLT_FRONT_S5K4H7_OTP);
	return EVEN_HLT_FRONT_S5K4H7_OTP->MID;
}
void EVEN_HLT_FRONT_S5K4H7_OTP_Release(void)
{
	kfree(EVEN_HLT_FRONT_S5K4H7_OTP);
}
void EVEN_HLT_FRONT_S5K4H7_OTP_Setting(void)
{
	even_hlt_front_s5k4h7_apply_OTP(EVEN_HLT_FRONT_S5K4H7_OTP);
}


