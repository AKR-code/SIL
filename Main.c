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
	VARIABLE,
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
	char *stringBuffer;
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

PSTAT *setupEnvironment() {
	PSTAT *info = malloc(sizeof(PSTAT));
	if (info == NULL) return NULL;
	info->varTable = NULL;
	info->string = NULL;
	info->tokenArr = NULL;
	info->noOfVars = 0;
	info->lineNum = 0;
	info->noOfTokens = 0;
	return info;
}

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
	info->string = NULL; //extra saftey
	char *temp = realloc(info->string, size * sizeof(char));
	if (temp == NULL) {
		printf("UNKNOWN ERROR 01: failed to allocate memory to a built in data structure");
		return EXITCHAR;
	}
	info->string = temp;

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
		c = (c == '\n' && braceDepth ? ';' : c);

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
		c = (c == '\n' && braceDepth ? ';' : c);
	} while (c != '\n' && c != EOF);
	if (!firstChar) return NOISSUE;

	info->string[index] = '\0';
	return lexer(info);
}

#undef INIT_BUFF_SIZE 

/***************************\
|-----------LEXER-----------|
\***************************/

char lexeKill(PSTAT *, int *);
char lexeLet(PSTAT *, int *);
char lexePut(PSTAT *, TOKEN [], int *);
char lexeGet(PSTAT *, int *);
char lexeIf(PSTAT *, TOKEN [], int *);
char lexeFor(PSTAT *, TOKEN [], int *);
char lexeTill(PSTAT *, TOKEN [], int *);
char lexeAsn(PSTAT *, TOKEN [], int *);
int pushToken(TOKEN_ARR *, PSTAT *);

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
	info->tokenArr = NULL; // extra saftey
	TOKEN_ARR *safteyTrigger = realloc(info->tokenArr, info->tokenArrCap * sizeof(TOKEN_ARR));
	if (safteyTrigger == NULL) {
		printf("UNKNOWN ERROR 03: failed to reallocate memory to a built in data structure");
		return EXITCHAR;
	}
	info->tokenArr = safteyTrigger;

	int finger;
	finger = 0;
	char tempBuffer[VAR_LENGTH];

	while (finger < (VAR_LENGTH - 1) &&
			info->string[finger] != ' ' &&
			info->string[finger] != '\0') {
		tempBuffer[finger] = info->string[finger];
		finger++;
	}
	tempBuffer[finger] = '\0';

	int lexerMode = -1;
	if(info->string[finger++] == '=') {
		for (int x = 0; x < info->noOfVars; x++) {
			if (!strcmp(tempBuffer, info->varTable[x].name)) {
				lexerMode = ASN;
				TOKEN_ARR temp;
				temp.token = VARIABLE;
				temp.stringBuffer = NULL;
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
				temp.stringBuffer = NULL;
				temp.var = NULL;
				if (!pushToken(&temp, info)) {
					printf("UNKOWN ERROR 05: failed to allocate memory to abuilt in data structure");
					return EXITCHAR;
				}
			}
		}
	}
	if (lexerMode == -1) {
		printError01(info->lineNum, tempBuffer);
		return NOISSUE;
	}

	switch (lexerMode) {
		case STP: return EXITCHAR; break;
		case KIL: return lexeKill(info, &finger); break;
		case LET: return lexeLet(info, &finger); break;
		case PUT: return lexePut(info, tokenTable, &finger); break;
		case GET: return lexeGet(info, &finger); break;
		case CON: return lexeIf(info, tokenTable, &finger); break;
		case FOR: return lexeFor(info, tokenTable, &finger); break;
		case TIL: return lexeTill(info, tokenTable, &finger); break;
		case ASN: return lexeAsn(info, tokenTable, &finger); break;
		default: break;
	}
}

typedef enum {
	ER_LONG_INPUT, ER_NO_INPUT, ER_WRONG_INPUT
} ER;

void printError00 (int lineNum, char string[], int type) {
	printf("         |\n");
	printf("line%5d: kill %s\n", lineNum, string);
	printf("    Error: ");
	switch (type) {
		case ER_LONG_INPUT: printf("invalid variable name or you cannot use expressions while killing a variable\n"); break;
		case ER_NO_INPUT: printf("variable name is not mentioned to kill\n"); break;
		case ER_WRONG_INPUT: printf("variable of given name doesnt exist to kill\n");
	}
	printf("         |\n");
}

char lexeKill (PSTAT *info, int *finger) {
	//searches for variable and adds var token in token array
	char tempBuffer[2 * VAR_LENGTH];
	int index = 0;
	while (index < (2 * VAR_LENGTH - 1) &&
			info->string[*finger] != '\0' &&
			info->string[*finger] != ',' &&
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
		printError00(info->lineNum, tempBuffer, ER_LONG_INPUT);
		return NOISSUE;
	} else if (index > VAR_LENGTH) {
		tempBuffer[index] = '\0';
		printError00(info->lineNum, tempBuffer, ER_LONG_INPUT);
		return NOISSUE;
	} else if (index == 0) {
		tempBuffer[index] = '\0';
		printError00(info->lineNum, tempBuffer, ER_NO_INPUT);
		return NOISSUE;
	}
	tempBuffer[index] = '\0';
	int match = 0;
	for (int x = 0; x < info->noOfVars; x++) {
		if (!strcmp(tempBuffer, info->varTable[x].name)) {
				match = 1;
				TOKEN_ARR temp;
				temp.token = VARIABLE;
				temp.stringBuffer = NULL;
				temp.var = &info->varTable[x];
				if (!pushToken(&temp, info)) {
					printf("UNKOWN ERROR 06: failed to allocate memory to a built in data structure");
					return EXITCHAR;
				}
		}
	}
	if (!match) printError00(info->lineNum, tempBuffer, ER_WRONG_INPUT);
	return parser(info);
}

/**********************************\
|------------LEXE_LET--------------|
\**********************************/

typedef enum {
	ER_LEAD_NUM, ER_SPC_CHAR, ER_WHT_SPA, ER_EXC_LEN
} ERROR_TYPE_02;

void printError02(int lineNum, char string[], int type) {
	printf("         |\n");
	printf("line%5d: let %s\n", lineNum, string);
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
	while (index < (2 * VAR_LENGTH - 1) &&
			info->string[*finger] != '\0' &&
			info->string[*finger] != ',' &&
			info->string[*finger] != ';' &&
			info->string[*finger] != '=') {
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
		printError02(info->lineNum, varNameStr, ER_LEAD_NUM);
		shouldReturn = 1;
	} 
	if (specialChar) {
		printError02(info->lineNum, varNameStr, ER_SPC_CHAR);
		shouldReturn = 1;
	}
	if (whiteSpace && (index - whiteSpace) != 1) {
		printError02(info->lineNum, varNameStr, ER_WHT_SPA);
		shouldReturn = 1;
	}
	if (index >= VAR_LENGTH - 1) {
		printError02(info->lineNum, varNameStr, ER_EXC_LEN);
		shouldReturn = 1;
	}
	if (shouldReturn) return NOISSUE;

	varNameStr[index] = '\0';
	TOKEN_ARR temp;
	temp.token = VARIABLE;
	temp.stringBuffer = varNameStr; 
	if (!pushToken(&temp, info)) {
		printf("UNKNOWN ERROR 07: failed to allocate memory to a built in data structure");
		return EXITCHAR;
	}
	return parser(info);
}

char lexePut (PSTAT *info, TOKEN tokenTable[], int *finger) {

	return parser(info);
}

char lexeGet (PSTAT *info, int *finger) {
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

//----------------------Lexer-Utilities--------------------------\\

int pushToken (TOKEN_ARR *temp, PSTAT * info) {
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


/***************************\
|---------PARSER------------|
\***************************/

char parser (PSTAT *info) {
	switch (info->tokenArr[0].token) {
		case STP: printf("PARSER: 'STP' tokken from lexer\n"); break;
		case KIL: printf("PARSER: 'KIL' tokken from lexer\n"); break;
		case LET: printf("PARSER: 'LET' tokken from lexer\n"); break;
		case PUT: printf("PARSER: 'PUT' tokken from lexer\n"); break;
		case GET: printf("PARSER: 'GET' tokken from lexer\n"); break;
		case CON: printf("PARSER: 'CON' tokken from lexer\n"); break;
		case FOR: printf("PARSER: 'FOR' tokken from lexer\n"); break;
		case TIL: printf("PARSER: 'TIL' tokken from lexer\n"); break;
		case ASN: printf("PARSER: 'ASN' tokken from lexer\n"); break;
		case VARIABLE: printf("PARSER: 'VARIABLE' tokken from lexer\n"); break;
		default : break;
	}

	return NOISSUE;
}

/**************************\
|------ERROR-OUPUTS--------|
\**************************/

void printError01 (int lineNum, char *string) {
	printf("         |\n");
	printf("line%5d: %s\n", lineNum, string);
	printf("    Error: Invalid Identifier or Variable name\n");
	printf("         |\n");
	return;
}
