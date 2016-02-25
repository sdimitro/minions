#ifndef __CPU_H
#define __CPU_H

#include "regs.h"

struct cpu {
	reg_t registers[REG_NB]; /* general purpose registers */
	reg_t pc;  /* program counter */
	reg_t npc; /* new program counter */             
	reg_t hi;  /* HI register (removed in Release 6) */
	reg_t lo;  /* LO register (removed in Release 6) */
};

/* initialization functions */
void init_regs(struct cpu *c);

#endif /* __CPU_H */
