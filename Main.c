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
	if (temp == NULL) return EXITCHAR;
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
			if (!temp) return EXITCHAR;
			info->string = temp;
		}

NEXT_CHAR:
		c = fgetc(inputLoc);
		c = (c == '\n' && braceDepth ? ';' : c);
	} while (c != '\n' && c != EOF);
	if (!firstChar) return NOISSUE;

	info->string[index] = '\0';
	char status = lexer(info);
	return status;
}

#undef INIT_BUFF_SIZE 

/***************************\
|-----------LEXER-----------|
\***************************/

#define INIT_BUFF_SIZE 32
#define INIT_STRING_SIZE 64
char lexer (PSTAT *info) {
	TOKEN tokenTable[TOTAL_TOKENS] = {
		{"stop", 0}, {"kill", 10}, {"let", 10}, {"put", 10}, {"get", 10}, {"if", 10}, {"for", 10}, {"till", 10}, {"=", 20},//functions
		{"/", 90}, {"*", 90}, {"-", 70}, {"+", 70},//arthematic operators
		{"==", 55}, {"!=", 55}, {"<", 50}, {">", 50}, {"<=", 50}, {">=", 50},//relational operators
		{"&", 40}, {"|", 30}//shortcircuit operators
	};

	info->lineNum++;
	TOKEN_ARR *safteyTrigger = realloc(info->tokenArr, INIT_BUFF_SIZE * sizeof(TOKEN_ARR));
	if (safteyTrigger == NULL) return EXITCHAR;
	info->tokenArr = safteyTrigger;

	int finger = 0, load = 0;
	char tempBuffer[VAR_LENGTH];
	char *localStringBuffer = malloc(INIT_STRING_SIZE * sizeof(char));
	if (localStringBuffer == NULL) return EXITCHAR;

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
				info->tokenArr[0] = temp;
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
				info->tokenArr[0] = temp;
			}
		}
	}
	if (lexerMode == -1) {
		printError01(info->lineNum, tempBuffer);
		return NOISSUE;
	}
	char status = parser(info);
	if (localStringBuffer != NULL) free(localStringBuffer);
	localStringBuffer = NULL;
	return status;
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
