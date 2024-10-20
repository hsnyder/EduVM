/*
	Harris M. Snyder, 2023
	This is free and unencumbered software released into the public domain.
*/

#define EVM_IMPLEMENTATION
#include "evm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>


typedef enum {
	TOK_INVALID = 0,
	TOK_FLOATLIT,
	TOK_INTLIT,
	TOK_STRINGLIT,
	TOK_ID,
	TOK_COMMA,
	TOK_COLON,
	TOK_EOF,
	TOK_EOL,
} token_type;

typedef struct {
	token_type type;
	int s_len;
	union {
		int i;
		float f;
		char *s;
	};
} token;

typedef struct {
	char *s;
	int len;
	int where;
} label;

typedef struct {
	int bufsz;
	int pos;
	char *buf;
	int mempos;
	label labels[40];
	int nlabels;
} parse_ctx;

void terminal_state(int newstate)
{
	static int st = 0;
	if (st != newstate) {
		st = newstate;
		if(st) {
			// clear screen
			printf("\x1b[2J");
			// turn on TUI mode
			printf("\x1b[?1049h");
		} else {
			// clear screen
			printf("\x1b[2J");
			// turn off TUI mode
			printf("\x1b[?1049l");
		}
	}
}

static _Noreturn void die (parse_ctx *p, char *fmt, ...)
{
	terminal_state(0);

	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fputc('\n', stderr);

	if (p) {
		fprintf(stderr, "Context: ");
		char *here = p->buf+p->pos;
		char *start = here;
		char *end = here;

		int linecount = 0;
		while (start > p->buf && linecount < 4) {
			start--;
			if(*start == '\n') linecount++;
		}
		linecount = 0;
		while (*end && linecount < 4) {
			end++;
			if(*end == '\n') linecount++;
		}

		char *x = start;
		while (x != end) {
			if (x == here) fprintf(stderr," HERE>>> ");
			char c = *x++;
			fputc(c, stderr);
		}
		fputc('\n', stderr);
	}
	exit(EXIT_FAILURE);
}

#define ssizeof(x) ((long long)sizeof(x))
#define COUNT_ARRAY(x) (ssizeof(x)/ssizeof(x[0]))
void add_label(parse_ctx *p, token t, int where) 
{
	if (p->nlabels == COUNT_ARRAY(p->labels))
	       die (p, "Max label count (%lli) exceeded", COUNT_ARRAY(p->labels));

	p->labels[p->nlabels++] = (label){
		.s = t.s,
		.len = t.s_len,
		.where = where,
	};
}

int lookup(parse_ctx *p, token t)
{
	assert(t.type == TOK_ID);

	for (int i = 0; i < p->nlabels; i++) {
		if(p->labels[i].len == t.s_len && !memcmp(p->labels[i].s, t.s, t.s_len)) {
			return p->labels[i].where;
		}
	}

	char buf[512] = {0};
	memcpy(buf, t.s, t.s_len < ssizeof(buf)-1 ? t.s_len : ssizeof(buf)-1);
	die(p, "No such label: %s", buf);
}


int try_parse_floatlit(parse_ctx *p, float *val)
{
	char *x = p->buf+p->pos;
	char *e = 0;
	float f = strtof(x, &e);
	int nchars_consumed = e-x;
	if (nchars_consumed > 0) *val = f;
	p->pos += nchars_consumed;
	return nchars_consumed;
}

int try_parse_intlit(parse_ctx *p, int *val)
{
	char *x = p->buf+p->pos;
	char *e = 0;
	long l = strtol(x, &e, 0);
	int nchars_consumed = e-x;
	if (nchars_consumed > 0) *val = l;
	p->pos += nchars_consumed;
	return nchars_consumed;
}

int try_parse_stringlit(parse_ctx *p, int *len, char **s)
{
	char *x = p->buf+p->pos;

	if (*x == '\'' || *x == '\"') {

		char *st = x++;
		while (*x != 0 && *x != *st && *x != '\n' && *x != '\r') {
			x++;
		}

		if (*x == *st) {
			*len = x-st-1;
			*s = st+1;
			p->pos = x+1 - p->buf;
			return 1;
		} else {
			die(p, "Unterminated string literal");
		}

	} else return 0;
}

int try_parse_identifier(parse_ctx *p, int *len, char **s)
{
	char *st = p->buf+p->pos;
	char *x = st;

	if ((*x > 0x40 && *x < 0x5b) || (*x > 0x60 && *x < 0x7b)) {
		while ( (*x > 0x40 && *x < 0x5b) || 
			(*x > 0x60 && *x < 0x7b) || 
			(*x > 0x2f && *x < 0x3a) ) {

			x++;
		}

		int l = x-st;
		if (l > 0) { 
			*len = l;
			*s = st;
			p->pos += l;
			return 1;
		}
	}
	return 0;
}

static void skipwhitespace(parse_ctx *p) 
{
	while (p->buf[p->pos] == ' ' || p->buf[p->pos] == '\t') p->pos++;
}

static void skipcomment(parse_ctx *p) 
{
	if(p->buf[p->pos] == '#') {
		while (p->buf[p->pos] != '\n' && p->buf[p->pos] != '\0') p->pos++;
	}
}


token tok_next(parse_ctx *p)
{
	skipwhitespace(p);
	skipcomment(p);

	token t = {0};

	switch (p->buf[p->pos]) {
	case 0:
		t.type = TOK_EOF;
		return t;
	case ':':
		t.type = TOK_COLON;
		p->pos++;
		return t;
	case ',':
		t.type = TOK_COMMA;
		p->pos++;
		return t;
	case '\n':
	case '\r':
		t.type = TOK_EOL;
		p->pos++;
		return t;
	}

	parse_ctx c = *p;
	int ncharsf = try_parse_floatlit(&c, &t.f);
	c = *p;
	int ncharsi = try_parse_intlit(&c, &t.i);
	c = *p;

	if (ncharsf > ncharsi && try_parse_floatlit(&c, &t.f)) {
		t.type = TOK_FLOATLIT;
		*p = c;
		return t;
	}

	c = *p;
	if(try_parse_intlit(&c, &t.i)) {
		t.type = TOK_INTLIT;
		*p = c;
		return t;
	}
	
	c = *p;
	if(try_parse_stringlit(&c, &t.s_len, &t.s)) {
		t.type = TOK_STRINGLIT;
		*p = c;
		return t;
	}

	c = *p;
	if(try_parse_identifier(&c, &t.s_len, &t.s)) {
		t.type = TOK_ID;
		*p = c;
		return t;
	}

	die(p, "Unrecognized token");
}

void swallow(parse_ctx *p, int n)
{
	for (int i = 0; i < n; i++) (void)tok_next(p);
}

int idcmp(token t, const char *str) 
{
	if(t.type != TOK_ID) return 0;
	int l = strlen(str);
	if(t.s_len != l) return 0;
	return !memcmp(str, t.s, l);
}


int data(parse_ctx *p, evm_word *mem)
{
	parse_ctx tmp = *p;
	token t[5] = {
		tok_next(&tmp),
		tok_next(&tmp),
		tok_next(&tmp),
		tok_next(&tmp),
		tok_next(&tmp),
	};
	
	if (t[0].type == TOK_EOF) {
		die(p, "Unexpected end of file (no 'start' statement)");
	}

	else if (t[0].type == TOK_EOL) {
		// empty line
		swallow(p, 1);
		return 1;
	}

	else if (
		t[0].type == TOK_ID &&
		t[1].type == TOK_COLON && 
		idcmp(t[2], "zeros") &&
		t[3].type == TOK_INTLIT &&
		t[4].type == TOK_EOL
		) 
	{ 
		// labeled zeros statement
		add_label(p, t[0], p->mempos);
		for (int i = 0; i < t[3].i; i++) {
			mem[p->mempos].i = 0;
			p->mempos += 1;
		}
		swallow(p, 5);
		return 1;
	}

	else if (
		t[0].type == TOK_ID &&
		t[1].type == TOK_COLON &&
		t[2].type == TOK_INTLIT &&
		t[3].type == TOK_EOL
		) 
	{
		// labeled int literal
		add_label(p, t[0], p->mempos);
		mem[p->mempos].i = t[2].i;
		p->mempos += 1;
		swallow(p, 4);
		return 1;
	}

	else if (
		t[0].type == TOK_ID &&
		t[1].type == TOK_COLON &&
		t[2].type == TOK_FLOATLIT &&
		t[3].type == TOK_EOL
		) 
	{
		// labeled float literal
		add_label(p, t[0], p->mempos);
		mem[p->mempos].f = t[2].f;
		p->mempos += 1;
		swallow(p, 4);
		return 1;
	}

	else if (
		t[0].type == TOK_ID &&
		t[1].type == TOK_COLON &&
		t[2].type == TOK_STRINGLIT &&
		t[3].type == TOK_EOL
		) 
	{
		// labeled string literal
		add_label(p, t[0], p->mempos);
		memcpy(mem+p->mempos, t[2].s, t[2].s_len);
		p->mempos += t[2].s_len/4;
		if(t[2].s_len % 4) p->mempos++;
		swallow(p, 4);
		return 1;
	}

	else if (
		idcmp(t[0], "zeros") &&
		t[1].type == TOK_INTLIT &&
		t[2].type == TOK_EOL
		) 
	{
		// unlabeled zeros statement
		for (int i = 0; i < t[1].i; i++) {
			mem[p->mempos].i = 0;
			p->mempos += 1;
		}
		swallow(p, 3);
		return 1;
	}

	else if (
		t[0].type == TOK_INTLIT &&
		t[1].type == TOK_EOL
		) 
	{
		// unlabeled int literal
		mem[p->mempos].i = t[0].i;
		p->mempos += 1;
		swallow(p, 2);
		return 1;
	}

	else if (
		t[0].type == TOK_FLOATLIT &&
		t[1].type == TOK_EOL
		) 
	{
		// unlabeled float literal
		mem[p->mempos].f = t[0].f;
		p->mempos += 1;
		swallow(p, 2);
		return 1;
	}

	else if (
		t[0].type == TOK_STRINGLIT &&
		t[1].type == TOK_EOL
		) 
	{
		// unlabeled string literal
		memcpy(mem+p->mempos, t[0].s, t[0].s_len);
		p->mempos += t[0].s_len/4;
		if(t[0].s_len % 4) p->mempos++;
		swallow(p, 2);
		return 1;
	}

	else if (idcmp(t[0], "start") && t[1].type == TOK_EOL) {
		swallow(p, 2);
		return 0;
	}

	die(p, "Invalid line in data section");
}

int parsereg(token t)
{
	if (t.type != TOK_ID) return -1;
	if (t.s_len != 2) return -1;
	if(!memcmp(t.s, "sp", 2)) return 0;
	char n[2] = {t.s[1], 0};
	int r = atoi(n);
	if (r < 1) return -1;
	if (r > EVM_NUMREGS) return -1;
	return r;
}

void checkarg(parse_ctx *p, token t, evm_op_t op, int argno)
{
	if (op.argtypes[argno] == EVM_REG && t.type == TOK_ID && (parsereg(t) >= 0)) return;
	if (op.argtypes[argno] == EVM_MEM && t.type == TOK_ID) return;
	if (op.argtypes[argno] == EVM_MEM && t.type == TOK_INTLIT) return;
	if (op.argtypes[argno] == EVM_IMMI && t.type == TOK_INTLIT) return;
	if (op.argtypes[argno] == EVM_IMMF && t.type == TOK_FLOATLIT) return;
		
	die(p, "%s instruction first argument must be %s", op.str,
			op.argtypes[argno] == EVM_REG ? "a register" : 
			op.argtypes[argno] == EVM_MEM ? "a label or memory address" :
			op.argtypes[argno] == EVM_IMMI ? "an immediate value (integer)" :
			op.argtypes[argno] == EVM_IMMF ? "an immediate value (float)" : "[bug]" );

}

int emit_argument(parse_ctx *p, evm_op_t op, token t, int a)
{
	if (op.argtypes[a] == EVM_REG) {
		int r = parsereg(t);
		if (r<0) die(p, "Invalid register (argument %i)", a+1);
		return r;
	} else 
	if (op.argtypes[a] == EVM_MEM) {
		if (t.type == TOK_ID) {
			return lookup(p, t);
		} else if (t.type == TOK_INTLIT) {
			return t.i;
		} else assert(0);
	} else 
	if (op.argtypes[a] == EVM_IMMI) {
		return t.i;
	} else 
	if (op.argtypes[a] == EVM_IMMF) {
		return t.i; // haha
	} else assert(0);
}


int statement(parse_ctx *p, int pass, evm_word *mem)
{
	parse_ctx tmp = *p;
	token t[5] = {
		tok_next(&tmp),
		tok_next(&tmp),
		tok_next(&tmp),
		tok_next(&tmp),
		tok_next(&tmp),
	};

	if (t[0].type == TOK_EOF) return 0;

	else if (t[0].type == TOK_EOL) {
		swallow(p,1);
		return 1;
	}

	else if (
		t[0].type == TOK_ID &&
		t[1].type == TOK_COLON 
		)
	{
		// label
		if(pass == 1) add_label(p, t[0], p->mempos);
		swallow(p,2);
		return 1;
	}

	else if (t[0].type == TOK_ID)
	{
		// potentially an instruction
		for(int i = 0; i < OP_INVAL; i++)
		{
			evm_op_t op = evm_ops[i];
			assert(op.opcode == i);

			if (idcmp(t[0],op.str)) {
				
				if (op.nargs == 0) {
					if (t[1].type != TOK_EOL) {
						die(p, "%s instruction takes no arguments (newline must follow)", op.str);
					}
					
					if (pass == 2) {
						mem[p->mempos].i = op.opcode;
					}
					p->mempos += 1;

					swallow(p,2);
					return 1;
				}

				if (op.nargs == 1) {
					checkarg(p, t[1], op, 0);
					if (t[2].type != TOK_EOL) {
						die(p, "%s instruction takes one argument (newline must follow)", op.str);
					}

					if (pass == 2) {
						mem[p->mempos].i = op.opcode;
						p->mempos += 1;
						mem[p->mempos].i = emit_argument(p, op, t[1], 0);
						p->mempos += 1;
					}
					else p->mempos += 2;

					swallow(p,3);
					return 1;
				}

				if (op.nargs == 2) {
					checkarg(p, t[1], op, 0);
					if (t[2].type != TOK_COMMA) {
						die(p, "instruction arguments must be separated by a comma");
					}
					checkarg(p, t[3], op, 1);

					if (pass == 2) {
						mem[p->mempos].i = op.opcode;
						p->mempos += 1;
						for (int a = 0; a < 2; a++) {
							token argtok = a == 0 ? t[1] : t[3];
							mem[p->mempos].i = emit_argument(p, op, argtok, a);
							p->mempos += 1;
						}
					}
					else p->mempos += 3;
					
					if (t[4].type != TOK_EOL) {
						die(p, "%s instruction takes one argument (newline must follow)", op.str);
					}
					swallow(p,5);
					return 1;
				}

				assert(0);

			}
		}
		die(p, "Invalid instruction");
	}
	
	die(p, "Invalid source line");
}

const char *assemble(int bufsz, char *buf)
{
	const int allocsz = 1<<20;
	evm_mem *img = malloc(allocsz);
	if(!img) die(0, "out of memory");
	img->magic = EVM_MAGIC;
	img->version = 1;

	parse_ctx p = {.bufsz=bufsz, .buf=buf};
	while (data(&p, img->mem));
	assert(p.mempos < allocsz);
	img->len_data = p.mempos;

	parse_ctx p_backup = p;
	while (statement(&p, 1, img->mem));
	assert(p.mempos < allocsz);

	p.pos = p_backup.pos;
	p.mempos = p_backup.mempos;
	while (statement(&p, 2, img->mem));
	assert(p.mempos < allocsz);
	img->len_code = p.mempos - img->len_data;

	fwrite(img, 1, ssizeof(*img) + 4*p.mempos, stdout);
	return 0;

	/* Debug the tokenizer 

	_Bool cont = 1;
	while(cont) {
		token t = tok_next(&p);

		switch (t.type) {
		case TOK_INVALID: 
			printf("TOK_INVALID\n");
			break;
		case TOK_FLOATLIT:
			printf("TOK_FLOATLIT %f\n", t.f);
			break;
		case TOK_INTLIT:
			printf("TOK_INTLIT %i\n", t.i);
			break;
		case TOK_STRINGLIT:
			printf("TOK_STRINGLIT ");
			for(int i = 0; i < t.s_len; i++) {
				char c = t.s[i];
				fputc(c, stdout);
			}
			fputc('\n', stdout);
			break;
		case TOK_ID:
			printf("TOK_ID ");
			for(int i = 0; i < t.s_len; i++) {
				char c = t.s[i];
				fputc(c, stdout);
			}
			fputc('\n', stdout);
			break;
		case TOK_COMMA:
			printf("TOK_COMMA\n");
			break;
		case TOK_COLON:
			printf("TOK_COLON\n");
			break;
		case TOK_EOF:
			printf("TOK_EOF\n");
			cont = 0;
			break;
		case TOK_EOL:
			printf("TOK_EOL\n");
			break;
		default:
			die(&p, "wtf?");
			break;
		}
	}
	*/
}

const char *disassemble(int bufsz, unsigned char *buf, int ip)
{
	evm_mem *img = (evm_mem*)buf;	
	const char *errmsg = validate_evm_mem(bufsz, img);
	if (errmsg) return errmsg;

	evm_word *mem = img->mem;

	printf("--- DATA SECTION ---------------------------------\n");
	printf("address     hex        decimal int      float   ascii\n");
	for (int i = 0; i < img->len_data; i++)
	{
		char ascii[5] = {0};
		memcpy(ascii, &mem[i], 4);
		for(int i = 0; i < 4; i++) 
			ascii[i] = (ascii[i] > 31 && ascii[i] < 127) ? ascii[i] : '.';
		printf("%.8x:   %.8x   %11i   %8f   %s\n", i, mem[i].u, mem[i].i, mem[i].f, ascii);
	}
	printf("--- CODE SECTION ---------------------------------\n");
	int i = img->len_data;

	while (i < img->len_code+img->len_data)
	{
		char indicator = i == ip ? '>' : ' ';

		int op = mem[i].i;
		if (op < OP_STOP || op >= OP_INVAL) 
			die(0, "Illegal instruction 0x%0.8x", op);
		assert(evm_ops[op].opcode == op);

		// print the address
		printf("%.8x:   ", i);

		// print the actual memory in hex
		printf("%.8x ", op);
		if (evm_ops[op].nargs > 0) 
			printf("%.8x ", mem[i+1].u);
		else 
			printf("         ");

		if (evm_ops[op].nargs == 2) 
			printf("%.8x %c%c", mem[i+2].u, indicator, indicator);
		else 
			printf("         %c%c", indicator, indicator);

		// print the disassembly 
		printf("%s", evm_ops[op].str);
		for(int k = strlen(evm_ops[op].str); k < 8; k++)
			fputc(' ', stdout);

		if (evm_ops[op].nargs > 0) 
		{
			if (evm_ops[op].argtypes[0] == EVM_REG) {
				if(mem[i+1].i == 0)
					printf("sp");
				else
					printf("r%i", mem[i+1].i);
			} else if (evm_ops[op].argtypes[0] == EVM_MEM) {
				printf("%x", mem[i+1].u);
			} else if (evm_ops[op].argtypes[0] == EVM_IMMI) {
				printf("%i", mem[i+1].i);
			} else if (evm_ops[op].argtypes[0] == EVM_IMMF) {
				printf("%f", mem[i+1].f);
			} else assert(0);

			if (evm_ops[op].nargs > 1) 
			{
				printf(", ");
				if (evm_ops[op].argtypes[1] == EVM_REG) {
					if(mem[i+2].i == 0)
						printf("sp");
					else
						printf("r%i", mem[i+2].i);
				} else if (evm_ops[op].argtypes[1] == EVM_MEM) {
					printf("%x", mem[i+2].u);
				} else if (evm_ops[op].argtypes[1] == EVM_IMMI) {
					printf("%i", mem[i+2].i);
				} else if (evm_ops[op].argtypes[1] == EVM_IMMF) {
					printf("%f", mem[i+2].f);
				} else assert(0);
			}
		}

		printf("\n");
		i += 1 + evm_ops[op].nargs;
	}

	return 0;
}

const char* interactive(int bufsz, unsigned char *buf)
{
	evm_mem *memory = (void*)buf;
	const char *err = validate_evm_mem(bufsz, memory);  
	if(err) return err;

	terminal_state(1);

	char garbage[500] = {0};

	int start_data = 0;
	int end_data   = memory->len_data;
	int start_code = end_data;
	int end_code   = start_code + memory->len_code;

	evm_regs state = {.ip = start_code, .sp = end_data-1};
	evm_status stat = {0};

	while(!stat.errmsg) {
		// clear screen
		printf("\x1b[2J");
		err = disassemble(bufsz, buf, state.ip);	

		printf("--- CPU STATE ------------------------------------\n");
		printf("\tip  %.8x\n", state.ip);
		printf("\tsp  %.8x\n", state.sp);
		for(int i = 1; i < ssizeof(state.r)/ssizeof(state.r[0]); i++) 
			printf("\tr%i  %.8x (%i) (%f)\n", i, state.r[i].u, state.r[i].i, state.r[i].f);

		if(stat.stop || err) break;

		fgets(garbage, ssizeof(garbage), stdin);
		stat = evm_run(bufsz, memory, 0, &state, 1);
		state = stat.r;
	}
	terminal_state(0);
	return err;
}



const char* ingest_file(int bufsz, unsigned char *buf, char *fname)
{
	FILE *f = fopen(fname, "rb");
	if(!f) return "couldn't open specified file";

	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);

	if(sz > bufsz) 
		return "couldn't read specified file, buffer too small";

	if(sz != fread(buf, 1, bufsz, f)) 
		return "error while reading specified file";

	fclose(f);
	return 0;
}

int main (int argc, char **argv)
{
	enum { RUN, ASSEMBLE, DISASSEMBLE, INTERACTIVE } mode = RUN;

	argv++;
	for(; *argv; argv++) {
		if (!strcmp(*argv, "-a")) {
			mode = ASSEMBLE;
		} else if (!strcmp(*argv, "-d")) {
			mode = DISASSEMBLE;
		} else if (!strcmp(*argv, "-i")) {
			mode = INTERACTIVE;
		} else {
			static unsigned char buf[4*1<<20] = {0};
			const char *err = ingest_file((int)sizeof(buf), buf, *argv);

			if(!err) switch(mode) {
			case ASSEMBLE:
				err = assemble((int)sizeof(buf), (char*)buf);
				break;
			case DISASSEMBLE:
				err = disassemble((int)sizeof(buf), buf, 0);
				break;
			case RUN: {
					evm_status s = evm_run((int)sizeof(buf), (evm_mem*)buf, 0, 0, 0);
					if (s.errmsg) {
						fprintf(stderr, "%s\n", s.errmsg);
						fprintf(stderr, "\tip  %i\n", s.r.ip);
						fprintf(stderr, "\tsp  %i\n", s.r.sp);
						for(int i = 1; i < ssizeof(s.r.r)/ssizeof(s.r.r[0]); i++) 
							fprintf(stderr, "\tr%i  %i (%x) (%f)\n", i, s.r.r[i].i, s.r.r[i].u, s.r.r[i].f);
						exit(EXIT_FAILURE);
					}
					exit(EXIT_SUCCESS);
				}
			case INTERACTIVE:
				err = interactive((int)sizeof(buf), buf);
				break;
			default:
				assert(0);
				break;
			}

			if(err) {
				fprintf(stderr, "%s\n", err);
				exit(EXIT_FAILURE);
			}

			break;
		}
	}
	return 0;
}
