#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define EXPECTARG 2
#define VAR_LENGTH 15
typedef struct {
	int nan;
	int inf;
	int value;
	char name[15];
} VAR;

typedef enum {
	STP, KIL, LET, PUT, GET, CON, ELCON, FOR, TIL, END_BLOCK, ASN, // Explicit functions
	DVD, MLT, SUB, ADD,							 	//Explicit arthematic operators
	EQL, NTE, LES, GRT, LOE, GOE,					//Explicit relational operators
	AND, ORR, NOT,										//Explicit Short circuit operators
	VARIABLE, STRING, CONSTANT, END_STMT, GRD,
	TOTAL_TOKENS
} TOKEN_VAL;

typedef struct {
	char pattern[10];
	int weight;
} TOKEN;

typedef struct {
	TOKEN_VAL token;
	int weight;
	int index;
	int length;
	int constVal;
	VAR *var;
} TOKEN_ARR;

typedef struct {
	VAR *varTable;		//pointer to symbol table for storing user created variables
	char *string;		//pointer to buffer, where string read from file is stored
	TOKEN_ARR *tokenArr;//lexer generated array which stores tokens and references for parsing and executing
	int noOfVars;
	int lineNum;
	int noOfTokens;
	int tokenArrCap;
	int varTableCap;
} PSTAT;

void aboutTool ();
void loop (FILE *, int);
char reader (FILE *, PSTAT *);
char lexer (PSTAT *);
char parser (PSTAT *);
PSTAT *setupEnvironment ();
void clearEnvironment (PSTAT *, FILE *);

/***************************\
|------------MAIN-----------|
\***************************/

int main (int argc, char *argv[]) {
	if (argc > EXPECTARG) {
		printf("Error: Too many targets\n");
		for (int i = 1; i < argc; i++) {
			printf("       %s\n", argv[i]);
		}
		return 1;
	}
	FILE *inputLoc = NULL;
	if (argc == 1) {
		inputLoc = stdin;
		aboutTool();
	} else {
		inputLoc = fopen(argv[1], "r");
		if (inputLoc == NULL) {
			printf("Error: Target not found in present working directory\n");
			return 1;
		}
	}//end of deciding
	loop(inputLoc, !(argc - 1));
	return 0;
}

/***************************\
|--------ABOUT_TOOL-_-------|
\***************************/

void aboutTool () {
	printf("SIL V0.1\n");
	printf("this is a prototype for LAMDA\n");
	return;
}

/***************************\
|-----------LOOP------------|
\***************************/

#define EXITCHAR 0
#define NOISSUE 1
#define LEXE_RECYCLE 2
#define CONTCHAR 3
#define TRUE 4
#define FALSE 5

void loop(FILE *inputLoc, int isShell) {
	char status = EXITCHAR;
	PSTAT *info = setupEnvironment(); 
	if (info == NULL) {
		printf("UNKOWN ERROR 00: Failed to initiate program");
		return;
	};
	do {
		if (isShell) printf("SIL: ");
		status = reader(inputLoc, info);
	} while (status != EXITCHAR);
	clearEnvironment(info, inputLoc); 
}

/*************************\
|----setupEnvironment-----|
\*************************/

#define INIT_BUFF_SIZE 32

PSTAT *setupEnvironment() {
	PSTAT *info = malloc(sizeof(PSTAT));
	if (info == NULL) return NULL;
	info->varTableCap = 16;
	info->varTable = malloc(info->varTableCap * sizeof(VAR));
	if (info->varTable == NULL) return NULL;
	info->string = NULL;
	info->tokenArr = NULL;
	info->noOfVars = 0;
	info->lineNum = 0;
	info->noOfTokens = 0;
	info->tokenArrCap = INIT_BUFF_SIZE;
	return info;
}

#undef INIT_BUFF_SIZE

/************************\
|----clearEnvironment----|
\************************/

void clearEnvironment(PSTAT *info, FILE *inputLoc) {
	if (info->varTable != NULL) free(info->varTable);
	if (info->string != NULL) free(info->string);
	if (info->tokenArr != NULL) free(info->tokenArr);
	free(info);
	if (inputLoc != stdin) fclose(inputLoc);
}

/***************************\
|----------READER-----------|
\***************************/

#define INIT_BUFF_SIZE 64
#define GROWTH_FACTOR 2

char reader(FILE *inputLoc, PSTAT *info) {	
	int c = fgetc(inputLoc);
	if (c == EOF) return EXITCHAR; 
	while (c == '\n') {
		info->lineNum++;
		c = fgetc(inputLoc);
	}

	int size = INIT_BUFF_SIZE, index = 0, comment = 0, firstChar = 0, doubleQuote = 0, extraSpace = 0, braceDepth = 0;
	if (info->string != NULL) free(info->string);
	info->string = malloc(size * sizeof(char));

	do {
		comment = (!doubleQuote && c == '@' ? !comment : comment);
		firstChar = (firstChar || (c != EOF && c != ' ' && c != '\t' && c != '@' && !comment) ? 1 : 0);
		doubleQuote = (c == '"' && info->string[index-1] != '\\' ? !doubleQuote : doubleQuote);
		if (!firstChar || (!doubleQuote && (comment || c == '@'))) goto NEXT_CHAR;

		if (!doubleQuote && (c == ' ' || c == '\t')) {
			if (info->string[index - 1] == '\n') goto NEXT_CHAR;
			c = ' ';
			if (extraSpace) goto NEXT_CHAR;
			extraSpace = 1;
		} else extraSpace = 0;

		if (!doubleQuote && c == '{') braceDepth++;
		if (braceDepth > 0 && !doubleQuote && c == '}') braceDepth--;

		info->string[index] = (char)c;
		index++;

		if (index + 1 >= size) {
			size *= GROWTH_FACTOR;
			char *temp = realloc(info->string, size * sizeof(char));
			if (!temp) {
				printf("UNKNOWN ERROR 02: failed to reallocate memory to a built in data structure");
				return EXITCHAR;
			}
			info->string = temp;
		}

NEXT_CHAR:
		c = fgetc(inputLoc);
	} while ((braceDepth || c != '\n') && c != EOF);
	if (!firstChar) {
		info->lineNum++;
		return NOISSUE;
	}
	
	if (info->string[index - 1] == ' ') info->string[index - 1] = '\0';
	else info->string[index] = '\0';
	return lexer(info);
}

#undef INIT_BUFF_SIZE 

/***************************\
|-----------LEXER-----------|
\***************************/

char lexeStop(PSTAT *, int *);
char lexeKill(PSTAT *, int *);
char lexeGet(PSTAT *, int *);
char lexeLet(PSTAT *, int *);
char lexePut(PSTAT *, int *);
char lexeIf(PSTAT *, TOKEN [], int *);
char lexeFor(PSTAT *, TOKEN [], int *);
char lexeTill(PSTAT *, TOKEN [], int *);
char lexeAsn(PSTAT *, TOKEN [], int *);
int pushToken(TOKEN_ARR *, PSTAT *);
char recycle (PSTAT *, int *);
char lexeVarFinder (PSTAT *, int *);
char lexeVarMatcher (PSTAT *, char [], int);
int checkCondition (PSTAT *, int *);
int checkVar (PSTAT *, int *);
void printError01 (PSTAT *);
char reportTrash (PSTAT *, int *);
char lexeExpression (PSTAT *, int *, int *);
char lexeNumber (PSTAT *, int *);
char lexeCondition (PSTAT *, TOKEN [], int *, int, int *);

#define INIT_BUFF_SIZE 32
#define INIT_STRING_SIZE 64

char parserTesting (PSTAT *info) {
	for (int i = 0; i < info->noOfTokens; i++) {
		printf("Token %d: ", i);
		switch (info->tokenArr[i].token) {
			case STP: printf("STOP\n"); break;
			case KIL: printf("KILL\n"); break;
			case LET: printf("LET\n"); break;
			case PUT: printf("PUT\n"); break;
			case GET: printf("GET\n"); break;
			case CON: printf("IF\n"); break;
			case ELCON: printf("ELIF\n"); break;
			case FOR: printf("FOR\n"); break;
			case TIL: printf("WHILE\n"); break;
			case ASN: printf("ASSIGNMENT\n"); break;
			case VARIABLE: printf("VARIABLE (%s)\n", info->tokenArr[i].var->name); break;
			case DVD: printf("DIVIDE\n"); break;
			case MLT: printf("MULTIPLY\n"); break;
			case SUB: printf("SUBTRACT\n"); break;
			case ADD: printf("ADD\n"); break;
			case EQL: printf("EQUALS\n"); break;
			case NTE: printf("NOT EQUALS\n"); break;
			case LES: printf("LESS THAN\n"); break;
			case GRT: printf("GREATER THAN\n"); break;
			case LOE: printf("LESS OR EQUALS\n"); break;
			case GOE: printf("GREATER OR EQUALS\n"); break;
			case AND: printf("AND\n"); break;
			case ORR: printf("OR\n"); break;
			case NOT: printf("NOT\n"); break;
			case CONSTANT: printf("CONSTANT (%d)\n", info->tokenArr[i].constVal); break;
			case STRING: 
				printf("STRING (");
				for (int j = 0; j < info->tokenArr[i].length; j++) {
					printf("%c", info->string[info->tokenArr[i].index + j]);
				}
				printf(")\n");
				break;
			case END_STMT: printf("END OF STATEMENT\n"); break;
			case END_BLOCK: printf("END OF BLOCK\n"); break;
			default: printf("OTHER TOKEN\n"); break;
		}
	}
}

char lexer (PSTAT *info) {
	TOKEN tokenTable[TOTAL_TOKENS] = {
		{"stop", 0}, {"kill", 10}, {"let", 10}, {"put", 10}, {"get", 10}, 
		{"if", 10}, {"} elif", 10}, {"for", 10}, {"while", 10}, {"}", 10}, {"=", 20}, 					 //functions
		{"/", 90}, {"*", 90}, {"-", 70}, {"+", 70},//arthematic operators
		{"==", 55}, {"!=", 55}, {"<", 50}, {">", 50}, {"<=", 50}, {">=", 50},//relational operators
		{"&", 40}, {"|", 30}, {"!", 20}  //shortcircuit operators
	};

	info->lineNum++;
	info->noOfTokens = 0;
	info->tokenArrCap = INIT_BUFF_SIZE;
	if (info->tokenArr != NULL) free(info->tokenArr);
	info->tokenArr = malloc(INIT_BUFF_SIZE * sizeof(TOKEN_ARR));

	int finger;
	finger = 0;

RE_LEXE:
	char tempBuffer[VAR_LENGTH];
	int index = 0;
	
	while (index < (VAR_LENGTH - 1) &&
		   info->string[finger] != ' ' &&
		   checkCondition(info, &finger) ) {
		tempBuffer[index] = info->string[finger];
		if (finger < strlen(info->string) - 1 && 
			info->string[finger] == '}' && 
			info->string[finger + 1] == ' ') {
			index++;
			finger++;
			tempBuffer[index] = ' ';
		}
		finger++;
		index++;
	}
	tempBuffer[index] = '\0';

	int lexerMode = -1;
	finger++;
	if(info->string[finger] == '=') {
		for (int x = 0; x < info->noOfVars; x++) {
			if (!strcmp(tempBuffer, info->varTable[x].name)) {
				lexerMode = ASN;
				TOKEN_ARR temp;
				temp.token = VARIABLE;
				temp.index = 0;
				temp.length = 0;
				temp.var = &info->varTable[x];
				if (!pushToken(&temp, info)) {
					printf("UNKNOWN ERROR 04: failed to allocate memory to a built in data structure");
					return EXITCHAR;
				}
				break;
			}
		}
	} else {
		for (int x = 0; x <= END_BLOCK; x++) {
			if (!strcmp(tempBuffer, tokenTable[x].pattern)) {
				lexerMode = x;
				TOKEN_ARR temp;
				temp.token = x;
				temp.weight = tokenTable[x].weight;
				temp.index = 0;
				temp.length = 0;
				temp.var = NULL;
				if (!pushToken(&temp, info)) {
					printf("UNKNOWN ERROR 05: failed to allocate memory to a built in data structure");
					return EXITCHAR;
				}
				break;
			} 
		}
	}


	if (lexerMode == -1) {
		//-----------------------------------------------------------------------------------
		printError01(info);
		return NOISSUE;
	}
	int status = EXITCHAR;
	switch (lexerMode) {
		case STP: status = lexeStop(info, &finger); break;
		case KIL: status = lexeKill(info, &finger); break;
		case LET: status = lexeLet(info, &finger); break;
		case PUT: status = lexePut(info, &finger); break;
		case GET: status = lexeGet(info, &finger); break;
		case CON: status = lexeIf(info, tokenTable, &finger); break;
		case ELCON: 
		if (info->noOfTokens == 1) {
			printf("         |\n");
			printf("line%5d: %s\n", info->lineNum, info->string);
			printf("    Error: elif cannot be used without a preceding if block\n");
			printf("         |\n");
			return NOISSUE;
		} 
		status = lexeIf(info, tokenTable, &finger); break;
		case END_BLOCK:
		if (info->noOfTokens == 1) {
			printf("         |\n");
			printf("line%5d: %s\n", info->lineNum, info->string);
			printf("    Error: closing bracket '}' cannot be used without a preceding block\n");
			printf("         |\n");
			return NOISSUE;
		}
		finger--;
		if (finger < strlen(info->string) - 1 &&
			(info->string[finger] == '\n' || 
			info->string[finger] == ';')) {
			status = recycle(info, &finger);
		} else status = parser(info); break;
		case FOR: status = lexeFor(info, tokenTable, &finger); break;
		case TIL: status = lexeTill(info, tokenTable, &finger); break;
		case ASN: status = lexeAsn(info, tokenTable, &finger); break;
		default: break;
	}

	if (status == NOISSUE) return NOISSUE;
	else if (status == EXITCHAR) return EXITCHAR;
	else if (status == LEXE_RECYCLE) {
		goto RE_LEXE;
	}
}

typedef enum {
	ER_LONG_INPUT, ER_NO_INPUT, ER_WRONG_INPUT
} ER;

void printError00 (PSTAT *info, int forLet) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: ");
	if (!forLet) printf("variable of given name doesn't exist\n");
	else printf("variable of given name already exists; cannot be created again\n");
	printf("         |\n");
	return;
}

char lexeKill (PSTAT *info, int *finger) {
	char status;
	status = lexeVarFinder(info, finger);
	if (status == EXITCHAR) return EXITCHAR;
	else if (status == NOISSUE) return NOISSUE;
	status = reportTrash(info, finger);
	if (status == NOISSUE) return NOISSUE;

	if (info->string[*finger] == ',') {
		TOKEN_ARR temp;
		temp.token = END_STMT;
		if (!pushToken (&temp, info)) {
			printf("ER");
			return EXITCHAR;
		}
		temp.token = KIL;
		if (!pushToken (&temp, info)) {
			printf("ERRR");
			return EXITCHAR;
		}
		(*finger)++;
		if (info->string[*finger] == ' ') (*finger)++;
		return lexeKill (info, finger);
	}
	else if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') 
		return recycle(info, finger);
	else return parser(info); 
}

char lexeGet (PSTAT *info, int *finger) {
	char status;
	status = lexeVarFinder(info, finger);
	if (status == EXITCHAR) return EXITCHAR;
	else if (status == NOISSUE) return NOISSUE;
	status = reportTrash(info, finger);
	if (status == NOISSUE) return NOISSUE;
	
	if (info->string[*finger] == ',') {
		TOKEN_ARR temp;
		temp.token = END_STMT;
		if (!pushToken (&temp, info)) {
			printf("ER");
			return EXITCHAR;
		}
		temp.token = GET;
		if (!pushToken (&temp, info)) {
			printf("ERRR");
			return EXITCHAR;
		}
		(*finger)++;
		if (info->string[*finger] == ' ') (*finger)++;
		return lexeGet (info, finger);
	}
	else if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') 
		return recycle(info, finger);
	else return parser(info);
}

/**********************************\
|------------LEXE_LET--------------|
\**********************************/

typedef enum {
	ER_LEAD_NUM, ER_SPC_CHAR, ER_WHT_SPA, ER_EXC_LEN, ER_USE_LOOP, ER_USE_CHAIN
} ERROR_TYPE_02;

void printError02(PSTAT *info, int type) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: ");
	switch (type) {
		case ER_LEAD_NUM: printf("variable name cannot have number at the beginning\n"); break;
		case ER_SPC_CHAR: printf("variable name cannot have special characters\n"); break;
		case ER_WHT_SPA: printf("variable name cannot have white spaces\n"); break;
		case ER_EXC_LEN: printf("variable name cannot have length greater than %d\n", VAR_LENGTH - 1); break;
		case ER_USE_LOOP: printf("let cannot be used in loops or conditions\n"); break;
		case ER_USE_CHAIN: printf("let cannot be used in chained statements\n"); break;
	}
	printf("         |\n");
	return;
}

char lexeLet (PSTAT *info, int *finger) {
	char varNameStr[2 * VAR_LENGTH];
	int index = 0, whiteSpace = 0, specialChar = 0, leadingNum = 0;
	int startPoint = *finger;
	while (index < (2 * VAR_LENGTH - 1) &&
			checkCondition(info, finger) &&
			info->string[*finger] != ',') {
		varNameStr[index] = info->string[*finger];

		if (index == 0 && 
			varNameStr[index] >= '0' && 
			varNameStr[index] <= '9') leadingNum = 1;
		if (!whiteSpace && varNameStr[index] == ' ') whiteSpace = index;
		if (!checkVar (info, finger) ||
			varNameStr[index] == ' ') specialChar = 1;

		index++;
		(*finger)++;
	}
	int shouldReturn = 0;
	if (leadingNum) {
		printError02(info, ER_LEAD_NUM);
		shouldReturn = 1;
	} 
	if (specialChar) {
		printError02(info, ER_SPC_CHAR);
		shouldReturn = 1;
	}
	if (whiteSpace && (index - whiteSpace) != 1) {
		printError02(info, ER_WHT_SPA);
		shouldReturn = 1;
	}
	if (index >= VAR_LENGTH - 1) {
		printError02(info, ER_EXC_LEN);
		shouldReturn = 1;
	}
	if (shouldReturn) return NOISSUE;

	varNameStr[index] = '\0';
	int status = lexeVarMatcher(info, varNameStr, 1);
	if (status == NOISSUE) return NOISSUE;

	TOKEN_ARR temp;
	temp.token = STRING;
	temp.index = startPoint;
	temp.length = index;
	if (!pushToken(&temp, info)) {
		printf("UNKNOWN ERROR 08: failed to allocate memory to a built in data structure");
		return EXITCHAR;
	}
	
	if (info->string[*finger] == ',') {
		temp.token = END_STMT;
		if (!pushToken(&temp, info)) {
			printf("UB");
			return EXITCHAR;
		}
		temp.token = LET;
		if (!pushToken(&temp, info)) {
			printf("UBB");
			return EXITCHAR;
		}
		(*finger)++;
		if (info->string[*finger] == ' ') (*finger)++;
		return lexeLet(info, finger);
	}
	else if (info->string[*finger] == '\n') {
		printError02(info, ER_USE_LOOP);
		return NOISSUE;
	} else if (info->string[*finger] == ';') {
		printError02(info, ER_USE_CHAIN);
		return NOISSUE;
	}
	else return parser(info);
}

/***********************************\
|------------LEXE-PUT---------------|
\***********************************/

typedef enum {
	ER_WANT_MRG, ER_EXTRA_MRG, ER_UNCLOSED_PAIR
} ER_PUT;

void printErrorPut(PSTAT *info, int type) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: ");
	switch (type) {
		case ER_WANT_MRG: printf("expected a comma ',' between two arguments\n"); break;
		case ER_EXTRA_MRG: printf("unnecessary commas ',' were used\n"); break;
		case ER_UNCLOSED_PAIR: printf("any pair must be opened and closed in the same line\n"); break;
	}
	printf("         |\n");
	return;
}

char lexePut (PSTAT *info, int *finger) {
	int mergeNeeded = 0, shouldReturn = 0, backSlashCount = 0;
	do {
		backSlashCount = (info->string[*finger] == '\\' ? backSlashCount + 1 : 0);
		if (info->string[*finger] == ' ' ||
			info->string[*finger] == '(' ||
			info->string[*finger] == ')'){
			//do nothing eat five star *****
		}else if (info->string[*finger] == '"') {
			if (mergeNeeded) {
				printErrorPut(info, ER_WANT_MRG);
				shouldReturn = 1;
			}
			(*finger)++;
			int startPoint = *finger;
			int infLoop = 0;
			while (!infLoop && 
				(info->string[*finger] != '"' || backSlashCount % 2 != 0)) {
				(*finger)++;
				if (!checkCondition(info, finger)) {
					printErrorPut(info, ER_UNCLOSED_PAIR);
					shouldReturn = 1;
					infLoop = 1;
				}
			}
			int len = *finger - startPoint;
			TOKEN_ARR temp;
			temp.token = STRING;
			temp.index = startPoint;
			temp.length = len;
			temp.var = NULL;
			if (!pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 09: failed to allocate memory to a built in data structure");
				return EXITCHAR;
			}
			mergeNeeded = 1;
		} else if (info->string[*finger] == ',') {
			if (!mergeNeeded) {
				printErrorPut(info, ER_EXTRA_MRG);
				shouldReturn = 1;
			}
			TOKEN_ARR temp;
			temp.token = END_STMT;
			if (!pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 10: failed to allocate memory to a built in data structure");
				return EXITCHAR;
			}
			temp.token = PUT;
			//token properties
			if (!pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 10: failed to allocate memory to a built in data structure");
				return EXITCHAR;
			}
			mergeNeeded = 0;
		} else {
			if (mergeNeeded) {
				printErrorPut(info, ER_WANT_MRG);
				shouldReturn = 1;
			}
			int load = 0;
			char stat = lexeExpression (info, finger, &load);
			if (stat == EXITCHAR) return EXITCHAR;
			else if (stat == NOISSUE) return NOISSUE;
			mergeNeeded = 1;
			continue;
		} 
		(*finger)++;
	} while (checkCondition(info, finger));
	
	if (shouldReturn) return NOISSUE;
	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') return recycle(info, finger);
	else return parser(info);
}

/************************************\
|------------LEXE-IF-----------------|
\************************************/

typedef enum {
	EXP_BLOCK, OPT_UNEXPECTED, OPT_NEEDED, MISSING_GDR
} ER_CON;

typedef enum {
	CALLER_IF, CALLER_FOR, CALLER_TIL
} CALLER;

void printErrorCon (PSTAT *info, int type, int caller) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: ");
	switch (type) {
		case EXP_BLOCK: printf("expected a block '{ }' after condition in the following format only\n");
			switch (caller) {
				case CALLER_IF:
					printf("         | if <condition> {\n");
					printf("         |     <statements>\n");
					printf("         | } elif <condition> {\n");
					printf("         |     <statements>\n");
					printf("         | } elif {\n");
					printf("         |     <statements>\n");
					printf("         | }\n");
					break;
				case CALLER_FOR:
					printf("         | for <constant/variable> {\n");
					printf("         |     <statements>\n");
					printf("         | }\n");
					break;	
				case CALLER_TIL:
					printf("         | while <condition> : <maximum iterations> {\n");
					printf("         |     <statements>\n");
					printf("         | }\n");
					break;
			}
			break;

		case OPT_UNEXPECTED: printf("operator is not expected without an expression\n"); break;
		case OPT_NEEDED: printf("operator is expected between two expressions\n"); break;
		case MISSING_GDR: printf("missing guardrail ':' after condition in while loop\n"); break;
	}
	printf("         |\n");
	return;
}

char lexeIf (PSTAT *info, TOKEN tokenTable[], int *finger) {
	int load = 0;
	if (info->string[*finger] == ' ') (*finger)++;
	if (info->string[*finger] == '{') { //Direct block
		(*finger)++;
		return recycle(info, finger);
	}
	char status = lexeCondition(info, tokenTable, finger, CALLER_IF, &load);
	if (status == EXITCHAR) return EXITCHAR;
	else if (status == NOISSUE) return NOISSUE;

	(*finger)++;
	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') {
		return recycle(info, finger);
	}
	else if (info->string[*finger] == '\0') {
		printErrorCon(info, EXP_BLOCK, CALLER_IF);
		return NOISSUE;
	}
}

char lexeFor (PSTAT *info, TOKEN tokenTable[], int *finger) {
	int load = 0;
	char status = lexeExpression(info, finger, &load);
	if (status == EXITCHAR) return EXITCHAR;
	else if (status == NOISSUE) return NOISSUE;
	
	(*finger)++;
	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') {
		return recycle(info, finger);
	}
	else if (info->string[*finger] == '\0') {
		printErrorCon(info, EXP_BLOCK, CALLER_FOR);
		return NOISSUE;
	}
}

char lexeTill (PSTAT *info, TOKEN tokenTable[], int *finger) {
	int load = 0;
	if (info->string[*finger] == ' ') (*finger)++;
	char status = lexeCondition(info, tokenTable, finger, CALLER_TIL, &load);
	if (status == EXITCHAR) return EXITCHAR;
	else if (status == NOISSUE) return NOISSUE;

	if (info->string[*finger] == ' ') (*finger)++;
	// Gaurdrails for preventing infinite loops
	if (info->string[*finger] != ':') {
		printErrorCon(info, MISSING_GDR, CALLER_TIL);
		return NOISSUE;
	} else {
		TOKEN_ARR temp;
		temp.token = GRD;
		temp.index = 0;
		temp.length = 0;
		temp.var = NULL;
		if (!pushToken(&temp, info)) {
			printf("UNKNOWN ERROR 12: failed to allocate memory to a built in data structure");
			return EXITCHAR;
		}
		(*finger)++;
	}
	int grdLoad = 0;
	char stat = lexeExpression(info, finger, &grdLoad);
	if (stat == EXITCHAR) return EXITCHAR;
	else if (stat == NOISSUE) return NOISSUE;
	(*finger)++;
	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') {
		return recycle(info, finger);
	} else if (info->string[*finger] == '\0') {
		printErrorCon(info, EXP_BLOCK, CALLER_TIL);
		return NOISSUE;
	}
}

typedef enum {
	EXPECTING_EXPR, TOO_MANY_CLO, OPERATOR_UNEXPECTED, OPERATOR_NEEDED, PAIR_CLO_ER
} ER_ASN;

void printErrorAsn (PSTAT *info, int type) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: ");
	switch (type) {
		case EXPECTING_EXPR: printf("expecting an expression to assign %s\n", info->tokenArr[0].var->name); break;
		case TOO_MANY_CLO: printf("closing too many paranthesis without opening\n"); break;
		case OPERATOR_UNEXPECTED: printf("operator is not expected without a variable or constant\n"); break;
		case OPERATOR_NEEDED: printf("operator is expected between two variables or constants\n"); break;
		case PAIR_CLO_ER: printf("opened paranthesis were not closed\n"); break;
	}
	printf("         |\n");
	return;

}

char lexeAsn (PSTAT *info, TOKEN tokenTable[], int *finger) {
	while (info->string[*finger] != '=' && 
		   checkCondition(info, finger)) (*finger)++;
	if (!checkCondition(info, finger)) {
		printErrorAsn (info, EXPECTING_EXPR);
		return NOISSUE;
	}
	TOKEN_ARR temp;
	temp.token = ASN;
	temp.weight = 20;
	temp.index = 0;
	temp.length = 0;
	temp.var = NULL;
	if (!pushToken(&temp, info)) {
		printf("UNKNOWN ERROR 13");
		return EXITCHAR;
	}
	(*finger)++;
	int load = 0;
	char status = lexeExpression(info, finger, &load);
	if (status == EXITCHAR) return EXITCHAR;
	else if (status == NOISSUE) return NOISSUE;

	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') return recycle(info, finger);
	else return parser(info);
}

char lexeStop (PSTAT *info, int *finger) {
	return parser(info);
}

//----------------------Lexer-Utilities--------------------------\\

int checkCondition (PSTAT *info, int *finger) {
	if (info->string[*finger] != ';' &&
		info->string[*finger] != '\n' &&
		info->string[*finger] != '\0' ) return 1;
	else return 0;
}

int checkVar (PSTAT *info, int *finger) {
	if ((info->string[*finger] >= 'a' && 
		 info->string[*finger] <= 'z') ||
		(info->string[*finger] >= 'A' &&
		 info->string[*finger] <= 'Z') ||
		(info->string[*finger] >= '0' &&
		 info->string[*finger] <= '9')) return 1;
	else return 0;
}

char lexeVarFinder (PSTAT *info, int *finger) {
	char tempBuffer[VAR_LENGTH];
	int index = 0;
	while (index < VAR_LENGTH - 1 &&
			info->string[*finger] != ' ' &&
			checkCondition(info, finger) &&
			checkVar(info, finger)) {
		tempBuffer[index] = info->string[*finger];
		index++;
		(*finger)++;
	}
	tempBuffer[index] = '\0';
	return lexeVarMatcher(info, tempBuffer, 0);
}

char lexeVarMatcher (PSTAT *info, char tempBuffer[VAR_LENGTH], int forLet) {
	int match = 0;
	for (int x = 0; x < info->noOfVars; x++) {
		if (!strcmp(tempBuffer, info->varTable[x].name)) {
			match = 1;
			TOKEN_ARR temp;
			temp.token = VARIABLE;
			temp.index = 0;
			temp.length = 0;
			temp.var = &info->varTable[x];
			if (!pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 11: failed to allocate memory to a built in data structure");
				return EXITCHAR;
			}
		}
	}
	if (!match && !forLet) {
		printError00(info, forLet);
		return NOISSUE;
	} else if (match && forLet) {
		printError00(info, forLet);
		return NOISSUE;
	}

	return CONTCHAR;
}

int pushToken (TOKEN_ARR *temp, PSTAT *info) {
	if (info->noOfTokens >= info->tokenArrCap) {
		info->tokenArrCap *= GROWTH_FACTOR;
		TOKEN_ARR *safteyTrigger = realloc(info->tokenArr, info->tokenArrCap * sizeof(TOKEN_ARR));
		if (safteyTrigger == NULL) return 0;
		info->tokenArr = safteyTrigger;
	}
	info->tokenArr[info->noOfTokens] = *temp;
	info->noOfTokens++;
	return 1;
}

char recycle (PSTAT *info, int *finger) {
	if (info->string[*finger] == '\n') {
		info->lineNum++;
	}
		TOKEN_ARR temp;
		temp.token = END_STMT;
		pushToken(&temp, info);
		(*finger)++;
		if (info->string[*finger] == ' ') (*finger)++;
		return LEXE_RECYCLE;
}

char reportTrash (PSTAT *info, int *finger) {
	int shouldReturn = 0;
	while (checkCondition(info, finger) &&
			info->string[*finger] != ',') {
		if (info->string[*finger] != ' ') shouldReturn = 1;
		(*finger)++;
	}
	if (shouldReturn) return NOISSUE;
	return CONTCHAR;
}

#define INT_SIZE 24;

char lexeNumber (PSTAT *info, int *finger) {
	int constant = 0, faceValue = 0;
	do {
		faceValue = info->string[*finger] - '0';
		constant = (constant * 10) + faceValue;
		(*finger)++;
	} while (info->string[*finger] >= '0' &&
			 info->string[*finger] <= '9');
	TOKEN_ARR temp;
	temp.token = CONSTANT;
	temp.index = 0;
	temp.length = 0;
	temp.var = NULL;
	temp.constVal = constant;
	if (!pushToken(&temp, info)) {
		printf("UNKNOWN ERROR 14");
		return EXITCHAR;
	} 
	return CONTCHAR;
}

char lexeExpression (PSTAT *info, int *finger, int *load) {
	int operator = 0, shouldReturn = 0;
	int initialLoad = *load;  // Remember starting load level
	do {
		if (info->string[*finger] == ' ') {}
		else if (info->string[*finger] == '(') (*load) += 1000;
		else if (info->string[*finger] == ')') {
			// Only process ) if it closes a ( that WE opened
			if ((*load) > initialLoad) {
				(*load) -= 1000;
				// Continue parsing, don't break
			} else {
				// This ) doesn't belong to us, stop here
				break;
			}
		} else if (info->string[*finger] >= '0' &&
				   info->string[*finger] <= '9') {
			if (operator) {
				printErrorAsn(info, OPERATOR_NEEDED);
				shouldReturn = 1;
			}

			char status = lexeNumber(info, finger);
			if (status == EXITCHAR) return EXITCHAR;
			operator = 1;
			continue;
		} else if (checkVar(info, finger)) {
			if (operator) {
				printErrorAsn(info, OPERATOR_NEEDED);
				shouldReturn = 1;
			}
			char status;
			status = lexeVarFinder(info, finger);
			if (status == EXITCHAR) return EXITCHAR;
			else if (status == NOISSUE) shouldReturn = 1;
			operator = 1;
			continue;
		} else if (info->string[*finger] == '+' ||
				   info->string[*finger] == '-' ||
				   info->string[*finger] == '*' ||
				   info->string[*finger] == '/') {
			if (!operator) {
				printErrorAsn(info, OPERATOR_UNEXPECTED);
				shouldReturn = 1;
			}
			char buffer = info->string[*finger];
			TOKEN_ARR temp;
			switch (buffer) {
				case '+': 
					temp.token = ADD; 
					temp.weight = 70 + (*load);
					break; 
				case '-': 
					temp.token = SUB;
					temp.weight = 70 + (*load);
					break;
				case '*': 
					temp.token = MLT;
					temp.weight = 90 + (*load);
					break;
				case '/': 
					temp.token = DVD; 
					temp.weight = 90 + (*load);
					break;
			}
			if (!pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 14");
				return EXITCHAR;
			}
			operator = 0;
		} else if (info->string[*finger] == '<' ||
			  info->string[*finger] == '=' ||
			  info->string[*finger] == '>' ||
			  info->string[*finger] == '&' ||
			  info->string[*finger] == '|' ||
			  info->string[*finger] == '!') {
			// These are comparison/logic operators - we don't handle them in arithmetic expressions
			// Stop here if we're not inside parens opened by us
			if ((*load) <= initialLoad) {
				break;
			}
			// If inside parens, this shouldn't happen - it means the condition was passed to us
			// This means lexeCondition or another caller is in an invalid state
			printError01(info);
			return NOISSUE;
		} else {
			//-----------------------------------------------------------------------------------
			printError01(info);
			return NOISSUE;
		}
		(*finger)++;
	} while (checkCondition(info, finger) &&
			 info->string[*finger] != ':' &&
			 info->string[*finger] != ',' &&
			 info->string[*finger] != '{' &&
			 ((*load > initialLoad) || 
			  (info->string[*finger] != '<' &&
			   info->string[*finger] != '=' &&
			   info->string[*finger] != '>' &&
			   info->string[*finger] != '&' &&
			   info->string[*finger] != '|' &&
			   info->string[*finger] != '!')));
	if ((*load) != initialLoad && (*load) > initialLoad) {
		printErrorAsn(info, PAIR_CLO_ER);
		return NOISSUE;
	} 
	if (shouldReturn) return NOISSUE;
	return CONTCHAR;
}

char lexeCondition (PSTAT *info, TOKEN tokenTable[], int *finger, int caller, int *load) {
	int comparision = 0, shouldReturn = 0;
	do {
		if (info->string[*finger] == ' ') {
		}
		else if (info->string[*finger] == '<' ||
			info->string[*finger] == '>' ||
			info->string[*finger] == '=' ||
			info->string[*finger] == '!') {
			if (!comparision && 
				info->string[*finger] != '!' &&
				info->string[*finger + 1] == '=') {
				printErrorCon(info, OPT_UNEXPECTED, caller);
				shouldReturn = 1;
			}
			comparision = 0;
			char buffer[3];
			buffer[0] = info->string[*finger];
			(*finger)++;
			if (info->string[*finger] == '=' ) { 
				buffer[1] = info->string[*finger];
				buffer[2] = '\0';
			} else {
				buffer[1] = '\0';
				(*finger)--;  // Move back since the next char is not part of the operator
			}
			TOKEN_ARR temp;
			int matched = 0;
			for (int x = EQL; x <= NOT; x++) {
				if (!strcmp(buffer, tokenTable[x].pattern)) {
					matched = 1;
					temp.token = x;
					temp.weight = tokenTable[x].weight;
					temp.index = 0;
					temp.length = 0;
					temp.var = NULL;
					break;
				}
			}
			if (matched && !pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 12");
				return EXITCHAR;
			} else if (!matched) {
				printErrorCon(info, OPT_UNEXPECTED, caller);
				shouldReturn = 1;
			}
		} else if (info->string[*finger] == '(') {
			// Recursively parse grouped condition
			(*load) += 1000;
		} else if (info->string[*finger] == ')') {
			// Close grouped condition
			(*load) -= 1000;
			if ((*load) < 0) {
				printErrorCon(info, TOO_MANY_CLO, caller);
				shouldReturn = 1;
			}
		} else if (checkVar(info, finger)) {
			if (comparision) {
				printErrorCon(info, OPT_NEEDED, caller);
				shouldReturn = 1;
			}
			comparision = 1;
			char status;
			status = lexeExpression(info, finger, load);
			if (status == EXITCHAR) return EXITCHAR;
			else if (status == NOISSUE) shouldReturn = 1;
			continue;
		} else if (*load > 0 && (info->string[*finger] == '&' || info->string[*finger] == '|')) {
			// Inside parentheses, & and | are parsed as tokens (not as recursive split)
			TOKEN_ARR temp;
			switch (info->string[*finger]) {
				case '&':
					temp.token = AND;
					temp.weight = 40;
					break;
				case '|':
					temp.token = ORR;
					temp.weight = 30;
					break;
			}
			temp.index = 0;
			temp.length = 0;
			temp.var = NULL;
			if (!pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 13");
				return EXITCHAR;
			}
			comparision = 0;
		} else {
			//-----------------------------------------------------------------------------------
			printError01(info);
			shouldReturn = 1;
		}
		(*finger)++;
	} while (checkCondition(info, finger) &&
			 info->string[*finger] != ':' &&
			 info->string[*finger] != '{' &&
			 ((*load) > 0 || (info->string[*finger] != '&' &&
			  info->string[*finger] != '|')));

	if (*load == 0 && (info->string[*finger] == '&' ||
		info->string[*finger] == '|')) {
		char buffer = info->string[*finger];
		TOKEN_ARR temp;
		switch (buffer) {
			case '&': 
				temp.token = AND; 
				temp.weight = 40;
				break; 
			case '|': 
				temp.token = ORR;
				temp.weight = 30;
				break;
		}
		if (!pushToken(&temp, info)) {
			printf("UNKNOWN ERROR 14");
			return EXITCHAR;
		}
		(*finger)++;
		char status = lexeCondition(info, tokenTable, finger, caller, load);
		if (status == EXITCHAR) return EXITCHAR;
		else if (status == NOISSUE) shouldReturn = 1;
	} else if (caller != CALLER_TIL && info->string[*finger] != '{') {
		printErrorCon(info, EXP_BLOCK, caller);
		shouldReturn = 1;
	}
	if (*load != 0) {
		printErrorCon(info, PAIR_CLO_ER, caller);
		shouldReturn = 1;
	}
	if (shouldReturn) {
		return NOISSUE;
	}
	return CONTCHAR;
}

/***************************\
|---------PARSER------------|
\***************************/

typedef struct {
	char prog;
	VAR number;
	int isString;
	int start;
	int end;
} RES;

RES recursiveParser(PSTAT *info, RES);

RES kill(PSTAT *, RES);
RES let(PSTAT *, RES);
RES put(PSTAT *, RES);
RES get(PSTAT *, RES);
RES asn(PSTAT *, RES, RES);
RES add(PSTAT *, RES, RES);
RES sub(PSTAT *, RES, RES);
RES mlt(PSTAT *, RES, RES);
RES dvd(PSTAT *, RES, RES);
RES eql(PSTAT *, RES, RES);
RES nte(PSTAT *, RES, RES);
RES grt(PSTAT *, RES, RES);
RES les(PSTAT *, RES, RES);
RES goe(PSTAT *, RES, RES);
RES loe(PSTAT *, RES, RES);
RES and(PSTAT *, RES, RES);
RES orr(PSTAT *, RES, RES);
RES not(PSTAT *, RES);

char parseIf(PSTAT *, int *);
char parseFor(PSTAT *, int *);
char parseTill(PSTAT *, int *);

void debugLog (PSTAT *info, RES init) {
	printf("               +------+----------+-------------+\n");
	printf("               |%6s|%10s| WINDOW INFO |\n", "si.no", "TOKEN");
	printf("               +------+----------+-------------+\n");
	for (int x = 0; x < info->noOfTokens; x++) {
		printf("               |%6d", x);
		switch(info->tokenArr[x].token) {
			case STP: printf("|%10s|", " STP "); break;
			case KIL: printf("|%10s|", " KIL "); break;
			case LET: printf("|%10s|", " LET "); break;
			case PUT: printf("|%10s|", " PUT "); break;
			case GET: printf("|%10s|", " GET "); break;
			case CON: printf("|%10s|", " CON "); break;
			case ELCON: printf("|%10s|", " ELCON "); break;
			case FOR: printf("|%10s|", " FOR "); break;
			case TIL: printf("|%10s|", " TIL "); break;
			case ASN: printf("|%10s|", " ASN "); break;
			case DVD: printf("|%10s|", " DVD "); break;
			case MLT: printf("|%10s|", " MLT "); break;
			case SUB: printf("|%10s|", " SUB "); break;
			case ADD: printf("|%10s|", " ADD "); break;
			case EQL: printf("|%10s|", " EQL "); break;
			case NTE: printf("|%10s|", " NTE "); break;
			case LES: printf("|%10s|", " LES "); break;
			case GRT: printf("|%10s|", " GRT "); break;
			case LOE: printf("|%10s|", " LOE "); break;
			case GOE: printf("|%10s|", " GOE "); break;
			case AND: printf("|%10s|", " AND "); break;
			case ORR: printf("|%10s|", " ORR "); break;
			case NOT: printf("|%10s|", " NOT "); break;
			case VARIABLE: printf("|%10s|", " VARIABLE "); break;
			case STRING: printf("|%10s|", " STRING "); break;
			case CONSTANT: printf("|%10s|", " CONSTANT "); break;
			case END_STMT: printf("|%10s|", " END_STMT "); break;
			case END_BLOCK: printf("|%10s|", " END_BLOCK "); break;
			default : printf("\a ERROR UNKNOWN TOKEN\n");
		}
		if (x >= init.start && x < init.end) {
			printf(" TO BE PARSED|\n");
		} else {
			printf("             |\n");
		}
		printf("               +------+----------+-------------+\n");
	}
}

char parseDB(PSTAT * info, int *tokenIndex) {
	while (*tokenIndex < info->noOfTokens &&
		   info->tokenArr[*tokenIndex].token != END_BLOCK &&
		   info->tokenArr[*tokenIndex].token != ELCON) {
		RES init;
		init.number.value = 0;
		init.number.inf = 0;
		init.number.nan = 0;
		init.isString = 0;
		if (info->tokenArr[*tokenIndex].token == CON){
			char status = parseIf(info, tokenIndex);
			if (status == EXITCHAR) return EXITCHAR;
			continue;
		} else if (info->tokenArr[*tokenIndex].token == FOR) {
			char status = parseFor(info, tokenIndex);
			if (status == EXITCHAR) return EXITCHAR;
			continue;
		} else if (info->tokenArr[*tokenIndex].token == TIL){
			char status = parseTill(info, tokenIndex);
			if (status == EXITCHAR)
				return EXITCHAR;
			continue;
		}
		init.start = *tokenIndex;
		while (*tokenIndex < info->noOfTokens && 
				info->tokenArr[*tokenIndex].token != END_STMT) {
			(*tokenIndex)++;
		}
		init.end = *tokenIndex;
		if (*tokenIndex < info->noOfTokens) (*tokenIndex)++;
		if (init.start >= init.end) continue;
		init = recursiveParser(info, init);
		if (init.prog == EXITCHAR) return EXITCHAR;
	}
	return NOISSUE;
}

// eval function
char eval (PSTAT *info, int *tokenIndex) {
	RES init;
	init.number.value = 0;
	init.number.inf = 0;
	init.number.nan = 0;
	init.isString = 0;
	init.start = *tokenIndex;
	while (info->tokenArr[*tokenIndex].token != END_STMT &&
		   info->tokenArr[*tokenIndex].token != GRD &&
		   *tokenIndex < info->noOfTokens) (*tokenIndex)++;
	init.end = *tokenIndex;
	if (init.start == init.end) {
		(*tokenIndex)++;
		return TRUE;
	}
	RES result = recursiveParser(info, init);
	(*tokenIndex)++;
	if (result.prog == EXITCHAR) return EXITCHAR;
	if (result.number.value) return TRUE;
	else return FALSE;
}

//jumpBlock function
void jumpBlock (PSTAT *info, int *tokenIndex, int openBraces) {
	int initialDepth = openBraces;  // Remember starting depth
	do {
		if (info->tokenArr[*tokenIndex].token == CON) {
			openBraces++;
		} else if (info->tokenArr[*tokenIndex].token == END_BLOCK) {
			openBraces--;
		}
		(*tokenIndex)++;
	} while ( (info->tokenArr[*tokenIndex].token != END_BLOCK || openBraces > initialDepth) &&
		(info->tokenArr[*tokenIndex].token != ELCON || openBraces > initialDepth) &&
	 	*tokenIndex < info->noOfTokens);
	return;
}


char parseIf (PSTAT *info, int *tokenIndex) {
	(*tokenIndex)++;
	char booleanVal = eval(info, tokenIndex);
	if (booleanVal == EXITCHAR) return EXITCHAR;
	else if (booleanVal == TRUE) {
		//forward to parseDB again
		char status = parseDB(info, tokenIndex);
		if (status == EXITCHAR) return EXITCHAR;
		if (info->tokenArr[*tokenIndex].token != END_BLOCK) {
			// jump the rest of the if-elif-else block
			jumpBlock(info, tokenIndex, 1);
		}
	} else if (booleanVal == FALSE) {
		//call jump function
		jumpBlock(info, tokenIndex, 0);
		//call parseIf recursively for elif/else
		if (info->tokenArr[*tokenIndex].token == ELCON) {
			char status = parseIf(info, tokenIndex);
			if (status == EXITCHAR) return EXITCHAR;
		}
	}
	if (info->tokenArr[*tokenIndex].token == END_BLOCK) (*tokenIndex)++;
	return NOISSUE;
}

char parseFor (PSTAT *info, int *tokenIndex) {
	(*tokenIndex)++;
	RES loopVarRes;
	loopVarRes.start = *tokenIndex;
	while (info->tokenArr[*tokenIndex].token != END_STMT &&
		   *tokenIndex < info->noOfTokens) {
		(*tokenIndex)++;
	}
	loopVarRes.end = *tokenIndex;
	loopVarRes = recursiveParser(info, loopVarRes);
	if (loopVarRes.prog == EXITCHAR) return EXITCHAR;
	int loopThrough = *tokenIndex;
	for (int i = 0; i < loopVarRes.number.value; i++) {
		*tokenIndex = loopThrough;
		char status = parseDB(info, tokenIndex);
		if (status == EXITCHAR) return EXITCHAR;
	} // to skip the block when false at initial evaluation
	if (loopVarRes.number.value == 0) {
		jumpBlock(info, tokenIndex, 1);
		(*tokenIndex)++;
	}
	return NOISSUE;
}

char parseTill (PSTAT *info, int *tokenIndex) {
	(*tokenIndex)++;
	int conditionAt = *tokenIndex;
	int copy = conditionAt;
	char booleanVal = eval(info, tokenIndex);

	if (booleanVal == EXITCHAR) return EXITCHAR;
	if (booleanVal == FALSE) {
		//initial condition is false, so we need to jump the whole block
		jumpBlock(info, tokenIndex, 0);
		(*tokenIndex)++;
		return NOISSUE;
	}
	RES maxIterRes;
	maxIterRes.start = *tokenIndex;
	while (info->tokenArr[*tokenIndex].token != END_STMT &&
		   *tokenIndex < info->noOfTokens) (*tokenIndex)++;
	maxIterRes.end = *tokenIndex;
	maxIterRes = recursiveParser(info, maxIterRes);
	if (maxIterRes.prog == EXITCHAR) return EXITCHAR;
	(*tokenIndex)++;
	int loopThrough = *tokenIndex;
	int iterations = 0;
	while (booleanVal == TRUE && iterations < maxIterRes.number.value) {
		*tokenIndex = loopThrough;
		char status = parseDB(info, tokenIndex);
		if (status == EXITCHAR) return EXITCHAR;
		booleanVal = eval(info, &conditionAt);
		conditionAt = copy;
		if (booleanVal == EXITCHAR) return EXITCHAR;
		iterations++;
	}
	return NOISSUE;
}

RES recursiveParser (PSTAT *info, RES postres) {
	postres.prog = NOISSUE;
	// rare saftey trigger for abnormal 0 window size case
	if (postres.start == postres.end) {
		// Return empty result instead of error - this can happen with certain operator combinations
		postres.number.value = 0;
		postres.number.nan = 0;
		postres.number.inf = 0;
		return postres;
	}

	//base case when the window is just one token
	if (postres.start + 1 == postres.end) {
		postres.isString = 0;
		switch (info->tokenArr[postres.start].token){
			case VARIABLE: 
				postres.number.value = info->tokenArr[postres.start].var->value;
				postres.number.inf = info->tokenArr[postres.start].var->inf;
				postres.number.nan = info->tokenArr[postres.start].var->nan;
				return postres;
			case CONSTANT:
				postres.number.value = info->tokenArr[postres.start].constVal;
				postres.number.inf = 0;
				postres.number.nan = 0;
				return postres;
			case STRING:
				postres.isString = 1;
				return postres;
			case STP:
				postres.prog = EXITCHAR;
				return postres;
		}
	}

	//find minimum weight token in the window
	int minWtIndex = -1;
	for (int x = postres.start; x < postres.end; x++) {
		if (info->tokenArr[x].token == VARIABLE || 
			info->tokenArr[x].token == STRING ||
			info->tokenArr[x].token == CONSTANT) continue;
		if ( minWtIndex == -1 || info->tokenArr[x].weight <= info->tokenArr[minWtIndex].weight) {
			minWtIndex = x;
		}
	}
	// lbranch recursion iff there exists an l branch
	RES lhprefres = {0, 0, 0, 0, 0};
	if (postres.start < minWtIndex) {
		lhprefres.start = postres.start;
		lhprefres.end = minWtIndex;
		lhprefres = recursiveParser (info, lhprefres);
	}

	// rbranch recursion
	RES rhprefres;
	rhprefres.start = minWtIndex + 1;
	rhprefres.end = postres.end;
	rhprefres = recursiveParser (info, rhprefres);

	// execution caller switch
	switch (info->tokenArr[minWtIndex].token) {
		case KIL:
			postres = kill(info, rhprefres); break;
		case LET:
			postres = let(info, rhprefres); break;
		case PUT:
			postres = put(info, rhprefres); break;
		case GET:
			postres = get(info, rhprefres); break;
		case ASN:
			postres = asn(info, lhprefres, rhprefres); break;
		case DVD:
			postres = dvd(info, lhprefres, rhprefres); break;
		case MLT:
			postres = mlt(info, lhprefres, rhprefres); break;
		case SUB:
			postres = sub(info, lhprefres, rhprefres); break;
		case ADD:
			postres = add(info, lhprefres, rhprefres); break;
		case EQL: 
			postres = eql(info, lhprefres, rhprefres); break;
		case NTE:
			postres = nte(info, lhprefres, rhprefres); break;
		case GRT:
			postres = grt(info, lhprefres, rhprefres); break;
		case LES:
			postres = les(info, lhprefres, rhprefres); break;
		case GOE:
			postres = goe(info, lhprefres, rhprefres); break;
		case LOE:	
			postres = loe(info, lhprefres, rhprefres); break;
		case AND: 
			postres = and(info, lhprefres, rhprefres); break;
		case ORR:
			postres = orr(info, lhprefres, rhprefres); break;
		case NOT:		
			postres = not(info, rhprefres); break;
	}
	return postres;
}

char parser(PSTAT *info) {
	int tokenIndex = 0;
	char status = parseDB(info, &tokenIndex);
	if (status == EXITCHAR) return EXITCHAR;
	return NOISSUE;
}
/**************************\
|---------ENGINE-----------|
\**************************/

RES eql (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	} else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	} else {
		if (lhs.number.value == rhs.number.value) {
			result.number.value = 1;
		} else {
			result.number.value = 0;
		}
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES nte (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result = eql(info, lhs, rhs);
	if (result.number.nan || result.number.inf) {
		return result;
	} else {
		result.number.value = !result.number.value;
	}
	return result;
}

RES grt (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	} else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	} else {
		if (lhs.number.value > rhs.number.value) {
			result.number.value = 1;
		} else {
			result.number.value = 0;
		}
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES les (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	} else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	} else {
		if (lhs.number.value < rhs.number.value) {
			result.number.value = 1;
		} else {
			result.number.value = 0;
		}
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES goe (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result = les(info, lhs, rhs);
	if (result.number.nan || result.number.inf) {
		return result;
	} else {
		result.number.value = !result.number.value;
	}
	return result;
}

RES loe (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result = grt(info, lhs, rhs);
	if (result.number.nan || result.number.inf) {
		return result;
	} else {
		result.number.value = !result.number.value;
	}
	return result;
}

RES and (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	} else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	} else {
		if (lhs.number.value && rhs.number.value) {
			result.number.value = 1;
		} else {
			result.number.value = 0;
		}
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES orr (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	} else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	} else {
		if (lhs.number.value || rhs.number.value) {
			result.number.value = 1;
		} else {
			result.number.value = 0;
		}
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES not (PSTAT *info, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	} else if (rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	} else {
		if (rhs.number.value) {
			result.number.value = 0;
		} else {
			result.number.value = 1;
		}
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES asn (PSTAT *info, RES lhs, RES rhs) {
	if (rhs.number.inf) {	
		info->tokenArr[lhs.start].var->value = 0;
		info->tokenArr[lhs.start].var->inf = 1;
		info->tokenArr[lhs.start].var->nan = 0;
	} else if (rhs.number.nan) {
		info->tokenArr[lhs.start].var->value = 0;
		info->tokenArr[lhs.start].var->nan = 1;
		info->tokenArr[lhs.start].var->inf = 0;
	} else {
		info->tokenArr[lhs.start].var->value = rhs.number.value;
		info->tokenArr[lhs.start].var->nan = 0;
		info->tokenArr[lhs.start].var->inf = 0;
	}
	RES result;
	result.prog = NOISSUE;
	return result;
}

RES add (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	} else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	} else {
		result.number.value = lhs.number.value + rhs.number.value;
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES sub (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	}
	else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	}
	else{
		result.number.value = lhs.number.value - rhs.number.value;
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES mlt (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	}
	else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	}
	else {
		result.number.value = lhs.number.value * rhs.number.value;
		result.number.inf = 0;
		result.number.nan = 0;
	}
	result.isString = 0;
	return result;
}

RES dvd (PSTAT *info, RES lhs, RES rhs) {
	RES result;
	result.prog = NOISSUE;
	if (lhs.number.nan || rhs.number.nan) {
		result.number.value = 0;
		result.number.nan = 1;
		result.number.inf = 0;
	}
	else if (lhs.number.inf || rhs.number.inf) {
		result.number.value = 0;
		result.number.inf = 1;
		result.number.nan = 0;
	}
	else {
		if (rhs.number.value == 0 && lhs.number.value == 0) {
			result.number.value = 0;
			result.number.nan = 1;
			result.number.inf = 0;
			return result;
		} else if (rhs.number.value == 0 && lhs.number.value != 0) {
			result.number.value = 0;
			result.number.inf = 1;
			result.number.nan = 0;
			return result;
		} else {
			result.number.value = lhs.number.value / rhs.number.value;
			result.number.inf = 0;
			result.number.nan = 0;
		}
	}
	result.isString = 0;
	return result;
}

RES let (PSTAT *info, RES references) {
	int index = 0;
	for (int x = info->tokenArr[references.start].index; x < (info->tokenArr[references.start].length + info->tokenArr[references.start].index); x++) {
		info->varTable[info->noOfVars].name[index] = info->string[x];
		index++;
	}
	info->varTable[info->noOfVars].name[index] = '\0';
	info->varTable[info->noOfVars].value = 0;
	info->varTable[info->noOfVars].inf = 0;
	info->varTable[info->noOfVars].nan = 0;
	info->noOfVars++;

	if (info->noOfVars >= info->varTableCap-1) {
		info->varTableCap *= GROWTH_FACTOR;
		VAR *temp = realloc(info->varTable, info->varTableCap * sizeof(VAR));
		if (temp == NULL) {
			printf("UNKOWN ERROR 12 memory allocation failure for variable");
			return references;
		}
		info->varTable = temp;
	}
	return references;
}

RES get (PSTAT *info, RES references) {
	int newVal;
	int error = scanf("%d", &newVal);
	if (error == 0) newVal = 0;
	while (getchar() != '\n');
	info->tokenArr[references.start].var->value = newVal;
	if (error == 0) info->tokenArr[references.start].var->nan = 1;
	return references;
}

RES kill (PSTAT *info, RES references) {
	int i = 0;
	while (strcmp (info->tokenArr[references.start].var->name, info->varTable[i].name)) i++;
	for (int x = i; x < info->noOfVars - 1; x++) {
		info->varTable[x] = info->varTable[x + 1];
	}
	info->noOfVars--;
	return references;
}

RES put (PSTAT *info, RES references) {
	int x = references.start;
	if (info->tokenArr[x].token == STRING) {
		for (int y = info->tokenArr[x].index; y < (info->tokenArr[x].length + info->tokenArr[x].index); y++) {
			if (info->string[y] == '\\') {
				y++;
				switch (info->string[y]) {
					case 'n': printf("\n"); break; //new line character
					case 't': printf("\t"); break; //tab character
					case '\'': printf("\'"); break; //single quote character
					case '\"': printf("\""); break; //double quote character
					case '\\': printf("\\"); break; //backslash character
					case '0': printf("\0"); break; //null character
					case 'r': printf("\r"); break; //carriage return character
					case 'b': printf("\b"); break; //backspace character
					case 'f': printf("\f"); break; //form feed character
					case 'v': printf("\v"); break; //vertical tab character
					case 'a': printf("\a"); break; //alert (bell) character
					default: y--;
				}
			}
			else printf("%c", info->string[y]);
		}
	} else {
		if (references.number.inf) printf("Infinity");
		else if (references.number.nan) printf("NaN");
		else printf("%d", references.number.value);
	}
	return references;
}

/**************************\
|------ERROR-OUPUTS--------|
\**************************/

void printError01 (PSTAT *info) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: invalid operation or variable name\n");
	printf("         |\n");
	return;
}
