# paths & programs
PREFIX  = /usr
DISTDIR = dist
CC      = gcc

# sources
CSRCS = $(shell find src/ -type f -name '*.c')
SSRCS = $(shell find src/ -type f -name '*.S')
OBJS  = $(patsubst src/%.c,$(DISTDIR)/%.c.o,$(CSRCS))
OBJS += $(patsubst src/%.S,$(DISTDIR)/%.S.o,$(SSRCS))
HDRS = $(wildcard inc/*.h)

# dirs
SRCDIRS = $(shell find src/* -type d)
OBJDIRS = $(patsubst src/%,$(DISTDIR)/%,$(SRCDIRS))

# compiler flags
CFLAGS  = -O3 -march=native -fstack-protector-strong -fcf-protection=full -fstack-clash-protection
INCLUDE = -I./inc
LIBS    = -lpthread

# options
CTORM_DEBUG        = 0
CTORM_JSON_SUPPORT = 1

ifeq ($(CTORM_JSON_SUPPORT), 1)
	LIBS += -lcjson
endif

all: $(DISTDIR)/libctorm.so

dist/libctorm.so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LIBS) $(CFLAGS)

$(DISTDIR)/%.c.o: src/%.c $(OBJDIRS)
	$(CC) $(CFLAGS) $(INCLUDE) -c -Wall -fPIC -o $@ $< $(LIBS) \
		-DCTORM_JSON_SUPPORT=$(CTORM_JSON_SUPPORT)               \
		-DCTORM_DEBUG=$(CTORM_DEBUG)

$(DISTDIR)/%.S.o: src/%.S $(OBJDIRS)
	$(CC) $(CFLAGS) $(INCLUDE) -c -Wall -fPIC -o $@ $< $(LIBS) \
		-DCTORM_JSON_SUPPORT=$(CTORM_JSON_SUPPORT)               \
		-DCTORM_DEBUG=$(CTORM_DEBUG)

$(OBJDIRS):
	@mkdir -pv $@

install:
	install -Dm755 $(DISTDIR)/libctorm.so $(DESTDIR)/$(PREFIX)/lib/libctorm.so
	install -Dm644 inc/ctorm.h            $(DESTDIR)/$(PREFIX)/include/ctorm.h

uninstall:
	rm -vf $(DESTDIR)/$(PREFIX)/lib/libctorm.so
	rm -vf $(DESTDIR)/$(PREFIX)/include/ctorm.h

format:
	clang-format -i -style=file $(SRCS) $(HDRS) example/*/*.c
	black scripts/*.py

clean:
	rm -rf dist

example:
	$(MAKE) -C $@

.PHONY: install uninstall format clean example
