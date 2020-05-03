#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>

#define PROTO_BUFFER 128

typedef struct st_proto {
	int32_t id;
	void *data;
	uint32_t size;
	uint32_t total_size;
	uint32_t offset;
	uint32_t read_only;

	struct st_proto* (*copy)(struct st_proto* proto, const void* item, uint32_t size);
	struct st_proto* (*add)(struct st_proto* proto, const void* item, uint32_t size);
	struct st_proto* (*add_int)(struct st_proto* proto, int32_t v);
	struct st_proto* (*add_str)(struct st_proto* proto, const char* v);
}proto_t;

proto_t* proto_init(proto_t* proto, void* data, uint32_t size);

proto_t* proto_copy(proto_t* proto, const void* data, uint32_t size);

proto_t* proto_new(void* data, uint32_t size);

proto_t* proto_new_int(int i);

proto_t* proto_new_str(const char* s);

proto_t* proto_add(proto_t* proto, const void* item, uint32_t size);

proto_t* proto_add_int(proto_t* proto, int32_t v);

proto_t* proto_add_str(proto_t* proto, const char* v);

void* proto_read(proto_t* proto, int32_t *size);

int32_t proto_read_to(proto_t* proto, void* to, int32_t size);

int32_t proto_read_int(proto_t* proto);

const char* proto_read_str(proto_t* proto);

proto_t* proto_clear(proto_t* proto);

void proto_free(proto_t* proto);

#endif