#!/bin/bash

if [ -f "./pmc_pmi.ko" ]; then
    lfence_line=$(objdump -d pmc_pmi.ko | grep -n lfence | tail -n1 | sed -e "s/:.*//g")
    wrmsr_line=$(objdump -d pmc_pmi.ko | grep -n wrmsr | sed -e "s/:.*//g" | awk '$1 <'"$lfence_line"' { print }' | tail -n1)
    objdump -d pmc_pmi.ko | head -n $(($lfence_line)) | tail -n $(($lfence_line-$wrmsr_line + 1))
else
    echo "pmc_pmi module not built. Exiting..."
fi
