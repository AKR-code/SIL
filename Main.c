#include<stdio.h>
#include<stdlib.h>

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
	MRG, ITS										//Implicit functions
} TOKVAL;

typedef struct {
	char pattern[5];
	int weight;
} TOKEN;

void aboutTool ();
void loop (FILE *, int);
char reader (FILE *, VAR *);
char lexer (char *, VAR *);

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
	VAR *varTable = NULL;
	do {
		if (isShell) printf("SIL: ");
		status = reader(inputLoc, varTable);
	} while (status != EXITCHAR);
}

/***************************\
|----------READER-----------|
\***************************/

#define INIT_BUFF_SIZE 64
#define GROWTH_FACTOR 2

char reader(FILE *inputLoc, VAR *varTable) {
	int c = fgetc(inputLoc);
	if (c == '\n') return NOISSUE;
	if (c == EOF) return EXITCHAR;
	int size = INIT_BUFF_SIZE, index = 0, comment = 0, firstChar = 0, doubleQuote = 0, extraSpace = 0, braceDepth = 0;

	char *string = malloc(size * sizeof(char));

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

		string[index] = (char)c;
		index++;

		if (index + 1 >= size) {
			size *= GROWTH_FACTOR;
			char *temp = realloc(string, size * sizeof(char));
			if (!temp) {
				free(string);
				return EXITCHAR;
			}
			string = temp;
		}

NEXT_CHAR:
		c = fgetc(inputLoc);
		c = (c == '\n' && braceDepth ? ',' : c);
	} while (c != '\n' && c != EOF);
	
	if (!firstChar) {
		free(string);
		string = NULL;
		return NOISSUE;
	}

	string[index] = '\0';
	char status = lexer(string, varTable);
	free(string);
	string = NULL;
	return status;
}

/***************************\
|-----------LEXER-----------|
\***************************/

char lexer (char *string, VAR *varTable) {
	printf("-%s-\n", string);
	return NOISSUE;
}
