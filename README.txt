EduVM: Educational Virtual Machine

R must be a register name
M/L must be a hardcoded memory address or a label
I must be an integer immediate or float immediate (with decimal)

have the file format have a code and data segment, so it has a header, and the disassembler can be smart.

reserve some space for your stack with zeros, at the end of your data segment

zeros N
start

(sput	M/L
(get	R
(fget	R
(sget	M/L, R  

stop		terminate program
nop		do nothing (no operation)
put	R	print integer
fput	R	print float

ld	R, M/L	load data into register
st	M/L, R	store data into memory
lda	R, M/L	load address of data into register
std	R, R	dereference first argument and store second argument there
ldd	R, R	dereference second argument and load the data stored there
cpy	R, R	copy register to another register
set	R, I	set a register equal to an integer immediate value
fset	R, I	set a register equal to a float immediate value

add	R, R	integer addition (+=)
sub	R, R	integer subtraction (-=)
mul	R, R	integer multiplication (*=)
div	R, R	integer division (/=)

fadd	R, R	float addition (+=)
fsub	R, R	float subtraction (-=)
fmul	R, R	float multiplication (*=)
fdiv	R, R	float division (/=)

cvtfi	R	convert float to integer
cvtif	R	convert integer to float

not	R	bitwise not
lnot	R	logical not
and	R, R	bitwise and
or	R, R	bitwise or
xor	R, R	bitwise xor

jp	R, M/L	jump if register positive
jpz	R, M/L	jump if register positive or zero
jz	R, M/L	jump if register equals zero
jn	R, M/L	jump if register negative
jnz	R, M/L	jump if register negative or zero
j	M/L	unconditional jump

push	R	push register contents onto the stack
pop	R	pop top of stack into register
syscall		system call


put and get instructions should just be shortcuts for syscalls 1 and 2 (with dtype indicated by a register arg)

syscall convention:
r1 holds syscall code, and holds return code on exit from syscall
r2 onwards hold the arguments
if there are more arguments that registers then the arguments are pushed onto the stack by the caller.
the syscall examines the stack but doesn't pop these arguments
