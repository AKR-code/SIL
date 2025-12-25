#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define EXPECTARG 2
#define VAR_LENGTH 15
typedef struct {
	int value;
	char name[15];
} VAR;

typedef enum {
	STP, KIL, LET, PUT, GET, CON, FOR, TIL, ASN, 	// Explicit functions
	DVD, MLT, SUB, ADD,							 	//Explicit arthematic operators
	EQL, NTE, LES, GRT, LOE, GOE,					//Explicit relational operators
	AND, ORR, 										//Explicit Short circuit operators
	MRG, ITS,										//Implicit functions
	VARIABLE, END_STMT,
	TOTAL_TOKENS
} TOKEN_VAL;

typedef struct {
	char pattern[5];
	int weight;
} TOKEN;

typedef struct {
	TOKEN_VAL token;
	int weight;
	int load;
	int index;
	int length;
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
} PSTAT;

void aboutTool ();
void loop (FILE *, int);
char reader (FILE *, PSTAT *);
char lexer (PSTAT *);
char parser (PSTAT *);
PSTAT *setupEnvironment ();
void clearEnvironment (PSTAT *, FILE *);
void printError01 (int, char *);

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
	info->varTable = NULL;
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

	int size = INIT_BUFF_SIZE, index = 0, comment = 0, firstChar = 0, doubleQuote = 0, extraSpace = 0, braceDepth = 0;
	if (info->string != NULL) free(info->string);
	info->string = malloc(size * sizeof(char));

	do {
		comment = (!doubleQuote && c == '@' ? !comment : comment);
		firstChar = (firstChar || (c != ' ' && c != '\t' && c != '@' && !comment) ? 1 : 0);
		doubleQuote = (c == '"' ? !doubleQuote : doubleQuote);
		if (!firstChar || (!doubleQuote && (comment || c == '@'))) goto NEXT_CHAR;

		if (!doubleQuote && (c == ' ' || c == '\t')) {
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
	if (!firstChar) return NOISSUE;

	info->string[index] = '\0';
	return lexer(info);
}

#undef INIT_BUFF_SIZE 

/***************************\
|-----------LEXER-----------|
\***************************/

char lexeStop(PSTAT *, int *);
char lexeGetOrKill(PSTAT *, int *, int);
char lexeLet(PSTAT *, int *);
char lexePut(PSTAT *, TOKEN [], int *);
char lexeIf(PSTAT *, TOKEN [], int *);
char lexeFor(PSTAT *, TOKEN [], int *);
char lexeTill(PSTAT *, TOKEN [], int *);
char lexeAsn(PSTAT *, TOKEN [], int *);
int pushToken(TOKEN_ARR *, PSTAT *);
char recycle (PSTAT *, int *);

#define INIT_BUFF_SIZE 32
#define INIT_STRING_SIZE 64

char lexer (PSTAT *info) {
	TOKEN tokenTable[MRG] = {
		{"stop", 0}, {"kill", 10}, {"let", 10}, {"put", 10}, {"get", 10}, 
		{"if", 10}, {"for", 10}, {"till", 10}, {"=", 20}, 					 //functions
		{"/", 90}, {"*", 90}, {"-", 70}, {"+", 70},//arthematic operators
		{"==", 55}, {"!=", 55}, {"<", 50}, {">", 50}, {"<=", 50}, {">=", 50},//relational operators
		{"&", 40}, {"|", 30}//shortcircuit operators
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
			info->string[finger] != '\0') {
		tempBuffer[index] = info->string[finger];
		finger++;
		index++;
	}
	tempBuffer[index] = '\0';

	int lexerMode = -1;
	if(info->string[finger++] == '=') {
		for (int x = 0; x < info->noOfVars; x++) {
			if (!strcmp(tempBuffer, info->varTable[x].name)) {
				lexerMode = ASN;
				TOKEN_ARR temp;
				temp.token = VARIABLE;
				temp.index = 0;
				temp.length = 0;
				temp.var = &info->varTable[x];
				if (!pushToken(&temp, info)) {
					printf("UNKOWN ERROR 04: failed to allocate memor to a built in data structure");
					return EXITCHAR;
				}
			}
		}
	} else {
		for (int x = 0; x <= TIL; x++) {
			if (!strcmp(tempBuffer, tokenTable[x].pattern)) {
				lexerMode = x;
				TOKEN_ARR temp;
				temp.token = x;
				temp.weight = tokenTable[x].weight;
				temp.load = 0;
				temp.index = 0;
				temp.length = 0;
				temp.var = NULL;
				if (!pushToken(&temp, info)) {
					printf("UNKOWN ERROR 05: failed `to allocate memory to abuilt in data structure");
					return EXITCHAR;
				}
			}
		}
	}
	if (lexerMode == -1) {
		printError01(info->lineNum, tempBuffer);
		return NOISSUE;
	}
	int status = EXITCHAR;
	switch (lexerMode) {
		case STP: status = lexeStop(info, &finger); break;
		case KIL: status = lexeGetOrKill(info, &finger, 0); break;
		case LET: status = lexeLet(info, &finger); break;
		case PUT: status = lexePut(info, tokenTable, &finger); break;
		case GET: status = lexeGetOrKill(info, &finger, 1); break;
		case CON: status = lexeIf(info, tokenTable, &finger); break;
		case FOR: status = lexeFor(info, tokenTable, &finger); break;
		case TIL: status = lexeTill(info, tokenTable, &finger); break;
		case ASN: status = lexeAsn(info, tokenTable, &finger); break;
		default: break;
	}

	if (status == NOISSUE) return NOISSUE;
	else if (status == EXITCHAR) return EXITCHAR;
	else if (status == LEXE_RECYCLE) goto RE_LEXE;
}

typedef enum {
	ER_LONG_INPUT, ER_NO_INPUT, ER_WRONG_INPUT
} ER;

void printError00 (PSTAT *info, int type, int get) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: ");
	switch (type) {
		case ER_LONG_INPUT: printf("invalid variable name or you cannot use expressions while");
							if (get) printf(" getting");
							else printf(" killing");
							printf(" a variable\n");
		case ER_NO_INPUT: printf("variable name is not mentioned to "); 
						  if (get) printf("get\n");
						  else printf("kill\n"); break;
		case ER_WRONG_INPUT: printf("variable of given name doesnt exist\n");
	}
	printf("         |\n");
}

char lexeGetOrKill (PSTAT *info, int *finger, int get) {
	//searches for variable and adds var token in token array
	char tempBuffer[2 * VAR_LENGTH];
	int index = 0;
	while (index < (2 * VAR_LENGTH - 1) &&
			info->string[*finger] != '\0' &&
			info->string[*finger] != '\n' &&
			info->string[*finger] != ';') {
		tempBuffer[index] = info->string[*finger];
		index++;
		(*finger)++;
	}
	if (index == (2 * VAR_LENGTH - 1)) {
		tempBuffer[2 * VAR_LENGTH - 3] = '.';
		tempBuffer[2 * VAR_LENGTH - 2] = '.';
		tempBuffer[2 * VAR_LENGTH - 1] = '.';
		tempBuffer[2 * VAR_LENGTH - 0] = '\0';
		printError00(info, ER_LONG_INPUT, get);
		return NOISSUE;
	} else if (index > VAR_LENGTH) {
		tempBuffer[index] = '\0';
		printError00(info, ER_LONG_INPUT, get);
		return NOISSUE;
	} else if (index == 0) {
		tempBuffer[index] = '\0';
		printError00(info, ER_NO_INPUT, get);
		return NOISSUE;
	}
	tempBuffer[index] = '\0';
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
					printf("UNKOWN ERROR 06: failed to allocate memory to a built in data structure");
					return EXITCHAR;
				}
		}
	}
	if (!match) printError00(info, ER_WRONG_INPUT, get);

	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') return recycle(info, finger);
	else return parser(info);
}

/**********************************\
|------------LEXE_LET--------------|
\**********************************/

typedef enum {
	ER_LEAD_NUM, ER_SPC_CHAR, ER_WHT_SPA, ER_EXC_LEN
} ERROR_TYPE_02;

void printError02(PSTAT *info, int type) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: variable name cannot have ");
	switch (type) {
		case ER_LEAD_NUM: printf("number at the beginning\n"); break;
		case ER_SPC_CHAR: printf("special characters\n"); break;
		case ER_WHT_SPA: printf("white spaces\n"); break;
		case ER_EXC_LEN: printf("length greater than %d\n", VAR_LENGTH - 1); break;
	}
	printf("         |\n");
}

char lexeLet (PSTAT *info, int *finger) {
	char varNameStr[2 * VAR_LENGTH];
	int index = 0, whiteSpace = 0, specialChar = 0, leadingNum = 0;
	int startPoint = *finger;
	while (index < (2 * VAR_LENGTH - 1) &&
			info->string[*finger] != '\0' &&
			info->string[*finger] != '\n' &&
			info->string[*finger] != ';' ) {
		varNameStr[index] = info->string[*finger];

		if (index == 0 && 
			varNameStr[index] >= '0' && 
			varNameStr[index] <= '9') leadingNum = 1;
		if (!whiteSpace && varNameStr[index] == ' ') whiteSpace = index;
		if (!((varNameStr[index] >= 'a' &&
				varNameStr[index] <= 'z') ||
				(varNameStr[index] >= 'A' &&
				 varNameStr[index] <= 'Z') ||
				(varNameStr[index] >= '0' &&
				 varNameStr[index] <= '9') ||
				varNameStr[index] == ' ')) specialChar = 1;

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
	TOKEN_ARR temp;
	temp.token = VARIABLE;
	temp.index = startPoint;
	temp.length = index;
	if (!pushToken(&temp, info)) {
		printf("UNKNOWN ERROR 08: failed to allocate memory to a built in data structure");
		return EXITCHAR;
	}
	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') return recycle(info, finger);
	else return parser(info);
}

char lexePut (PSTAT *info, TOKEN tokenTable[], int *finger) {
	return parser(info);
}

char lexeIf (PSTAT *info, TOKEN tokenTable[], int *finger) {
	return parser(info);
}

char lexeFor (PSTAT *info, TOKEN tokenTable[], int *finger) {
	return parser(info);
}

char lexeTill (PSTAT *info, TOKEN tokenTable[], int *finger) {
	return parser(info);
}

char lexeAsn (PSTAT *info, TOKEN tokenTable[], int *finger) {
	return parser(info);
}

char lexeStop (PSTAT *info, int *finger) {
	return parser(info);
}

//----------------------Lexer-Utilities--------------------------\\

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
/***************************\
|---------PARSER------------|
\***************************/

char parser (PSTAT *info) {
	printf("line %d", info->lineNum);
	for (int x = 0; x < info->noOfTokens; x++) {
		switch (info->tokenArr[x].token) {
			case STP: printf("'STP', "); return EXITCHAR;
			case KIL: printf("'KIL', "); break;
			case LET: printf("'LET', "); break;
			case PUT: printf("'PUT', "); break;
			case GET: printf("'GET', "); break;
			case CON: printf("'CON', "); break;
			case FOR: printf("'FOR', "); break;
			case TIL: printf("'TIL', "); break;
			case ASN: printf("'ASN', "); break;
			case VARIABLE: printf("'VARIABLE', "); 
						   for (int y = info->tokenArr[x].index; y < (info->tokenArr[x].length + info->tokenArr[x].index); y++) {
							   printf("%c", info->string[y]);
						   }break;
			default : break;
		}
	}
	printf("\n");
	return NOISSUE;
}

/**************************\
|------ERROR-OUPUTS--------|
\**************************/

void printError01 (int lineNum, char *string) {
	printf("         |\n");
	printf("line%5d: %s\n", lineNum, string);
	printf("    Error: invalid operation or variable name\n");
	printf("         |\n");
	return;
}
