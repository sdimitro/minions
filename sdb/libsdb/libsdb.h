#ifndef __LIBSDB_H
#define __LIBSDB_H

#include <libproc.h>
#include <sys/corectl.h>

#define MAX_EXECNAME 512

typedef struct lsdb_obj {
	struct ps_prochandle *pph; /* Process Handle */
	struct ps_lwphandle *plh;  /* Current Process LWP Handle */
	int err;                   /* Error code */
	const char *err_msg;       /* Error message */
} lsdb_obj_t;

lsdb_obj_t *lsdb_obj_alloc(void);
void lsdb_obj_free(lsdb_obj_t *);

int lsdb_attach2pid(lsdb_obj_t *, long, int);
int lsdb_grab_core(lsdb_obj_t *, const char *, const char *, int);
int lsdb_grab_file(lsdb_obj_t *, const char *);
void lsdb_release(lsdb_obj_t *, int);

int lsdb_execname(lsdb_obj_t *, char *, size_t);
core_content_t lsdb_getcontents(lsdb_obj_t *);
long lsdb_getareg(lsdb_obj_t *, int);

#endif /* __LIBSDB_H */
