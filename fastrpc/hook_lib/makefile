CC=g++ -g
ifeq ($(debug), 1)
DEBUGFLAGS = -g
else
DEBUGFLAGS = -O3
endif

ROOT_DIR	= $(shell pwd)
SRC_DIR		= $(ROOT_DIR)
PRO_ROOT    = ../../
SRC_FILES_CPP  := $(shell find $(SRC_DIR) -name '*.cpp')
SRC_FILES_C  := $(shell find $(SRC_DIR) -name '*.c')
SRC_FILES_CC  := $(shell find $(SRC_DIR) -name '*.cc')
OBJ_FILES  := $(SRC_FILES_CPP:.cpp=.o )
OBJ_FILES  += $(SRC_FILES_C:.c=.o)
OBJ_FILES  += $(SRC_FILES_CC:.cc=.o)

CFLAGS= -Wall -DUNIX -Wno-deprecated -D_REENTRANT -lrt -pthread -lpthread -ldl
OFLAG = -o

INC_DIR=-I./

.PHONY: all clean

all: main;

main: $(OBJ_FILES)
	$(CC)  $(OBJ_FILES) $(INC_DIR) $(LIBS) $(CFLAGS) $(DEBUGFLAGS) $(OFLAG) $(RELEASE)$@
	@echo *********Build $@ Successful*********
	@echo

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC_DIR) -c $(DEBUGFLAGS) $< -o $@
%.o: %.cc
	$(CC) $(CFLAGS) $(INC_DIR) -c $(DEBUGFLAGS) $< -o $@

clean:
	rm -f main $(LIB_TARGET:.a=.so) $(OBJ_FILES)
