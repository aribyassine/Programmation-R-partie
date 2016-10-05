#			Programmation Repartie (4I400)
#		Makefile pour TM 1. Fichiers et Processus


# Documentation: http://www.gnu.org/software/make/manual/make.html
CC =gcc
LDFLAGS =
CFLAGS =-W -Wall -ansi -pedantic -Iinclude

DIR=.
BIN=$(DIR)/bin/
OBJ=$(DIR)/obj/
INCLUDE=$(DIR)/include/
LIB=$(DIR)/lib/
SRC=$(DIR)/src/

HC=
O=

.SUFFIXES:
.PHONY: all clean test test-mycp test-remonte test-lectures
all: $(BIN)mycp $(BIN)remonte $(BIN)lectures $(OBJ)forkfile.o

test: test-mycp test-remonte test-lectures

test-mycp: $(BIN)mycp
	-$$PWD/bin/mycp src/mycp.c cp.c

test-remonte: $(BIN)remonte
	-$$PWD/bin/remonte 8 aleas

test-lectures: $(BIN)lectures
	-$$PWD/bin/lectures -p src/lectures.c; $$PWD/bin/lectures -C src/lectures.c

clean: 
	rm -rf $(OBJ)*.o $(BIN)*

$(BIN)%: $(OBJ)%.o $O
	@if [ -d $(BIN) ]; then : ; else mkdir $(BIN); fi
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ)%.o: $(SRC)%.c $(HC)
	@if [ -d $(OBJ) ]; then : ; else mkdir $(OBJ); fi
	$(CC) $(CFLAGS) -o $@ -c $<

$(INCLUDE)%.h:
	@if [ -d $(INCLUDE) ]; then : ; else mkdir $(INCLUDE); fi

