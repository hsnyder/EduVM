#ifndef EVM_H
#define EVM_H

#define EVM_NUMREGS 4
#define EVM_MAGIC (((int)'E')<<16 | ((int)'V')<<8 | ((int)'M'))

typedef union {
	float    f;
	int      i;
	unsigned u;
} evm_word;

typedef struct {
	int ip;
	union{
		int sp;
		evm_word r[EVM_NUMREGS+1]; // first reg is stack pointer
	};
} evm_regs;

typedef struct {
	int magic;
	int version;
	int len_data;
	int len_code;
	evm_word mem[];
} evm_mem;

typedef evm_regs (*evm_syscall_callback) (evm_regs, evm_mem *);

typedef struct {
	const char *errmsg;
	evm_regs r;
	int stop;
} evm_status;

evm_status evm_run (int mem_bufsz, evm_mem *memory, evm_syscall_callback syscall, evm_regs *initial_state, int single_step);

typedef enum {
	OP_STOP   = 0x00,
	OP_NOP    = 0x01,
	OP_SYSCALL= 0x02,
	OP_LD     = 0x03,
	OP_ST     = 0x04,
	OP_SET    = 0x05,
	OP_FSET   = 0x06,
	OP_CPY    = 0x07,
	OP_PUSH   = 0x08,
	OP_POP    = 0x09,
	OP_ADD    = 0x0a,
	OP_SUB    = 0x0b,
	OP_MUL    = 0x0c,
	OP_DIV    = 0x0d,
	OP_FADD   = 0x0e,
	OP_FSUB   = 0x0f,
	OP_FMUL   = 0x10,
	OP_FDIV   = 0x11,
	OP_NOT    = 0x12,
	OP_AND    = 0x13,
	OP_OR     = 0x14,
	OP_XOR    = 0x15,
	OP_JP     = 0x16,
	OP_JPZ    = 0x17,
	OP_JZ     = 0x18,
	OP_JN     = 0x19,
	OP_JNZ    = 0x1a,
	OP_J      = 0x1b,
	OP_CVTFI  = 0x1c,
	OP_CVTIF  = 0x1d,
	OP_PUT    = 0x1e,
	OP_FPUT   = 0x1f,
	OP_LNOT   = 0x20,
	OP_LDA    = 0x21,
	OP_LDD    = 0x22,
	OP_STD    = 0x23,

	OP_INVAL  = 0x24,
} evm_op;

typedef enum {
	EVM_MEM,
	EVM_IMMI,
	EVM_IMMF,
	EVM_REG,
} evm_arg_type;

typedef struct {
	int opcode;
	const char * str;
	int nargs;
	evm_arg_type argtypes[2];
} evm_op_t;

const evm_op_t evm_ops[] = {
	[OP_STOP]    = {OP_STOP, "stop", 0},
	[OP_NOP]     = {OP_NOP,  "nop",  0},
	[OP_SYSCALL] = {OP_SYSCALL, "syscall", 0},
	[OP_LD]      = {OP_LD,   "ld",   2, {EVM_REG, EVM_MEM}},
	[OP_ST]      = {OP_ST,   "st",   2, {EVM_MEM, EVM_REG}},
	[OP_SET]     = {OP_SET,  "set",  2, {EVM_REG, EVM_IMMI}},
	[OP_FSET]    = {OP_FSET, "fset", 2, {EVM_REG, EVM_IMMF}},
	[OP_CPY]     = {OP_CPY,  "cpy",  2, {EVM_REG, EVM_REG}},
	[OP_PUSH]    = {OP_PUSH, "push", 1, {EVM_REG}},
	[OP_POP]     = {OP_POP,  "pop",  1, {EVM_REG}},
	[OP_ADD]     = {OP_ADD,  "add",  2, {EVM_REG, EVM_REG}},
	[OP_SUB]     = {OP_SUB,  "sub",  2, {EVM_REG, EVM_REG}},
	[OP_MUL]     = {OP_MUL,  "mul",  2, {EVM_REG, EVM_REG}},
	[OP_DIV]     = {OP_DIV,  "div",  2, {EVM_REG, EVM_REG}},
	[OP_FADD]    = {OP_FADD, "fadd", 2, {EVM_REG, EVM_REG}},
	[OP_FSUB]    = {OP_FSUB, "fsub", 2, {EVM_REG, EVM_REG}},
	[OP_FMUL]    = {OP_FMUL, "fmul", 2, {EVM_REG, EVM_REG}},
	[OP_FDIV]    = {OP_FDIV, "fdiv", 2, {EVM_REG, EVM_REG}},
	[OP_NOT]     = {OP_NOT,  "not",  1, {EVM_REG}},
	[OP_AND]     = {OP_AND,  "and",  2, {EVM_REG, EVM_REG}},
	[OP_OR]      = {OP_OR,   "or",   2, {EVM_REG, EVM_REG}},
	[OP_XOR]     = {OP_XOR,  "xor",  2, {EVM_REG, EVM_REG}},
	[OP_JP]      = {OP_JP,   "jp",   2, {EVM_REG, EVM_MEM}},
	[OP_JPZ]     = {OP_JPZ,  "jpz",  2, {EVM_REG, EVM_MEM}},
	[OP_JZ]      = {OP_JZ,   "jz",   2, {EVM_REG, EVM_MEM}},
	[OP_JN]      = {OP_JN,   "jn",   2, {EVM_REG, EVM_MEM}},
	[OP_JNZ]     = {OP_JNZ,  "jnz",  2, {EVM_REG, EVM_MEM}},
	[OP_J]       = {OP_J,    "j",    1, {EVM_MEM}},
	[OP_CVTFI]   = {OP_CVTFI,"cvtfi",1, {EVM_REG}},
	[OP_CVTIF]   = {OP_CVTIF,"cvtif",1, {EVM_REG}},
	[OP_PUT]     = {OP_PUT,  "put",  1, {EVM_REG}},
	[OP_FPUT]    = {OP_FPUT, "fput", 1, {EVM_REG}},
	[OP_LNOT]    = {OP_LNOT, "lnot", 1, {EVM_REG}},
	[OP_LDA]     = {OP_LDA,  "lda",  2, {EVM_REG, EVM_MEM}},
	[OP_LDD]     = {OP_LDD,  "ldd",  2, {EVM_REG, EVM_REG}},
	[OP_STD]     = {OP_STD,  "std",  2, {EVM_REG, EVM_REG}},
};


const char * validate_evm_mem(int mem_bufsz, evm_mem *memory);

#endif

#ifdef EVM_IMPLEMENTATION
#define ssizeof(x) ((long long)sizeof(x))

#include <stdio.h> //TODO remove

const char * validate_evm_mem(int mem_bufsz, evm_mem *memory) 
{
	if (mem_bufsz < ssizeof(*memory)) 
		return "invalid memory image: buffer too small";

	if (ssizeof(evm_mem) + memory->len_data + memory->len_code > mem_bufsz) 
		return "invalid memory image: header indicates memory overflows provided buffer";

	if (memory->len_data < 0 || memory->len_code < 0)
		return "invalid memory image: data or code segment length is negative";

	if (memory->magic != EVM_MAGIC) 
		return "invalid memory image: wrong magic number";

	return 0;
}

evm_status evm_run(int mem_bufsz, evm_mem *memory, evm_syscall_callback syscall, evm_regs *initial_state, int single_step)
{
	const char *val_err = validate_evm_mem(mem_bufsz, memory);  
	if(val_err) return (evm_status){.errmsg = val_err};

	int start_data = 0;
	int end_data   = memory->len_data;
	int start_code = end_data;
	int end_code   = start_code + memory->len_code;

	evm_word *mem = memory->mem;

	evm_regs r = {.ip = start_code, .sp = end_data-1};
	if (initial_state) r = *initial_state;

	#ifdef EVM_UNSAFE
	#define CHKREG(x) 
	#define CHKMEM(x) 
	#define CHKCOD(x) 
	#else
	#define CHKREG(x) if(x < 0 || x > EVM_NUMREGS) return (evm_status){.errmsg="encountered invalid register", .r=r};
	#define CHKMEM(x) if(x < start_data || x >= end_data) return (evm_status){.errmsg="encountered invalid memory address", .r=r};
	#define CHKCOD(x) if(x < start_code || x >= end_code) return (evm_status){.errmsg="encountered invalid code address", .r=r};
	#endif

	do {
		#ifndef EVM_UNSAFE
		if (r.ip < start_code || r.ip >= end_code) 
			return (evm_status) {
				.errmsg = "instruction pointer out of code segment",
				.r = r,
			};
		if (r.sp < start_data || r.sp >= end_data) 
			return (evm_status) {
				.errmsg = "stack pointer out of data segment",
				.r = r,
			};
		#endif

		int op   = mem[r.ip].i;

		int arg1    = mem[r.ip+1].i;
		float arg1f = mem[r.ip+1].f;

		int arg2    = mem[r.ip+2].i;
		float arg2f = mem[r.ip+2].f;

		switch (op) {
		case OP_STOP:
			return (evm_status) {.r=r, .stop=1};
		case OP_NOP: 
			break;
		case OP_SYSCALL:
			if(!syscall) return (evm_status) {
				.errmsg = "encountered syscall instruction, but no syscall callback provided",
				.r = r,
			};
			r = syscall(r, memory);
			break;
		case OP_LD:
			CHKREG(arg1);
			CHKMEM(arg2);
			r.r[arg1].i = mem[arg2].i;
			break;
		case OP_ST:
			CHKMEM(arg1);
			CHKREG(arg2);
			mem[arg1].i = r.r[arg2].i;
			break;
		case OP_SET:
			CHKREG(arg1);
			r.r[arg1].i = arg2;
			break;
		case OP_FSET:
			CHKREG(arg1);
			r.r[arg1].f = arg2f;
			break;
		case OP_CPY:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].i = r.r[arg2].i;
			break;
		case OP_PUSH:
			CHKREG(arg1);
			mem[r.sp-- ].i = r.r[arg1].i;
			break;
		case OP_POP:
			CHKREG(arg1);
			r.r[arg1].i = mem[r.sp++ ].i;
			break;
		case OP_ADD:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].i += r.r[arg2].i;
			break;
		case OP_SUB:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].i -= r.r[arg2].i;
			break;
		case OP_MUL:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].i *= r.r[arg2].i;
			break;
		case OP_DIV:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].i /= r.r[arg2].i;
			break;
		case OP_FADD:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].f += r.r[arg2].f;
			break;
		case OP_FSUB:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].f -= r.r[arg2].f;
			break;
		case OP_FMUL:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].f *= r.r[arg2].f;
			break;
		case OP_FDIV:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].f /= r.r[arg2].f;
			break;
		case OP_NOT:
			CHKREG(arg1);
			r.r[arg1].u = ~ r.r[arg1].u;
			break;
		case OP_LNOT:
			CHKREG(arg1);
			r.r[arg1].i = ! r.r[arg1].i;
			break;
		case OP_AND:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].u &= r.r[arg2].u;
			break;
		case OP_OR:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].u |= r.r[arg2].u;
			break;
		case OP_XOR:
			CHKREG(arg1);
			CHKREG(arg2);
			r.r[arg1].u ^= r.r[arg2].u;
			break;
		case OP_JP:
			CHKREG(arg1);
			CHKCOD(arg2);
			if(r.r[arg1].i > 0) {
				r.ip = arg2;
				continue;
			}
			break;
		case OP_JPZ:
			CHKREG(arg1);
			CHKCOD(arg2);
			if(r.r[arg1].i >= 0) {
				r.ip = arg2;
				continue;
			}
			break;
		case OP_JZ:
			CHKREG(arg1);
			CHKCOD(arg2);
			if(r.r[arg1].i == 0) {
				r.ip = arg2;
				continue;
			}
			break;
		case OP_JN:
			CHKREG(arg1);
			CHKCOD(arg2);
			if(r.r[arg1].i < 0) {
				r.ip = arg2;
				continue;
			}
			break;
		case OP_JNZ:
			CHKREG(arg1);
			CHKCOD(arg2);
			if(r.r[arg1].i <= 0) {
				r.ip = arg2;
				continue;
			}
			break;
		case OP_J:
			CHKCOD(arg1);
			r.ip = arg1;
			continue;
		case OP_CVTFI:
			CHKREG(arg1);
			r.r[arg1].i = r.r[arg1].f;
			break;
		case OP_CVTIF:
			CHKREG(arg1);
			r.r[arg1].f = r.r[arg1].i;
			break;
		case OP_PUT:
			CHKREG(arg1);
			printf("%i\n", r.r[arg1].i);
			break;
		case OP_FPUT:
			CHKREG(arg1);
			printf("%f\n", r.r[arg1].f);
			break;
		case OP_LDA:
			CHKREG(arg1);
			CHKMEM(arg2);
			r.r[arg1].i = arg2;
			break;
		case OP_LDD:
			CHKREG(arg1);
			CHKREG(arg2);
			CHKMEM(r.r[arg2].i);
			r.r[arg1].i = mem[r.r[arg2].i].i;
			break;
		case OP_STD:
			CHKREG(arg1);
			CHKREG(arg2);
			CHKMEM(r.r[arg1].i);
			mem[r.r[arg1].i].i = r.r[arg2].i;
			break;
		default: 
			return (evm_status) {
				.errmsg = "encountered unrecognized instruction",
				.r = r,
			};
		}

		r.ip += 1 + evm_ops[op].nargs;

	} while (!single_step);

	return (evm_status) {.r=r};
}

#endif
