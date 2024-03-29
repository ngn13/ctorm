PREFIX = /usr
CC = gcc

DEBUG  = 0
SRCS   = $(wildcard src/*.c)
OBJS   = $(patsubst src/%.c,dist/%.o,$(SRCS))
HDRS   = $(wildcard include/*.h) 
CFLAGS = -O3 -march=native
LIBS   = -levent -lpthread

dist/libctorm.so: $(OBJS) 
	mkdir -p dist
	$(CC) -shared -o $@ $^ $(LIBS) $(CFLAGS) 

dist/%.o: src/%.c 
	mkdir -p dist
	$(CC) -c -Wall -fPIC -o $@ $^ $(LIBS) $(CFLAGS) -DDEBUG=${DEBUG}

test:
	$(CC) -L./dist example/main.c -lctorm -o dist/example
	LD_LIBRARY_PATH=./dist dist/example

install:
	install -m755 dist/libctorm.so $(DESTDIR)$(PREFIX)/lib/libctorm.so
	mkdir -pv $(DESTDIR)/usr/include/ctorm
	cp $(HDRS) $(DESTDIR)/usr/include/ctorm

uninstall:
	rm $(DESTDIR)$(PREFIX)/lib/libctorm.so
	rm -r $(DESTDIR)/usr/include/ctorm

.PHONY: test install uninstall 
