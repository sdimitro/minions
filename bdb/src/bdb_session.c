#include "bdb_utils.h"

#include "bdb_session.h"

struct bdb_session *
make_bdb_session(void)
{
	struct bdb_session *b;

	b = xmalloc(sizeof(struct bdb_session));
	b->pid = -1;
	b->verbose = 0;
	b->type = NONE;
	b->stub_type = 0;

	return b;
}

void
clean_bdb_session(struct bdb_session *b)
{
	xfree(b);
}

