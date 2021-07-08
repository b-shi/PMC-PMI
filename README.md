# PMC-PMI

PMC-PMI is a linux kernel module which uses performance monitoring interrupts(PMI) to record performance counter events each cpu cycle. This is one method to observe some of the microarchitectural behaviours of the underlying hardware. The idea was inspired by the wonderfully written blog post https://gamozolabs.github.io/metrology/2019/08/19/sushi_roll.html. For a summary of the main ideas please read the the blog post.  

**DISCLAIMER**: This kernel module was primarily a learning exercise for me and has had very limited testing. It may contain bugs that I may not be aware of. **You are proceeding at your own risk when running it.**

## Initial Configuration
Before running the kernel it is recommended to do the following:
* Disable Hyper-Threading in BIOS or possibly via kernel boot parameter
* Disable kernel lockdown (one way is by disabling secure boot in BIOS)
* Using the kernel boot parameter "isolcpus" to isolate cpus you plan to run the module on
* Root access (physical access to machine would be good as well)

##  Building / Getting started
To build the kernel module run

```shell
git clone https://github.com/b-shi/PMC-PMI.git
cd PMC-PMI && make
```

In general to measure the PMC for a given code the steps consists of the following:

```shell
make
./run cpu_num
./parse pmc_pmi-data.log
python ./plot.py
```

##  Highlevel Description
I am assuming the reader has read and understands the main ideas in the [Sushi Roll Kernel blog post](https://gamozolabs.github.io/metrology/2019/08/19/sushi_roll.html). Instead of using PMI in a specialized kernel, I am hoping that using a LKM, sufficient cpu isolation, and large number of samples will result in a good enough approximation.

To track the PMC changes for a piece of code, the code is first added to a segment of the init function in the LKM and then the module is rebuilt. The test code is added in pmc_pmi.c, with the section marked in the comments, an example is shown below

```c
... Other Code
/*================================================================================================*/
/*=================================== CODE TO TEST STARTS HERE ===================================*/
/*================================================================================================*/
        
  __asm__ __volatile__(
    "add $0x8, %rsp \n\t"
    "sub $0x8, %rsp \n\t"
    );
    
/*================================================================================================*/
/*=================================== CODE TO TEST ENDS HERE =====================================*/
/*================================================================================================*/
... Other Code
```

The performance counter events can be set in the array PMC_EVENT, and the macro NUM_COUNTERS should be changed accordingly.

```c
...
static uint32_t PMC_EVENT[NUM_COUNTERS][4] = {
  INST_RETIRED_ANY_P,
  UOPS_ISSUED_ANY,
  ...
};
...
```
NUM_COUNTERS represents the number of counters that we would like to track, and can be greater than the number of counters supported by the cpu. In that case the counters are tracked in groups of size equal to the number of supported counters and will require 

```c
(NUM_COUNTERS/SUPPORTED_COUNTERS+1)*(NUM_CYCLES)*(TRIALS)
```
iterations. Refer to the comments in the src for more details. The perf events are located in the "defs_utils.h" header, a complete list can be found in Intel SDM Vol. 3b.  
 
When the module is loaded, the performance counters/interrupt are set up and measurements are taken and dumped to /proc/pmc_pmi-data. This file should not be accessed until the module loading has completed. Depending on the number of cycles and trials per cycle specified the module load may take a while. 

After the module load has completed, /proc/pmc_pmi-data can be copied to the current directory. This file is essentially a binary dump of the pmc counter values for a given trial and a given cycle. When the data is copied, the module is no longer needed and should be unloaded. Use the script "run.sh" to load the module, copy the data, and unload the module. For example

```shell
./run n
```
will pin the loading of the module to cpu n (essentially measuring the pmc data on cpu n). In this case, cpu n should be included in the isolcpus kernel boot parameter. The script also calls another script "disable_nmi_watchdog.sh", disabling various kernel watchdogs which affects PMC values or pollutes dmesg with warning messages.

To read the binary dump use the executable "parse". This utility compiles the statistics for each PMC at each cycle and outputs it in a table.

To view the assembly output of the code snippet tested use the script "extract_code.sh". Here is an example output for the test code used above.

```asm
 38d:	0f 30                	wrmsr  
 38f:	48 83 c4 08          	add    $0x8,%rsp
 393:	48 83 ec 08          	sub    $0x8,%rsp
 397:	0f ae f0             	mfence 
 39a:	0f ae e8             	lfence 
```
"wrmsr" will always be the first instruction since it is the one which enables the perf monitoring events.

### Using parse
"parse" is used to read data and collect statistics. Example usuage:

```shell
./parse pmc_pmi-data.log [optional number]
```
"optional number" can take on the following values.
* -1  :  Print the [min, second most frequent, most frequent , max] counter values for each event, each cycle.
* 0<= i <= NUM_COUNTERS-1  :  Print the [min, second most frequent, most frequent , max], and the counter value distribution for counter i for each cycle.
* NUM_COUNTERS  :  Print the [min, second most frequent, most frequent , max], and the counter value distribution for each counter, each cycle.
* NUM_COUNTERS+1  :  Print the [min, second most frequent, most frequent , max], and the counter value distribution for counters whose "second most frequent" * TOL > "most frequent" , each cycle. TOL is specified as a macro.
* NUM_COUNTERS+2  :  Print the [most frequent] for each counter, each cycle.

### Using plot.py
**Requires:** matplotlib

Reads the input file "stripped_for_plotting.txt" generated by the parser above and graphs the data. Optional parameters:
* -x: Maximum Number of Cycles to plot
* -y: Maximum y range in the graph
* -l: List of labels for the perf counters, comma delimited. (PMCi is used as label if nothing provided)

example:
```shell
python ./plot.py -x 100 -y 200 -l "INST_RETIRED,PORT 0, PORT 1"
```

## Examples
A few sample plots and associated test codes are included in the Examples directory. Examples were generated on a Intel Core M 5Y71 (Broadwell).

* test_code{1,3}.txt : Examples taken from Sushi Roll blog. See corresponding figures
* test_code2.txt : A look into FMA execution.
* test_code4.txt : Another load and store example, with data in L1.

##  Support/Limitations
* Intel CPU only
    * AMD CPU support probably not possible.
* APIC base register is hard coded for now. 
* The LKM tries to support performance monitoring arch 3,4 (and 5?)
    * These were the only machines I had access to.
* Tested on Ubuntu 18.XX and 21.04.
* Tested with following:
    * Broadwell
    * Coffee Lake
    * Ice Lake
* Expect code which makes accesses in L3/Memory to have very noisy data.
* Expect code with lots of branches to be have noisy data. 
    * Some arch msr can be used to reduce the noise, but not entirely.
    * Not sure how to completely address this, would be interested in any ideas.
* Unable to reproduce the speculative load example described in the Sushi Roll blog.
    * Don't have chips with TSX enabled..
* When plotting multiple counters, overlapping data points can make the graph hard to read. I don't have a great solution for this at the moment. One possibility is to make the marker size different but this can be a problem if a large number of counters is used. 

##  More WARNINGS
* LKM is **operating in ring 0**, be careful of what code you are measuring.
* Use a small number for the "TRIALS" macro to test that the LKM can load to completion. The LKM disables interrupts and preemption so its not likely that the load can be stopped mid way.
* The system may hang till module loading is completed if measuring lots of cycles/events/trials etc. Should be okay...
* You may need to hard reset if the LKM takes too long to load or crashes during load. 
    * You will not be able to reload the module if it crashes. A reboot is required.

##  Contributions
Suggestions, improvements, bug fixes, optimizations, new ideas, etc.. are all welcome. 

## Links
* Sushi Roll: https://gamozolabs.github.io/metrology/2019/08/19/sushi_roll.html
* Intel SDM: https://software.intel.com/en-us/articles/intel-sdm
* Agner Fog Optimization Manuals: https://www.agner.org/optimize/
* Good Microarchitecture and Microbenchmarking Articles: 
    * http://blog.stuffedcow.net/
    * https://xania.org/201602/bpu-part-one
* Awesome Q&A: https://stackoverflow.com/

## Licensing
GPL3
