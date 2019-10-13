#!/bin/bash

echo 0 | sudo tee /proc/sys/kernel/nmi_watchdog 
echo 0 | sudo tee /proc/sys/kernel/soft_watchdog 
echo 0 | sudo tee /proc/sys/kernel/softlockup_panic
