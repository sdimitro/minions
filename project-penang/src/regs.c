#include "regs.h"

static int check_property(reg_t reg, uint16_t mask)
{
	return ((reg.properties & mask) == mask) ? 1 : 0;
}

static void apply_property(reg_t *reg, uint16_t mask)
{
	if (mask == NO_PR)
		reg->properties &= NO_PR;
	else
		reg->properties |= mask;
}

int is_readable(reg_t reg)
{
	return check_property(reg, RG_RD);
}

int is_writeable(reg_t reg)
{
	return check_property(reg, RG_WR);
}

int is_general_purpose(reg_t reg)
{
	return !(check_property(reg, SP_RG));
}

int is_special(reg_t reg)
{
	return check_property(reg, SP_RG);
}

void make_readable(reg_t *reg)
{
	return apply_property(reg, RG_RD);
}

void make_writeable(reg_t *reg)
{
	return apply_property(reg, RG_WR);
}

void make_special(reg_t *reg)
{
	return apply_property(reg, SP_RG);
}

void init_properties(reg_t *reg)
{
	return apply_property(reg, NO_PR);
}
