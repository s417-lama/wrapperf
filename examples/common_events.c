#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <wrapperf/wrapperf.h>

static inline uint64_t gettime_in_nsec() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
}

int main(int argc, char* argv[]) {
  wrapperf_t wp;
  wrapperf_init(&wp);

  wrapperf_cpu_cycle_init(&wp);
  wrapperf_cpu_ref_cycle_init(&wp);

  wrapperf_cpu_cycle_start(&wp);
  wrapperf_cpu_ref_cycle_start(&wp);

  uint64_t t0 = gettime_in_nsec();
  uint64_t t1 = gettime_in_nsec();
  while (t1 - t0 < 100000000) {
    t1 = gettime_in_nsec();
  }

  wrapperf_cpu_cycle_stop(&wp);
  wrapperf_cpu_ref_cycle_stop(&wp);

  printf("========== All  ==========\n");
  wrapperf_cpu_cycle_print_all(&wp);
  wrapperf_cpu_ref_cycle_print_all(&wp);

  printf("========== Sum ==========\n");
  wrapperf_cpu_cycle_print_sum(&wp);
  wrapperf_cpu_ref_cycle_print_sum(&wp);

  wrapperf_cpu_cycle_fini(&wp);
  wrapperf_cpu_ref_cycle_fini(&wp);

  wrapperf_fini(&wp);

  return 0;
}
