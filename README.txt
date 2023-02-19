EVM: educational virtual machine

32 bit int and float VM
registers: [N] + SP + IP
interactive mode plus compiler

R must be a register name
M must be a hardcoded memory address or a label
I must be an integer immediate or float immediate (with decimal)

have the file format have a code and data segment, so it has a header, and the disassembler can be smart.

stack can be set at compile time but default 10kB


zeros N
start

stop
nop
syscall
put	R
fput	R

(sput	M/L
(get	R
(fget	R
(sget	M/L, R  


ld	R, M/L
st	M/L, R
cpy	R, R
set	R, I
fset	R, I

push	R
pop	R

add	R, R
sub	R, R
mul	R, R
div	R, R

fadd	R, R
fsub	R, R
fmul	R, R
fdiv	R, R

cvtfi	R, R
cvtif	R, R

not	R
lnot	R
and	R, R
or	R, R
xor	R, R

jp	R, M/L
jpz	R, M/L
jz	R, M/L
jn	R, M/L
jnz	R, M/L
j	M/L


