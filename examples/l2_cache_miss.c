#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <wrapperf/wrapperf.h>

__attribute__((optimize("O0")))
static void do_calc(char* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    data[i] += i;
  }
}

int main(int argc, char* argv[]) {
  wrapperf_t wp;
  wrapperf_init(&wp);
  wrapperf_l2_cache_miss_init(&wp);

  /* # of L2 cache misses should be:
   * 10 MB / 64 B (cacheline) * 100 ~ 16 M */
  int n_iter = 100;
  size_t size = 10000000;
  char* data = malloc(size);

  wrapperf_l2_cache_miss_start(&wp);

  for (int it = 0; it < n_iter; it++) {
    do_calc(data, size);
  }

  wrapperf_l2_cache_miss_stop(&wp);

  printf("========== All ==========\n");
  wrapperf_l2_cache_miss_print_all(&wp);

  printf("========== Sum ==========\n");
  wrapperf_l2_cache_miss_print_sum(&wp);

  wrapperf_l2_cache_miss_fini(&wp);
  wrapperf_fini(&wp);

  return 0;
}
