CC = gcc
CFLAGS = -Wall -Werror
OBJS = dns.o dns_lib.o
PROG = dns
HEADER = dns_lib.h

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

dns.o: $(HEADER)
dns_lib.o: $(HEADER)

clean:
	rm *.o $(PROG)

test: all
	python3 dns_testing.py


tar: all
	tar -cf xnekul04.tar tests dns.c dns_lib.h dns_lib.c dns_testing.py Makefile manual.pdf README
