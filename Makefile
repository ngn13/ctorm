PREFIX = /usr
CC = gcc

DEBUG  = 0
SRCS   = $(wildcard src/*.c)
OBJS   = $(patsubst src/%.c,dist/%.o,$(SRCS))
HDRS   = $(wildcard include/*.h) 
CFLAGS = -O3 -march=native -fstack-protector-strong -fcf-protection=full -fstack-clash-protection
LIBS   = -lpthread -levent

dist/libctorm.so: $(OBJS) 
	mkdir -p dist
	$(CC) -shared -o $@ $^ $(LIBS) $(CFLAGS) 

dist/%.o: src/%.c 
	mkdir -p dist
	$(CC) -c -Wall -fPIC -o $@ $^ $(LIBS) $(CFLAGS) -DDEBUG=${DEBUG}

install:
	install -m755 dist/libctorm.so $(DESTDIR)$(PREFIX)/lib/libctorm.so
	mkdir -pv $(DESTDIR)/usr/include/ctorm
	cp $(HDRS) $(DESTDIR)/usr/include/ctorm

uninstall:
	rm $(DESTDIR)$(PREFIX)/lib/libctorm.so
	rm -r $(DESTDIR)/usr/include/ctorm

format:
	clang-format -i -style=file src/*.c include/*.h example/*/*.c

.PHONY: test install uninstall format 
