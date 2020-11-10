#pragma once
#ifndef WRAPPERF_COMMON_H_
#define WRAPPERF_COMMON_H_

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

typedef struct wrapperf_event {
  int      fd;
  uint64_t value;
  int      monitoring;
} wrapperf_event_t;

static inline int _wrapperf_raw_event_open(struct perf_event_attr* pe,
                                           pid_t                   pid,
                                           int                     cpu,
                                           int                     group_fd,
                                           unsigned long           flags) {
  int ret = syscall(__NR_perf_event_open, pe, pid, cpu, group_fd, flags);
  return ret;
}

static inline void _wrapperf_raw_event_attr_init(struct perf_event_attr* pe) {
  memset(pe, 0, sizeof(struct perf_event_attr));
  pe->size     = sizeof(struct perf_event_attr);
  pe->disabled = 1;
}


static inline uint64_t _wrapperf_raw_event_read(int fd) {
  uint64_t count;
  if (read(fd, &count, sizeof(uint64_t)) != sizeof(uint64_t)) {
    fprintf(stderr, "Failed to read perf events\n");
    exit(EXIT_FAILURE);
  }
  return count;
}

static inline void _wrapperf_event_init(wrapperf_event_t*       wpe,
                                        struct perf_event_attr* pe,
                                        pid_t                   pid,
                                        int                     cpu) {
  wpe->fd = _wrapperf_raw_event_open(pe, pid, cpu, -1, 0);
  if (wpe->fd == -1) {
    perror("perf_event_open");
    exit(EXIT_FAILURE);
  }
  wpe->value      = 0;
  wpe->monitoring = 0;
}

static inline void _wrapperf_event_fini(wrapperf_event_t* wpe) {
  ioctl(wpe->fd, PERF_EVENT_IOC_DISABLE, 0);
  close(wpe->fd);
}

static inline void _wrapperf_event_start(wrapperf_event_t* wpe) {
  ioctl(wpe->fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(wpe->fd, PERF_EVENT_IOC_ENABLE, 0);
  wpe->value = _wrapperf_raw_event_read(wpe->fd);
  if (wpe->monitoring) {
    fprintf(stderr, "Event monitoring has already been started.\n");
    exit(EXIT_FAILURE);
  }
  wpe->monitoring = 1;
}

static inline void _wrapperf_event_stop(wrapperf_event_t* wpe) {
  wpe->value = _wrapperf_raw_event_read(wpe->fd) - wpe->value;
  ioctl(wpe->fd, PERF_EVENT_IOC_DISABLE, 0);
  if (!wpe->monitoring) {
    fprintf(stderr, "Event monitoring is not started but stop is called.\n");
    exit(EXIT_FAILURE);
  }
  wpe->monitoring = 0;
}

static inline uint64_t _wrapperf_event_get_value(wrapperf_event_t* wpe) {
  if (wpe->monitoring) {
    fprintf(stderr, "Please call stop before getting a value.\n");
    exit(EXIT_FAILURE);
  }
  return wpe->value;
}

/*
 * For all-core events
 */

typedef struct wrapperf_allcore_event {
  int               n_core;
  wrapperf_event_t* events;
  const char*       event_name;
} wrapperf_allcore_event_t;

static inline void _wrapperf_allcore_event_init(wrapperf_allcore_event_t* wpae,
                                                int                       n_core,
                                                void (*init_per_core_fn)(wrapperf_event_t*, int),
                                                const char*               event_name) {
  wpae->n_core     = n_core;
  wpae->events     = (wrapperf_event_t*)malloc(n_core * sizeof(wrapperf_event_t));
  wpae->event_name = event_name;
  for (int i = 0; i < n_core; i++) {
    init_per_core_fn(&wpae->events[i], i);
  }
}

static inline void _wrapperf_allcore_event_fini(wrapperf_allcore_event_t* wpae) {
  for (int i = 0; i < wpae->n_core; i++) {
    _wrapperf_event_fini(&wpae->events[i]);
  }
  free(wpae->events);
}

static inline void _wrapperf_allcore_event_start(wrapperf_allcore_event_t* wpae) {
  for (int i = 0; i < wpae->n_core; i++) {
    _wrapperf_event_start(&wpae->events[i]);
  }
}

static inline void _wrapperf_allcore_event_stop(wrapperf_allcore_event_t* wpae) {
  for (int i = 0; i < wpae->n_core; i++) {
    _wrapperf_event_stop(&wpae->events[i]);
  }
}

static inline void _wrapperf_allcore_event_print_all(wrapperf_allcore_event_t* wpae) {
  for (int i = 0; i < wpae->n_core; i++) {
    uint64_t c = _wrapperf_event_get_value(&wpae->events[i]);
    printf("Core %d   |   %s: %ld\n", i, wpae->event_name, c);
  }
}

static inline void _wrapperf_allcore_event_print_sum(wrapperf_allcore_event_t* wpae) {
  uint64_t c = 0;
  for (int i = 0; i < wpae->n_core; i++) {
    c += _wrapperf_event_get_value(&wpae->events[i]);
  }
  printf("%s: %ld\n", wpae->event_name, c);
}

/*
 * Common events
 */

static inline void _wrapperf_cpu_cycle_init(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_HARDWARE;
  pe.config         = PERF_COUNT_HW_CPU_CYCLES;
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

static inline void _wrapperf_cpu_ref_cycle_init(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_HARDWARE;
  pe.config         = PERF_COUNT_HW_REF_CPU_CYCLES;
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

static inline void _wrapperf_l1d_cache_miss_init(wrapperf_event_t* wpe, int cpu) {
  struct perf_event_attr pe;
  _wrapperf_raw_event_attr_init(&pe);
  pe.type           = PERF_TYPE_HW_CACHE;
  pe.config         = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
  pe.exclude_kernel = 1;
  pe.exclude_hv     = 1;

  _wrapperf_event_init(wpe, &pe, 0, cpu);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WRAPPERF_COMMON_H_ */
