#ifndef __BDB_SESSION_H
#define __BDB_SESSION_H

enum bdb_session_type {
	EXEC_ATTACH,
	PID_ATTACH,
	COREFILE,
	RUN_ARGS,
	NONE
};

struct bdb_session {
	int pid;
	enum bdb_session_type type;
	int verbose;
	int stub_type;
};

struct bdb_session *make_bdb_session(void);
void clean_bdb_session(struct bdb_session *b);

#endif /* __BDB_SESSION_H */

