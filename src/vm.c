#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_LITERAL 0x7fff
#define REG_START 0x8000

/* STACK */
uint16_t *stack;
uint64_t sp = 0, stack_size = 0x800;
#define PUSH(V) if (sp == stack_size)				\
			stack = realloc(stack, sizeof(uint16_t) \
					* (stack_size *= 2));	\
		stack[sp++] = V
#define POP() stack[--sp]
#define EMPTY() !sp

/* HEAP + REGISTERS */
uint16_t heap[REG_START + 8];

/* OPCODE DISPATCHER */
uint64_t ip = 0;
#define DISPATCH() goto *dispatch_table[heap[ip++]]

/* ARGUMENT HANDLING */
uint16_t a, b, c;
#define __IR(X) X = heap[ip++]; if (X > MAX_LITERAL) X = heap[X]
#define R_IR() a = heap[ip++]; __IR(b)
#define R_IR_IR() R_IR(); __IR(c)
#define IR() __IR(a)
#define IR_IR() IR(); __IR(b)

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
	R_IR();
	heap[a] = b;
	DISPATCH();
do_push:
	IR();
	PUSH(a);
	DISPATCH();
do_pop:
	if (EMPTY())
		return 1;
	a = heap[ip++];
	heap[a] = POP();
	DISPATCH();
do_eq:
	R_IR_IR();
	heap[a] = b == c;
	DISPATCH();
do_gt:
	R_IR_IR();
	heap[a] = b > c;
	DISPATCH();
do_jmp:
	IR();
	ip = a;
	DISPATCH();
do_jt:
	IR_IR();
	if (a)
		ip = b;
	DISPATCH();
do_jf:
	IR_IR();
	if (!a)
		ip = b;
	DISPATCH();
do_add:
	R_IR_IR();
	heap[a] = (b + c) & MAX_LITERAL;
	DISPATCH();
do_mult:
	R_IR_IR();
	heap[a] = (b * c) & MAX_LITERAL;
	DISPATCH();
do_mod:
	R_IR_IR();
	heap[a] = b % c;
	DISPATCH();
do_and:
	R_IR_IR();
	heap[a] = b & c;
	DISPATCH();
do_or:
	R_IR_IR();
	heap[a] = b | c;
	DISPATCH();
do_not:
	R_IR();
	heap[a] = ~b & MAX_LITERAL;
	DISPATCH();
do_rmem:
	R_IR();
	heap[a] = heap[b];
	DISPATCH();
do_wmem:
	IR_IR();
	heap[a] = b;
	DISPATCH();
do_call:
	IR();
	PUSH(ip);
	ip = a;
	DISPATCH();
do_ret:
	if (EMPTY())
		goto do_halt;
	ip = POP();
	DISPATCH();
do_out:
	IR();
	putchar(a);
	DISPATCH();
do_in:
	heap[heap[ip++]] = getchar();
	DISPATCH();
do_halt:
	return 0;
}
