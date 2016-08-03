TARGET = nes
CC=gcc
SRCS := $(wildcard src/*.c)
INC_DIR = inc
OBJ_DIR = obj
BIN_DIR = bin
SRC_DIR = src
OBJS_C := $(addprefix obj/,$(notdir $(SRCS:.c=.o)))
CFLAGS=-Wall -std=c11 -g
LFLAGS=-lSDL2 -std=c11 -g

$(BIN_DIR)/$(TARGET): $(OBJS_C)
	$(CC) -o $@  $(OBJS_C) $(LFLAGS)

$(OBJS_C): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
.PHONY: clean
clean:
	-@$(RM) -f $(OBJ_DIR)/*.o
	-@$(RM) -f $(BIN_DIR)/nes
