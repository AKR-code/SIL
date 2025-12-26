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
	VARIABLE, STRING, END_STMT,
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
char lexeIdentifier(PSTAT *, int *);
char lexeVarFinder (PSTAT *, int *);
char lexeVarMatcher (PSTAT *, char []);
int checkCondition (PSTAT *, int *);
int checkVar (PSTAT *, int *);
void printError01 (PSTAT *);

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

void printError00 (PSTAT *info, int type) {
	printf("         |\n");
	printf("line%5d: %s\n", info->lineNum, info->string);
	printf("    Error: ");
	switch (type) {
		case ER_LONG_INPUT: printf("invalid variable name or you cannot use expressions in this operation\n"); break;
		case ER_NO_INPUT: printf("variable name is not mentioned\n"); break;
		case ER_WRONG_INPUT: printf("variable of given name doesn't exist\n");
	}
	printf("         |\n");
	return;
}

char lexeKill (PSTAT *info, int *finger) {
	char status;
	status = lexeVarFinder(info, finger);
	if (status == EXITCHAR) return EXITCHAR;
	else if (status == NOISSUE) return NOISSUE;

	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') return recycle(info, finger);
	else return parser(info);
}

char lexeGet (PSTAT *info, int *finger) {
	char status;
	status = lexeVarFinder(info, finger);
	if (status == EXITCHAR) return EXITCHAR;
	else if (status == NOISSUE) return NOISSUE;

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
	return;
}

char lexeLet (PSTAT *info, int *finger) {
	char varNameStr[2 * VAR_LENGTH];
	int index = 0, whiteSpace = 0, specialChar = 0, leadingNum = 0;
	int startPoint = *finger;
	while (index < (2 * VAR_LENGTH - 1) &&
			checkCondition(info, finger)) {
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
	int usedBrackets = 0, mergeNeeded = 0, shouldReturn = 0, neverUsedBrackets = 1;
	do {
		if (info->string[*finger] == '[') {
			usedBrackets++;
			neverUsedBrackets = 0;
		} else if (info->string[*finger] == ']') usedBrackets--;
		else if (info->string[*finger] == ' '){
			//do nothing eat five star ***
		}else if (info->string[*finger] == '"') {
			if (mergeNeeded) {
				printErrorPut(info, ER_WANT_MRG);
				shouldReturn = 1;
			}
			(*finger)++;
			int startPoint = *finger;
			while (info->string[*finger] != '"') {
				(*finger)++;
				if (!checkCondition(info, finger)) {
					printErrorPut(info, ER_UNCLOSED_PAIR);
					shouldReturn = 1;
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
			temp.token = MRG;
			temp.index = 0;
			temp.length = 0;
			temp.var = NULL;
			if (!pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 10: failed to allocate memory to a built in data structure");
				return EXITCHAR;
			}
			mergeNeeded = 0;
		} else if (checkVar(info, finger)) {
			if (mergeNeeded) {
				printErrorPut(info, ER_WANT_MRG);
				shouldReturn = 1;
			}
			char status;
			status = lexeVarFinder(info, finger);
			if (status == EXITCHAR) return EXITCHAR;
			else if (status == NOISSUE) shouldReturn = 1;
			mergeNeeded = 1;
		} else {
			printError01(info);
			shouldReturn = 1;
		}
		(*finger)++;
	} while (!(!neverUsedBrackets && usedBrackets == 0) && checkCondition(info, finger));

	if (shouldReturn) return NOISSUE;
	if (info->string[*finger] == '\n' ||
		info->string[*finger] == ';') return recycle(info, finger);
	else return parser(info);
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

char lexeIdentifier (PSTAT *info, int *finger) {
	//searches for variable and adds var token in token array
	char tempBuffer[2 * VAR_LENGTH];
	int index = 0;
	while (index < (2 * VAR_LENGTH - 1) &&
			checkCondition(info, finger)) {
		tempBuffer[index] = info->string[*finger];
		index++;
		(*finger)++;
	}
	if (index == (2 * VAR_LENGTH - 1)) {
		tempBuffer[2 * VAR_LENGTH - 3] = '.';
		tempBuffer[2 * VAR_LENGTH - 2] = '.';
		tempBuffer[2 * VAR_LENGTH - 1] = '.';
		tempBuffer[2 * VAR_LENGTH - 0] = '\0';
		printError00(info, ER_LONG_INPUT);
		return NOISSUE;
	} else if (index > VAR_LENGTH) {
		tempBuffer[index] = '\0';
		printError00(info, ER_LONG_INPUT);
		return NOISSUE;
	} else if (index == 0) {
		tempBuffer[index] = '\0';
		printError00(info, ER_NO_INPUT);
		return NOISSUE;
	}
	return lexeVarMatcher(info, tempBuffer);
}

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
	return lexeVarMatcher(info, tempBuffer);
}

char lexeVarMatcher (PSTAT *info, char tempBuffer[VAR_LENGTH]) {
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
	if (!match) {
		printError00(info, ER_WRONG_INPUT);
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

/***************************\
|---------PARSER------------|
\***************************/

char kill(PSTAT *);
char let(PSTAT *);
char put(PSTAT *);
char get(PSTAT *);

char parser (PSTAT *info) {
	char status;
	//printf("line %d", info->lineNum);
	//for (int x = 0; x < info->noOfTokens; x++) {
		switch (info->tokenArr[0].token) {
			case STP: return EXITCHAR;
			case KIL: status = kill(info); break;
			case LET: status = let(info); break;
			case PUT: status = put(info); break;
			case GET: status = get(info); break;
			case CON: printf("'CON', "); break;
			case FOR: printf("'FOR', "); break;
			case TIL: printf("'TIL', "); break;
			case ASN: printf("'ASN', "); break;
			/*case VARIABLE: printf("'VARIABLE', "); 
					for (int y = info->tokenArr[x].index; y < (info->tokenArr[x].length + info->tokenArr[x].index); y++) {
						printf("%c", info->string[y]);
					}break;
			case END_STMT: printf("'END', ");break;
			case MRG: printf("'MRG', ");break;
			case STRING: printf("'STR', ");
					for (int y = info->tokenArr[x].index; y < (info->tokenArr[x].length + info->tokenArr[x].index); y++) {
						printf("%c", info->string[y]);
					}break;*/
			default : break;
		}
	//}
	return status;
}

/**************************\
|---------ENGINE-----------|
\**************************/

//this only works for single statement per single line because I blindly written arrayelement 
//numbers instead of somehow calculating; and these blind numbers dont work
//for multiple stmts per single line

char let (PSTAT *info) {
	int index = 0;
	for (int x = info->tokenArr[1].index; x < (info->tokenArr[1].length + info->tokenArr[1].index); x++) {
		info->varTable[info->noOfVars].name[index] = info->string[x];
		index++;
	}
	info->varTable[info->noOfVars].name[index] = '\0';
	info->varTable[info->noOfVars].value = 0;
	info->noOfVars++;

	if (info->noOfVars >= info->varTableCap-1) {
		info->varTableCap *= GROWTH_FACTOR;
		VAR *temp = realloc(info->varTable, info->varTableCap * sizeof(VAR));
		if (temp == NULL) {
			printf("UNKOWN ERROR 12 memory allocation failure for variable");
			return EXITCHAR;
		}
		info->varTable = temp;
	}
	return NOISSUE;
}

char get (PSTAT *info) {
	int newVal;
	int error = scanf("%d", &newVal);
	if (error == EOF) return NOISSUE;
	if (error == 0) {
		while (getchar() != '\n');
		return NOISSUE;
	}
	info->tokenArr[1].var->value = newVal;
	return NOISSUE;
}

char kill (PSTAT *info) {
	info->tokenArr[1].var->name[0] = '\0';
	return NOISSUE;
}

char put (PSTAT *info) {
	int x = 1;
	while (x < info->noOfTokens) {
		if (info->tokenArr[x].token == STRING) {
			for (int y = info->tokenArr[x].index; y < (info->tokenArr[x].length + info->tokenArr[x].index); y++) {
				printf("%c", info->string[y]);
			}
		} else if (info->tokenArr[x].token == VARIABLE) {
			printf("%d", info->tokenArr[x].var->value);
		}
		x++;
	}
	printf("\n");
	return NOISSUE;
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
