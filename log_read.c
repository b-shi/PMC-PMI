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
