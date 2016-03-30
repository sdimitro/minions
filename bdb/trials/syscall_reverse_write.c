#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For ORIG_EAX constant */
#include <sys/reg.h>

/* For SYS_write */
#include <sys/syscall.h>

/* For user_regs_struct */
#include <sys/user.h>

const int long_size = sizeof(long);

void
reverse(char *str)
{
	int i, j;
	char temp;
	for (i = 0, j = strlen(str) - 2; i <= j; ++i, --j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
	}
}

void
getdata(pid_t pid, long addr, char *str, int len)
{
	char *laddr;
	int i, j;

	union u {
		long val;
		char chars[long_size];
	} data;

	i = 0;
	j = len / long_size;
	laddr = str;

	while (i < j) {
		data.val = ptrace(PTRACE_PEEKDATA,
		                  pid, addr + i * 4,
		                  NULL);

		memcpy(laddr, data.chars, long_size);
		++i;
		laddr += long_size;
	}

	j = len % long_size;

	if (j != 0) {
		data.val = ptrace(PTRACE_PEEKDATA,
		                  pid, addr + i * 4,
		                  NULL);
		memcpy(laddr, data.chars, j);
	}
	str[len] = '\0';
}

void
putdata(pid_t pid, long addr, char *str, int len)
{
	char *laddr;
	int i, j;
	union u {
		long val;
		char chars[long_size];
	} data;

	i = 0;
	j = len / long_size;
	laddr = str;

	while (i < j) {
		memcpy(data.chars, laddr, long_size);
		ptrace(PTRACE_POKEDATA, pid,
		       addr + i * 4, data.val);
		++i;
		laddr += long_size;
	}

	j = len % long_size;

	if (j != 0) {
	    memcpy(data.chars, laddr, j);
	    ptrace(PTRACE_POKEDATA, pid,
	           addr + i * 4, data.val);
	}
}

int main()
{
    pid_t pid;
	struct user_regs_struct regs;

    if ((pid = fork()) == 0) {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("/bin/ls", "ls", NULL);
    } else {
		long orig_eax;
		int status;
		char *s;
		int toggle = 0;

		for(;;) {
			wait(&status);

			if (WIFEXITED(status))
				break;

			orig_eax = ptrace(PTRACE_PEEKUSER,
			                  pid, 4 * ORIG_EAX,
			                  NULL);

			if (orig_eax == SYS_write) {
				if(!toggle) {
					toggle = 1;

					ptrace(PTRACE_GETREGS, pid, NULL, &regs);
					s = (char *) calloc((regs.edx + 1), sizeof(char));

					getdata(pid, regs.ecx, s, regs.edx);
					reverse(s);
					putdata(pid, regs.ecx, s, regs.edx);
				} else {
				    toggle = 0;
				}
			}
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
		}
    }
    return 0;
}
