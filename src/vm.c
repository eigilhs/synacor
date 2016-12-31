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
#define RA heap[heap[ip+1]]
#define IR(N) ({ const uint16_t x = heap[ip+N]; x & REG_START ? heap[x] : x; })
#define A IR(1)
#define B IR(2)
#define C IR(3)

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
set:	RA = B;				GOTO(ip+3);
out:	putchar(A);			GOTO(ip+2);
in:	RA = getchar();			GOTO(ip+2);
jmp:					GOTO(A);
jt:	if (A) GOTO(B);			GOTO(ip+3);
jf:	if (!A) GOTO(B);		GOTO(ip+3);
add:	RA = B + C & MAX_LITERAL;	GOTO(ip+4);
mult:	RA = B * C & MAX_LITERAL;	GOTO(ip+4);
mod:	RA = B % C;			GOTO(ip+4);
and:	RA = B & C;			GOTO(ip+4);
or:	RA = B | C;			GOTO(ip+4);
gt:	RA = B > C;			GOTO(ip+4);
eq:	RA = B == C;			GOTO(ip+4);
not:	RA = B ^ MAX_LITERAL;		GOTO(ip+3);
rmem:	RA = heap[B];			GOTO(ip+3);
wmem:	heap[A] = B;			GOTO(ip+3);
push:	PUSH(A);			GOTO(ip+2);
pop:	RA = POP();			GOTO(ip+2);
call:	PUSH(ip+2);			GOTO(A);
ret:	if (__builtin_expect(!!sp, 1))	GOTO(POP());
halt:	free(stack);
}
