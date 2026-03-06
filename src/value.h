#ifndef KEEPERFX_VALUE_H
#define KEEPERFX_VALUE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VALUE_TYPE {
    VALUE_NULL = 0,
    VALUE_BOOL,
    VALUE_INT32,
    VALUE_UINT32,
    VALUE_INT64,
    VALUE_UINT64,
    VALUE_FLOAT,
    VALUE_DOUBLE,
    VALUE_STRING,
    VALUE_ARRAY,
    VALUE_DICT,
} VALUE_TYPE;

typedef struct VALUE {
    uint32_t _stub;
} VALUE;

VALUE* value_array_get(const VALUE* v, size_t index);
VALUE* value_array_append(VALUE* v);
VALUE* value_array_insert(VALUE* v, size_t index);
size_t value_array_size(const VALUE* v);
VALUE* value_dict_get(const VALUE* v, const char* key);
VALUE* value_dict_add(VALUE* v, const char* key);
VALUE* value_dict_add_(VALUE* v, const char* key, size_t key_len);
VALUE* value_dict_get_or_add(VALUE* v, const char* key);
int value_bool(const VALUE* v);
int32_t value_int32(const VALUE* v);
uint32_t value_uint32(const VALUE* v);
int64_t value_int64(const VALUE* v);
uint64_t value_uint64(const VALUE* v);
float value_float(const VALUE* v);
double value_double(const VALUE* v);
VALUE_TYPE value_type(const VALUE* v);
const char* value_string(const VALUE* v);
void value_fini(VALUE* v);
int value_init_array(VALUE* v);
int value_init_bool(VALUE* v, int b);
int value_init_dict(VALUE* v);
int value_init_double(VALUE* v, double d);
int value_init_float(VALUE* v, float f);
int value_init_int32(VALUE* v, int32_t i32);
int value_init_int64(VALUE* v, int64_t i64);
void value_init_null(VALUE* v);
int value_init_string_(VALUE* v, const char* str, size_t len);
int value_init_uint32(VALUE* v, uint32_t u32);
int value_init_uint64(VALUE* v, uint64_t u64);

static inline int value_init_string(VALUE* v, const char* str)
{
    return value_init_string_(v, str, (str != NULL) ? strlen(str) : 0);
}

#ifdef __cplusplus
}
#endif

#endif
