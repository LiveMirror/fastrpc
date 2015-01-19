CC=g++
ifeq ($(debug), 1)
DEBUGFLAGS = -g
else
DEBUGFLAGS = -O3
endif
ROOT_DIR= $(shell pwd)
SRC_DIR= $(ROOT_DIR)

SRC_FILES  := $(shell find $(SRC_DIR) -name '*.cpp')
SRC_FILES  += $(shell find $(SRC_DIR) -name '*.c')
HEAD_FILES := $(shell find $(SRC_DIR) -name '*.h*')
OBJ_FILES  := $(SRC_FILES:.cpp=.o)

CFLAGS= -lrt -Wall -DUNIX $(DEBUG) -Wno-deprecated -D_REENTRANT

INC_DIR= -I./
INC_DIR= -I../protobuf/include
INC_DIR+= -I../xcore/include
INC_DIR+= -I../ipcs_common/include -I../http_codec/api -I../http_codec/http
INC_DIR+= -I../svr
INC_DIR+= -I../ependingpool/output/include

LIB_NAME   := rpcclient
LIB_SUFFIX := .a
LIB_TARGET := lib$(LIB_NAME)$(LIB_SUFFIX)

.PHONY: all clean

all: $(LIB_TARGET);

$(LIB_TARGET): $(OBJ_FILES)
	$(AR) -rc $@ $(OBJ_FILES)
	@if [ -d $(ROOT_DIR)/output ]; then rm -rf $(ROOT_DIR)/output; fi
	@if [ -d $(ROOT_DIR)/include ]; then rm -rf $(ROOT_DIR)/include; fi
	@mkdir $(ROOT_DIR)/output;
	@mkdir $(ROOT_DIR)/include;
	cp $(HEAD_FILES) $(ROOT_DIR)/include;
	cp $(OBJ_FILES) $(ROOT_DIR)/output;
	cp $(LIB_TARGET) $(ROOT_DIR)/output;
	@echo *********Build $@ $(LIB_TARGET:.a=.so) Successful*********
	@echo

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@
%.o: %.cc
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@

clean:
	rm -f $(LIB_TARGET) $(LIB_TARGET:.a=.so) $(OBJ_FILES)
	@if [ -d $(ROOT_DIR)/output ]; then rm -rf $(ROOT_DIR)/output; fi
	@if [ -d $(ROOT_DIR)/include ]; then rm -rf $(ROOT_DIR)/include; fi
