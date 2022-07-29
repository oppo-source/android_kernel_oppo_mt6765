# SPDX-License-Identifier: GPL-2.0
# ifdef VENDOR_EDIT
$(warning "tee config----- CONFIG_TEE: $(CONFIG_TEE)")
$(warning "tee config----- CONFIG_TRUSTONIC_TEE_SUPPORT: $(CONFIG_TRUSTONIC_TEE_SUPPORT)")
$(warning "tee config----- CONFIG_TRUSTONIC_TEE_VERSION: $(CONFIG_TRUSTONIC_TEE_VERSION)")
# endif /*VENDOR_EDIT*/
obj-$(CONFIG_TEE) += tee.o
tee-objs += tee_core.o
tee-objs += tee_shm.o
tee-objs += tee_shm_pool.o
obj-$(CONFIG_OPTEE) += optee/
obj-$(CONFIG_TRUSTONIC_TEE_SUPPORT) += gud/
obj-$(CONFIG_MICROTRUST_TEE_SUPPORT) += teei/
obj-$(CONFIG_TRUSTKERNEL_TEE_SUPPORT) += tkcore/
