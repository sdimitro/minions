#ifndef __REGS_H
#define __REGS_H

#include <stdint.h>

#define REG_NB 32

typedef struct reg {
	uint32_t contents;
	uint16_t properties;
} reg_t;

/* Property masks */
#define NO_PR 0x0000 /* No properties */
#define RG_RD 0x0001 /* Read-Enabled  */
#define RG_WR 0x0002 /* Write-Enabled */
#define SP_RG 0x1000 /* Special Register */

/* Register indexes */
#define REG_ZR  0
#define REG_AT  1
#define REG_V0  2
#define REG_V1  3
#define REG_A0  4
#define REG_A1  5
#define REG_A2  6
#define REG_A3  7
#define REG_T0  8
#define REG_T1  9
#define REG_T2 10
#define REG_T3 11
#define REG_T4 12
#define REG_T5 13
#define REG_T6 14
#define REG_T7 15
#define REG_S0 16
#define REG_S1 17
#define REG_S2 18
#define REG_S3 19
#define REG_S4 20
#define REG_S5 21
#define REG_S6 22
#define REG_S7 23
#define REG_T8 24
#define REG_T9 25
#define REG_K0 26
#define REG_K1 27
#define REG_GP 28
#define REG_SP 29
#define REG_S8 30
#define REG_RA 31

/* utility functions */
int is_readable(reg_t reg);
int is_writeable(reg_t reg);
int is_general_purpose(reg_t reg);
int is_special(reg_t reg);

void make_readable(reg_t *reg);
void make_writeable(reg_t *reg);
void make_special(reg_t *reg);
void init_properties(reg_t *reg);

#endif /* __REGS_H */
