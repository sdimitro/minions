#ifndef __BDB_OPERATIONS_H
#define __BDB_OPERATIONS_H

#include <sys/user.h>

#include "bdb_session.h"

void bdb_register_as_tracee(void);
void bdb_attach_to_process(struct bdb_session *s);

void bdb_get_registers(struct bdb_session *s, struct user_regs_struct *r);

#endif /* __BDB_OPERATIONS_H */

