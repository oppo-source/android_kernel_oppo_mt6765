ccflags-y += -I$(src)			# needed for trace events

##obj-y					+= ion/

obj-$(CONFIG_ASHMEM)			+= ashmem.o
obj-$(CONFIG_ANDROID_VSOC)		+= vsoc.o
obj-$(CONFIG_ANDROID_INTF_ALARM_DEV)	+= alarm-dev.o

## once CONFIG_MTK_ION is set as y, means this project
##     will use MTK_ION, NOT aosp ION.

## CONFIG_ION will always equals to y when use ION

ifeq ($(CONFIG_MTK_ION),y)
  $(info ION:enable mtk ion)
  obj-$(CONFIG_MTK_ION)             += mtk_ion/
else ifeq ($(CONFIG_ION),y)
  $(info ION: enable aosp ion)
  obj-$(CONFIG_ION)                 += aosp_ion/
else
  $(info ION: ION disable)
endif
#ifdef VENDOR_EDIT
obj-y   += oppo_ion/
#endif /* VENDOR_EDIT */

#ifdef OPLUS_FEATURE_HANS_FREEZE
obj-$(CONFIG_OPPO_HANS) += hans.o
obj-$(CONFIG_OPPO_HANS) += hans_netfilter.o
#endif /*OPLUS_FEATURE_HANS_FREEZE*/