#pragma once
#ifndef WRAPPERF_WRAPPERF_H_
#define WRAPPERF_WRAPPERF_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wrapperf {
  int       n_socket;
  int       n_cha;
  int       n_l3_cache_miss_fd;
  int*      l3_cache_miss_fd;
  uint64_t* l3_cache_miss_count;
} wrapperf_t;

static inline int _wrapperf_event_open(struct perf_event_attr* hw_event,
                                       pid_t                   pid,
                                       int                     cpu,
                                       int                     group_fd,
                                       unsigned long           flags) {
  int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
  return ret;
}

static inline int _wrapperf_init_common(int cpu, struct perf_event_attr* pe) {
  int fd = _wrapperf_event_open(pe, -1, cpu, -1, 0);
  if (fd == -1) {
    perror("perf_event_open");
    exit(EXIT_FAILURE);
  }
  return fd;
}

static inline void _wrapperf_fini_common(int fd) {
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
  close(fd);
}

static inline void _wrapperf_start_common(int fd) {
  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
}

static inline void _wrapperf_stop_common(int fd) {
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
}

static inline uint64_t _wrapperf_read_value(int fd) {
  uint64_t count;
  if (read(fd, &count, sizeof(uint64_t)) != sizeof(uint64_t)) {
    fprintf(stderr, "Failed to read perf events\n");
    exit(1);
  }
  return count;
}

static inline void _wrapperf_event_attr_init(struct perf_event_attr* pe) {
  memset(pe, 0, sizeof(struct perf_event_attr));
  pe->size     = sizeof(struct perf_event_attr);
  pe->disabled = 1;
}

static inline int _wrapperf_init_raw(uint64_t event) {
  struct perf_event_attr pe;
  _wrapperf_event_attr_init(&pe);
  pe.type   = PERF_TYPE_RAW;
  pe.config = event;

  return _wrapperf_init_common(-1, &pe);
}

/*
 * Skylake
 */

static inline void _wrapperf_init(wrapperf_t* wp) {
  wp->n_socket = 2;
  wp->n_cha = 28;
}

static inline uint32_t _wrapperf_cha_filter_0(uint16_t tid, uint16_t state) {
  uint32_t ret = 0;
  // config1:0-8 (/sys/bus/event_source/devices/uncore_cha_0/format/filter_tid)
  ret |= (tid & 0x000001ff);
  // config1:17-26 (/sys/bus/event_source/devices/uncore_cha_0/format/filter_state)
  ret |= (state & 0x000003ff) << 17;
  return ret;
}

static inline int _wrapperf_uncore_cha_init_ith(uint64_t event,
                                                int      cpu,
                                                uint16_t filter_tid,
                                                uint16_t filter_state,
                                                int      i) {
  struct perf_event_attr pe;
  _wrapperf_event_attr_init(&pe);
  pe.type    = 30 + i; // /sys/bus/event_source/devices/uncore_cha_0/type
  pe.config  = event;
  pe.config1 = _wrapperf_cha_filter_0(filter_tid, filter_state);

  return _wrapperf_init_common(cpu, &pe);
}

static inline void _wrapperf_uncore_cha_init(wrapperf_t* wp) {
  wp->n_l3_cache_miss_fd  = wp->n_socket * wp->n_cha;
  wp->l3_cache_miss_fd    = (int*)malloc(wp->n_l3_cache_miss_fd * sizeof(int));
  wp->l3_cache_miss_count = (uint64_t*)malloc(wp->n_l3_cache_miss_fd * sizeof(uint64_t));

  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      int cpu = s * wp->n_cha; // not sure if it is portable; see /sys/bus/event_source/devices/uncore_cha_0/cpumask
      uint64_t event = 0x1134; // umask: 0x11, event: 0x34
      int fd = _wrapperf_uncore_cha_init_ith(event, cpu, 0, 0x1, i);
      int idx = s * wp->n_cha + i;
      wp->l3_cache_miss_fd[idx] = fd;
    }
  }
}

static inline void _wrapperf_uncore_cha_fini(wrapperf_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      int idx = s * wp->n_cha + i;
      _wrapperf_fini_common(wp->l3_cache_miss_fd[idx]);
    }
  }
  free(wp->l3_cache_miss_fd);
}

static inline void _wrapperf_uncore_cha_start(wrapperf_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      int idx = s * wp->n_cha + i;
      _wrapperf_start_common(wp->l3_cache_miss_fd[idx]);
      uint64_t c = _wrapperf_read_value(wp->l3_cache_miss_fd[idx]);
      wp->l3_cache_miss_count[idx] = c;
    }
  }
}

static inline void _wrapperf_uncore_cha_stop(wrapperf_t* wp) {
  for (int i = 0; i < wp->n_cha; i++) {
    for (int s = 0; s < wp->n_socket; s++) {
      int idx = s * wp->n_cha + i;
      uint64_t c = _wrapperf_read_value(wp->l3_cache_miss_fd[idx]);
      wp->l3_cache_miss_count[idx] = c - wp->l3_cache_miss_count[idx];
      _wrapperf_stop_common(wp->l3_cache_miss_fd[idx]);
    }
  }
}

static inline void _wrapperf_uncore_cha_print_all(wrapperf_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      int idx = s * wp->n_cha + i;
      uint64_t c = wp->l3_cache_miss_count[idx];
      printf("Socket %d - CHA %-2d   |   L3 Cache Misses: %ld\n", s, i, c);
    }
  }
}

static inline void _wrapperf_uncore_cha_print_per_socket(wrapperf_t* wp) {
  for (int s = 0; s < wp->n_socket; s++) {
    uint64_t c = 0;
    for (int i = 0; i < wp->n_cha; i++) {
      int idx = s * wp->n_cha + i;
      c += wp->l3_cache_miss_count[idx];
    }
    printf("Socket %d   |   L3 Cache Misses: %ld\n", s, c);
  }
}

static inline void _wrapperf_uncore_cha_print_sum(wrapperf_t* wp) {
  uint64_t c = 0;
  for (int s = 0; s < wp->n_socket; s++) {
    for (int i = 0; i < wp->n_cha; i++) {
      int idx = s * wp->n_cha + i;
      c += wp->l3_cache_miss_count[idx];
    }
  }
  printf("L3 Cache Misses: %ld\n", c);
}

/*
 * Public API
 */

static inline void wrapperf_init(wrapperf_t* wp) {
  _wrapperf_init(wp);
}

static inline void wrapperf_fini(wrapperf_t* wp) {
}

static inline void wrapperf_l3_cache_miss_init(wrapperf_t* wp) {
  _wrapperf_uncore_cha_init(wp);
}

static inline void wrapperf_l3_cache_miss_fini(wrapperf_t* wp) {
  _wrapperf_uncore_cha_fini(wp);
}

static inline void wrapperf_l3_cache_miss_start(wrapperf_t* wp) {
  _wrapperf_uncore_cha_start(wp);
}

static inline void wrapperf_l3_cache_miss_stop(wrapperf_t* wp) {
  _wrapperf_uncore_cha_stop(wp);
}

static inline void wrapperf_l3_cache_miss_print_all(wrapperf_t* wp) {
  _wrapperf_uncore_cha_print_all(wp);
}

static inline void wrapperf_l3_cache_miss_print_per_socket(wrapperf_t* wp) {
  _wrapperf_uncore_cha_print_per_socket(wp);
}

static inline void wrapperf_l3_cache_miss_print_sum(wrapperf_t* wp) {
  _wrapperf_uncore_cha_print_sum(wp);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WRAPPERF_WRAPPERF_H_ */
