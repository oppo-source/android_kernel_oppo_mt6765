# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2019 MediaTek Inc.

LOCAL_PATH := $(call my-dir)

ifeq ($(notdir $(LOCAL_PATH)),$(strip $(LINUX_KERNEL_VERSION)))
ifneq ($(strip $(TARGET_NO_KERNEL)),true)

include $(LOCAL_PATH)/kenv.mk


#ifdef OPLUS_ARCH_INJECT
my_feature_file := $(LOCAL_PATH)/oplus_native_features.sh

$(shell echo "#!/bin/bash" > $(my_feature_file))
$(shell echo 'export OPLUS_NATIVE_FEATURE_SET="$(strip $(SOONG_CONFIG_oplusNativeFeaturePlugin))"' >> $(my_feature_file))
$(foreach key,$(SOONG_CONFIG_oplusNativeFeaturePlugin), \
  $(shell echo 'export $(key)=$(SOONG_CONFIG_oplusNativeFeaturePlugin_$(key))' >> $(my_feature_file))\
)
#endif /* OPLUS_ARCH_INJECT */

ifeq ($(wildcard $(TARGET_PREBUILT_KERNEL)),)
KERNEL_MAKE_DEPENDENCIES := $(shell find $(KERNEL_DIR) -name .git -prune -o -type f | sort)

$(TARGET_KERNEL_CONFIG): PRIVATE_DIR := $(KERNEL_DIR)
$(TARGET_KERNEL_CONFIG): $(KERNEL_CONFIG_FILE) $(LOCAL_PATH)/Android.mk
$(TARGET_KERNEL_CONFIG): $(KERNEL_MAKE_DEPENDENCIES)
	#ifdef OPLUS_FEATURE_FORCE_SELINUX
	OBSOLETE_KEEP_ADB_SECURE=$(OBSOLETE_KEEP_ADB_SECURE) \
	OPLUS_MEMLEAK_DETECT=$(TARGET_MEMLEAK_DETECT_TEST) \
	OPLUS_AGING_TEST=$(AGING_DEBUG_MASK) \
	OPPO_AGING_TYPE=$(OPPO_AGING_TYPE) \
	$(KERNEL_DIR)/tools/changeConfig.sh $(KERNEL_CONFIG_FILE)
	#endif OPLUS_FEATURE_FORCE_SELINUX
	$(hide) mkdir -p $(dir $@)
#ifdef OPLUS_ARCH_INJECT
	source kernel-4.19/oplus_native_features.sh ; \
	export OPPO_TARGET_DEVICE=$(OPPO_TARGET_DEVICE);\
	$(PREBUILT_MAKE_PREFIX)$(MAKE) -C $(PRIVATE_DIR) $(KERNEL_MAKE_OPTION) $(KERNEL_DEFCONFIG)
#endif /* OPLUS_ARCH_INJECT */

.KATI_RESTAT: $(KERNEL_ZIMAGE_OUT)
$(KERNEL_ZIMAGE_OUT): PRIVATE_DIR := $(KERNEL_DIR)
$(KERNEL_ZIMAGE_OUT): $(TARGET_KERNEL_CONFIG) $(KERNEL_MAKE_DEPENDENCIES)
	$(hide) mkdir -p $(dir $@)
#ifdef OPLUS_ARCH_INJECT
	source kernel-4.19/oplus_native_features.sh ; \
	export OPPO_TARGET_DEVICE=$(OPPO_TARGET_DEVICE);\
	$(PREBUILT_MAKE_PREFIX)$(MAKE) -C $(PRIVATE_DIR) $(KERNEL_MAKE_OPTION)
#endif /* OPLUS_ARCH_INJECT */
	$(hide) $(call fixup-kernel-cmd-file,$(KERNEL_OUT)/arch/$(KERNEL_TARGET_ARCH)/boot/compressed/.piggy.xzkern.cmd)
	# check the kernel image size
	python device/mediatek/build/build/tools/check_kernel_size.py $(KERNEL_OUT) $(KERNEL_DIR) $(PROJECT_DTB_NAMES)

$(BUILT_KERNEL_TARGET): $(KERNEL_ZIMAGE_OUT) $(TARGET_KERNEL_CONFIG) $(LOCAL_PATH)/Android.mk | $(ACP)
	$(copy-file-to-target)

$(TARGET_PREBUILT_KERNEL): $(BUILT_KERNEL_TARGET) $(LOCAL_PATH)/Android.mk | $(ACP)
	$(copy-file-to-new-target)

endif #TARGET_PREBUILT_KERNEL is empty

$(INSTALLED_KERNEL_TARGET): $(BUILT_KERNEL_TARGET) $(LOCAL_PATH)/Android.mk | $(ACP)
	$(copy-file-to-target)

.PHONY: kernel save-kernel kernel-savedefconfig kernel-menuconfig menuconfig-kernel savedefconfig-kernel clean-kernel
kernel: $(INSTALLED_KERNEL_TARGET) $(INSTALLED_MTK_DTB_TARGET)
save-kernel: $(TARGET_PREBUILT_KERNEL)

kernel-savedefconfig: $(TARGET_KERNEL_CONFIG)
	cp $(TARGET_KERNEL_CONFIG) $(KERNEL_CONFIG_FILE)

kernel-menuconfig:
	$(hide) mkdir -p $(KERNEL_OUT)
#ifdef OPLUS_ARCH_INJECT
	source kernel-4.19/oplus_native_features.sh ; \
	export OPPO_TARGET_DEVICE=$(OPPO_TARGET_DEVICE);\
	$(MAKE) -C $(KERNEL_DIR) $(KERNEL_MAKE_OPTION) menuconfig
#endif /* OPLUS_ARCH_INJECT */

menuconfig-kernel savedefconfig-kernel:
	$(hide) mkdir -p $(KERNEL_OUT)
#ifdef OPLUS_ARCH_INJECT
	source kernel-4.19/oplus_native_features.sh ; \
	export OPPO_TARGET_DEVICE=$(OPPO_TARGET_DEVICE);\
	$(MAKE) -C $(KERNEL_DIR) $(KERNEL_MAKE_OPTION) $(patsubst %config-kernel,%config,$@)
#endif /* OPLUS_ARCH_INJECT */

#ifdef OPLUS_FEATURE_CHG_BASIC
#2021/4/1 Add for dynamic modify charger config.
ifneq ($(filter oppo6765_20361 oppo6765_20271, $(OPPO_TARGET_DEVICE)),)
$(shell sed -i 's/# CONFIG_CHARGER_SY697X is not set/CONFIG_CHARGER_SY697X=y/g' $(KERNEL_CONFIG_FILE))
$(shell sed -i 's/# CONFIG_CHARGER_SGM4151X is not set/CONFIG_CHARGER_SGM4151X=y/g' $(KERNEL_CONFIG_FILE))
else
$(shell sed -i 's/CONFIG_CHARGER_SY697X=y/# CONFIG_CHARGER_SY697X is not set/g' $(KERNEL_CONFIG_FILE))
$(shell sed -i 's/CONFIG_CHARGER_SGM4151X=y/# CONFIG_CHARGER_SGM4151X is not set/g' $(KERNEL_CONFIG_FILE))
endif
#endif /*OPLUS_FEATURE_CHG_BASIC*/

ifneq ($(filter oppo6765_20271 oppo6765_21281 oppo6762_21285, $(OPPO_TARGET_DEVICE)),)
$(shell sed -i 's/CONFIG_TCPC_CLASS=y/# CONFIG_TCPC_CLASS is not set/g' $(KERNEL_CONFIG_FILE))
$(shell sed -i 's/CONFIG_MTK_USB_TYPEC=y/# CONFIG_MTK_USB_TYPEC is not set/g' $(KERNEL_CONFIG_FILE))
$(shell sed -i 's/CONFIG_TCPC_RT1711H=y/# CONFIG_TCPC_RT1711H is not set/g' $(KERNEL_CONFIG_FILE))
endif

clean-kernel:
	$(hide) rm -rf $(KERNEL_OUT) $(INSTALLED_KERNEL_TARGET)

### DTB build template
MTK_DTBIMAGE_DTS := $(addsuffix .dts,$(addprefix $(KERNEL_DIR)/arch/$(KERNEL_TARGET_ARCH)/boot/dts/,$(PLATFORM_DTB_NAME)))
include device/mediatek/build/core/build_dtbimage.mk

CUSTOMER_DTBO_PROJECT := $(subst $\",,$(shell grep DTB_OVERLAY_IMAGE_NAMES $(KERNEL_CONFIG_FILE) | sed 's/.*=//' ))
MTK_DTBOIMAGE_DTS := $(addsuffix .dts,$(addprefix $(KERNEL_DIR)/arch/$(KERNEL_TARGET_ARCH)/boot/dts/,$(CUSTOMER_DTBO_PROJECT)))
include device/mediatek/build/core/build_dtboimage.mk

endif #TARGET_NO_KERNEL
endif #LINUX_KERNEL_VERSION
