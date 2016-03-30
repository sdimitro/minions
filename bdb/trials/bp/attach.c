#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/user.h>

int
main(int c, char *v[])
{
	pid_t tracee;
	struct user_regs_struct regs;
	long instr;

	if (c != 2) {
		printf("Usage: %s <pid>\n", v[0]);
		exit(EXIT_FAILURE);
	}

	tracee = atoi(v[1]);

	ptrace(PTRACE_ATTACH, tracee, NULL, NULL);
	wait(NULL);

	ptrace(PTRACE_GETREGS, tracee, NULL, &regs);
	instr = ptrace(PTRACE_PEEKTEXT, tracee, regs.eip, NULL);
	printf("EIP: %lx Instruction executed: %lx\n", regs.eip, instr);
	ptrace(PTRACE_DETACH, tracee, NULL, NULL);

	return 0;
}
