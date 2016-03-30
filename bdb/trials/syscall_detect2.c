#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>

/* For ORIG_EAX constant */
#include <sys/reg.h>

/* For SYS_write */
#include <sys/syscall.h>

int
main(void)
{
    pid_t pid;
    long orig_eax, eax;
    long params[3];
    int status, insyscall = 0;

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
				if(!insyscall) {
					/* Syscall entry */
					insyscall = 1;

					params[0] = ptrace(PTRACE_PEEKUSER,
					                   pid, 4 * EBX,
					                   NULL);

					params[1] = ptrace(PTRACE_PEEKUSER,
					                   pid, 4 * ECX,
					                   NULL);

					params[2] = ptrace(PTRACE_PEEKUSER,
					                   pid, 4 * EDX,
					                   NULL);

					printf("Write called with "
					       "%ld, %ld, %ld\n",
					       params[0], params[1],
					       params[2]);
				} else {
					/* Syscall exit */
					eax = ptrace(PTRACE_PEEKUSER,
					             pid, 4 * EAX,
								 NULL);

					printf("Write returned "
					       "with %ld\n", eax);

					insyscall = 0;
				}
			}

			ptrace(PTRACE_SYSCALL,
			       pid, NULL, NULL);
		}
    }
    return 0;
}
