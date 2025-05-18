# programs
DOXYGEN = doxygen
CC      = gcc

export DOXYGEN
export CC

# paths
PREFIX  = /usr
DISTDIR = $(abspath dist)
INCDIR  = $(abspath inc)
MANDIR  = $(DISTDIR)/man/man3

export PREFIX
export DISTDIR
export INCDIR
export MANDIR

# sources & objects
CSRCS  = $(shell find src/ -type f -name '*.c')
SSRCS  = $(shell find src/ -type f -name '*.S')
OBJS   = $(patsubst src/%.c,$(DISTDIR)/%.c.o,$(CSRCS))
OBJS  += $(patsubst src/%.S,$(DISTDIR)/%.S.o,$(SSRCS))
HDRS   = $(wildcard inc/*.h)

# dirs
SRCDIRS = $(shell find src/* -type d)
OBJDIRS = $(patsubst src/%,$(DISTDIR)/%,$(SRCDIRS))

# optimization flags
CFLAGS   = -O3 -march=native

# memory protection flags
CFLAGS  += -fstack-protector-strong -fcf-protection=full
CFLAGS  += -fstack-clash-protection

# get rid of 'missing .note.GNU-stack section' warning
CFLAGS  += -z noexecstack

# other flags
CFLAGS  += -Wall -Wextra -Werror -std=gnu99 -pedantic
INCLUDE  = -I./inc
LIBS     = -lpthread

# compile time options
CTORM_DEBUG        = 0
CTORM_JSON_SUPPORT = 1

ifeq ($(CTORM_JSON_SUPPORT), 1)
LIBS += -lcjson
endif

all: $(DISTDIR)/libctorm.so

$(DISTDIR)/libctorm.so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LIBS) $(CFLAGS)

$(DISTDIR)/%.c.o: src/%.c $(HDRS)
	@mkdir -pv $(OBJDIRS)
	$(CC) $(CFLAGS) $(INCLUDE) -c -Wall -fPIC -o $@ $< $(LIBS) \
		-DCTORM_JSON_SUPPORT=$(CTORM_JSON_SUPPORT)               \
		-DCTORM_DEBUG=$(CTORM_DEBUG)

$(DISTDIR)/%.S.o: src/%.S $(HDRS)
	@mkdir -pv $(OBJDIRS)
	$(CC) $(CFLAGS) $(INCLUDE) -c -Wall -fPIC -o $@ $< $(LIBS) \
		-DCTORM_JSON_SUPPORT=$(CTORM_JSON_SUPPORT)               \
		-DCTORM_DEBUG=$(CTORM_DEBUG)

install:
ifeq (,$(wildcard $(DISTDIR)/libctorm.so))
	@$(error you should first compile libctorm)
endif
	install -Dm755  $(DISTDIR)/libctorm.so  $(DESTDIR)/$(PREFIX)/lib/libctorm.so
	install -dm655  $(DESTDIR)/$(PREFIX)/include/ctorm
	for header in $(HDRS); do \
		install -m644  $$header $(DESTDIR)/$(PREFIX)/include/ctorm; \
	done
ifneq (,$(wildcard $(MANDIR)))
	for man in $(MANDIR)/*; do \
		install -Dm644 $$man $(DESTIDR)/$(PREFIX)/share/man/man3; \
	done
endif

uninstall:
	rm -vf $(DESTDIR)/$(PREFIX)/lib/libctorm.so
	rm -vrf $(DESTDIR)/$(PREFIX)/include/ctorm
	find $(DESTDIR)/$(PREFIX)/share/man/man3 \
		-type f \
		-name 'ctorm*' \
		-exec rm -v {} \;

format:
	clang-format -i -style=file $(CSRCS) $(HDRS) example/*/*.c
	black -l 80 scripts/*.py

check:
	clang-format -n --Werror -style=file $(CSRCS) $(HDRS) example/*/*.c
	black -l 80 --check scripts/*.py

lint:
	clang-tidy --warnings-as-errors --config= $(CSRCS) $(HDRS)

clean:
	rm -rf $(DISTDIR)

docs:
	$(DOXYGEN)
	cd $(MANDIR) && \
		find * -type f -not -name 'ctorm*' -exec mv -v {} ctorm_{} \;

example:
	$(MAKE) -C $@

test:
	make example
	./scripts/test.sh

.PHONY: install uninstall docs format check lint clean example test
