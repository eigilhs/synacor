#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_LITERAL 0x7fff
#define REG_START 0x8000

/* STACK */
uint16_t *stack;
uint64_t sp = 0, stack_size = 0x800;
#define PUSH(V) ({	if (sp == stack_size)				\
				stack = realloc(stack, sizeof(uint16_t) \
						* (stack_size *= 2));	\
			stack[sp++] = V; })
#define POP() stack[--sp]
#define EMPTY() (!sp)

/* HEAP + REGISTERS */
uint16_t heap[REG_START + 8];

/* OPCODE DISPATCHER */
uint64_t ip = 0;
#define DISPATCH() goto *dispatch_table[heap[ip++]]

/* ARGUMENT HANDLING */
uint16_t a, b, c;
#define REGIMM() ({	a = heap[ip++];				\
			b = heap[ip++];				\
			if (b > MAX_LITERAL)			\
				b = heap[b]; })
#define REGIMMIMM() ({	REGIMM();				\
			c = heap[ip++];				\
			if (c > MAX_LITERAL)			\
				c = heap[c]; })
#define REG() ({	a = heap[ip++];				\
			if (a > MAX_LITERAL)			\
				a = heap[a]; })
#define REGREG() ({	REG();					\
			b = heap[ip++];				\
			if (b > MAX_LITERAL)			\
				b = heap[b]; })

int main(int argc, char **argv)
{
	FILE *fp = fopen(argv[1], "r");
	fread(heap, 2, REG_START, fp);
	stack = malloc(sizeof(uint16_t) * stack_size);

	static void *dispatch_table[] = {
		&&do_halt, &&do_set, &&do_push, &&do_pop, &&do_eq, &&do_gt,
		&&do_jmp, &&do_jt, &&do_jf, &&do_add, &&do_mult, &&do_mod,
		&&do_and, &&do_or, &&do_not, &&do_rmem, &&do_wmem, &&do_call,
		&&do_ret, &&do_out, &&do_in, &&do_noop
	};

do_noop:
	DISPATCH();
do_set:
	REGIMM();
	heap[a] = b;
	DISPATCH();
do_push:
	REG();
	PUSH(a);
	DISPATCH();
do_pop:
	if (EMPTY())
		return 1;
	a = heap[ip++];
	heap[a] = POP();
	DISPATCH();
do_eq:
	REGIMMIMM();
	heap[a] = b == c;
	DISPATCH();
do_gt:
	REGIMMIMM();
	heap[a] = b > c;
	DISPATCH();
do_jmp:
	REG();
	ip = a;
	DISPATCH();
do_jt:
	REGREG();
	if (a)
		ip = b;
	DISPATCH();
do_jf:
	REGREG();
	if (!a)
		ip = b;
	DISPATCH();
do_add:
	REGIMMIMM();
	heap[a] = (b + c) & MAX_LITERAL;
	DISPATCH();
do_mult:
	REGIMMIMM();
	heap[a] = (b * c) & MAX_LITERAL;
	DISPATCH();
do_mod:
	REGIMMIMM();
	heap[a] = b % c;
	DISPATCH();
do_and:
	REGIMMIMM();
	heap[a] = b & c;
	DISPATCH();
do_or:
	REGIMMIMM();
	heap[a] = b | c;
	DISPATCH();
do_not:
	REGIMM();
	heap[a] = ~b & MAX_LITERAL;
	DISPATCH();
do_rmem:
	REGIMM();
	heap[a] = heap[b];
	DISPATCH();
do_wmem:
	REGREG();
	heap[a] = b;
	DISPATCH();
do_call:
	REG();
	PUSH(ip);
	ip = a;
	DISPATCH();
do_ret:
	if (EMPTY())
		goto do_halt;
	ip = POP();
	DISPATCH();
do_out:
	REG();
	putchar(a);
	DISPATCH();
do_in:
	heap[heap[ip++]] = getchar();
	DISPATCH();
do_halt:
	return 0;
}
