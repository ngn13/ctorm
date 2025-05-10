# WARN: you should run the example binaries with LD_LIBRARY_PATH=../dist

DIRS = $(shell find * -maxdepth 0 -type d)
BINS = $(patsubst %,$(DISTDIR)/example_%,$(DIRS))

all: $(BINS)

$(DISTDIR)/libctorm.so:
ifneq ($(DONT_CHECK_LIBCTORM), 1)
	ifeq (,$(wildcard $(DISTDIR)/libctorm.so))
		$(error you should first compile libctorm)
	endif
endif

$(DISTDIR)/example_%: %/main.c $(DISTDIR)/libctorm.so
	@mkdir -pv $(DISTDIR)
	$(CC) -I$(INCDIR) -L$(DISTDIR) $< -lctorm -lcjson -o $@
