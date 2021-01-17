#pragma once
#ifndef WRAPPERF_SKYLAKE_H_
#define WRAPPERF_SKYLAKE_H_

#include "wrapperf/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wrapperf_arch {
  int               n_core;
  int               n_socket;
  int               n_cha;

  wrapperf_allcore_event_t l2_cache_miss_events;
  wrapperf_event_t*        l3_cache_miss_events;

  wrapperf_allcore_event_t l1d_stall_cycle_events;
  wrapperf_allcore_event_t l2_stall_cycle_events;
  wrapperf_allcore_event_t l3_stall_cycle_events;
} wrapperf_arch_t;

static inline void _wrapperf_init(wrapperf_arch_t* wp) {
  wp->n_core = 56;
  wp->n_socket = 2;
  wp->n_cha = 28;
}

static inline void _wrapperf_fini(wrapperf_arch_t* wp) {
}

/*
 * event monitoring for L2 cache misses
 */

static inline void _wrapperf_l2_cache_miss_init_per_core(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_RAW;
  pe.config         = 0x3F24; // umask: 0x3F, event: 0x24
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

static inline void _wrapperf_l2_cache_miss_init(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_init(&wp->l2_cache_miss_events, wp->n_core,
                               _wrapperf_l2_cache_miss_init_per_core, "L2 Cache Misses");
}

static inline void _wrapperf_l2_cache_miss_fini(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_fini(&wp->l2_cache_miss_events);
}

static inline void _wrapperf_l2_cache_miss_start(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_start(&wp->l2_cache_miss_events);
}

static inline void _wrapperf_l2_cache_miss_stop(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_stop(&wp->l2_cache_miss_events);
}

static inline void _wrapperf_l2_cache_miss_print_all(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->l2_cache_miss_events);
}

static inline void _wrapperf_l2_cache_miss_print_sum(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->l2_cache_miss_events);
}

static inline uint64_t _wrapperf_l2_cache_miss_get_ith(wrapperf_arch_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->l2_cache_miss_events, i);
}

static inline uint64_t _wrapperf_l2_cache_miss_get_sum(wrapperf_arch_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->l2_cache_miss_events);
}

/*
 * uncore_cha event monitoring for L3 cache misses
 */

static inline uint32_t _wrapperf_cha_filter_0(uint16_t tid, uint16_t state) {
  uint32_t ret = 0;
  // config1:0-8 (/sys/bus/event_source/devices/uncore_cha_0/format/filter_tid)
  ret |= (tid & 0x000001ff);
  // config1:17-26 (/sys/bus/event_source/devices/uncore_cha_0/format/filter_state)
  ret |= (state & 0x000003ff) << 17;
  return ret;
}

static inline void _wrapperf_uncore_cha_init_ith(wrapperf_event_t* wpe,
                                                 uint64_t event,
                                                 int      cpu,
                                                 uint16_t filter_tid,
                                                 uint16_t filter_state,
                                                 int      i) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type    = 30 + i; // /sys/bus/event_source/devices/uncore_cha_0/type
  pe.config  = event;
  pe.config1 = _wrapperf_cha_filter_0(filter_tid, filter_state);

  _wrapperf_event_init(wpe, &pe, -1, cpu);
}

static inline void _wrapperf_l3_cache_miss_init(wrapperf_arch_t* wp) {
  int n_event = wp->n_socket * wp->n_cha;
  wp->l3_cache_miss_events = (wrapperf_event_t*)malloc(n_event * sizeof(wrapperf_event_t));

  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      int cpu = s * wp->n_cha; // not sure if it is portable; see /sys/bus/event_source/devices/uncore_cha_0/cpumask
      uint64_t event = 0x1134; // umask: 0x11, event: 0x34
      _wrapperf_uncore_cha_init_ith(wpe, event, cpu, 0, 0x1, i);
    }
  }
}

static inline void _wrapperf_l3_cache_miss_fini(wrapperf_arch_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      _wrapperf_event_fini(wpe);
    }
  }
  free(wp->l3_cache_miss_events);
}

static inline void _wrapperf_l3_cache_miss_start(wrapperf_arch_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      _wrapperf_event_start(wpe);
    }
  }
}

static inline void _wrapperf_l3_cache_miss_stop(wrapperf_arch_t* wp) {
  for (int i = 0; i < wp->n_cha; i++) {
    for (int s = 0; s < wp->n_socket; s++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      _wrapperf_event_stop(wpe);
    }
  }
}

static inline void _wrapperf_l3_cache_miss_print_all(wrapperf_arch_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      uint64_t c = _wrapperf_event_get_value(wpe);
      printf("Socket %d - CHA %-2d   |   L3 Cache Misses: %ld\n", s, i, c);
    }
  }
}

static inline void _wrapperf_l3_cache_miss_print_per_socket(wrapperf_arch_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    uint64_t c = 0;
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      c += _wrapperf_event_get_value(wpe);
    }
    printf("Socket %d   |   L3 Cache Misses: %ld\n", s, c);
  }
}

static inline uint64_t _wrapperf_l3_cache_miss_get_sum(wrapperf_arch_t* wp) {
  uint64_t c = 0;
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      c += _wrapperf_event_get_value(wpe);
    }
  }
  return c;
}

static inline void _wrapperf_l3_cache_miss_print_sum(wrapperf_arch_t* wp) {
  uint64_t c = _wrapperf_l3_cache_miss_get_sum(wp);
  printf("L3 Cache Misses: %ld\n", c);
}

/*
 * event monitoring for L1D stall cycle
 */

static inline void _wrapperf_l1d_stall_cycle_init_per_core(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_RAW;
  pe.config         = 0xc000ca3; // cycle_activity.stalls_l1d_miss
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

static inline void _wrapperf_l1d_stall_cycle_init(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_init(&wp->l1d_stall_cycle_events, wp->n_core,
                               _wrapperf_l1d_stall_cycle_init_per_core, "L1D Stall Cycle");
}

static inline void _wrapperf_l1d_stall_cycle_fini(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_fini(&wp->l1d_stall_cycle_events);
}

static inline void _wrapperf_l1d_stall_cycle_start(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_start(&wp->l1d_stall_cycle_events);
}

static inline void _wrapperf_l1d_stall_cycle_stop(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_stop(&wp->l1d_stall_cycle_events);
}

static inline void _wrapperf_l1d_stall_cycle_print_all(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->l1d_stall_cycle_events);
}

static inline void _wrapperf_l1d_stall_cycle_print_sum(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->l1d_stall_cycle_events);
}

static inline uint64_t _wrapperf_l1d_stall_cycle_get_ith(wrapperf_arch_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->l1d_stall_cycle_events, i);
}

static inline uint64_t _wrapperf_l1d_stall_cycle_get_sum(wrapperf_arch_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->l1d_stall_cycle_events);
}

/*
 * event monitoring for L2 stall cycle
 */

static inline void _wrapperf_l2_stall_cycle_init_per_core(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_RAW;
  pe.config         = 0x50005a3; // cycle_activity.stalls_l2_miss
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

static inline void _wrapperf_l2_stall_cycle_init(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_init(&wp->l2_stall_cycle_events, wp->n_core,
                               _wrapperf_l2_stall_cycle_init_per_core, "L2 Stall Cycle");
}

static inline void _wrapperf_l2_stall_cycle_fini(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_fini(&wp->l2_stall_cycle_events);
}

static inline void _wrapperf_l2_stall_cycle_start(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_start(&wp->l2_stall_cycle_events);
}

static inline void _wrapperf_l2_stall_cycle_stop(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_stop(&wp->l2_stall_cycle_events);
}

static inline void _wrapperf_l2_stall_cycle_print_all(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->l2_stall_cycle_events);
}

static inline void _wrapperf_l2_stall_cycle_print_sum(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->l2_stall_cycle_events);
}

static inline uint64_t _wrapperf_l2_stall_cycle_get_ith(wrapperf_arch_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->l2_stall_cycle_events, i);
}

static inline uint64_t _wrapperf_l2_stall_cycle_get_sum(wrapperf_arch_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->l2_stall_cycle_events);
}

/*
 * event monitoring for L3 stall cycle
 */

static inline void _wrapperf_l3_stall_cycle_init_per_core(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_RAW;
  pe.config         = 0x60006a3; // cycle_activity.stalls_l3_miss
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

static inline void _wrapperf_l3_stall_cycle_init(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_init(&wp->l3_stall_cycle_events, wp->n_core,
                               _wrapperf_l3_stall_cycle_init_per_core, "L3 Stall Cycle");
}

static inline void _wrapperf_l3_stall_cycle_fini(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_fini(&wp->l3_stall_cycle_events);
}

static inline void _wrapperf_l3_stall_cycle_start(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_start(&wp->l3_stall_cycle_events);
}

static inline void _wrapperf_l3_stall_cycle_stop(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_stop(&wp->l3_stall_cycle_events);
}

static inline void _wrapperf_l3_stall_cycle_print_all(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->l3_stall_cycle_events);
}

static inline void _wrapperf_l3_stall_cycle_print_sum(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->l3_stall_cycle_events);
}

static inline uint64_t _wrapperf_l3_stall_cycle_get_ith(wrapperf_arch_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->l3_stall_cycle_events, i);
}

static inline uint64_t _wrapperf_l3_stall_cycle_get_sum(wrapperf_arch_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->l3_stall_cycle_events);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WRAPPERF_SKYLAKE_H_ */
