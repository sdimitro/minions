#include "gemtc_api.h"
#include <stdlib.h>
#include <assert.h>

#define QUE_SZ  10
#define WORKERS  2
#define TASK_ID  0
#define SLEEP_DURATION 30

int main(void)
{
	unsigned int *sleep_time = malloc (sizeof(unsigned int));
	int *id = (int *) malloc(sizeof(int));
	void *params;

	*id = -1;
	*sleep_time = SLEEP_DURATION;

	gemtc_setup(QUE_SZ, WORKERS);

	gemtc_push(1,0, TASK_ID, (void *)sleep_time);
	gemtc_poll(id,&params);

	assert(*id == 0);
	assert(*sleep_time == 0);
	assert(sleep_time == params);

	gemtc_cleanup();
	free(sleep_time);
	free(id);

	return(0);
}
