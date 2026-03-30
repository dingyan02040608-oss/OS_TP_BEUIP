CC = gcc
CFLAGS = -Wall -Wextra -Werror -g
LDFLAGS = -lreadline
 
EXEC = biceps
SERV = servbeuip
 
SRC_BICEPS = bicep.c gescom.c creme.c
OBJ_BICEPS = $(SRC_BICEPS:.c=.o)
 
ifdef TRACE
CFLAGS += -DTRACE
endif
 

all: $(EXEC) $(SERV)
 
$(EXEC): $(OBJ_BICEPS)
	$(CC) -o $(EXEC) $(OBJ_BICEPS) $(LDFLAGS)
 
$(SERV): servbeuip.c
	$(CC) $(CFLAGS) -o $(SERV) servbeuip.c
 
%.o: %.c
	$(CC) $(CFLAGS) -c $<
 
clean:
	rm -f $(OBJ_BICEPS) $(EXEC) $(SERV)
 
check:
	@echo "--- Symboles exportés par creme.o ---"
	nm creme.o | grep ' T '
	@echo "--------------------------------------"
 
.PHONY: all clean check