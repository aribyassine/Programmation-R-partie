#	Programmation Repartie (4I400)
#	Makefile pour le projet http.

CC =gcc
LDFLAGS = -lpthread -lrt
CFLAGS =-W -Wall -ansi -pedantic -Iinclude -std=c99

DIR=.
BIN=$(DIR)/bin/
OBJ=$(DIR)/obj/
INCLUDE=$(DIR)/include/
LIB=$(DIR)/lib/
SRC=$(DIR)/src/

HC=include/util.h include/mime.h include/log.h
O=$(OBJ)util.o $(OBJ)mime.o $(OBJ)log.o


.SUFFIXES:
.PHONY: all clean

all: $(BIN)httpd

test:
	-$(BIN)httpd

clean:
	rm -rf $(OBJ)*.o $(BIN)*


$(BIN)%: $(OBJ)%.o $O
	@if [ -d $(BIN) ]; then : ; else mkdir $(BIN); fi
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ)%.o: $(SRC)%.c $(HC)
	@if [ -d $(OBJ) ]; then : ; else mkdir $(OBJ); fi
	$(CC) -o $@ -c $< $(CFLAGS)

$(INCLUDE)%.h:
	@if [ -d $(INCLUDE) ]; then : ; else mkdir $(INCLUDE); fi
