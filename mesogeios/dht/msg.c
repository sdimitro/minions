#include "msg.h"

#include <stdlib.h>
#include <string.h>

int valid_field_size(const char *s, size_t byte_limit)
{
	return (strlen(s) * (sizeof(char)) <= byte_limit) ? 1 : 0;
}

void *alloc_message(char command, const char *key, const char *val)
{
	char *message = malloc(MSG_BYTES);

	if (!valid_field_size(key, KEY_BYTES) ||
			!valid_field_size(val, VAL_BYTES))
		return NULL;

	memcpy(message, &command, HDR_BYTES);
	memcpy(message + HDR_BYTES, key, KEY_BYTES);
	memcpy(message + HDR_BYTES + KEY_BYTES, val, VAL_BYTES);

	return message;
}

void decode_message(char *message, char *command, char *key, char *val)
{
	memcpy(command, message, KEY_BYTES);
	memcpy(key, message + HDR_BYTES, KEY_BYTES);
	memcpy(val, message + HDR_BYTES + KEY_BYTES, VAL_BYTES);
}

void free_message(void *message)
{
	free(message);
}

