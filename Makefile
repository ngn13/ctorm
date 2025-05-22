# architecture
ARCH = $(shell uname -m)

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
ifeq ($(ARCH),$(filter $(ARCH),x86_64 x86-64 amd64))
  SSRCS  = $(shell find src/amd64 -type f -name '*.S')
else ifeq ($(ARCH),$(filter $(ARCH),i386 386))
  SSRCS  = $(shell find src/i386 -type f -name '*.S')
else
  $(error unsupported architecture: $(ARCH))
endif
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

docs:
	$(DOXYGEN)
	cd $(MANDIR) && \
		find * -type f -not -name 'ctorm*' -exec mv -v {} ctorm_{} \;

clean:
	rm -rf $(DISTDIR)

install:
ifeq (,$(wildcard $(DISTDIR)/libctorm.so))
	@$(error you should first compile libctorm)
endif
	install -Dm755  $(DISTDIR)/libctorm.so  $(DESTDIR)/$(PREFIX)/lib/libctorm.so
	install -dm655  $(DESTDIR)/$(PREFIX)/include/ctorm
	for header in $(HDRS); do \
		install -Dm644  $$header $(DESTDIR)/$(PREFIX)/include/ctorm; \
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
	black -q -l 80 scripts/*.py

example: $(DISTDIR)/libctorm.so
	$(MAKE) -C $@

test: example
	bash ./scripts/test.sh

check_scripts:
	# run check scripts
	@for script in scripts/check_*.sh; do \
		echo "running check script: $$script"; \
		bash $$script; \
	done

check_lint:
	# check for lint errors
	clang-tidy --warnings-as-errors --config= $(CSRCS) $(HDRS) -- $(INCLUDE)

check_format:
	# check formatting
	clang-format -n --Werror -style=file $(CSRCS) $(HDRS) example/*/*.c
	black -q -l 80 --check scripts/*.py

check: check_scripts check_lint check_format

.PHONY: docs clean install uninstall format example test \
	check_scripts check_lint check_format check
