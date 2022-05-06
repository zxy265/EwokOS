#ifndef SVC_H
#define SVC_H

#include <kernel/context.h>
#include <syscalls.h>

extern void svc_handler(int32_t code, int32_t arg0, int32_t arg1, int32_t arg2, context_t* ctx);

#endif
