#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <wrapperf/wrapperf.h>

int main(int argc, char* argv[]) {
  wrapperf_t wp;
  wrapperf_init(&wp);

  wrapperf_cpu_cycle_start(&wp);

  sleep(1);

  wrapperf_cpu_cycle_stop(&wp);

  wrapperf_cpu_cycle_print(&wp);

  wrapperf_fini(&wp);

  return 0;
}
