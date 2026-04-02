NAME := cutecc

SRC_DIR := src
SRCS	:= \
	cutecc.c \
	lexer.c
SRCS    := $(SRCS:%=$(SRC_DIR)/%)

BUILD_DIR := build
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

BIN_DIR := bin

CC		:= gcc
CFLAGS	:= -Wall -Wextra -ggdb

RM			:= rm -f
MAKEFLAGS	+= --no-print-directory
DIR_DUP     = mkdir -p $(@D)

all: $(BIN_DIR)/$(NAME)

$(BIN_DIR)/$(NAME): $(OBJS)
	$(DIR_DUP)
	$(CC) $(OBJS) -o $@
	$(info CREATED $@)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) -c -o $@ $<
	$(info CREATED $@)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(BIN_DIR)/$(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: clean fclean re
.SILENT:
