#pragma once
#ifndef WRAPPERF_SKYLAKE_H_
#define WRAPPERF_SKYLAKE_H_

#include "wrapperf/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wrapperf_skylake {
  int               n_socket;
  int               n_cha;
  wrapperf_event_t* l3_cache_miss_events;
} wrapperf_skylake_t;

static inline void _wrapperf_init(wrapperf_skylake_t* wp) {
  wp->n_socket = 2;
  wp->n_cha = 28;
}

/*
 * uncore_cha performance monitoring for L3 cache misses
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

static inline void _wrapperf_uncore_cha_init(wrapperf_skylake_t* wp) {
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

static inline void _wrapperf_uncore_cha_fini(wrapperf_skylake_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      _wrapperf_event_fini(wpe);
    }
  }
  free(wp->l3_cache_miss_events);
}

static inline void _wrapperf_uncore_cha_start(wrapperf_skylake_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      _wrapperf_event_start(wpe);
    }
  }
}

static inline void _wrapperf_uncore_cha_stop(wrapperf_skylake_t* wp) {
  for (int i = 0; i < wp->n_cha; i++) {
    for (int s = 0; s < wp->n_socket; s++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      _wrapperf_event_stop(wpe);
    }
  }
}

static inline void _wrapperf_uncore_cha_print_all(wrapperf_skylake_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      uint64_t c = _wrapperf_event_get_value(wpe);
      printf("Socket %d - CHA %-2d   |   L3 Cache Misses: %ld\n", s, i, c);
    }
  }
}

static inline void _wrapperf_uncore_cha_print_per_socket(wrapperf_skylake_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    uint64_t c = 0;
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      c += _wrapperf_event_get_value(wpe);
    }
    printf("Socket %d   |   L3 Cache Misses: %ld\n", s, c);
  }
}

static inline void _wrapperf_uncore_cha_print_sum(wrapperf_skylake_t* wp) {
  uint64_t c = 0;
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      wrapperf_event_t* wpe = &wp->l3_cache_miss_events[s * wp->n_cha + i];
      c += _wrapperf_event_get_value(wpe);
    }
  }
  printf("L3 Cache Misses: %ld\n", c);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WRAPPERF_SKYLAKE_H_ */
