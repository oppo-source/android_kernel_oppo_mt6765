#!/bin/bash
if [ "$#" != "1" ]; then
  echo "usage: changeConfig.sh <config_path>"
  exit -1
fi
configFile=$1
if [ "$OBSOLETE_KEEP_ADB_SECURE" != "1" ] || \
   [ "$EUCLID_BUILD_INSECURE_KERNEL" == "1" ] || \
   [ "$TARGET_BUILD_VARIANT" != "user" ] ; then \
  sed -i '/CONFIG_SECURITY_SELINUX_DEVELOP.*/d' $configFile
  echo -e "CONFIG_SECURITY_SELINUX_DEVELOP=y\n" >> $configFile
else
  sed -i '/CONFIG_SECURITY_SELINUX_DEVELOP.*/d' $configFile
fi
echo  "===OBSOLETE_KEEP_ADB_SECURE = $OBSOLETE_KEEP_ADB_SECURE"
echo  "===EUCLID_BUILD_INSECURE_KERNEL = $EUCLID_BUILD_INSECURE_KERNEL"
echo  "===TARGET_BUILD_VARIANT = $TARGET_BUILD_VARIANT"

function setting_kernel_config()
{
	config=$1
	value=$2
	string=$3

	sed -i "/${config}.*/d" $configFile

	if [ $string -eq 0 ]
	then
		echo -e "${config}=${value}" >> $configFile
	else
		echo -e "${config}=\"${value}\"" >> $configFile
	fi
}


#ifdef OPLUS_BUG_DEBUG

function delete_kernel_config()
{
        echo  "configFile=== $configFile "
        config=$1
        sed -i "/${config}.*/d" $configFile
}



if [[ "$OPLUS_AGING_TYPE" == "KASAN" ]]
then
        #kasan
        setting_kernel_config CONFIG_RANDOMIZE_BASE n 0
        setting_kernel_config CONFIG_SLUB_DEBUG_ON y 0
        setting_kernel_config CONFIG_KASAN y 0
        setting_kernel_config CONFIG_KASAN_INLINE y 0
        setting_kernel_config CONFIG_FRAME_WARN 0 0

elif [[ "$OPLUS_AGING_TYPE" == "MEMLEAK" ]]
then
        #memleak
        delete_kernel_config CONFIG_SLUB_DEBUG
        delete_kernel_config CONFIG_KMALLOC_DEBUG
        delete_kernel_config CONFIG_VMALLOC_DEBUG
        delete_kernel_config CONFIG_DUMP_TASKS_MEM
        delete_kernel_config CONFIG_SVELTE
        setting_kernel_config CONFIG_RANDOMIZE_BASE n 0
        setting_kernel_config CONFIG_DEBUG_FS y 0
        setting_kernel_config CONFIG_SECURITY_SELINUX_DEVELOP y 0
        setting_kernel_config CONFIG_DEBUG_KMEMLEAK y 0
        setting_kernel_config CONFIG_HAVE_DEBUG_KMEMLEAK y 0
        setting_kernel_config CONFIG_DEBUG_KMEMLEAK_DEFAULT_OFF n 0
        setting_kernel_config CONFIG_DEBUG_KMEMLEAK_EARLY_LOG_SIZE 4000 0
fi


echo  "===OPLUS_AGING_TEST = $OPLUS_AGING_TEST ;OPLUS_AGING_TYPE = $OPLUS_AGING_TYPE"

#endif /* OPLUS_BUG_DEBUG */


if [[ "$OPLUS_AGING_TEST" == 1 ]] && [[ "$OPLUS_MEMLEAK_DETECT" != 0 ]]
then
	# enable memleak detect thread
	setting_kernel_config CONFIG_OPLUS_MEMLEAK_DETECT true 1
	setting_kernel_config CONFIG_MEMLEAK_DETECT_THREAD y 0
elif [[ "$OPLUS_MEMLEAK_DETECT" == 1 ]]
then
	# enable memleak detect thread
	setting_kernel_config CONFIG_OPLUS_MEMLEAK_DETECT true 1
	setting_kernel_config CONFIG_MEMLEAK_DETECT_THREAD y 0
elif [[ "$OPLUS_MEMLEAK_DETECT" == 2 ]]
then
	# enable memleak detect thread
	setting_kernel_config CONFIG_OPLUS_MEMLEAK_DETECT true 1
	setting_kernel_config CONFIG_MEMLEAK_DETECT_THREAD y 0

	# enable slub debug
	setting_kernel_config CONFIG_SLUB_DEBUG y 0
	setting_kernel_config CONFIG_SLUB_DEBUG_ON y 0
	setting_kernel_config CONFIG_SLUB_DEBUG_PANIC_ON y 0

	# enable kasan
	setting_kernel_config CONFIG_KASAN y 0
	setting_kernel_config CONFIG_KASAN_EXTRA y 0
	setting_kernel_config CONFIG_KASAN_OUTLINE y 0
fi

echo  "===OPLUS_MEMLEAK_DETECT = $OPLUS_MEMLEAK_DETECT"
