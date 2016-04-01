#include <sys/ptrace.h>
#include <sys/user.h>

#include "bdb_operations.h"

void
bdb_register_as_tracee(void)
{
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);
}

void
bdb_attach_to_process(struct bdb_session *s)
{
	ptrace(PTRACE_ATTACH, s->pid, NULL, NULL);
}

void
bdb_get_registers(struct bdb_session *s, struct user_regs_struct *r)
{
	ptrace(PTRACE_GETREGS, s->pid, NULL, r);
}

