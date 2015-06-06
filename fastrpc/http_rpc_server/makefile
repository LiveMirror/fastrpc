CC=g++
CC += -g

ROOT_DIR	= $(shell pwd)
SRC_DIR		= $(ROOT_DIR)
SRC_FILES_CPP  := $(shell find $(SRC_DIR) -name '*.cpp')
SRC_FILES_C  := $(shell find $(SRC_DIR) -name '*.c')
SRC_FILES_CC  := $(shell find $(SRC_DIR) -name '*.cc')
OBJ_FILES  := $(SRC_FILES_CPP:.cpp=.o )
OBJ_FILES  += $(SRC_FILES_C:.c=.o)
OBJ_FILES  += $(SRC_FILES_CC:.cc=.o)


INC_DIR=-I./
INC_DIR+=-I../svr -I../http_codec/api -I../ependingpool/output/include/
INC_DIR+=-I../http_codec/http
INC_DIR+=-I../comm/include
INC_DIR+=-I../protobuf/include
INC_DIR+=-I../xcore/include
INC_DIR+=-I../http_rpc_client

CFLAGS     := -lpthread -Wwrite-strings -w #-fPIC

LIB_NAME   := rpc_server
LIB_SUFFIX := .a
LIB_TARGET := lib$(LIB_NAME)$(LIB_SUFFIX)

.PHONY: all clean

all: $(LIB_TARGET);

$(LIB_TARGET): $(OBJ_FILES)
	$(AR) -rc $@ $(OBJ_FILES)
	@echo *********Build $@ $(LIB_TARGET:.a=.so) Successful*********
	@echo

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@
%.o: %.cc
	$(CC) $(CFLAGS) $(INC_DIR) -c $(DEBUGFLAGS) $< -o $@

clean:
	rm -f $(LIB_TARGET) $(LIB_TARGET:.a=.so) $(OBJ_FILES)
