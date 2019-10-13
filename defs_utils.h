#pragma once

/**************************************************************************************************
 **************************************************************************************************
 * * COUNTER EVENTS, UMASK, CMASK, and INV codes
 **************************************************************************************************
 ***************************************************************************************************/

/**
 * These are the event/umask/cmask/inv values
 * to specify the event to count
 * refer to Intel SDM Ch. 18
 *
 * The events, umask, cmask, and inv are stored as an array of ints.
 * { CMASK, [INV]1001110, UMASK, EVENT_SEL } -> [CMASK][0x42,0xC2][UMASK][EVENT_SEL]
 * For INV=1 set use 0xC2 instead of 0x42
 *
 * The events below are ones I have used.
 *
 * Note: Probably a better way of doing this but.....
 */

#define INST_RETIRED_ANY_P                                  {0x00, 0x42, 0x00, 0xc0}
#define MEM_LOAD_RETIRED_L1_HIT                             {0x00, 0x42, 0x01, 0xd1}
#define MEM_LOAD_RETIRED_L2_HIT                             {0x00, 0x42, 0x02, 0xd1}
#define MEM_LOAD_RETIRED_L3_HIT                             {0x00, 0x42, 0x04, 0xd1}
#define UOPS_ISSUED_ANY                                     {0x00, 0x42, 0x01, 0x0e}
#define UOPS_RETIRED_ALL                                    {0x00, 0x42, 0x01, 0xc2}
#define LDS_UOPS                                            {0x00, 0x42, 0x01, 0xa8}
#define IDQ_MITE_UOPS                                       {0x00, 0x42, 0x04, 0x79}
#define IDQ_DSB_UOPS                                        {0x00, 0x42, 0x08, 0x79}
#define UOPS_EXECUTED_CORE                                  {0x00, 0x42, 0x02, 0xb1}
#define UOPS_DISPATCHED_PORT_PORT_0                         {0x00, 0x42, 0x01, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_1                         {0x00, 0x42, 0x02, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_2                         {0x00, 0x42, 0x04, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_3                         {0x00, 0x42, 0x08, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_4                         {0x00, 0x42, 0x10, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_5                         {0x00, 0x42, 0x20, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_6                         {0x00, 0x42, 0x40, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_7                         {0x00, 0x42, 0x80, 0xa1}
#define FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE            {0x00, 0x42, 0x10, 0xc7}
#define FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE            {0x00, 0x42, 0x20, 0xc7}
#define MEM_INST_RETIRED_ALL_LOADS                          {0x00, 0x42, 0x81, 0xd0}
// Doesn't seem to work.
#define UOPS_EXECUTED_PORT015                               {0x00, 0x42, 0x40, 0xb1}
#define MACRO_INSTS_DECODED                                 {0x00, 0x42, 0x01, 0xaa}
#define MACRO_INSTS_CISC_DECODED                            {0x00, 0x42, 0x08, 0xaa}
#define BR_MISP_RETIRED_ALL_BRANCHES                        {0x00, 0x42, 0x04, 0xc5}
#define BR_MISP_RETIRED_NEAR_CALL                           {0x00, 0x42, 0x02, 0xc5}
#define BR_INST_RETIRED_ALL_BRANCHES                        {0x00, 0x42, 0x04, 0xc4}
#define BR_INST_RETIRED_NEAR_CALL                           {0x00, 0x42, 0x04, 0xc4}
#define BR_MISSP_EXEC                                       {0x00, 0x42, 0x00, 0x89}
#define BR_RET_EXEC                                         {0x00, 0x42, 0x00, 0x8f}
#define BOGUS_BR                                            {0x00, 0x42, 0x00, 0xe4}
#define BTB_MISSES                                          {0x00, 0x42, 0x01, 0xe2}

/* For older archs? .. */
#define BPU_CLEARS_EARLY                                    {0x00, 0x42, 0x01, 0xe8}
#define BPU_CLEARS_Late                                     {0x00, 0x42, 0x02, 0xe8}
#define INT_MISC_RECOVERY_CYCLES                            {0x00, 0x42, 0x01, 0x0d}
#define INT_MISC_CLEAR_RESTEER_CYCLES                       {0x00, 0x42, 0x80, 0x0d}
#define BACLEARS_ANY                                        {0x00, 0x42, 0x01, 0xe6}
/* Count # of mispredicted RET instructions */
#define BR_RET_MISSP_EXEC                                   {0x00, 0x42, 0x00, 0x90}
// Skylake, Kaby Lake, Coffee Lake only?
#define ICACHE_64B_IFTAG_HIT                                {0x00, 0x42, 0x01, 0x83}
#define ICACHE_64B_IFTAG_MISS                               {0x00, 0x42, 0x02, 0x83}


//Additionally set MSR_PEBS_FRONTEND.EVTSEL=12H.
#define FRONTEND_RETIRED_L1I_MISS                           {0x00, 0x42, 0x01, 0xc6}
//Additionally set MSR_PEBS_FRONTEND.EVTSEL=13H.
#define FRONTEND_RETIRED_L2I_MISS                           {0x00, 0x42, 0x01, 0xc6}

#define NOT_USED

/**************************************************************************************************
 **************************************************************************************************
 * * Setup PMC and PMI
 **************************************************************************************************
 **************************************************************************************************
 */

/**
 * Performance Counters Selector for %ecx in wrmsr
 * Specify the base MSR address
 * This is where event/umask selection is written to
 */
#define PERFEVTSELX    0x186

/**
 * MSR Performance Counter for the above selector
 * This is where the counters values are read from.
 */
#define PMCX    0xc1

#define IA32_FIXED_CTR0_ADDR 0x309
#define IA32_FIXED_CTR1_ADDR 0x30A

/**
 * The following looks to be the same for all archs??
 * IA32_DEBUGCTL, IA32_DEBUGCTL_ADDR
 * IA32_FIXED_CTR_CTRL, IA32_FIXED_CTR_CTRL_ADDR
 * LVT_PERFCON, LVT_PERFCON_ADDR
 */

/**
 * Enable PMC freeze on PMI: Set bit 12 to 1.
 */
#define IA32_DEBUGCTL 0x001000
#define IA32_DEBUGCTL_ADDR 0x01D9

/**
 * Enable PMI for all fixed counters, all threads
 */
#define IA32_FIXED_CTR_CTRL  0x3B3 // Enable PMI on overflow only for ctr1
#define IA32_FIXED_CTR_CTRL_NO_PMI 0x333 // F = [1|1|11]
#define IA32_FIXED_CTR_CTRL_ADDR 0x038D


/**
 * Set LVT Values to use NMI Handler
 * System must be in x2APIC mode
 * See "x2APIC Register Address Space" in Intel SDM
 * LVT_PERFCON = 0x00[4]EE = 0X00[0|100]EE = 0x00[Idle|NMI]EE
 */
#define LVT_PERFCON 0x004EE

// LVT Performance Monitoring MSR Register addr
#define LVT_PERFCON_ADDR 0x0834

// LVT Performance Monitoring memory mapped addr
#define LVT_PERFCON_MM_ADDR 0xFEE00340


//===================================================================================================
// Set up for different PMC ARCH Versions
//===================================================================================================

#define IA32_PERF_GLOBAL_CTRL_ADDR                                      0x038F
#define IA32_PERF_GLOBAL_STATUS_ADDR                                    0x038E

#if defined(PMC_ARCH_V3)

#define IA32_PERF_GLOBAL_OVF_RESET_ADDR                                 0x0390

#elif defined(PMC_ARCH_V4)

#define IA32_PERF_GLOBAL_STATUS_SET_ADDR                                0x0391


/**
 * IA32_PERF_GLOBAL_STATUS_RESET_LOW
 * will be 0xff or 0xf depending on the number
 * of supported counters.
 */
#define IA32_PERF_GLOBAL_STATUS_RESET_ADDR                              0x0390
#define IA32_PERF_GLOBAL_STATUS_RESET_HIGH                              0x08000003

/**
 * These need to be set for certain events.
 */
#define MSR_PEBS_FRONTEND_ADDR                                          0x03F7
/**
 * [63:23 - Reserved | 22:20 - IDQ_Bubble_Width |
 *  19:8 - IDQ_Bubble_Length | 7:0 - EVTSEL ]
 */
#define MSR_PEBS_FRONTEND_VAL                                           0x012
#endif

/**
 * Handler needs to reset the counter values so they update again.
 * Counters are disabled first so after the interrupt the values
 * can be stored. They will be enabled in the main loop.
 *
 * For counters to work, they must be enabled AND reset.
 * For the interrupt to work again, it must be acknowledged.
 */
static int  my_nmi_handler(unsigned int cmd, struct pt_regs *regs){


// Disable all counters.
  __asm__ __volatile__ ("wrmsr" : : "c"(IA32_PERF_GLOBAL_CTRL_ADDR), "a"(0x00), "d"(0x00));

#if defined(PMC_ARCH_V3)
  uint32_t mask_high, mask_low;
  __asm__ __volatile__ ("rdmsr" : "=a"(mask_low), "=d"(mask_high) : "c"(IA32_PERF_GLOBAL_STATUS_ADDR));
  // Reset overflow status.
  __asm__ __volatile__ ("wrmsr" : : "c"(IA32_PERF_GLOBAL_OVF_RESET_ADDR), "a"(mask_low), "d"(0x00));
#elif defined(PMC_ARCH_V4)
  // Reset overflow status.
#if SUPPORTED_COUNTERS == 8
  __asm__ __volatile__ ("wrmsr" : : "c"(IA32_PERF_GLOBAL_STATUS_RESET_ADDR),
                        "a"(0x0ff),
                        "d"(IA32_PERF_GLOBAL_STATUS_RESET_HIGH));
#elif SUPPORTED_COUNTERS == 4
  __asm__ __volatile__ ("wrmsr" : : "c"(IA32_PERF_GLOBAL_STATUS_RESET_ADDR),
                        "a"(0x0f),
                        "d"(IA32_PERF_GLOBAL_STATUS_RESET_HIGH));
#endif
#endif
  /**
   * LVT_PERFCON bits are set here, need to reset. This allows
   * the NMI to occur again next time
   */

  /*Ack*/
#ifdef X2APIC
  __asm__ __volatile__ ("wrmsr" : : "c"(LVT_PERFCON_ADDR), "a"(LVT_PERFCON), "d"(0x00));
#else
  // TODO
#endif
  return NMI_HANDLED;
}

/**************************************************************************************************
 **************************************************************************************************
 * * PROCFS related things
 **************************************************************************************************
 **************************************************************************************************
 */

#define PROCFS_NAME          "pmc_pmi-data"
#define PROCFS_MAX_SIZE ((TRIALS)*(NUM_CYCLES + MAX_PMC_SHIFT)*(NUM_COUNTERS) +  (EXTRA_HEADER_SPACE) )*(sizeof(int64_t))


/**
 * This structure holds information about the /proc file
 */
static struct proc_dir_entry *ent;

/**
 * Structure containing logged values.
 */
static int64_t *data_log = NULL;


/**
 * Will and should never be used.
 */
static ssize_t mywrite(struct file *file,
                       const char __user *ubuf,
                       size_t count, loff_t *ppos) {
  return -1;
}

static ssize_t myread(struct file *file,
                      char __user *ubuf,
                      size_t len, loff_t *offp){
  if(data_log == NULL) return -1;
  size_t count = len, log_len = PROCFS_MAX_SIZE;
  ssize_t retval = 0;
  uint64_t ret = 0;

  if (*offp >= log_len)
    goto out;
  if (*offp + len > log_len)
    count = log_len - *offp;

  ret = copy_to_user(ubuf, (const char*)data_log + *offp, count);
  *offp += count - ret;
  retval = count - ret;

out:
  return retval;
}

static struct file_operations myops = {
  .owner = THIS_MODULE,
  .read = myread,
  .write = mywrite,
};


/**************************************************************************************************
 **************************************************************************************************
 * * Useful Functions and Macros
 **************************************************************************************************
 **************************************************************************************************
 */

/**
 * Disables Hardware prefetch (Sandy-bridge..)
 * See Intel SDM vol. 4
 */
#define MSR_MISC_FEATURE_CONTROL_ADDR     0x1A4

/* L1D flush, wb and inval*/
#define IA32_FLUSH_CMD_ADDR               0x10B

/* MSR Registers for BPU control */
#define IA32_SPEC_CTRL_ADDR               0x48
#define IA32_PRED_CMD_ADDR                0x49

/* Disables L2 prefetch after current line*/
#define PREFETCH_DISABLE_L2_CUR           0x01
/* Disables L2 prefetch to adjacent line*/
#define PREFETCH_DISABLE_L2_ADJ           0x02
/* Disables all L2 prefetch*/
#define PREFETCH_DISABLE_L2               0x03
/* Disables L1 Data and Inst prefetch*/
#define PREFETCH_DISABLE_L1               0x0C
/* Disables L1 Data or Inst prefetch resp.*/
#define PREFETCH_DISABLE_LD1              0x04
#define PREFETCH_DISABLE_LI1              0x08
// Disables prefetched to L2 and L1.
#define PREFETCH_DISABLE_L1L2             0x0F
#define PREFETCH_ENABLE                   0x00

static inline void __attribute__((always_inline)) prefetch_control(unsigned val){
  __asm__ __volatile__ ("wrmsr" : : "c"(MSR_MISC_FEATURE_CONTROL_ADDR), "a"(val), "d"(0x00));
}

/**
 * Write back all entries in L1D, and invalidate.
 * Does not work on some arch...
 */
static inline void __attribute__((always_inline)) flush_l1d(void){
  __asm__ __volatile__ ("wrmsr" : : "c"(IA32_FLUSH_CMD_ADDR), "a"(0x1), "d"(0x00));
}

/**
 * This invalidates the cacheline containing addr m.
 * Looks like already included in lnx kernel.
 */
/*static inline void __attribute__((always_inline)) clflush(void *m){
  asm volatile ("clflush (%0)" :: "b"(m): "memory");
  }
*/

/**
 * This invalidates the page table entry correponding to address m
 */
static inline void __attribute__((always_inline))invlpg(void* m){
  __asm__ __volatile__ ( "invlpg (%0)" : : "b"(m) : "memory" );
}

/**
 * Inserts sequence of no-ops to fully occupy ROB
 */
static inline void __attribute__((always_inline)) lots_of_nops(void){
  __asm__ __volatile__ (
    ".align 64\n\t"
    ".rept 256 \n\t"
    "nop\n\t"
    ".endr\n\t"
    );
}

/***********************************************************************
 * This attempts to force the eviction of a cache line in set = set_no
 * Assumes 64B cache line, 64 sets, 8-way associative.
 *
 * For a given address in set = set_no, calling this macro
 * right after the address is loaded into l1-D should evict
 * the cache line containing that address. Assumming LRU-like policy
 *
 * Note: Only tested/working on coffeelake
 */
#define clcache_l1(scratch_space, set_no)                               \
  do{                                                                   \
    scratch_space[4096 + (set_no)] = i;                                 \
    scratch_space[2*4096 + (set_no)] = scratch_space[3*4096 + (set_no)] = i; \
    scratch_space[4*4096 + (set_no)] = scratch_space[5*4096 + (set_no)] = i; \
    scratch_space[6*4096 + (set_no)] = scratch_space[7*4096 + (set_no)] = i; \
    scratch_space[8*4096 + (set_no)] = scratch_space[9*4096 + (set_no)] = i; \
  } while(0)


/**************************************************************
 * Generates ALOT of branches to hopefully evict
 * current BTB entries. Still not sure if this works..
 *
 * Amount of branches needed varies depending on
 * arch, model....
 *
 * Note: Very costly, adjust TRIALS accordingly..
 */
static inline void __attribute__((always_inline)) BPU_fill(void){
  int32_t i;
  for(i = 0; i < 16; i++){
    __asm__ __volatile__ (
      ".align   4096             \n\t"
      "mov      $0x1,    %%al    \n\t"
      /* Number of branches to create */
      ".rept    16384            \n\t"
      "test     %%al,    %%al    \n\t"
      ".intel_syntax             \n\t"
      ".align   2                \n\t"
      "jne      short    $+2     \n\t"
      "jne      short    $+2     \n\t"
      ".att_syntax               \n\t"
      ".endr                     \n\t"
      :::"%al"
      );
  }
  return;
}
/** 
 * Similar version to above, but the spacing between jumps can 
 * be varied.
 */
static inline void __attribute__((always_inline)) BPU_fill_V2(void){
  int32_t i;
  for(i = 0; i < 10; i++){
    __asm__ __volatile__ (
      ".align   4096             \n\t"
      "mov      $0x1,    %%al    \n\t"
      /* Number of branches to create */
      ".rept    12288            \n\t"
      "test     %%al,    %%al    \n\t"
      ".intel_syntax             \n\t"
      /* Distance to target address
       * The jne distance should always
       * be 2 greater than the rept value
       * below. i.e
       *
       * jXX short $+(n+2)
       * .rept n
       * .......
       * .......
       * jXX short $+(n+2)
       * .rept n
       */
      ".align   2                \n\t"
      "jne      short    $+2     \n\t"
      ".rept    0                \n\t"
      "nop                       \n\t"
      ".endr                     \n\t"
      "jmp      short    $+2     \n\t"
      ".rept    0                \n\t"
      "nop                       \n\t"
      ".endr                     \n\t"
      ".att_syntax               \n\t"
      ".endr                     \n\t"
      :::"%al"
      );
  }
  return;
}
