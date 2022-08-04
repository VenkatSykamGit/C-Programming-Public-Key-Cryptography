EXEC = keygen encrypt decrypt

CC = clang
CFLAGS = -Wall -Werror -Wextra -Wpedantic $(shell pkg-config --cflags gmp)
LFLAGS = $(shell pkg-config --libs gmp) 

all: $(EXEC)

keygen: keygen.o numtheory.o randstate.o rsa.o
	$(CC) -o $@ $^ $(LFLAGS)

encrypt: encrypt.o numtheory.o randstate.o rsa.o
	$(CC) $(LFLAGS) -o $@ $^

decrypt: decrypt.o numtheory.o randstate.o rsa.o
	$(CC) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o keygen encrypt decrypt

format:
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build make
