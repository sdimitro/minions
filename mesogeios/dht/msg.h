#ifndef __MSG_H
#define __MSG_H

#include <stdlib.h>

#define HDR_BYTES 1
#define KEY_BYTES 24
#define VAL_BYTES 1000
#define MSG_BYTES (HDR_BYTES + KEY_BYTES + VAL_BYTES)

int valid_field_size(const char *s, size_t byte_limit);
void *alloc_message(char command, const char *key, const char *val);
void decode_message(char *message, char *command, char *key, char *val);
void free_message(void *message);

#endif

