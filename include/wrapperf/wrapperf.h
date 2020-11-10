#pragma once
#ifndef WRAPPERF_WRAPPERF_H_
#define WRAPPERF_WRAPPERF_H_

#include "wrapperf/common.h"
#include "wrapperf/skylake.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wrapperf {
  wrapperf_event_t cpu_cycle_event;

  wrapperf_skylake_t skylake;
} wrapperf_t;

static inline void wrapperf_init(wrapperf_t* wp) {
  _wrapperf_cpu_cycle_init(&wp->cpu_cycle_event);
  _wrapperf_init(&wp->skylake);
}

static inline void wrapperf_fini(wrapperf_t* wp) {
  _wrapperf_event_fini(&wp->cpu_cycle_event);
}

static inline void wrapperf_cpu_cycle_start(wrapperf_t* wp) {
  _wrapperf_event_start(&wp->cpu_cycle_event);
}

static inline void wrapperf_cpu_cycle_stop(wrapperf_t* wp) {
  _wrapperf_event_stop(&wp->cpu_cycle_event);
}

static inline void wrapperf_cpu_cycle_print(wrapperf_t* wp) {
  uint64_t c = _wrapperf_event_get_value(&wp->cpu_cycle_event);
  printf("CPU Cycles: %ld\n", c);
}

/*
 * L3 cache miss
 */

static inline void wrapperf_l3_cache_miss_init(wrapperf_t* wp) {
  _wrapperf_uncore_cha_init(&wp->skylake);
}

static inline void wrapperf_l3_cache_miss_fini(wrapperf_t* wp) {
  _wrapperf_uncore_cha_fini(&wp->skylake);
}

static inline void wrapperf_l3_cache_miss_start(wrapperf_t* wp) {
  _wrapperf_uncore_cha_start(&wp->skylake);
}

static inline void wrapperf_l3_cache_miss_stop(wrapperf_t* wp) {
  _wrapperf_uncore_cha_stop(&wp->skylake);
}

static inline void wrapperf_l3_cache_miss_print_all(wrapperf_t* wp) {
  _wrapperf_uncore_cha_print_all(&wp->skylake);
}

static inline void wrapperf_l3_cache_miss_print_per_socket(wrapperf_t* wp) {
  _wrapperf_uncore_cha_print_per_socket(&wp->skylake);
}

static inline void wrapperf_l3_cache_miss_print_sum(wrapperf_t* wp) {
  _wrapperf_uncore_cha_print_sum(&wp->skylake);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WRAPPERF_WRAPPERF_H_ */
