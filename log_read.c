#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int64_t num_cycles;
int64_t num_trials;
int32_t num_counters;
int32_t extra_header_space;

#define TOL 1.5f
#define NUM_STATS 8
#define NUM_CONSISTENT_INFO 4
#define MAX_LOOP 200000000
#define PROB_CUT 0.9

void update_stats(int64_t *stats, int64_t *data, int i, int j, int count_num){
  int data_ind = j*num_trials*num_counters + num_counters*i + count_num;
  int stat_ind = count_num*NUM_STATS;
  if(i == 0){
    stats[stat_ind] = data[data_ind];
    stats[stat_ind + 1] = data[data_ind];
    stats[stat_ind + 2] = data[data_ind];
  }
  else{
    // Compute min
    stats[stat_ind + 0] = (stats[stat_ind + 0] < data[data_ind]) ?
      stats[stat_ind + 0] : data[data_ind];
    // Compute average
    stats[stat_ind + 1] += data[data_ind];
    // Compute max
    stats[stat_ind + 2] = (stats[stat_ind + 2] > data[data_ind]) ?
      stats[stat_ind + 2] : data[data_ind];
  }
  return;
}

int print_hist_bool;
void print_histogram(int64_t *stats, int j, int count_num, int64_t *hist){
  int stat_ind = count_num*NUM_STATS,i;
  print_hist_bool = 1;
  printf("  *** PMC%d: ",count_num);
  for(i = 0; i < hist[0]; i++){
    printf("[%ld : %ld] ",hist[2*(i+1)], hist[2*(i+1)+1]);
  }
  printf("\n");
}

void reset_tail(int64_t *list, int64_t *incr_table_range, int64_t ***hist_table,
                int64_t vstart, int64_t vend, int64_t tail, int64_t k, int64_t val){
  if(tail == vend) return;
  int64_t i,j,temp = val, len_hist;

  for(j = tail + 1; j <= vend; j++){
    incr_table_range[3*(j-vstart) + 1] = incr_table_range[3*(j-vstart)];
    i = incr_table_range[3*(j-vstart) + 1];
    len_hist = incr_table_range[3*(j-vstart) + 2];
    while( i <= len_hist ){
      if( hist_table[j][k][2*(i+1)] >= temp){
        temp = hist_table[j][k][2*(i+1)];
        list[j - vstart] = hist_table[j][k][2*(i+1)];
        break;
      }
      incr_table_range[3*(j-vstart) + 1]++;
      i++;
    }
  }
}

void incr_list(int64_t *list, int64_t *incr_table_range,
               int64_t ***hist_table, int64_t vstart, int64_t vend, int64_t k){
  int count = -1;
  int i = vend - vstart;
  int64_t ran_e,ran_c;
  int64_t val;
  /**
   * Find the first element from the right
   * that can be incremented.
   */
  while( count == -1 && i >= 0){
    // The maximum index for each element
    ran_e = incr_table_range[3*i+2];

    if( list[i] < hist_table[i + vstart][k][2*(ran_e+1)]){
      /**
       * list[i] can be incremented.
       */
      ran_c = incr_table_range[3*i+1] + 1;

      if(i < vend - vstart){
        if( hist_table[i + vstart][k][2*(ran_c+1)] <= list[i+1] ){
          count = vstart + i;
          incr_table_range[3*i+1]++;
          list[i] = hist_table[i + vstart][k][2*(ran_c+1)];
        }
      }
      else{
        count = vstart + i;
        incr_table_range[3*i+1]++;
        list[i] = hist_table[i + vstart][k][2*(ran_c+1)];
      }
    }
    i--;
  }
  val = (i != vend-vstart) ? list[i+1] : 0;
  if(count != -1) reset_tail(list, incr_table_range, hist_table, vstart, vend, count, k, val);
  return;
}

void calc_prob(int64_t **stats, int64_t ***hist_table,
               int64_t vstart, int64_t vend,
               int64_t num_trials, int64_t num_cycles, int64_t count_num, int64_t *consistent){

  int64_t stat_ind = NUM_STATS*count_num, j, i, k = count_num;
  int64_t num_in_range,product = 1;
  // len_hist should not be 1.
  int64_t len_hist;

  int64_t *incr_table_range = (int64_t*) malloc(sizeof(int64_t)*3*(vend-vstart+1));
  int64_t *min_list = (int64_t*) malloc(sizeof(int64_t)*(vend-vstart+1));
  int64_t *max_list = (int64_t*) malloc(sizeof(int64_t)*(vend-vstart+1));
  int64_t *best_list = (int64_t*) malloc(sizeof(int64_t)*(vend-vstart+1));

  int64_t max_val = stats[vend + 1][stat_ind + 4];
  int64_t min_val = stats[vstart - 1][stat_ind + 4];

  int64_t temp = min_val,count;
  int64_t ran_s,ran_e;

  int32_t min_eq_max = 0,num_perm = 0;
  int64_t max_prob = 0,temp_prob = 0,prob_inc, loop_detect = 0, itr = 0;
  int64_t prob_limit = 0;
  int32_t impossible = 0;

  consistent[NUM_CONSISTENT_INFO*k]++;

  if(vend - vstart > consistent[NUM_CONSISTENT_INFO*k+1])
    consistent[NUM_CONSISTENT_INFO*k+1] = vend-vstart;

  if( min_val == max_val ){
    printf(" [ min_val == max_val ] :)\n");
    goto end_func;
  }

  for(j = vstart; j <= vend; j++){
    ran_s = ran_e = -1;
    for( i = 0; i < hist_table[j][k][0]; i++){
      if( hist_table[j][k][2*(i+1)] >= min_val && ran_s == -1){
        ran_s = i;
      }
      if( hist_table[j][k][2*(i+1)] <= max_val){
        ran_e = i;
      }
    }
    incr_table_range[3*(j-vstart)] = ran_s;
    incr_table_range[3*(j-vstart) + 1] = ran_s;
    incr_table_range[3*(j-vstart) + 2] = ran_e;
  }

  for(j = vstart; j <= vend; j++){
    i = incr_table_range[3*(j-vstart)];
    len_hist = incr_table_range[3*(j-vstart) + 2];
    while( i <= len_hist ){
      if( hist_table[j][k][2*(i+1)] >= temp){
        temp = hist_table[j][k][2*(i+1)];
        min_list[j - vstart] = hist_table[j][k][2*(i+1)];
        break;
      }
      i++;
      incr_table_range[3*(j-vstart) + 1] = i;
    }
    if(!min_list[j - vstart]){
      printf("Inconsistent\n");
      consistent[NUM_CONSISTENT_INFO*k + 2]++;
      impossible = 1;
      goto end_func;
    }
    best_list[j-vstart] = min_list[j - vstart];

    prob_limit += stats[j][stat_ind + 6];
  }

  temp = max_val;
  for(j = vend; j >= vstart; j--){
    len_hist = incr_table_range[3*(j-vstart) + 2];
    i = len_hist;
    while( i >= incr_table_range[3*(j-vstart)] ){
      if( hist_table[j][k][2*(i+1)] <= temp){
        temp = hist_table[j][k][2*(i+1)];
        max_list[j - vstart] = hist_table[j][k][2*(i+1)];
        break;
      }
      i--;
    }
  }

  for(j = vstart; j <= vend; j++){
    prob_inc = incr_table_range[3*(j-vstart) + 1];
    max_prob += hist_table[j][k][2*(prob_inc+1) + 1];
  }

  printf("Initial prob = %d, Max possible = %d, Min = %d, Max = %d\n",
         max_prob, prob_limit,
         min_val,max_val);

  while(!min_eq_max && loop_detect < MAX_LOOP){
    incr_list(min_list, incr_table_range, hist_table, vstart, vend, k);
    min_eq_max = 1;
    temp_prob = 0;
    for(j = vstart; j <= vend; j++){
      prob_inc = incr_table_range[3*(j-vstart) + 1];
      temp_prob += hist_table[j][k][2*(prob_inc+1) + 1];
      if(max_list[j-vstart] != min_list[j-vstart]) min_eq_max = 0;
    }

    if(vstart == 34){
      printf("%d: ", num_perm+1);
      for(j = vstart; j <= vend; j++){
        printf("%d:%d ",min_list[j-vstart], max_list[j-vstart]);
      }
      printf(" : prob = %d\n",temp_prob);
    }
    if( (num_perm & 0x0000000000ffffff) == 0)
      printf("Current max_prob = %d, inc = %d, size = %d\n",max_prob,num_perm,vend-vstart);

    if(temp_prob > max_prob){
      max_prob = temp_prob;
      for(j = vstart; j <= vend; j++){
        best_list[j-vstart] = min_list[j-vstart];
      }
      itr = num_perm;
    }

    num_perm++;
    loop_detect++;
  }

  printf("Took %d permutations our of %d with max_prob = %d\n",itr, num_perm, max_prob);

end_func:

  for(j = vstart; j <= vend; j++){
    if(impossible)
      stats[j][stat_ind + 7] = stats[j][stat_ind + 4];
    else if( min_val == max_val )
      stats[j][stat_ind + 7] = min_val;
    else if( min_val < max_val )
      stats[j][stat_ind + 7] = best_list[j-vstart];
  }

  free(incr_table_range);
  free(min_list);
  free(max_list);
  free(best_list);
  return;
}

int main(int argc, char *argv[]){
  int hist_info = 0;
  if(argc > 2) hist_info = atoi(argv[2]);
  FILE *file = fopen(argv[1], "rb");
  FILE *file1 = fopen("stripped_for_plotting.txt","w");
  int64_t header[4];
  int i,j,k;
  int ret;
  ret = fread(header, sizeof(int64_t), 4, file);

  num_cycles = header[0];
  num_trials = header[1];
  num_counters = ((int32_t*)&header[2])[0];
  extra_header_space = ((int32_t*)&header[2])[1];

  int32_t *count_problem = (int32_t*) malloc(sizeof(int32_t)*2*num_counters);
  for(i = 0; i < num_counters; i++){
    count_problem[2*i] = 0;
    count_problem[2*i+1] = -1;
  }
  printf("Number of Cycles simulated : %ld\n",header[0]);
  printf("Number of Trials per cycle : %ld\n",header[1]);
  printf("Number of Events counted   : %d\n",num_counters);
  printf("Extra Header Space         : %d\n",extra_header_space);

  printf("Random Data....            : %ld\n",header[3]);

  uint32_t *pmc_info = (uint32_t*) malloc(sizeof(int64_t)*(extra_header_space-4));
  ret = fread(pmc_info, sizeof(int64_t), extra_header_space-4, file);

  uint32_t evnt_mask = 0xff, umask = 0xff00, cmask = 0xff000000, inv_mask = 0x0080000;
  for(i = 0; i < 2*(extra_header_space - 4); i++){
    printf("PMC%-2d Event Selector       : 0x%-8X", i, pmc_info[i]);
    if(pmc_info[i] != 0xdeadbeef){
      printf("   CMASK: 0x%02X", (pmc_info[i] & cmask) >> 24);
      printf("   INV: 0x%02X", (pmc_info[i] & inv_mask) >> 23);
      printf("   UMASK: 0x%02X", (pmc_info[i] & umask) >> 8);
      printf("   EVENT: 0x%02X\n", pmc_info[i] & evnt_mask);
    }
    else{
      printf("\n");
    }
  }

  printf("PMCX: [Min, 2nd Most Freq, Most Freq, Max]\n");
  printf("==============================================================\n");
  printf("==============================================================\n");

  int64_t num_data = num_cycles*num_trials*num_counters;
  int64_t *data_ptr = (int64_t*)malloc(sizeof(int64_t)*num_data);
  ret = fread(data_ptr, sizeof(int64_t), num_data, file);

  int64_t **stats = (int64_t**) malloc(sizeof(int64_t*)*(num_cycles+1));
  for(j = 1; j < num_cycles; j++)
    stats[j] = (int64_t*) malloc(sizeof(int64_t)*num_counters*NUM_STATS);

  int64_t ***hist_table = (int64_t***) malloc(sizeof(int64_t**)*(num_cycles+1));
  for(j = 1; j < num_cycles; j++)
    hist_table[j] = (int64_t**) malloc(sizeof(int64_t*)*num_counters);

  int64_t *hist_temp;

  int64_t *consistent = (int64_t*) malloc(sizeof(int64_t)*NUM_CONSISTENT_INFO*(num_counters));
  for(k = 0; k < NUM_CONSISTENT_INFO*num_counters; k++)
    consistent[k] = 0;

  int64_t stat_ind, data_ind, max, second_max, non_zero,temp_count;

  for(j = 1; j < num_cycles; j++){
    for(k = 0; k < NUM_STATS*num_counters; k++) stats[j][k] = 0;
    for(i = 0; i < num_trials; i++){
      for(k = 0; k < num_counters; k++){
        /**
         * Compute the max/min and average PMC values
         * from trial datum..
         */
        update_stats(stats[j], data_ptr, i, j, k);
      }
    }
    for(k = 0; k < num_counters; k++){
      /**
       * Allocate array to hold histogram for each PMC
       */
      stat_ind = k*NUM_STATS;
      int64_t range = stats[j][stat_ind+2] - stats[j][stat_ind];

      hist_temp = (int64_t*) malloc(sizeof(int64_t)*( range + 1));
      for(i = 0; i < range + 1; i++){
        hist_temp[i] = 0;
      }

      /**
       * Populate hist_temp with frequencies.
       */
      for(i = 0; i < num_trials; i++){
        data_ind = j*num_trials*num_counters + num_counters*i + k;
        hist_temp[data_ptr[data_ind] - stats[j][stat_ind]]++;
      }

      /**
       * Count the number of nonzero counts.
       */
      non_zero = 0;
      for(i = 0; i < range + 1; i++){
        if(hist_temp[i]) non_zero++;
      }

      /**
       * {len, 0, index1,count1,index2,count2,...index_len,count_len}
       * This is one based indexed......
       */
      hist_table[j][k] = (int64_t*) malloc(sizeof(int64_t)*2*(non_zero+1));
      hist_table[j][k][0] = non_zero;
      temp_count = 1;
      for(i = 0; i < range + 1; i++){
        if(hist_temp[i]){
          // Counter value
          hist_table[j][k][2*temp_count] = i + stats[j][stat_ind];
          // Counter freq.
          hist_table[j][k][2*temp_count + 1] = hist_temp[i];
          temp_count++;
        }
      }
      free(hist_temp);

      /**
       * Compute the max and second max..
       */
      if( hist_table[j][k][0] > 1 ){
        for(i = 0; i < hist_table[j][k][0]; i++){
          if(i == 0){
            max = second_max = hist_table[j][k][2*(i+1) + 1];
            stats[j][stat_ind + 4] = hist_table[j][k][2*(i+1)];
          }
          else{
            if(hist_table[j][k][2*(i+1) + 1] > max) {
              second_max = max;
              max = hist_table[j][k][2*(i+1) + 1];
              stats[j][stat_ind + 3] = stats[j][stat_ind + 4];
              stats[j][stat_ind + 4] = hist_table[j][k][2*(i+1)];
            }
            else if( (hist_table[j][k][2*(i+1) + 1] < max &&
                      hist_table[j][k][2*(i+1) + 1] > second_max)
                     || ( hist_table[j][k][2*(i+1) + 1] < max && max == second_max) ) {
              second_max = hist_table[j][k][2*(i+1) + 1];
              stats[j][stat_ind + 3] = hist_table[j][k][2*(i+1)];
            }
          }
        }
        stats[j][stat_ind + 5] = second_max;
        stats[j][stat_ind + 6] = max;
      }
      else{
        stats[j][stat_ind + 4] = stats[j][stat_ind + 3] = stats[j][stat_ind];
      }
    }
  }


  int64_t vstart, vend, condition1, condition2, condition3;

  if(hist_info == 666){
    for(k = 0; k < num_counters; k++){
      vstart = vend = -1;
      for(j = 1; j < num_cycles; j++){
        if(j > 1 && j < num_cycles - 1){

/*        condition1 = (stats[j][NUM_STATS*k] != stats[j][NUM_STATS*k+2] &&
          ((double)stats[j][NUM_STATS*k+6])/( (double) num_trials) < PROB_CUT );*/
          condition1 = (stats[j][NUM_STATS*k] != stats[j][NUM_STATS*k+2] &&
                        ( (stats[j][NUM_STATS*k + 4] < stats[j-1][NUM_STATS*k + 4]) ) );
          condition2 = stats[j][NUM_STATS*k +4] < stats[j-1][NUM_STATS*k +4];
          condition3 = (stats[j+1][NUM_STATS*k] != stats[j+1][NUM_STATS*k+2]) &&
            (((double)stats[j+1][NUM_STATS*k+6])/( (double) num_trials) < PROB_CUT );

          if( condition1 || vstart != -1 ){
            if( vstart == -1 && j < num_cycles - 2 ){
              printf("\nPMC%d: ",k);
              vstart = j;
            }
            else if( (!condition3 || j == num_cycles-2 ) && vstart != -1 ){
              vend = j;
              printf("[%d, %d] ",vstart,vend);
              /**
               * Dirty hack to try to deal with mis-matching endpoints..
               */
              while( (stats[vend + 1][NUM_STATS*k + 4] < stats[vstart - 1][NUM_STATS*k + 4])){
                if(vstart > 2) vstart--;
                if(vend < num_cycles - 2) vend++;
                printf("-> [%d, %d] ",vstart,vend);
              }
              calc_prob(stats, hist_table, vstart, vend, num_trials, num_cycles, k, consistent);
              vstart = vend = -1;
            }
            else{
              stats[j][NUM_STATS*k+7] = stats[j][NUM_STATS*k+4];
            }
          }
          else{
            stats[j][NUM_STATS*k+7] = stats[j][NUM_STATS*k+4];
          }
        }
        else{
          stats[j][NUM_STATS*k+7] = stats[j][NUM_STATS*k+4];
        }
      }
    }

    printf("==============================================================\n");
    printf("==============================================================\n");
  }
  for(j = 1; j < num_cycles; j++){
    /**
     * Print the information in a nice format.
     */
    printf("Cycle: %-4d |",j);
    fprintf(file1, "%d ",j);
    for(k = 0; k < num_counters; k++){
      if(hist_info == num_counters + 2){
        printf("   PC%d: %-4ld |",
               k, stats[j][NUM_STATS*k+4]);
      }
      else{
        printf("   PC%d: [%-4ld, %-4ld, %-4ld, %-4ld]",
               k,
               stats[j][NUM_STATS*k], // Min
               //(double)stats[NUM_STATS*k+1]/(num_trials*1.0f), // Average
               stats[j][NUM_STATS*k+3],
               hist_info == 666 ? stats[j][NUM_STATS*k+7] : stats[j][NUM_STATS*k+4], // Occurred Most
               stats[j][NUM_STATS*k+2]); // Max
      }
      if(hist_info == 666)
        fprintf(file1, "%ld ",stats[j][NUM_STATS*k+7]);
      else
        fprintf(file1, "%ld ",stats[j][NUM_STATS*k+4]);
    }
    printf("\n");
    fprintf(file1, "\n");

    /**
     * Depending on hist_info value:
     *
     *   IF hist_info in [0,num_counters-1]
     *     - Always print histogram for that PMC for each cycle
     *   IF hist_info = num_counters
     *     - Print all histogram for each PCM each cycle
     *   IF hist_info = num_counters + 1
     *     - Print all histogram for PCM where
     *       second_max*TOL > max for each cycle
     */
    if(hist_info >= 0){
      print_hist_bool = 0;
      if(hist_info == num_counters){
        for(k = 0; k < num_counters; k++)
          print_histogram(stats[j], j, k, hist_table[j][k]);
      }
      else if(hist_info == num_counters + 1)
        for(k = 0; k < num_counters; k++){
          if( (double)stats[j][NUM_STATS*k+5] * TOL > (double) stats[j][NUM_STATS*k+6] &&
              stats[j][NUM_STATS*k] != stats[j][NUM_STATS*k + 2]){
            print_histogram(stats[j], j, k, hist_table[j][k]);
            count_problem[2*k]++;
            if( count_problem[2*k+1] == -1 ) count_problem[2*k+1] = j;
          }
        }
      else if(hist_info < num_counters ){
        print_histogram(stats[j], j, hist_info, hist_table[j][hist_info]);
      }
      if(print_hist_bool) printf("\n");
    }
  }

  if(hist_info == 666){
    printf("Number Of Replacements\n");
    for(k = 0; k < num_counters; k++){
      printf("PMC%d: %d, ",
             k, consistent[NUM_CONSISTENT_INFO*k]);
    }
    printf("\n");

    printf("Longest Sequence Replaced\n");
    for(k = 0; k < num_counters; k++){
      printf("PMC%d: %d, ",
             k, consistent[NUM_CONSISTENT_INFO*k+1]);

    }
    printf("\n");


    int64_t consistent_sum = 0;
    printf("Consistent Check\n");
    for(k = 0; k < num_counters; k++){
      printf("PMC%d: %d, ",
             k, consistent[NUM_CONSISTENT_INFO*k+2]);
      consistent_sum += consistent[NUM_CONSISTENT_INFO*k+2];
    }
    printf("\n");
    if(consistent_sum) printf("Data too noisy, consider increasing number of trials..\n");
  }
  for(j = 1; j < num_cycles; j++){
    for(k = 0; k < num_counters; k++){
      free(hist_table[j][k]);
    }
    free(hist_table[j]);
  }

  for(j = 1; j < num_cycles; j++){
    free(stats[j]);
  }
  fclose(file);
  fclose(file1);
  free(consistent);
  free(hist_table);
  free(count_problem);
  free(data_ptr);
  free(stats);
  free(pmc_info);
  return 0;

}
