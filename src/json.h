#ifndef KEEPERFX_JSON_H
#define KEEPERFX_JSON_H

#include <stddef.h>
#include <stdint.h>
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JSON_INPUT_POS {
    int line_number;
    int column_number;
} JSON_INPUT_POS;

typedef int (*JSON_DUMP_CALLBACK)(const char* str, size_t size, void* userdata);

#define JSON_ERR_OUTOFMEMORY (-2)
#define JSON_DOM_DUMP_MINIMIZE (1)

int json_dom_parse(char* in, size_t in_size, void* memctx, int flags, VALUE* out, JSON_INPUT_POS* pos);
int json_dom_dump(const VALUE* root, JSON_DUMP_CALLBACK writer, void* userdata, int indent, int flags);

void json_analyze_number(const char* num, size_t num_size,
    int* p_is_int32_compatible,
    int* p_is_uint32_compatible,
    int* p_is_int64_compatible,
    int* p_is_uint64_compatible);

int32_t json_number_to_int32(const char* num, size_t num_size);
uint32_t json_number_to_uint32(const char* num, size_t num_size);
int64_t json_number_to_int64(const char* num, size_t num_size);
uint64_t json_number_to_uint64(const char* num, size_t num_size);
int json_number_to_double(const char* num, size_t num_size, double* p_result);

#ifdef __cplusplus
}
#endif

#endif
