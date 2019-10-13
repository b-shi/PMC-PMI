ARCH_VERSION   := $(shell $(PWD)/cpuid_test | head -n1)
SUPP_COUNTERS  := $(shell $(PWD)/cpuid_test | tail -n1)
obj-m += pmc_pmi.o
CFLAGS_pmc_pmi.o := -O3 -fno-optimize-sibling-calls -march=native -mmmx -msse2 -Wno-declaration-after-statement -Wno-unused-variable -DPMC_ARCH_V$(ARCH_VERSION) -DSUPPORTED_COUNTERS=$(SUPP_COUNTERS)
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) CONFIG_STACK_VALIDATION= modules
	g++ -O3 log_read.c -o parse -Wno-format
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f parse *.log stripped_for_plotting.txt *~

