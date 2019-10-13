#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <asm/nmi.h>
#include <asm/fpu/api.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("B.S.");
MODULE_DESCRIPTION("PMI Implementation");
MODULE_VERSION("0.01");

// Comment or un-comment depending on whether x2apic is enabled.
#define X2APIC

/**
  * This value should be greater equal to maxium value of
  * PMC_SHIFT. Needed in "defs_utils.h"
  */
#define MAX_PMC_SHIFT        20

/* Number of trials to run per cycle */
#define TRIALS               1024LL
/* Number of cycles to measure data */
#define NUM_CYCLES           200LL

/**
 * End trials if code being tested ends
 * before NUM_CYCLES has been reached.
 * Comment out if data upto NUM_CYCLES is needed
 */
#define QUICK_EXIT

#define NUM_COUNTERS         2 //Should match num events in PMC_EVENT
#define EXTRA_HEADER_SPACE   (4 + (NUM_COUNTERS/2) + 1)

/* Contains PMC values and useful functions*/
#include "defs_utils.h"

/**
 * Macros defined in header above, just add to list.
 */
static uint32_t PMC_EVENT[NUM_COUNTERS][4] = {
  INST_RETIRED_ANY_P,
  UOPS_ISSUED_ANY
};

/**
 * Combines all event,umask,cmask,inv values into a
 * single 32bit value for each counter.
 */
static uint32_t PMCX_EVENTS[NUM_COUNTERS];

/**
 * PERFEVTSEL: addr where wrmsr writes to.
 * PMC: addr where rdmsr reads data from.
 */
static uint32_t PERFEVTSEL[SUPPORTED_COUNTERS];
static uint32_t PMC[SUPPORTED_COUNTERS];

/**
 * Some of the pmc do not count do not refer to the
 * current cycle. This is to compensate for that by
 * shifting the pmc count at cycle=I to
 * cycle=I + PMC_SHIFTS[].
 *
 * One way to check for required shifts is to use
 * multiple counters with the same event and
 * make sure their values align each cycle.
 *
 * Note: These values are probably very
 * dependendent on the hw config. Size of array should
 * be at least min(NUM_COUNTERS,SUPPORTED_COUNTERS)
 *
 */
//static uint32_t PMC_SHIFTS[] = {0,6,0,0}; // Broadwell
//static uint32_t PMC_SHIFTS[] = {0,8,0,0}; // Coffee Lake
static uint32_t PMC_SHIFTS[] = {0,0,0,0}; // Ice Lake

/**
 * Offset the pmc counter values. Use similar method as above
 * to determine offsets to that all aligned per cycle.
 *
 * Note: These values are probably very
 * dependendent on the counters/events used...
 */
static uint32_t PMC_OFFSETS[NUM_COUNTERS] = {0};

static char *scratch_space = NULL;

static int __init pmc_pmi_init(void) {

#ifdef X2APIC
  /* If using X2APIC we need to make sure its enabled first.. */
  int32_t apic_check_low, apic_check_high;
  __asm__ __volatile__ ("rdmsr" : "=a"(apic_check_low), "=d"(apic_check_high) : "c"(0x1b));

  if( !(apic_check_low & 0x0400) ) {
    /**
     * We need x2apic to be enabled (bit 10 of msr addr 0x1B)
     * This is needed so we can write to LVT_PMI msr 0x834
     *
     * Note: Enabling it here causes my machine to hang.
     */
    printk(KERN_INFO "x2apic not enabled, enable and try again. Exiting...\n");
    return 0;
  }
#endif

  /* Create the /proc/ file entry where all the counter data is dumped */
  ent = proc_create(PROCFS_NAME,0660,NULL,&myops);

  int cpu_id = get_cpu();
  int i,j,k,l;
  unsigned long flags;
  /* Collecting counter data in groups of pmc_per_it*/
  int pmc_per_it = (SUPPORTED_COUNTERS < NUM_COUNTERS ) ? SUPPORTED_COUNTERS : NUM_COUNTERS;

  /**
   * Init all counter addresses, event selector and temp variables.
   */
  for(i = 0; i < pmc_per_it; i++){
    PERFEVTSEL[i] = PERFEVTSELX + i;
    PMC[i] = PMCX + i;
  }

  for(i = 0; i < NUM_COUNTERS; i++){
    PMCX_EVENTS[i] =
      // CMASK | 0x42 | UMASK | EVENT
      (PMC_EVENT[i][0]<<24)|(PMC_EVENT[i][1]<<16)|(PMC_EVENT[i][2]<<8)|(PMC_EVENT[i][3]);
  }

  uint32_t rdmsr_lohi[SUPPORTED_COUNTERS][2]; // {low, high}
  int64_t count_res[SUPPORTED_COUNTERS];

  uint32_t test_low,test_high;
  int64_t count_test;

  /**
   * Setting up array to store all collected data.
   * This will be dumped to user via copying the /proc file
   */
  data_log = (int64_t*)vmalloc(PROCFS_MAX_SIZE);
  if(!data_log){
    printk(KERN_INFO
           "Allocation Error when trying to allocate %lld bytes.. exiting.\n", PROCFS_MAX_SIZE);
    return 0;
  }
  int64_t *data_ptr = data_log + EXTRA_HEADER_SPACE;
  uint32_t *pmc_info = (uint32_t*)(data_log + 4);
  int64_t header_size = EXTRA_HEADER_SPACE;

  /* Header for the log file with important info. */
  data_log[0] = NUM_CYCLES;
  data_log[1] = TRIALS;
  data_log[2] = ( header_size << 32) | (NUM_COUNTERS);
  data_log[3] = 0xffffffffffffffff;

  /* Store which counters were used in log file */
  for( i = 0; i < 2*(header_size - 4); i++){
    pmc_info[i] = i < NUM_COUNTERS ? PMCX_EVENTS[i] : 0xdeadbeef;
  }

  /* Init all data values to -123456789 */
  for( i = 0; i < (NUM_CYCLES + MAX_PMC_SHIFT)*TRIALS*NUM_COUNTERS; i++){
    // Hopefully no logged counters get this value...
    data_ptr[i] = -123456789;
  }


  /**
   * Allocating 8MB scratch space, probably more than needed.
   * Write to this array to evict caches..
   * Looks like the addr are already page aligned.
   */
  scratch_space = (char*)vmalloc(65536*128*sizeof(char));

  /******************************************************************
   ******************************************************************
   *   Additional variables which can be used in code to be tested..
   *   Misc variables, if needed...
   ******************************************************************
   ******************************************************************/
  volatile int32_t *list_ptr = (int32_t*)kmalloc(sizeof(int32_t)*13000, GFP_USER);
  volatile uint64_t *array_ptr = (uint64_t *)list_ptr;
  volatile int64_t x_test = 1, y_test = 0, z_test = 0;
  volatile double doubles[4];
  volatile int32_t var1;

  /* Used to set fixed counter for overflowing.. */
  uint32_t cycles_low, cycles_high;

  __asm__ __volatile__ ("wrmsr" : : "c"(IA32_DEBUGCTL_ADDR), "a"(IA32_DEBUGCTL), "d"(0x00));

  /**
   * Enable the NMI interrupt to occur and register the interrupt handler
   */
#ifdef X2APIC
  __asm__ __volatile__ ("wrmsr" : : "c"(LVT_PERFCON_ADDR), "a"(LVT_PERFCON), "d"(0x00));
#else
  /**
   * TODO: Need to write directrly to MMIO VA FFEOXXXX, APIC addr, to
   * enable PMI.
   */
  LVT_PERFCON_VA_ADDR = ioremap(LVT_PERFCON_MM_ADDR, 8);
  *((volatile uint32_t*)(LVT_PERFCON_VA_ADDR)) = LVT_PERFCON;

#endif
  register_nmi_handler(NMI_LOCAL, my_nmi_handler, 0, "pmi_handler");

/***********************************************************************
 ***********************************************************************
 ** Done with all initializing, this is the main loop
 ***********************************************************************
 ************************************************************************/
  for(l = 0; l < NUM_COUNTERS; l += pmc_per_it){

    /**
     * If NUM_COUNTERS > SUPPORTED_COUNTERS, we do multiple iterations
     * in groups of SUPPORTED_COUNTERS.
     */
    pmc_per_it = (SUPPORTED_COUNTERS < NUM_COUNTERS - l ) ? SUPPORTED_COUNTERS : NUM_COUNTERS - l;

    /* Program each performance counter with the specified selector*/
    for(k = 0; k < pmc_per_it; k++){
      __asm__ __volatile__ ("wrmsr" : : "c"(PERFEVTSEL[k]), "a"(PMCX_EVENTS[k + l]), "d"(0x00));

#if defined(PMC_ARCH_V4) || defined(PMC_ARCH_V5)
      if(PMCX_EVENTS[k + l] == 0x004201c6) {
        // These are for the PEBS_FRONTEND events
        __asm__ __volatile__ ("wrmsr" : : "c"(MSR_PEBS_FRONTEND_ADDR), "a"(MSR_PEBS_FRONTEND_VAL), "d"(0x00));
      }
#endif
    }

    for(j = 1; j < NUM_CYCLES; j++){

      /* Set to overflow in j cycles */
      cycles_low = -(j);
      cycles_high = 0x0FFFF;

      /**
       * WBINVD here does not impact the performance as much
       * looks to be a good balance at least for lower number
       * of cycles..
       * Maybe not even needed??
       */
      __asm__ __volatile__ ("wbinvd");
      for(i = 0; i < TRIALS; i++){

        /* Disable context switching, interrupts and save fp stack*/
        preempt_disable();
        raw_local_irq_save(flags);
        kernel_fpu_begin();

        /* Disable all counters so their values can be preset*/
        __asm__ __volatile__ ("wrmsr" : : "c"(IA32_PERF_GLOBAL_CTRL_ADDR), "a"(0x00), "d"(0x00));

        /* Enable PMI on overflow for fixed counter 1 (cycles)*/
        __asm__ __volatile__ ("wrmsr" : : "c"(IA32_FIXED_CTR_CTRL_ADDR), "a"(IA32_FIXED_CTR_CTRL), "d"(0x00));

        /* Set up fixed cycle value to overflow */
        __asm__ __volatile__ ("wrmsr" : : "c"(IA32_FIXED_CTR1_ADDR), "a"(cycles_low), "d"(cycles_high));

        /* Write zero values to each performance counter prepare for counting */
#pragma GCC unroll 32
        for(k = 0; k < pmc_per_it; k++){
          __asm__ __volatile__ ("wrmsr" : : "c"(PMC[k]), "a"(0x00), "d"(0x00));
        }

        /*******************************************************************
         * Add stuff here to try to flush the
         * pipeline, and reduce noise...
         * Some may not be needed. These were got through
         * experimentation.
         *******************************************************************/

        /* Disables hardware prefetching */
        //prefetch_control(PREFETCH_DISABLE_L1L2);

        /* Single Thread Indirect Branch Predictors */
        __asm__ __volatile__ ("wrmsr" : : "c"(IA32_SPEC_CTRL_ADDR), "a"(0x02), "d"(0x00));

        /* Indirect Branch Predictor Barrier (IBPB) */
        __asm__ __volatile__ ("wrmsr" : : "c"(IA32_PRED_CMD_ADDR), "a"(0x01), "d"(0x00));

        /* Flush L1 Data Cache (May not be supported on older arch) */
        //flush_l1d();

        /* Insert lots of nops to ROB */
        lots_of_nops();


        /**
         * Clear all caches:
         * Use only if needed. HUGE slowdown if used here.
         * Should lower NUM_TRIALS accordingly...
         * Other options: invlpg or clflush
         */
        //__asm__ __volatile__ ("wbinvd");

        /**
         * Other stuff to do before enabling PCM counters...
         * These are testing code specific stuff, i.e set up
         * variables, bring data to cache, flush data from cache
         */
/*================================================================================================*/
/*================================================================================================*/
        //clcache_l1(scratch_space,0);
        //clflush(list_ptr);
        //clflush(&list_ptr);

/*================================================================================================*/
/*================================================================================================*/

        /**
         * Enable all PMC counters
         * First instruction retired in collected data should be wrmsr.
         */
#if SUPPORTED_COUNTERS == 8
        __asm__ __volatile__ ("wrmsr" : : "c"(IA32_PERF_GLOBAL_CTRL_ADDR), "a"(0x0ff), "d"(0x07));
#elif SUPPORTED_COUNTERS == 4
        __asm__ __volatile__ ("wrmsr" : : "c"(IA32_PERF_GLOBAL_CTRL_ADDR), "a"(0x0f), "d"(0x07));
#endif
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
        /**
         * Prevent instructions from above to spill over, and
         * prevent rdmsr insts to influence the measurements.
         * Also provides a good marker when looking at disassembly :)
         */
        __asm__ __volatile__ ("mfence");
        __asm__ __volatile__ ("lfence");

        /*
         * Collect counter information, values should be
         * frozen globally and counter should be disabled.
         */
#pragma GCC unroll 32
        for(k = 0; k < pmc_per_it; k++){
          __asm__ __volatile__ ("rdmsr" : "=a"(rdmsr_lohi[k][0]), "=d"(rdmsr_lohi[k][1]) : "c"(PMC[k]));
        }

        /**
         * Make sure all the rdmsr completes before reading the pmc0 counter again.
         * If the values change from the first one, it means that the interrupt occured
         * after the code to test (NUM_CYCLES is too large).
         */
        __asm__ __volatile__ ("mfence");
        __asm__ __volatile__ ("rdmsr" : "=a"(test_low), "=d"(test_high) : "c"(PMC[0]));

        /**
         * Disable PMI, at this point.
         */
        __asm__ __volatile__ ("wrmsr" : : "c"(IA32_FIXED_CTR_CTRL_ADDR), "a"(IA32_FIXED_CTR_CTRL_NO_PMI), "d"(0x00));

        prefetch_control(PREFETCH_ENABLE);

        /* Enable everything that was disabled.. */
        kernel_fpu_end();
        raw_local_irq_restore(flags);
        preempt_enable();

        /* Combine high and low bits and save pmc values */
        for(k = 0; k < pmc_per_it; k++){
          count_res[k] = ((int64_t)rdmsr_lohi[k][0] | (int64_t)rdmsr_lohi[k][1] << 32);
        }

        count_test = ((int64_t)test_low | (int64_t)test_high<<32);

        /**
         * Check that we are not measuring code that is beyond what we want tested.
         * There is probably a cleaner way to check, but this is good enough.
         */
#if defined(QUICK_EXIT)
        if( count_test != count_res[0] ){
          if( j != NUM_CYCLES) data_log[0] = (j - 1) < data_log[0] ? j - 1 : data_log[0];
          i = TRIALS;
          j = NUM_CYCLES;
        }
        else
#endif
          for(k = 0; k < pmc_per_it; k++){
            /*Make sure we are not writing past the array... just being more cautious.*/
            if(NUM_COUNTERS*TRIALS*( j + PMC_SHIFTS[k] )+NUM_COUNTERS*i + (k + l)
               < (PROCFS_MAX_SIZE - EXTRA_HEADER_SPACE)/sizeof(int64_t)){
              data_ptr[NUM_COUNTERS*TRIALS*( j + PMC_SHIFTS[k] )+NUM_COUNTERS*i + (k + l)]
                = count_res[k] - PMC_OFFSETS[k + l];
              if( PMC_SHIFTS[k] > 0 && j <= PMC_SHIFTS[k] )
                data_ptr[NUM_COUNTERS*TRIALS*j+NUM_COUNTERS*i + (k + l)] = -1;
            }
            else
              printk(KERN_INFO "Potential out of bounds detected..\n");
          }

      }
    }
  }

  kfree((const void *)list_ptr);
  return 0;
}

static void __exit pmc_pmi_exit(void) {
  unregister_nmi_handler(NMI_LOCAL, "pmi_handler");
  vfree(data_log);
  vfree(scratch_space);
  proc_remove(ent);
  printk(KERN_INFO "Goodbye, World!\n");
}

module_init(pmc_pmi_init);
module_exit(pmc_pmi_exit);
