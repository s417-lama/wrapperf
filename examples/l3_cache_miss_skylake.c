#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <wrapperf/wrapperf.h>

int main(int argc, char* argv[]) {
  wrapperf_t wp;
  wrapperf_init(&wp);
  wrapperf_l3_cache_miss_init(&wp);

  int n_iter = 10;
  size_t size = 500000000;
  char* data = malloc(size);

  wrapperf_l3_cache_miss_start(&wp);

  for (int it = 0; it < n_iter; it++) {
    for (size_t i = 0; i < size; i++) {
      data[i] = it;
    }
  }

  wrapperf_l3_cache_miss_stop(&wp);

  printf("========== All ==========\n");
  wrapperf_l3_cache_miss_print_all(&wp);

  printf("========== Per socket ==========\n");
  wrapperf_l3_cache_miss_print_per_socket(&wp);

  printf("========== Sum ==========\n");
  wrapperf_l3_cache_miss_print_sum(&wp);

  wrapperf_l3_cache_miss_fini(&wp);
  wrapperf_fini(&wp);

  return 0;
}
