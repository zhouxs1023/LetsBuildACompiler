//
// Scanning functions
//

// scanning the input affects the following global variables:
//  Token   - a string holding the scanned token
//  TokenId - the integer id of the token
//  Look    - the next non-whitespace char in the stream
//

char *Keywords[] = {
	"IF",
	"ELSE",
	"ENDIF",
	"END",
	"BEGIN",
	"VAR",
	"WHILE",
	"ENDWHILE",
	"READ",
	"WRITE"
	};

#define NUMKEYWORDS sizeof(Keywords)/sizeof(Keywords[0])

int IsOp(char c)
{
	switch(c){
	// fallthru everywhere
	case '+':
	case '-':
	case '/':
	case '*':
	case '<':
	case '>':
	case ':':
	case '=':
		return 1;
		break;
	}
	return 0;
}

// writes an identifier (or keyword)  to the string at `name`
void GetName(char *name)
{
	int i=0;
	if (!IsAlpha(Look)) {
		Expected("Identifier");
	}
	while ( IsAlNum(Look) && i<MAXNAME-1 ) {
		name[i++] = toupper(Look);
		GetChar();
	}
	name[i] = '\0';
}

// Look for a number and return it as an int
void GetNum(void)
{
	Value = 0;
	if (!IsDigit(Look)) {
		Expected("Integer");
	}
	while ( IsDigit(Look) ) {
		Value = (10*Value) + (int)(Look - '0');
		GetChar();
	}
	TokenId = T_NUMBER;
}

// writes an operator to string `opstr`
void GetOp(char *opstr)
{
	int i=0;
	if (!IsOp(Look)) {
		Expected("Operator");
	}
	while ( IsOp(Look) && i<1 ) {
		opstr[i++] = Look;
		GetChar();
	}
	opstr[i] = '\0';
	TokenId = T_OPER;
}

void GetPunctuation(char *puncstr)
{
	message("Grabbing punctuation");
	puncstr[0] = Look;
	puncstr[1] = '\0';
	TokenId = T_OTHER;
	GetChar();
}

// Next() - get the next identifier, number, keyword or operator
// in the input stream. Jumps any leading whitespace, including
// newlines.
void Next(void)
{
	SkipWhite();
	message("Next() ->");
	if (IsAlpha(Look) ) {
		GetName(Token);
	} else if (IsDigit(Look) ) {
		GetNum();
	} else if (IsOp(Look) ) {
		GetOp(Token);
	} else {
		GetPunctuation(Token);
	}
}

// match a string to the current read token
void MatchString(char *str)
{
	message("Checking \"%s\" vs \"%s\"", str, Token);
	if ( strncmp(str, Token, MAXNAME) != 0 ) {
		Expected(str);
	}
	Next();
}

// optionally match a semi-colon
void Semi(void)
{
	message("Optional Semicolon...");
	if (Token[0]==';') {
		MatchString(";");
	}
}

void clear_ident(char *ident)
{
	int i;
	for (i=0; i<MAXNAME; i++) {
		ident[i] = '\0';
	}
}

// If the input string matches an entry in the table, return
// the entry index+1. If not, return 0.
// brute force. maybe in future explore gperf
int Lookup(char *token) 
{
	int found;
	int tok_id;

	//printf("Is %s in the list of keywords?\n", token);	
	tok_id = NUMKEYWORDS;
	found = 0;
	while ( (tok_id>0) && (found==0) ) {
		//printf(" \"%s\" vs \"%s\" -> ", Keywords[tok_id-1], token);
		//printf(" %d\n", strncmp(token, Keywords[tok_id-1], MAXNAME) );
		if ( strncmp(token, Keywords[tok_id-1], MAXNAME) == 0 ) {
			found = 1;
		} else {
			tok_id--;
		}
	}
	return tok_id;
}

// Scan sets Token and TokenID
// if it's not in the keyword list, say it's an identifier
void Scan(void)
{
	TokenId = Lookup(Token);
	if (TokenId == T_OTHER) {
		TokenId = T_IDENT;
	}
	//printf("# Scanned...\n");
	//printf("#  Token: \"%s\"\n", Token);
	//printf("#     Id: %d\n", TokenId);
}

/* skip comments.
 First comment character in the stream is replaced by '@'
*/
void SkipComment(void) {
	message("Skipping comments...");
	while (1) {
		GetCharX();
		if (Look == '*') {
			GetCharX();
			if (Look == '/') {
				break;
			}
		}
		/* need to still update line/col counts in comments */
		if (Look == '\n') {
			lineno++;
			colno = 0;
		}
	}
	GetChar();
	message("Comment done");
}

