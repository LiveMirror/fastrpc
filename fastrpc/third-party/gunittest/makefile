CC=g++
DEBUG = -D_USE_HTTP_PROTO_
ROOT_DIR	= $(shell pwd)
SRC_DIR		= $(ROOT_DIR)
SRC_FILES  := $(shell find $(SRC_DIR) -name '*.cc')
OBJ_FILES  := $(SRC_FILES:.cc=.o)

CFLAGS= -Wall -DUNIX $(DEBUG) -Wno-deprecated -D_REENTRANT
INC_DIR+=-I./

LIB_NAME   := gtest
LIB_SUFFIX := .a
LIB_TARGET := lib$(LIB_NAME)$(LIB_SUFFIX)

.PHONY: all clean

all: $(LIB_TARGET);

$(LIB_TARGET): $(OBJ_FILES)
	$(AR) -rc $@ $(OBJ_FILES)
	#$(CC) -shared -fpic -o $(LIB_TARGET:.a=.so) $(OBJ_FILES) $(LDFLAGS)
	@echo *********Build $@ $(LIB_TARGET:.a=.so) Successful*********
	@echo

%.o: %.cc
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@

clean:
	rm -f $(LIB_TARGET) $(LIB_TARGET:.a=.so) $(OBJ_FILES)
