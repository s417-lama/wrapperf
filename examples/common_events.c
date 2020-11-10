#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <wrapperf/wrapperf.h>

int main(int argc, char* argv[]) {
  wrapperf_t wp;
  wrapperf_init(&wp);
  wrapperf_cpu_cycle_init(&wp);

  wrapperf_cpu_cycle_start(&wp);

  sleep(1);

  wrapperf_cpu_cycle_stop(&wp);

  printf("========== All ==========\n");
  wrapperf_cpu_cycle_print_all(&wp);

  printf("========== Sum ==========\n");
  wrapperf_cpu_cycle_print_sum(&wp);

  wrapperf_cpu_cycle_fini(&wp);
  wrapperf_fini(&wp);

  return 0;
}
