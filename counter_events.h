/**************************************************************************************************
 **************************************************************************************************
 * * COUNTER EVENTS, UMASK, CMASK, and INV codes
 **************************************************************************************************
 ***************************************************************************************************/

/***************************************************************************************************
 ***************************************************************************************************
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
 ***************************************************************************************************
 ***************************************************************************************************
 */


// Counting covers macro-fused instructions individually (that is, increments by two).
#define INST_RETIRED_ANY_P                                  {0x00, 0x42, 0x00, 0xc0} 

//Only available on IA32_PMC1.
#define INST_RETIRED_ANY_P_PREC_DIST                        {0x00, 0x42, 0x01, 0xc0} 
#define INST_RETIRED_ANY                                    {0x00, 0x42, 0x01, 0x00}

#define UOPS_ISSUED_ANY                                     {0x00, 0x42, 0x01, 0x0e}
#define UOPS_RETIRED_ALL                                    {0x00, 0x42, 0x01, 0xc2}
#define UOPS_RETIRED_RETIRE_SLOTS                           {0x00, 0x42, 0x02, 0xc2}
#define UOPS_ISSUED_STALL_CYCLES                            {0x00, 0x42, 0x01, 0x0e} //CounterMask=1, Invert=1
#define RS_EVENTS_EMPTY_CYCLES                              {0x01, 0x42, 0x01, 0x5e}
#define RS_EVENTS_EMPTY_END                                 {0x01, 0x42, 0x01, 0x5e} //CounterMask=1, Invert=1, EdgeDetect=1

#define UOPS_DISPATCHED_PORT_PORT_0                         {0x00, 0x42, 0x01, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_1                         {0x00, 0x42, 0x02, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_2                         {0x00, 0x42, 0x04, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_2_3                       {0x00, 0x42, 0x04, 0xa1} // Ice Lake
#define UOPS_DISPATCHED_PORT_PORT_3                         {0x00, 0x42, 0x08, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_4                         {0x00, 0x42, 0x10, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_4_9                       {0x00, 0x42, 0x10, 0xa1} // Ice Lake
#define UOPS_DISPATCHED_PORT_PORT_5                         {0x00, 0x42, 0x20, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_6                         {0x00, 0x42, 0x40, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_7                         {0x00, 0x42, 0x80, 0xa1}
#define UOPS_DISPATCHED_PORT_PORT_7_8                       {0x00, 0x42, 0x80, 0xa1} // Ice Lake

#define UOPS_EXECUTED_CORE                                  {0x00, 0x42, 0x02, 0xb1}
#define RESOURCE_STALLS_SCOREBOARD                          {0x00, 0x42, 0x02, 0xa2} // Ice Lake

#define EXE_ACTIVITY_EXE_BOUND_0_PORTS                      {0x00, 0x42, 0x01, 0xa6}

#define MEM_UOPS_RETIRED_STLB_MISS_LOADS                    {0x00, 0x42, 0x11, 0xd0}
#define MEM_UOPS_RETIRED_STLB_MISS_STORE                    {0x00, 0x42, 0x12, 0xd0}
#define MEM_UOPS_RETIRED_LOCK_LOADS                         {0x00, 0x42, 0x21, 0xd0}
#define MEM_UOPS_RETIRED_SPLIT_LOADS                        {0x00, 0x42, 0x41, 0xd0}
#define MEM_UOPS_RETIRED_SPLIT_STORES                       {0x00, 0x42, 0x42, 0xd0}

#define MEM_INST_RETIRED_ALL_LOADS                          {0x00, 0x42, 0x81, 0xd0}
#define MEM_UOPS_RETIRED_ALL_LOADS                          {0x00, 0x42, 0x81, 0xd0}
#define MEM_INST_RETIRED_ALL_STORES                         {0x00, 0x42, 0x82, 0xd0}
#define MEM_UOPS_RETIRED_ALL_STORES                         {0x00, 0x42, 0x82, 0xd0}

#define MEM_LOAD_RETIRED_L1_HIT                             {0x00, 0x42, 0x01, 0xd1}
#define MEM_LOAD_RETIRED_L2_HIT                             {0x00, 0x42, 0x02, 0xd1}
#define MEM_LOAD_RETIRED_L3_HIT                             {0x00, 0x42, 0x04, 0xd1}
#define MEM_LOAD_RETIRED_LFB_HIT                            {0x00, 0x42, 0x08, 0xd1}

#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_MISS             {0x00, 0x42, 0x01, 0xd2}
#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_HIT              {0x00, 0x42, 0x02, 0xd2}
#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_HITM             {0x00, 0x42, 0x04, 0xd2}
#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_NONE             {0x00, 0x42, 0x08, 0xd2}

#define LDS_UOPS                                            {0x00, 0x42, 0x01, 0xa8}
#define IDQ_MITE_UOPS                                       {0x00, 0x42, 0x04, 0x79}
#define IDQ_DSB_UOPS                                        {0x00, 0x42, 0x08, 0x79}
#define IDQ_MS_UOPS                                         {0x00, 0x42, 0x30, 0x79}

#define INST_DECODED_DEC0                                   {0x00, 0x42, 0x01, 0x18}

#define FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE            {0x00, 0x42, 0x10, 0xc7}
#define FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE            {0x00, 0x42, 0x20, 0xc7}
#define FP_ARITH_INST_RETIRED_512B_PACKED_SINGLE            {0x00, 0x42, 0x80, 0xc7}
#define FP_ARITH_INST_RETIRED_SCALAR_SINGLE                 {0x00, 0x42, 0x02, 0xc7}
#define ARITH_DIVIDER_ACTIVE                                {0x01, 0x42, 0x01, 0x14}

// Doesn't seem to work.
#define UOPS_EXECUTED_PORT015                               {0x00, 0x42, 0x40, 0xb1}
#define MACRO_INSTS_DECODED                                 {0x00, 0x42, 0x01, 0xaa}
#define MACRO_INSTS_CISC_DECODED                            {0x00, 0x42, 0x08, 0xaa}
#define BR_MISP_RETIRED_ALL_BRANCHES                        {0x00, 0x42, 0x04, 0xc5}
#define BR_MISP_RETIRED_NEAR_CALL                           {0x00, 0x42, 0x02, 0xc5}
#define BR_INST_RETIRED_ALL_BRANCHES                        {0x00, 0x42, 0x04, 0xc4}
#define BR_INST_RETIRED_NEAR_CALL                           {0x00, 0x42, 0x04, 0xc4}
#define BR_INST_EXEC                                        {0x00, 0x42, 0x00, 0x88}
#define BR_MISSP_EXEC                                       {0x00, 0x42, 0x00, 0x89}
#define BR_BAC_MISSP_EXEC                                   {0x00, 0x42, 0x00, 0x8a}
#define BR_CND_EXEC                                         {0x00, 0x42, 0x00, 0x8b}
#define BR_CND_MISSP_EXEC                                   {0x00, 0x42, 0x00, 0x8c}

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
