PREFIX = /usr
CC = gcc

# sources
SRCS   = $(shell find src/ -type f -name '*.c')
OBJS   = $(patsubst src/%.c,dist/%.o,$(SRCS))
HDRS   = $(wildcard include/*.h)

# compiler flags
CFLAGS = -O3 -march=native -fstack-protector-strong -fcf-protection=full -fstack-clash-protection
LIBS   = -lpthread

# options
CTORM_DEBUG = 0
CTORM_JSON_SUPPORT = 1

ifeq ($(CTORM_JSON_SUPPORT), 1)
	LIBS += -lcjson
endif

all: dist dist/libctorm.so

dist:
	mkdir -pv dist/encoding

dist/libctorm.so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LIBS) $(CFLAGS)

dist/%.o: src/%.c
	$(CC) -c -Wall -fPIC -o $@ $^ $(LIBS) $(CFLAGS) \
		-DCTORM_JSON_SUPPORT=$(CTORM_JSON_SUPPORT)    \
		-DCTORM_DEBUG=$(CTORM_DEBUG)

install:
	install -m755 dist/libctorm.so $(DESTDIR)$(PREFIX)/lib/libctorm.so
	mkdir -pv $(DESTDIR)/usr/include/ctorm
	cp $(HDRS) $(DESTDIR)/usr/include/ctorm

uninstall:
	rm $(DESTDIR)$(PREFIX)/lib/libctorm.so
	rm -r $(DESTDIR)/usr/include/ctorm

format:
	clang-format -i -style=file $(SRCS) $(HDRS) example/*/*.c

clean:
	rm -rf dist

example:
	$(MAKE) -C $@

.PHONY: test install uninstall format clean example
