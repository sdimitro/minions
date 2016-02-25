#include "cpu.h"
#include "util.h"

void init_regs(struct cpu *c)
{
	reg_t *r = &c->registers[REG_ZR];
	
	init_properties(r);
	make_readable(r);
	r->contents = 0;
	
	int i;
	for (i = 1; i < ARRAY_SIZE(c->registers); i++) {
		init_properties(&c->registers[i]);
		make_readable(&c->registers[i]);
		make_writeable(&c->registers[i]);
		c->registers[i].contents = 0;
	}

	r = &c->pc;
	init_properties(r);
	make_writeable(r);
	make_special(r);
	r->contents = 0;

	r = &c->npc;
	init_properties(r);
	make_special(r);
	r->contents = 0;
	
	r = &c->hi;
	init_properties(r);
	make_readable(r);
	make_special(r);
	r->contents = 0;

	r = &c->lo;
	init_properties(r);
	make_readable(r);
	make_special(r);
	r->contents = 0;
}
