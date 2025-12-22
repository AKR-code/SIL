#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define EXPECTARG 2

typedef struct {
	int value;
	char name[50];
} VAR;

typedef enum {
	STP, ASN, LET, PUT, GET, CON, FOR, TIL, KIL, 	// Explicit functions
	DVD, MLT, SUB, ADD,							 	//Explicit arthematic operators
	EQL, NTE, LES, GRT, LOE, GOE,					//Explicit relational operators
	AND, ORR, 										//Explicit Short circuit operators
	MRG, ITS,										//Implicit functions
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
	int strStart;
	int strEnd;
	int *var;
} TOKEN_ARR;

typedef struct {
	VAR *varTable;		//pointer to symbol table for storing user created variables
	TOKEN *tokenTable;	//pointer to predefined array of tokens and thier properties like pattern for matching and weight
	char *string;		//pointer to buffer, where string read from file is stored
	TOKEN_ARR *tokenArr;//lexer generated array which stores tokens and references for parsing and executing
} PSTAT;

void aboutTool ();
void loop (FILE *, int);
char reader (FILE *, PSTAT *);
char lexer (PSTAT *);
PSTAT *setupEnvironment ();
void clearEnvironment (PSTAT *);

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
	clearEnvironment(info); 
}

/*************************\
|----setupEnvironment-----|
\*************************/

PSTAT *setupEnvironment() {
	PSTAT *info = malloc(sizeof(PSTAT));
	if (info == NULL) return NULL;
	info->varTable = NULL;
	TOKEN tokenTableTmp[TOTAL_TOKENS] = {
		{"ext", 0}, {"=", 20}, {"let ", 10}, {"put ", 10}, {"get ", 10}, {"con ", 10}, {"for ", 10}, {"til ", 10}, {"kil ", 10},//functions
		{"/", 90}, {"*", 90}, {"-", 70}, {"+", 70},//arthematic operators
		{"==", 55}, {"!=", 55}, {"<", 50}, {">", 50}, {"<=", 50}, {">=", 50},//relational operators
		{"&", 40}, {"|", 30}//shortcircuit operators
	};
	info->tokenTable = tokenTableTmp;
	info->string = NULL;
	info->tokenArr = NULL;
	return info;
}

/************************\
|----clearEnvironment----|
\************************/

void clearEnvironment(PSTAT *info) {
	free(info->varTable);
	free(info->string);
	free(info->tokenArr);
	free(info);
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
		if (!doubleQuote && c == '}') braceDepth--;
		c = (c == '\n' && braceDepth ? ',' : c);

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
		c = (c == '\n' && braceDepth ? ',' : c);
	} while (c != '\n' && c != EOF);
	if (!firstChar) return NOISSUE;

	info->string[index] = '\0';
	char status = lexer(info);
	return status;
}

/***************************\
|-----------LEXER-----------|
\***************************/

char lexer (PSTAT *info) {
	printf("-%s-\n", info->string);
	return NOISSUE;
}
