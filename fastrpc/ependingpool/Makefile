
LIBS = libependingpool.a
OUTPUT = ./output/

SRCS = ependingpool.cpp
HEADS = $(wildcard *.h)

OBJS = $(SRCS:.cpp=.o)

GCC = g++
AR = ar

LIBPATH=../

KVER = $(findstring 2.4, $(shell uname -r))
ifeq ($(KVER), 2.4)
  EPOOLOBJ = epoll.o
  DEFINES  = -DKERNEL24
endif




INCLUDE = -I$(LIBPATH)ullib/include
#CPPFLAGS = -g $(USERFLAGS) -finline-functions -W -Wall -Winline -pipe
CPPFLAGS = $(USERFLAGS) -finline-functions -W -Wall -Winline -pipe -lpthread


.cpp.o :
	$(GCC) -c $(CPPFLAGS) $(INCLUDE) $(DEFINES) $<

all : deps $(LIBS) $(OUTPUT)

$(LIBS) : $(OBJS) $(EPOOLOBJ)
	@$(AR) -r $@ $^

$(OUTPUT) : $(LIBS)
	@if [ -d $@ ]; then rm -rf $@; fi
	@if [ -d $@/include ]; then rm -rf $@/include; fi
	@mkdir $@;
	@mkdir $@/include;
	cp $^ $@;
	cp $(HEADS) $@/include;


deps : $(SRCS) $(HEADS)
	@$(GCC) -MM -MG $(CPPFLAGS) $(INCLUDE) $(DEFINES) $(filter %.c %.cpp, $^) > $@

clean :
	rm -rf $(LIBS) *.o
	rm -rf deps $(OUTPUT)

# Clean and make all
rebuild : clean all

-include deps

ccp_check:
	ccp -dR ./ --formatter vim --quiet
