ifeq ($(origin CC), default)
CC := gcc
# CC := icc
endif
CFLAGS  := -I../include -std=c99 -O3 -g $(CFLAGS)
LDFLAGS := -lm

# CFLAGS += -DWRAPPERF_ARCH=WRAPPERF_ARCH_SKYLAKE
CFLAGS += -DWRAPPERF_ARCH=WRAPPERF_ARCH_BROADWELL

# CFLAGS += -O0 -g

SOURCES := $(wildcard *.c)
TARGETS := $(patsubst %.c,%.out,$(SOURCES))

all: $(TARGETS)

%.out: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o *.out
