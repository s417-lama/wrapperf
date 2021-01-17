#pragma once
#ifndef WRAPPERF_WRAPPERF_H_
#define WRAPPERF_WRAPPERF_H_

#include "wrapperf/common.h"

#if WRAPPERF_ARCH == WRAPPERF_ARCH_SKYLAKE
#include "wrapperf/skylake.h"
#elif WRAPPERF_ARCH == WRAPPERF_ARCH_BROADWELL
#include "wrapperf/broadwell.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wrapperf {
  wrapperf_allcore_event_t cpu_cycle_events;
  wrapperf_allcore_event_t cpu_ref_cycle_events;
  wrapperf_allcore_event_t l1d_cache_miss_events;

  wrapperf_arch_t arch_wp;
} wrapperf_t;

static inline void wrapperf_init(wrapperf_t* wp) {
  _wrapperf_init(&wp->arch_wp);
}

static inline void wrapperf_fini(wrapperf_t* wp) {
  _wrapperf_fini(&wp->arch_wp);
}

/*
 * CPU cycle
 */

static inline void wrapperf_cpu_cycle_init(wrapperf_t* wp) {
  int n_core = wp->arch_wp.n_core;
  _wrapperf_allcore_event_init(&wp->cpu_cycle_events, n_core,
                               _wrapperf_cpu_cycle_init, "CPU Cycle");
}

static inline void wrapperf_cpu_cycle_fini(wrapperf_t* wp) {
  _wrapperf_allcore_event_fini(&wp->cpu_cycle_events);
}

static inline void wrapperf_cpu_cycle_start(wrapperf_t* wp) {
  _wrapperf_allcore_event_start(&wp->cpu_cycle_events);
}

static inline void wrapperf_cpu_cycle_stop(wrapperf_t* wp) {
  _wrapperf_allcore_event_stop(&wp->cpu_cycle_events);
}

static inline uint64_t wrapperf_cpu_cycle_get_ith(wrapperf_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->cpu_cycle_events, i);
}

static inline uint64_t wrapperf_cpu_cycle_get_sum(wrapperf_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->cpu_cycle_events);
}

static inline void wrapperf_cpu_cycle_print_all(wrapperf_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->cpu_cycle_events);
}

static inline void wrapperf_cpu_cycle_print_sum(wrapperf_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->cpu_cycle_events);
}

/*
 * CPU ref cycle
 */

static inline void wrapperf_cpu_ref_cycle_init(wrapperf_t* wp) {
  int n_core = wp->arch_wp.n_core;
  _wrapperf_allcore_event_init(&wp->cpu_ref_cycle_events, n_core,
                               _wrapperf_cpu_ref_cycle_init, "CPU Reference Cycle");
}

static inline void wrapperf_cpu_ref_cycle_fini(wrapperf_t* wp) {
  _wrapperf_allcore_event_fini(&wp->cpu_ref_cycle_events);
}

static inline void wrapperf_cpu_ref_cycle_start(wrapperf_t* wp) {
  _wrapperf_allcore_event_start(&wp->cpu_ref_cycle_events);
}

static inline void wrapperf_cpu_ref_cycle_stop(wrapperf_t* wp) {
  _wrapperf_allcore_event_stop(&wp->cpu_ref_cycle_events);
}

static inline uint64_t wrapperf_cpu_ref_cycle_get_ith(wrapperf_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->cpu_ref_cycle_events, i);
}

static inline uint64_t wrapperf_cpu_ref_cycle_get_sum(wrapperf_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->cpu_ref_cycle_events);
}

static inline void wrapperf_cpu_ref_cycle_print_all(wrapperf_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->cpu_ref_cycle_events);
}

static inline void wrapperf_cpu_ref_cycle_print_sum(wrapperf_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->cpu_ref_cycle_events);
}

/*
 * L1 data cache miss
 */

static inline void wrapperf_l1d_cache_miss_init(wrapperf_t* wp) {
  int n_core = wp->arch_wp.n_core;
  _wrapperf_allcore_event_init(&wp->l1d_cache_miss_events, n_core,
                               _wrapperf_l1d_cache_miss_init, "L1 Data Cache Miss");
}

static inline void wrapperf_l1d_cache_miss_fini(wrapperf_t* wp) {
  _wrapperf_allcore_event_fini(&wp->l1d_cache_miss_events);
}

static inline void wrapperf_l1d_cache_miss_start(wrapperf_t* wp) {
  _wrapperf_allcore_event_start(&wp->l1d_cache_miss_events);
}

static inline void wrapperf_l1d_cache_miss_stop(wrapperf_t* wp) {
  _wrapperf_allcore_event_stop(&wp->l1d_cache_miss_events);
}

static inline uint64_t wrapperf_l1d_cache_miss_get_ith(wrapperf_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->l1d_cache_miss_events, i);
}

static inline uint64_t wrapperf_l1d_cache_miss_get_sum(wrapperf_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->l1d_cache_miss_events);
}

static inline void wrapperf_l1d_cache_miss_print_all(wrapperf_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->l1d_cache_miss_events);
}

static inline void wrapperf_l1d_cache_miss_print_sum(wrapperf_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->l1d_cache_miss_events);
}

/*
 * L2 cache miss
 */

static inline void wrapperf_l2_cache_miss_init(wrapperf_t* wp) {
  _wrapperf_l2_cache_miss_init(&wp->arch_wp);
}

static inline void wrapperf_l2_cache_miss_fini(wrapperf_t* wp) {
  _wrapperf_l2_cache_miss_fini(&wp->arch_wp);
}

static inline void wrapperf_l2_cache_miss_start(wrapperf_t* wp) {
  _wrapperf_l2_cache_miss_start(&wp->arch_wp);
}

static inline void wrapperf_l2_cache_miss_stop(wrapperf_t* wp) {
  _wrapperf_l2_cache_miss_stop(&wp->arch_wp);
}

static inline uint64_t wrapperf_l2_cache_miss_get_ith(wrapperf_t* wp, int i) {
  return _wrapperf_l2_cache_miss_get_ith(&wp->arch_wp, i);
}

static inline uint64_t wrapperf_l2_cache_miss_get_sum(wrapperf_t* wp) {
  return _wrapperf_l2_cache_miss_get_sum(&wp->arch_wp);
}

static inline void wrapperf_l2_cache_miss_print_all(wrapperf_t* wp) {
  _wrapperf_l2_cache_miss_print_all(&wp->arch_wp);
}

static inline void wrapperf_l2_cache_miss_print_sum(wrapperf_t* wp) {
  _wrapperf_l2_cache_miss_print_sum(&wp->arch_wp);
}

/*
 * L3 cache miss
 */

static inline void wrapperf_l3_cache_miss_init(wrapperf_t* wp) {
  _wrapperf_l3_cache_miss_init(&wp->arch_wp);
}

static inline void wrapperf_l3_cache_miss_fini(wrapperf_t* wp) {
  _wrapperf_l3_cache_miss_fini(&wp->arch_wp);
}

static inline void wrapperf_l3_cache_miss_start(wrapperf_t* wp) {
  _wrapperf_l3_cache_miss_start(&wp->arch_wp);
}

static inline void wrapperf_l3_cache_miss_stop(wrapperf_t* wp) {
  _wrapperf_l3_cache_miss_stop(&wp->arch_wp);
}

static inline void wrapperf_l3_cache_miss_print_all(wrapperf_t* wp) {
  _wrapperf_l3_cache_miss_print_all(&wp->arch_wp);
}

static inline void wrapperf_l3_cache_miss_print_per_socket(wrapperf_t* wp) {
  _wrapperf_l3_cache_miss_print_per_socket(&wp->arch_wp);
}

static inline uint64_t wrapperf_l3_cache_miss_get_sum(wrapperf_t* wp) {
  return _wrapperf_l3_cache_miss_get_sum(&wp->arch_wp);
}

static inline void wrapperf_l3_cache_miss_print_sum(wrapperf_t* wp) {
  _wrapperf_l3_cache_miss_print_sum(&wp->arch_wp);
}

/*
 * L1D stall cycle
 */

static inline void wrapperf_l1d_stall_cycle_init(wrapperf_t* wp) {
  _wrapperf_l1d_stall_cycle_init(&wp->arch_wp);
}

static inline void wrapperf_l1d_stall_cycle_fini(wrapperf_t* wp) {
  _wrapperf_l1d_stall_cycle_fini(&wp->arch_wp);
}

static inline void wrapperf_l1d_stall_cycle_start(wrapperf_t* wp) {
  _wrapperf_l1d_stall_cycle_start(&wp->arch_wp);
}

static inline void wrapperf_l1d_stall_cycle_stop(wrapperf_t* wp) {
  _wrapperf_l1d_stall_cycle_stop(&wp->arch_wp);
}

static inline uint64_t wrapperf_l1d_stall_cycle_get_ith(wrapperf_t* wp, int i) {
  return _wrapperf_l1d_stall_cycle_get_ith(&wp->arch_wp, i);
}

static inline uint64_t wrapperf_l1d_stall_cycle_get_sum(wrapperf_t* wp) {
  return _wrapperf_l1d_stall_cycle_get_sum(&wp->arch_wp);
}

static inline void wrapperf_l1d_stall_cycle_print_all(wrapperf_t* wp) {
  _wrapperf_l1d_stall_cycle_print_all(&wp->arch_wp);
}

static inline void wrapperf_l1d_stall_cycle_print_sum(wrapperf_t* wp) {
  _wrapperf_l1d_stall_cycle_print_sum(&wp->arch_wp);
}

/*
 * L2 stall cycle
 */

static inline void wrapperf_l2_stall_cycle_init(wrapperf_t* wp) {
  _wrapperf_l2_stall_cycle_init(&wp->arch_wp);
}

static inline void wrapperf_l2_stall_cycle_fini(wrapperf_t* wp) {
  _wrapperf_l2_stall_cycle_fini(&wp->arch_wp);
}

static inline void wrapperf_l2_stall_cycle_start(wrapperf_t* wp) {
  _wrapperf_l2_stall_cycle_start(&wp->arch_wp);
}

static inline void wrapperf_l2_stall_cycle_stop(wrapperf_t* wp) {
  _wrapperf_l2_stall_cycle_stop(&wp->arch_wp);
}

static inline uint64_t wrapperf_l2_stall_cycle_get_ith(wrapperf_t* wp, int i) {
  return _wrapperf_l2_stall_cycle_get_ith(&wp->arch_wp, i);
}

static inline uint64_t wrapperf_l2_stall_cycle_get_sum(wrapperf_t* wp) {
  return _wrapperf_l2_stall_cycle_get_sum(&wp->arch_wp);
}

static inline void wrapperf_l2_stall_cycle_print_all(wrapperf_t* wp) {
  _wrapperf_l2_stall_cycle_print_all(&wp->arch_wp);
}

static inline void wrapperf_l2_stall_cycle_print_sum(wrapperf_t* wp) {
  _wrapperf_l2_stall_cycle_print_sum(&wp->arch_wp);
}

/*
 * L3 stall cycle
 */

static inline void wrapperf_l3_stall_cycle_init(wrapperf_t* wp) {
  _wrapperf_l3_stall_cycle_init(&wp->arch_wp);
}

static inline void wrapperf_l3_stall_cycle_fini(wrapperf_t* wp) {
  _wrapperf_l3_stall_cycle_fini(&wp->arch_wp);
}

static inline void wrapperf_l3_stall_cycle_start(wrapperf_t* wp) {
  _wrapperf_l3_stall_cycle_start(&wp->arch_wp);
}

static inline void wrapperf_l3_stall_cycle_stop(wrapperf_t* wp) {
  _wrapperf_l3_stall_cycle_stop(&wp->arch_wp);
}

static inline void wrapperf_l3_stall_cycle_print_all(wrapperf_t* wp) {
  _wrapperf_l3_stall_cycle_print_all(&wp->arch_wp);
}

static inline uint64_t wrapperf_l3_stall_cycle_get_sum(wrapperf_t* wp) {
  return _wrapperf_l3_stall_cycle_get_sum(&wp->arch_wp);
}

static inline void wrapperf_l3_stall_cycle_print_sum(wrapperf_t* wp) {
  _wrapperf_l3_stall_cycle_print_sum(&wp->arch_wp);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WRAPPERF_WRAPPERF_H_ */
