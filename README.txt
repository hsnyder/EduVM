EduVM: Educational Virtual Machine
==================================

This is a simple virtual machine, intended for teaching how computers work.
It's capable of doing math on 32 bit signed integers and floats. 
It consists of:

- this readme, which explains the assembly language / instruction set
- an assembler (converts assembly language to bytecode)
- a disassembler (converts byte code back to assembly language)
- a byte code interpreter (executes bytecode programs)
- an interactive interpreter mode, where the user can step through execution

Build: just compile main.c, e.g. `cc main.c -o evm`

Assemble:    `./evm -a sourcecode.evm > bytecode.bin`

Disassemble: `./evm -d bytecode.bin`

Execute:     `./evm bytecode.bin`

Interactive: `./evm -i bytecode.bin`


Machine registers and address format
-----------------

The machine has an instruction pointer (`ip`), a stack pointer (`sp`)
and four 32 bit general purpose registers (`r1`, `r2`, ...). The stack pointer
can be used as a general purpose register if desired, but the instruction pointer
cannot be modified by code (except with a jump instruction of course).

Memory addresses refer to four byte chunks, and are zero-based.
Example: address 0 refers to the first 32 bits of memory, address 1 to the next 32
bits and so on.

It is unspecified whether the machine is little or big endian (depends on where
the assembler was compiled).

Code file format
----------------

The start of a code file is the data section. All content of the source code file
is treated as part of the data segment until the `start` pseudo-instruction appears,
which marks the start of the code segment. At run time, execution begins at the first 
instruction following `start`.

Inside the data segment, source lines must adhere to one of the following patterns:

- An optional label, followed by an integer or floating point literal value.
  This reserves space for that value in the data segment and initializes it to the 
  specified value. 

- An optional label, followed by a single or double quoted ASCII string. 
  This reserves space for that string in the data segment and initializes it to the
  specified value. Strings always take up a multiple of four bytes since addresses
  refer to 4 byte chunks.

- An optional label, followed by `zeros n` where n is an integer. This reserves
  space for `n` 32 bit values, and initlizes them all to 0x00000000.

Labels are followed by colons. 


At the start of execution, the stack pointer holds the address of the end of the data
segment. However, there's no space reserved for a stack automatically. If you want a
stack, reserve some space with `zeros` at the end of the data segment. 
The stack grows downwards (`push` decreases `sp`).

See the instruction set section for what to put inside the code segment.

Instruction Set
---------------

The instructions are as follows

Key: 
  R	a register name (e.g. r1, r2, r3, r4, sp)
  M	a hardcoded memory address or a label
  I	an integer or floating point "immediate" value (i.e. a literal number)

stop		terminate program
nop		do nothing (no operation)
put	R	print integer
fput	R	print float

ld	R, M	load data into register
st	M, R	store data into memory
lda	R, M	load address of data into register
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

jp	R, M	jump if register positive
jpz	R, M	jump if register positive or zero
jz	R, M	jump if register equals zero
jn	R, M	jump if register negative
jnz	R, M	jump if register negative or zero
j	M	unconditional jump

push	R	push register contents onto the stack
pop	R	pop top of stack into register

syscall		system call

Syscall convention
------------------

NOTE: main.c doesn't implement any syscalls.

r1 holds syscall code, and holds return code on exit from syscall
r2 onwards hold the arguments
if there are more arguments that registers then the arguments are pushed onto the stack by the caller.
the syscall examines the stack but doesn't pop these arguments
