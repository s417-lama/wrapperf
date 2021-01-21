#pragma once
#ifndef WRAPPERF_BROADWELL_H_
#define WRAPPERF_BROADWELL_H_

#include "wrapperf/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wrapperf_arch {
  int               n_core;
  int               n_socket;
  int               n_cbox;

  wrapperf_allcore_event_t l2_cache_miss_events;
  wrapperf_event_t*        l3_cache_miss_events;

  wrapperf_allcore_event_t total_stall_cycle_events;
  wrapperf_allcore_event_t mem_stall_cycle_events;
  wrapperf_allcore_event_t l1d_stall_cycle_events;
  wrapperf_allcore_event_t l2_stall_cycle_events;
  wrapperf_allcore_event_t l3_stall_cycle_events;
} wrapperf_arch_t;

static inline void _wrapperf_init(wrapperf_arch_t* wp) {
  wp->n_core = 96;
  wp->n_socket = 4;
  wp->n_cbox = 24;
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
 * uncore_cbox event monitoring for L3 cache misses
 */

static inline uint32_t _wrapperf_cbox_filter_0(uint16_t tid, uint16_t state) {
  uint32_t ret = 0;
  // config1:0-5 (/sys/bus/event_source/devices/uncore_cbox_0/format/filter_tid)
  ret |= (tid & 0x0000001f);
  // config1:17-23 (/sys/bus/event_source/devices/uncore_cbox_0/format/filter_state)
  ret |= (state & 0x0000007f) << 17;
  return ret;
}

static inline void _wrapperf_uncore_cbox_init_ith(wrapperf_event_t* wpe,
                                                  uint64_t event,
                                                  int      cpu,
                                                  uint16_t filter_tid,
                                                  uint16_t filter_state,
                                                  int      i) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type    = 26 + i; // /sys/bus/event_source/devices/uncore_cbox_0/type
  pe.config  = event;
  pe.config1 = _wrapperf_cbox_filter_0(filter_tid, filter_state);

  _wrapperf_event_init(wpe, &pe, -1, cpu);
}

static inline void _wrapperf_l3_cache_miss_init(wrapperf_arch_t* wp) {
  int n_event = wp->n_socket * wp->n_cbox;
  wp->l3_cache_miss_events = (wrapperf_event_t*)malloc(n_event * sizeof(wrapperf_event_t));

  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cbox; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cbox + i];
      int cpu = s * wp->n_cbox; // not sure if it is portable; see /sys/bus/event_source/devices/uncore_cbox_0/cpumask
      uint64_t event = 0x1134; // umask: 0x11, event: 0x34
      _wrapperf_uncore_cbox_init_ith(wpe, event, cpu, 0, 0x1, i);
    }
  }
}

static inline void _wrapperf_l3_cache_miss_fini(wrapperf_arch_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cbox; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cbox + i];
      _wrapperf_event_fini(wpe);
    }
  }
  free(wp->l3_cache_miss_events);
}

static inline void _wrapperf_l3_cache_miss_start(wrapperf_arch_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cbox; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cbox + i];
      _wrapperf_event_start(wpe);
    }
  }
}

static inline void _wrapperf_l3_cache_miss_stop(wrapperf_arch_t* wp) {
  for (int i = 0; i < wp->n_cbox; i++) {
    for (int s = 0; s < wp->n_socket; s++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cbox + i];
      _wrapperf_event_stop(wpe);
    }
  }
}

static inline void _wrapperf_l3_cache_miss_print_all(wrapperf_arch_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cbox; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cbox + i];
      uint64_t c = _wrapperf_event_get_value(wpe);
      printf("Socket %d - CBOX %-2d   |   L3 Cache Misses: %ld\n", s, i, c);
    }
  }
}

static inline void _wrapperf_l3_cache_miss_print_per_socket(wrapperf_arch_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    uint64_t c = 0;
    for (int i = 0; i < wp->n_cbox; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cbox + i];
      c += _wrapperf_event_get_value(wpe);
    }
    printf("Socket %d   |   L3 Cache Misses: %ld\n", s, c);
  }
}

static inline uint64_t _wrapperf_l3_cache_miss_get_sum(wrapperf_arch_t* wp) {
  uint64_t c = 0;
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cbox; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cbox + i];
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
 * event monitoring for total stall cycle
 */

static inline void _wrapperf_total_stall_cycle_init_per_core(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_RAW;
  pe.config         = 0x40004a3; // cycle_activity.stalls_total
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

static inline void _wrapperf_total_stall_cycle_init(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_init(&wp->total_stall_cycle_events, wp->n_core,
                               _wrapperf_total_stall_cycle_init_per_core, "Total Stall Cycle");
}

static inline void _wrapperf_total_stall_cycle_fini(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_fini(&wp->total_stall_cycle_events);
}

static inline void _wrapperf_total_stall_cycle_start(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_start(&wp->total_stall_cycle_events);
}

static inline void _wrapperf_total_stall_cycle_stop(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_stop(&wp->total_stall_cycle_events);
}

static inline void _wrapperf_total_stall_cycle_print_all(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->total_stall_cycle_events);
}

static inline void _wrapperf_total_stall_cycle_print_sum(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->total_stall_cycle_events);
}

static inline uint64_t _wrapperf_total_stall_cycle_get_ith(wrapperf_arch_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->total_stall_cycle_events, i);
}

static inline uint64_t _wrapperf_total_stall_cycle_get_sum(wrapperf_arch_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->total_stall_cycle_events);
}

/*
 * event monitoring for stall cycle in any memory subsystem
 */

static inline void _wrapperf_mem_stall_cycle_init_per_core(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_RAW;
  pe.config         = 0x60006a3; // cycle_activity.stalls_mem_any
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

static inline void _wrapperf_mem_stall_cycle_init(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_init(&wp->mem_stall_cycle_events, wp->n_core,
                               _wrapperf_mem_stall_cycle_init_per_core, "Mem Stall Cycle");
}

static inline void _wrapperf_mem_stall_cycle_fini(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_fini(&wp->mem_stall_cycle_events);
}

static inline void _wrapperf_mem_stall_cycle_start(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_start(&wp->mem_stall_cycle_events);
}

static inline void _wrapperf_mem_stall_cycle_stop(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_stop(&wp->mem_stall_cycle_events);
}

static inline void _wrapperf_mem_stall_cycle_print_all(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_all(&wp->mem_stall_cycle_events);
}

static inline void _wrapperf_mem_stall_cycle_print_sum(wrapperf_arch_t* wp) {
  _wrapperf_allcore_event_print_sum(&wp->mem_stall_cycle_events);
}

static inline uint64_t _wrapperf_mem_stall_cycle_get_ith(wrapperf_arch_t* wp, int i) {
  return _wrapperf_allcore_event_get_ith(&wp->mem_stall_cycle_events, i);
}

static inline uint64_t _wrapperf_mem_stall_cycle_get_sum(wrapperf_arch_t* wp) {
  return _wrapperf_allcore_event_get_sum(&wp->mem_stall_cycle_events);
}

/*
 * event monitoring for L1D stall cycle
 */

static inline void _wrapperf_l1d_stall_cycle_init_per_core(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_RAW;
  pe.config         = 0xc000ca3; // cycle_activity.stalls_l1d_pending
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
  pe.config         = 0x50005a3; // cycle_activity.stalls_l2_pending
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
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

static inline void _wrapperf_l3_stall_cycle_init(wrapperf_arch_t* wp) {
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

static inline void _wrapperf_l3_stall_cycle_fini(wrapperf_arch_t* wp) {
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

static inline void _wrapperf_l3_stall_cycle_start(wrapperf_arch_t* wp) {
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

static inline void _wrapperf_l3_stall_cycle_stop(wrapperf_arch_t* wp) {
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

static inline void _wrapperf_l3_stall_cycle_print_all(wrapperf_arch_t* wp) {
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

static inline void _wrapperf_l3_stall_cycle_print_sum(wrapperf_arch_t* wp) {
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

static inline uint64_t _wrapperf_l3_stall_cycle_get_ith(wrapperf_arch_t* wp, int i) {
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

static inline uint64_t _wrapperf_l3_stall_cycle_get_sum(wrapperf_arch_t* wp) {
  fprintf(stderr, "Broadwell does not support L3 stall cycle count.\n");
  exit(EXIT_FAILURE);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WRAPPERF_BROADWELL_H_ */
