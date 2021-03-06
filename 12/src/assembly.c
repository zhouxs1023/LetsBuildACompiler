char asm_header[] = "#PURPOSE:  KISS Program \n\
.section .data \
\n";

char asm_prolog[] = "\
.section .text \n\
.include \"io.s\" \n\
.include \"conversions.s\" \n\
.globl _start \n\
\n\
_start: \n\
";

char asm_epilog[] = "\
.epilog:\n\
\tmovl %eax,%ebx \n\
\tmovl $1, %eax	# sys_exit \n\
\tint $0x80	# call \n\
";

void AsmHeader(void) { printf("%s\n", asm_header); }
void AsmProlog(void) { printf("%s\n", asm_prolog); }
void AsmEpilog(void) { printf("%s\n", asm_epilog); }

void Emit(const char *msg)
{
	printf("\t%s", msg);
}

void EmitLn(const char *msg)
{
	Emit(msg);
	printf("\n");
}

/* ------------------------------------------------------------------------ */
// Arithmetic

void Clear(void) { EmitLn("movl $0, %eax"); }
void Negate(void) { EmitLn("neg %eax"); }
void Push(void) { EmitLn("pushl %eax"); }

void LoadConst(int val) {
	char code[MAXMSG];
	snprintf(code, MAXMSG, "movl $%d, %%eax", val);
	EmitLn(code);
}

void Undefined(char *name)
{
	char msg[MAXMSG];
	snprintf(msg, MAXMSG, "Undefined identifier \'%s\'", name);
	Abort(msg);
}

void LoadVar(char *name) {
	char code[MAXMSG];
	if (!InTable(name) ) {
		Undefined(name);
	}
	snprintf(code, MAXMSG, "movl $%s, %%edx", name);
	EmitLn(code);
	EmitLn("movl (%edx), %eax");
}

void PopAdd(void)
{
	EmitLn("popl %ebx");
	EmitLn("addl %ebx, %eax");
}

void PopSub(void)
{
	EmitLn("popl %ebx");
	EmitLn("subl %ebx, %eax");
	Negate();
}

void PopMul(void)
{
	EmitLn("popl %ebx");
	EmitLn("imul %ebx");
}

void PopDiv(void)
{
	EmitLn("movl %eax,%ebx");
	EmitLn("popl %eax");
	EmitLn("movl $0,%edx");
	EmitLn("divl %ebx");
}

void Store(char *name)
{
	char code[MAXMSG];
	if (!InTable(name) ) {
		Undefined(name);
	}
	snprintf(code, MAXMSG, "movl $%s,%%edx", name);
	EmitLn(code);
	EmitLn("movl %eax,(%edx)\t\t# assignment");
}

/* ------------------------------------------------------------------------ */
// Boolean

void NotIt(void) { EmitLn("notl %eax"); }

void PopOr(void) {
	EmitLn("popl %ebx");
	EmitLn("orl %edx, %eax");
}

void PopXor(void) {
	EmitLn("popl %ebx");
	EmitLn("xorl %ebx, %eax");
}

void PopAnd(void) {
	EmitLn("popl %ebx");
	EmitLn("andl %ebx, %eax");
}

void PopCompare(void) {
	EmitLn("popl %ebx");
	EmitLn("cmpl %eax, %ebx");
}

/* NOTE: do logical opposite then subtract 1 to get true == all ones*/
void SetEqual(void) {
	EmitLn("setne %al");
	EmitLn("movsx %al, %eax");
	EmitLn("subl $1, %eax");
}

/* NOTE: do logical opposite then subtract 1 to get true == all ones*/
void SetNEqual(void) {
	EmitLn("sete %al");
	EmitLn("movsx %al, %eax");
	EmitLn("subl $1, %eax");
}

/* NOTE: do logical opposite then subtract 1 to get true == all ones*/
void SetGreater(void) {
	EmitLn("setle %al");
	EmitLn("movsx %al, %eax");
	EmitLn("subl $1, %eax");
}

/* NOTE: do logical opposite then subtract 1 to get true == all ones*/
void SetLess(void) {
	EmitLn("setge %al");
	EmitLn("movsx %al, %eax");
	EmitLn("subl $1, %eax");
}

/* NOTE: do logical opposite then subtract 1 to get true == all ones*/
void SetLessOrEqual(void) {
	EmitLn("setg %al");
	EmitLn("movsx %al, %eax");
	EmitLn("subl $1, %eax");
}

/* NOTE: do logical opposite then subtract 1 to get true == all ones*/
void SetGreaterOrEqual(void) {
	EmitLn("setl %al");
	EmitLn("movsx %al, %eax");
	EmitLn("subl $1, %eax");
}


/* ------------------------------------------------------------------------ */
// Conditional
void Branch(char *label)
{
	char code[MAXMSG];
	message("JUMP to %s", label);
	snprintf(code, MAXMSG, "jmp .%s", label);
	EmitLn(code);
}

void BranchFalse(char *label)
{
	char code[MAXMSG];
	message("Branch if 0 to %s", label);
	EmitLn("testl %eax, %eax");
	snprintf(code, MAXMSG, "jz .%s", label);
	EmitLn(code);
}

/* ------------------------------------------------------------------------ */
// IO

// Loads accumulator with integer read from the command line
void ReadVar(char *name)
{
	EmitLn("call read_int");
	Store(name);
}

// Print %eax value in terminal
void WriteVar()
{
	//EmitLn("pushl %eax");
	EmitLn("call write_int");
	//EmitLn("popl %eax");
}

/* ------------------------------------------------------------------------ */
// Memory allocation

void Alloc(char *name)
{
	int val = 0;
	char sign = ' ';
	char msg[MAXMSG];

	message("Allocating");

	// check for dupes
	if (InTable(name)) {
		snprintf(msg, MAXMSG, "Dupe variable: %s\n", name);
		Abort(msg);
	} 
	AddEntry(name);

	// write storage in .data section
	Next(); // sucks up the identifier
	if (Token[0]=='=') {
		MatchString("=");
		if (Token[0]=='-') {
			sign = '-';
			MatchString("-");
		}
		val = Value;
		Next(); // sucks up integer
	}
	printf("%s:\t .long %c%d\n", name, sign, val );
	printf("### Token: \"%s\"\n", Token);
}
