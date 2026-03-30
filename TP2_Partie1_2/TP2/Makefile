CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

TARGETS = servudp cliudp servbeuip clibeuip
 
all: $(TARGETS)
 
servudp: servudp.c
	$(CC) $(CFLAGS) -o servudp servudp.c
 
cliudp: cliudp.c
	$(CC) $(CFLAGS) -o cliudp cliudp.c
 
servbeuip: servbeuip.c
	$(CC) $(CFLAGS) -o servbeuip servbeuip.c
 
clibeuip: clibeuip.c
	$(CC) $(CFLAGS) -o clibeuip clibeuip.c
 
clean:
	rm -f $(TARGETS)