# WARN: you should run the example binaries with LD_LIBRARY_PATH=../dist

CC   = gcc
INCD = ../inc
DIST = ../dist

DIRS = $(shell find * -maxdepth 0 -type d)
BINS = $(patsubst %,$(DIST)/example_%,$(DIRS))

all: $(BINS)

$(DIST)/libctorm.so:
ifeq (,$(wildcard $(DIST)/libctorm.so))
	$(error you should first compile libctorm)
endif

$(DIST)/example_%: %/main.c $(DIST)/libctorm.so
	@mkdir -pv $(DIST)
	$(CC) -I$(INCD) -L$(DIST) $< -lctorm -lcjson -o $@
