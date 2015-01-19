CC=g++
CC=g++
ifeq ($(debug), 1)
CC += -g
else
CC += -O3
endif

ROOT_DIR	= $(shell pwd)
SRC_DIR		= $(ROOT_DIR)/json $(ROOT_DIR)/http $(ROOT_DIR)/api
SRC_FILES  := $(shell find $(SRC_DIR) -name '*.cpp')
OBJ_FILES  := $(SRC_FILES:.cpp=.o)
INC_DIR    := -I$(ROOT_DIR)/ -I$(ROOT_DIR)/http -I$(ROOT_DIR)/api -D_USE_HTTP_PROTO_
CFLAGS     :=

LIB_NAME   := httpapi
LIB_SUFFIX := .a
LIB_TARGET := lib$(LIB_NAME)$(LIB_SUFFIX)

.PHONY: all clean

all: $(LIB_TARGET);

$(LIB_TARGET): $(OBJ_FILES)
	$(AR) -rc $@ $(OBJ_FILES)
	#$(CC) -shared -fpic -o $(LIB_TARGET:.a=.so) $(OBJ_FILES) $(LDFLAGS)
	$(CC) -o test $(OBJ_FILES)
	@echo *********Build $@ $(LIB_TARGET:.a=.so) Successful*********
	@echo

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@

clean:
	rm -f $(LIB_TARGET) $(LIB_TARGET:.a=.so) $(OBJ_FILES)
	rm -f test
