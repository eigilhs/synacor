#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_LITERAL 0x7fff
#define REG_START 0x8000

uint16_t heap[REG_START + 8], *stack;
uint64_t sp = 0, stack_size = 0x800, ip;
#define PUSH(V) if (__builtin_expect(sp == stack_size, 0))	\
			stack = realloc(stack, sizeof(uint16_t) \
					* (stack_size *= 2));	\
		stack[sp++] = V
#define POP() stack[--sp]
#define GOTO(N) goto *dispatch_table[heap[ip = N]]
#define R heap[heap[++ip]]
#define RI ({ uint16_t x = heap[++ip]; x & REG_START ? heap[x] : x; })

int main(int argc, char **argv)
{
	fread(heap, 2, REG_START, fopen(argv[1], "r"));
	stack = malloc(sizeof(uint16_t) * stack_size);

	static void *dispatch_table[] = {
		&&halt, &&set, &&push, &&pop, &&eq, &&gt, &&jmp,
		&&jt, &&jf, &&add, &&mult, &&mod, &&and, &&or, &&not,
		&&rmem, &&wmem,	&&call, &&ret, &&out, &&in, &&noop
	};

	GOTO(0);
noop:					GOTO(ip+1);
set:	R = RI;				GOTO(ip+1);
out:	putchar(RI);			GOTO(ip+1);
in:	R = getchar();			GOTO(ip+1);
jmp:					GOTO(RI);
jt:	if (RI) GOTO(RI);		GOTO(ip+2);
jf:	if (!RI) GOTO(RI);		GOTO(ip+2);
add:	R = RI + RI & MAX_LITERAL;	GOTO(ip+1);
mult:	R = RI * RI & MAX_LITERAL;	GOTO(ip+1);
mod:	R = RI % RI;			GOTO(ip+1);
and:	R = RI & RI;			GOTO(ip+1);
or:	R = RI | RI;			GOTO(ip+1);
gt:	R = RI > RI;			GOTO(ip+1);
eq:	R = RI == RI;			GOTO(ip+1);
not:	R = RI ^ MAX_LITERAL;		GOTO(ip+1);
rmem:	R = heap[RI];			GOTO(ip+1);
wmem:	heap[RI] = RI;			GOTO(ip+1);
push:	PUSH(RI);			GOTO(ip+1);
pop:	R = POP();			GOTO(ip+1);
call:	PUSH(ip+2);			GOTO(RI);
ret:	if (__builtin_expect(!!sp, 1))	GOTO(POP());
halt:	free(stack);
}
