CC = gcc
CFLAGS = -std=c99

SRC = main.c
TARGET = bf

export BF_INTERPRETER = ${TARGET}

# CFLAGS += -g
CFLAGS += -O3 -s -D_NDEBUG

all: ${TARGET}

${TARGET}: ${SRC} *.h
	${CC} ${CFLAGS} ${SRC} -o $@

.PHONY: clean
clean:
	rm ${TARGET}

.PHONY: test
test:
	@make -C test
