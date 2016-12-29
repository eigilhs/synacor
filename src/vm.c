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
#define R_IR() a = heap[ip++]; IR(b)
#define IR(X) X = heap[ip++]; if (X & REG_START) X = heap[X]
#define LOAD_2ARY(EXPR) R_IR(); IR(c); heap[a] = EXPR; DISPATCH()
#define EXPR_2ARY(EXPR) IR(a); IR(b); EXPR; DISPATCH()
#define LOAD_1ARY(EXPR) R_IR(); heap[a] = EXPR; DISPATCH()
#define EXPR_1ARY(EXPR) IR(a); EXPR; DISPATCH()
#define LOAD_0ARY(EXPR) heap[heap[ip++]] = EXPR; DISPATCH()

int main(int argc, char **argv)
{
	FILE *fp = fopen(argv[1], "r");
	fread(heap, 2, REG_START, fp);
	stack = malloc(sizeof(uint16_t) * stack_size);

	static void *dispatch_table[] = {
		&&halt, &&set, &&push, &&pop, &&eq, &&gt, &&jmp, &&jt, &&jf,
		&&add, &&mult, &&mod, &&and, &&or, &&not, &&rmem, &&wmem,
		&&call, &&ret, &&out, &&in, &&noop
	};

noop:	DISPATCH();
set:	LOAD_1ARY(b);
out:	EXPR_1ARY(putchar(a));
in:	LOAD_0ARY(getchar());
jmp:	EXPR_1ARY(ip = a);
jt:	EXPR_2ARY(if (a) ip = b);
jf:	EXPR_2ARY(if (!a) ip = b);
add:	LOAD_2ARY(b + c & MAX_LITERAL);
mult:	LOAD_2ARY(b * c & MAX_LITERAL);
mod:	LOAD_2ARY(b % c);
and:	LOAD_2ARY(b & c);
or:	LOAD_2ARY(b | c);
gt:	LOAD_2ARY(b > c);
eq:	LOAD_2ARY(b == c);
not:	LOAD_1ARY(b ^ MAX_LITERAL);
rmem:	LOAD_1ARY(heap[b]);
wmem:	EXPR_2ARY(heap[a] = b);
push:	EXPR_1ARY(PUSH(a));
pop:	LOAD_0ARY(POP());
call:	EXPR_1ARY(PUSH(ip); ip = a);
ret:	if (__builtin_expect(EMPTY(), 0))
		goto halt;
	ip = POP();
	DISPATCH();
halt:	return 0;
}
