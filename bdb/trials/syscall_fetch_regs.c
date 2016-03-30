#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>

/* For ORIG_EAX constant */
#include <sys/reg.h>

/* For SYS_write */
#include <sys/syscall.h>

/* For user_regs_struct */
#include <sys/user.h>

int
main(void)
{
    pid_t pid;
    long orig_eax;
    int status, insyscall = 0;

	struct user_regs_struct regs;

    if ((pid = fork()) == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    } else {
		for(;;) {
			wait(&status);
			if(WIFEXITED(status))
				break;

			orig_eax = ptrace(PTRACE_PEEKUSER,
				              pid, 4 * ORIG_EAX,
							  NULL);

			if (orig_eax == SYS_write) {
				if (!insyscall) {
					/* Syscall entry */
					insyscall = 1;
					ptrace(PTRACE_GETREGS, pid, NULL, &regs);
					printf("Write called with %ld, %ld, %ld\n",
					       regs.ebx, regs.ecx, regs.edx);
				} else {
					/* Syscall exit */
					insyscall = 0;
					ptrace(PTRACE_GETREGS, pid, NULL, &regs);
					printf("Write returned with %ld\n", regs.eax);
				}
			}

			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
		}
    }
    return 0;
}
