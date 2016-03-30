#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>

/* For ORIG_EAX constant */
#include <sys/reg.h>

int
main(void)
{
	pid_t pid;
	long orig_eax;

	if ((pid = fork()) == 0) {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("/bin/ls", "ls", NULL);
	} else {
		wait(NULL);

		/*
		 * If my frakenstein machine (the one I currently use)
		 * was 64-bit, the 3rd argument to ptrace would have
		 * been 8 * ORIG_RAX instead of 4 * ORIG_EAX. Also,
		 * the resulting syscall number on the output would
		 * have been 59 instead of 11.
		 *
		 * For more info on Linux look at:
		 * /usr/include/sys/reg.h
		 */
		orig_eax = ptrace(PTRACE_PEEKUSER,
				  pid, 4 * ORIG_EAX,
				  NULL);

		printf("The child issued a system call: %ld\n", orig_eax);
		ptrace(PTRACE_CONT, pid, NULL, NULL);
	}

	return 0;
}
