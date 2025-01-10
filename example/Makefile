CC = gcc

# you should run the binaries with LD_LIBRARY_PATH=../dist

all: ../dist/example_hello ../dist/example_echo ../dist/example_middleware

../dist/libctorm.so:
ifeq (,$(wildcard ../dist/libctorm.so))
	@echo "!!!! you should first compile libctorm !!!!"
	exit 1
endif

../dist/example_hello: hello/*.c ../dist/libctorm.so
	mkdir -pv ../dist
	$(CC) -I../inc -L../dist hello/*.c -lctorm -o $@

../dist/example_echo: echo/*.c ../dist/libctorm.so
	mkdir -pv ../dist
	$(CC) -I../inc -L../dist echo/*.c -lctorm -o $@

../dist/example_middleware: middleware/*.c ../dist/libctorm.so
	mkdir -pv ../dist
	$(CC) -I../inc -L../dist middleware/*.c -lctorm -lcjson -o $@
