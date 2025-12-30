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
	STP, KIL, LET, PUT, GET, CON, FOR, TIL, ASN, 	// Explicit functions
	DVD, MLT, SUB, ADD,							 	//Explicit arthematic operators
	EQL, NTE, LES, GRT, LOE, GOE,					//Explicit relational operators
	AND, ORR, 										//Explicit Short circuit operators
	VARIABLE, STRING, CONSTANT, END_STMT,
	TOTAL_TOKENS
} TOKEN_VAL;

typedef struct {
	char pattern[5];
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
		doubleQuote = (c == '"' && info->string[index-1] != '\\' ? !doubleQuote : doubleQuote);
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
char lexeVarFinder (PSTAT *, int *);
char lexeVarMatcher (PSTAT *, char [], int);
int checkCondition (PSTAT *, int *);
int checkVar (PSTAT *, int *);
void printError01 (PSTAT *);
char reportTrash (PSTAT *, int *);
char lexeExpression (PSTAT *, int *);
char lexeNumber (PSTAT *, int *);

#define INIT_BUFF_SIZE 32
#define INIT_STRING_SIZE 64

char lexer (PSTAT *info) {
	TOKEN tokenTable[TOTAL_TOKENS] = {
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
					printf("UNKOWN ERROR 04: failed to allocate memor to a built in data structure");
					return EXITCHAR;
				}
				break;
			}
		}
	} else {
		for (int x = 0; x <= TIL; x++) {
			if (!strcmp(tempBuffer, tokenTable[x].pattern)) {
				lexerMode = x;
				TOKEN_ARR temp;
				temp.token = x;
				temp.weight = tokenTable[x].weight;
				temp.index = 0;
				temp.length = 0;
				temp.var = NULL;
				if (!pushToken(&temp, info)) {
					printf("UNKOWN ERROR 05: failed `to allocate memory to abuilt in data structure");
					return EXITCHAR;
				}
				break;
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
			info->string[*finger] == '[' ||
			info->string[*finger] == ']' ||
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
			continue;
		} else {
			printError01(info);
			shouldReturn = 1;
		}
		(*finger)++;
	} while (checkCondition(info, finger));
	
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
	char status = lexeExpression(info, finger);
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

char lexeExpression (PSTAT *info, int *finger) {
	int operator = 0, load = 0, shouldReturn = 0;
	do {
		if (info->string[*finger] == ' ') {}
		else if (info->string[*finger] == '(') load += 1000;
		else if (info->string[*finger] == ')') {
			load -= 1000;
			if (load < 0) {
				printErrorAsn(info, TOO_MANY_CLO);
				shouldReturn = 1;
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
					temp.weight = 70 + load;
					break; 
				case '-': 
					temp.token = SUB;
					temp.weight = 70 + load;
					break;
				case '*': 
					temp.token = MLT;
					temp.weight = 90 + load;
					break;
				case '/': 
					temp.token = DVD; 
					temp.weight = 90 + load;
					break;
			}
			if (!pushToken(&temp, info)) {
				printf("UNKNOWN ERROR 14");
				return EXITCHAR;
			}
			operator = 0;
		} else {
			printError01(info);
			return NOISSUE;
		}
		(*finger)++;
	} while (checkCondition(info, finger));
	if (load != 0 && load > 0) {
		printErrorAsn(info, PAIR_CLO_ER);
		return NOISSUE;
	} 
	if (shouldReturn) return NOISSUE;
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

char parser (PSTAT *info) {
	int tokenIndex = 0;
	RES init;
	init.number.value = 0;
	init.number.inf = 0;
	init.number.nan = 0;
	init.isString = 0;
	while (tokenIndex < info->noOfTokens) {
		init.start = tokenIndex;
		while (tokenIndex < info->noOfTokens && 
				info->tokenArr[tokenIndex].token != END_STMT) {
			// pending work =================================================================== conditional block selections, and loops should be implemented here =======

			// after pending work
			tokenIndex++;
		}
		init.end = tokenIndex;
		if (tokenIndex < info->noOfTokens) tokenIndex++;
		//printf ("parsing from %d to %d\n", init.start, init.end);
		RES m = recursiveParser(info, init);
		if (m.prog == EXITCHAR) return EXITCHAR;
	}
	return NOISSUE;
}

RES recursiveParser (PSTAT *info, RES postres) {
	postres.prog = NOISSUE;
	// rare saftey trigger for abnormal 0 window size case
	if (postres.start == postres.end) {
		printf("Abnormal error found by Parser");
		return postres;
	}

	//base case when the window is just one token
	if (postres.start + 1 == postres.end) {
		//printf("Base case hit at token index %d\n", postres.start);
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
	//printf("Min wt tok in the win fro %d to %d at index %d\n", postres.start, postres.end, minWtIndex);
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
		// more cases will come in future
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
		// more cases will come in future
	}
	return postres;
}

/**************************\
|---------ENGINE-----------|
\**************************/

//this only works for single statement per single line because I blindly written arrayelement 
//numbers instead of somehow calculating; and these blind numbers dont work
//for multiple stmts per single line

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
	} else if (info->tokenArr[x].token == VARIABLE) {
		if (info->tokenArr[x].var->inf) printf("Infinity");
		else if (info->tokenArr[x].var->nan) printf("NaN");
		else printf("%d", info->tokenArr[x].var->value);
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
