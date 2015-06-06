#ifndef C_COROUTINE_H
#define C_COROUTINE_H

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

#include "closure.h"
#include <google/protobuf/stubs/common.h>

struct schedule;

struct schedule * coroutine_open(void);
void coroutine_close(struct schedule *);

int coroutine_new(struct schedule *, Closure<void>* closure);
int coroutine_new(struct schedule *,
                  ::google::protobuf::Closure* closure);
void coroutine_resume(struct schedule *, int id);
int coroutine_status(struct schedule *, int id);
int coroutine_running(struct schedule *);
void coroutine_yield(struct schedule *);


typedef struct schedule * CroMgr;
extern CroMgr singleton;
CroMgr GetCroMgr();
bool ProcessWithNewCro(Closure<void>* closure);
bool ProcessWithNewCro(::google::protobuf::Closure* closure);

#endif
