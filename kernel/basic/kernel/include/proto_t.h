#ifndef PROTO_T_H
#define PROTO_T_H

#include <stdint.h>
#include <stdbool.h>
enum {
	PROTO_PKG = 0,
	PROTO_INT
};

#define PROTO_INT_NUM 4

typedef struct {
	uint32_t type;
	void *data;
	int32_t ints[PROTO_INT_NUM];
	uint32_t size;
	uint32_t total_size;
	uint32_t offset;
	bool pre_alloc;
} proto_t;

#endif
