CC=g++
ifeq ($(debug), 1)
DEBUGFLAGS = -g
else
DEBUGFLAGS = -O3
endif

DEBUGFLAGS += -lrt

ROOT_DIR	= $(shell pwd)
SRC_DIR		= $(ROOT_DIR)
SRC_FILES  := $(shell find $(SRC_DIR) -name '*.cpp')
SRC_FILES  += $(shell find $(SRC_DIR) -name '*.c')
SRC_FILES  += $(shell find $(SRC_DIR) -name '*.cc')
OBJ_FILES  := $(SRC_FILES:.cpp=.o )
OBJ_FILES  := $(OBJ_FILES:.c=.o)
OBJ_FILES  := $(OBJ_FILES:.cc=.o)

CFLAGS= -Wall -DUNIX -Wno-deprecated -D_REENTRANT -lpthread
OFLAG = -o

INC_DIR=-I/usr/include -I/usr/local/include
INC_DIR+=-I./
INC_DIR+=-I./protobuf
INC_DIR+=-I../svr -I../http_codec/api -I../ependingpool/output/include/
INC_DIR+=-I../http_codec/http
INC_DIR+=-I../comm/include
#INC_DIR+=-I../mongo-driver/include
#INC_DIR+=-I/usr/include/mysql
INC_DIR+=-I../protobuf/include
INC_DIR+=-I../ipcs_common/include
INC_DIR+=-I../xcore/include

LIBS = -L/usr/lib  -lpthread
LIBS += ../xcore/libxcore.a
LIBS += ../svr/libasvr.a ../http_codec/libhttpapi.a
LIBS += ../ependingpool/output/libependingpool.a ../ullib/libullib.a
LIBS += ../comm/output/libcomm.a
#LIBS += -L/usr/lib64/mysql/
#LIBS += ../mongo-driver/lib/libmongoclient.a
LIBS += ../protobuf/lib/libprotobuf.a
LIBS += ../ipcs_common/libipcs_common.a
#LIBS += -lmysqlclient -lboost_filesystem -lboost_program_options -lboost_system -lboost_thread-mt

.PHONY: all clean

all: main;

main: $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(INC_DIR) $(LIBS) $(DEBUGFLAGS) $(OFLAG) $(RELEASE)$@
	@echo *********Build $@ Successful*********
	@echo

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC_DIR) -c $(DEBUGFLAGS) $< -o $@
%.o: %.cc
	$(CC) $(CFLAGS) $(INC_DIR) -c $(DEBUGFLAGS) $< -o $@

clean:
	rm -f main $(LIB_TARGET:.a=.so) $(OBJ_FILES)
