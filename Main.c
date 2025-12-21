#include<stdio.h>
#include<stdlib.h>

#define EXPECTARG 2
#define EXITCHAR 0
#define NOISSUE 1

typedef struct {
	int value;
	char name[50];
} VAR; 

void aboutTool ();
void loop (FILE *, int);
char reader (FILE *);
char lexer (char *, int);

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

void aboutTool () {
	printf("SIL V0.1\n");
	printf("this is a prototype for LAMDA\n");
	return;
}

void loop(FILE *inputLoc, int isShell) {
	char status = EXITCHAR;
	VAR *varTable = NULL;
	do {
		if (isShell) printf("SIL: ");
		status = reader(inputLoc);
	} while (status != EXITCHAR);
}

char reader(FILE *inputLoc) {
	int c = fgetc(inputLoc);
	if (c == '\n') return NOISSUE;
	if (c == EOF) return EXITCHAR;

	int size = 32, index = 0, comment = 0, firstChar = 0, doubleQuote = 0, extraSpace = 0;

	char *string = malloc(size * sizeof(char));

	do {
		comment = (c == '@' ? !comment : comment);
		firstChar = (firstChar || (c != ' ' && c != '\t' && c != '@' && !comment) ? 1 : 0);
		if (!firstChar || comment || c == '@') continue;

		doubleQuote = (doubleQuote || c == '"' ? !doubleQuote : doubleQuote);
		if (!doubleQuote && (c == ' ' || c == '\t')) {
			c = ' ';
			if (extraSpace) continue;
			extraSpace = 1;
		} else extraSpace = 0;

		string[index] = (char)c;
		index++;

		if (index + 1 >= size) {
			size *= 2;
			char *temp = realloc(string, size * sizeof(char));
			if (!temp) {
				free(string);
				return EXITCHAR;
			}
			string = temp;
		}
	} while ((c = fgetc(inputLoc)) != '\n' && c != EOF);
	
	if (!firstChar) {
		free(string);
		string = NULL;
		return NOISSUE;
	}

	string[index] = '\0';
	char status = lexer(string, index);
	free(string);
	string = NULL;
	return status;
}

char lexer (char *string, int len) {
	printf("-%s-\n", string);
	return 1;
}


