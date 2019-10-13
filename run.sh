#!/bin/bash

sudo ./disable_nmi_watchdog.sh > /dev/null
sudo dmesg -C
if [ "$1" != "" ]; then
    echo "Loading module.."
    sudo taskset -c $1 insmod pmc_pmi.ko
    sudo dmesg
else
    echo "Usage: ./run.sh proc_num"
fi
if [ -f "/proc/pmc_pmi-data" ]; then
    echo "Data dump found, copying to current directory.."
    sudo cp /proc/pmc_pmi-data ./pmc_pmi-data.log
    sudo chown $(logname):$(logname) ./pmc_pmi-data.log
fi
if [ "$1" != "" ]; then
    echo "Unloading module"
    sudo rmmod pmc_pmi.ko
fi

