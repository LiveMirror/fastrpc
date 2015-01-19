CC=g++ -g
DEBUG = -D_USE_HTTP_PROTO_
ROOT_DIR= $(shell pwd)
SRC_DIR=$(ROOT_DIR)/encoding
SRC_DIR+=$(ROOT_DIR)/atomic $(ROOT_DIR)/base $(ROOT_DIR)/crypto/aes
SRC_DIR+=$(ROOT_DIR)/crypto/hash $(ROOT_DIR)/system $(ROOT_DIR)/system/concurrency
SRC_DIR+=$(ROOT_DIR)/system/time $(ROOT_DIR)/thread $(ROOT_DIR)/timer
SRC_FILES  := $(shell find $(SRC_DIR) -name '*.cpp')
HEAD_FILES := $(shell find $(SRC_DIR) -name '*.h')
OBJ_FILES  := $(SRC_FILES:.cpp=.o)

CFLAGS= -Wall -DUNIX $(DEBUG) -Wno-deprecated -D_REENTRANT

INC_DIR=-I./
INC_DIR+=-I../third-party/gunittest/ -I./encoding -I./atomic/ -I./atomic/detail/
INC_DIR+=-I./base/ -I./crypto/aes -I./crypto/hash -I./system -I./system/concurrency
INC_DIR+=-I./system/time -I./thread -I./timer
INC_DIR+=-I../protobuf/include

LIB_NAME   := ipcs_common
LIB_SUFFIX := .a
LIB_TARGET := lib$(LIB_NAME)$(LIB_SUFFIX)

.PHONY: all clean

all: $(LIB_TARGET);

$(LIB_TARGET): $(OBJ_FILES)
	$(AR) -rc $@ $(OBJ_FILES)
	#$(CC) -shared -fpic -o $(LIB_TARGET:.a=.so) $(OBJ_FILES) $(LDFLAGS)
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

clean:
	rm -f $(LIB_TARGET) $(LIB_TARGET:.a=.so) $(OBJ_FILES)
