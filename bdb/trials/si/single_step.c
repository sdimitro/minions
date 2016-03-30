#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>

/* For SYS_write */
#include <sys/syscall.h>

/* For user_regs_struct */
#include <sys/user.h>

const int long_size = sizeof(long);

int
main(void)
{
	pid_t pid;

	if ((pid = fork()) == 0) {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("./dummy", "dummy", NULL);
	} else {
		int status;

		struct user_regs_struct regs;

		int start = 0;
		long ins;

		while (1) {
			wait(&status);

			if(WIFEXITED(status))
				break;

			ptrace(PTRACE_GETREGS, pid, NULL, &regs);

			if (start == 1) {
				ins = ptrace(PTRACE_PEEKTEXT, pid, regs.eip, NULL);
				printf("EIP: %lx Instruction executed: %lx\n",
				       regs.eip, ins);
			}

			if (regs.orig_eax == SYS_write) {
				start = 1;
				ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
			} else {
				ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			}
		}
	}

	return 0;
}
