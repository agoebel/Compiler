// atggg3 - Andrew Goebel - codegen - 12/10/12
#include "scanner.h"
#include "token.h"
#include "parser.h"
#include "semantic.h"

// declar vars for access accross all files
extern char gchar;
extern tok currentTok;
extern FILE *fsaTok;
extern VStack SS;
extern VElement* MARKER;
extern const int DISPLAY_INDENT;

char* filename = "std.in";
char* outFileName = "result.asm";

void initInputFileName(char* fname)
{
	if (fsaTok != stdin)
	{
		int n = strlen(fname);
		int pos = -1;
		int i;
		for(i = n - 1; i >= 0; i--)
		{
			if (fname[i] == '\\')
			{
				pos = i + 1;
				break;
			}
		}

		if (pos == -1)
		{
			filename = (char *)malloc(n + 1);
			strcpy(filename, fname);
		}else{
			filename = (char*)malloc(n - pos + 2);
			strcpy(filename, fname + pos);
		}
	}
}

void initOutputFileName(char* fname)
{
	char result[100];
	int i;
	for(i = 0; i < 100; i++)
	{
		result[i] = 0;
	}

	int pos = -1;
	int n = strlen(fname);
	for(i = n - 1; i >= 0; i--)
	{
		if (fname[i] == '/')
		{
			pos = i + 1;
			break;
		}
	}

	if (pos == -1)
	{
		outFileName = (char *)malloc(n + 5);
		strcpy(outFileName, fname);
		strcat(outFileName, ".asm\0");
	}else{
		outFileName = (char*)malloc(n - pos + 6);
		strcpy(outFileName, fname + pos);
		strcat(outFileName, ".asm\0");
	}
}

void init()
{
	MARKER = initMarker();
	initStack(&SS);
}

void exitWithSyntax()
{
	puts("Execution: generator {filename} [-disptree]\n generator [-disptree]< {filename}\ngenerator [-disptree]");
	exit(0);
}

// main driver
int main(int argc, char **argv){
	int dispTree = 0;
	gchar = 'q';
	currentTok.line = 1;

	switch (argc)
	{
	case 1:
		fsaTok = stdin;
		break;
	case 2:
		if (!strcmp(argv[1], "-disptree"))
		{
			printf("PARSE ENABLED\n");
			dispTree = 1;
			fsaTok = stdin;
			break;
		}

		initOutputFileName(argv[1]);

		if ((fsaTok = fopen((strcat(argv[1], ".cp")), "r")) == NULL)
		{
			puts("File was not opened...\n");
			exit(0);
		}
		break;
	case 3:
		if (!strcmp(argv[2], "-disptree"))
		{
			dispTree = 1;
		}else{
			exitWithSyntax();
		}

		initOutputFileName(argv[1]);
		if ((fsaTok = fopen((strcat(argv[1], ".cp")), "r")) == NULL)
		{
			puts("File was not opened...\n");
			exit(0);
		}
		break;
	default:
		exitWithSyntax();
	}

	init();
	initInputFileName(argv[1]);

	gettable();
	gchar = getc(fsaTok);
	scanner();
	//SYNTAX ANALYSIS
	APTNode* apt = parse();
	fclose(fsaTok);
	//SEMANTIC ANALYSIS
	performSemanticAnalysis(apt);
	printf("No semantic errors.\n");

	//DISPLAY TREE
	if (dispTree)
	{
		printf("Parse Tree is:\n");
		displayPreOrderAPT(apt, 0);
	}

	//generate code
	FILE* out = fopen(outFileName, "w");
	generate(apt, out);
	fclose(out);
	printf("ASM file generated succesfully.\n");

	return 0;
}
// grab lookuptable file and place in usable table
void gettable(void){
	int i, j;
	FILE *fsaFile;
	fsaFile = fopen("table.dat", "r");
	for(i = 0; i <= 75; i++)
	{
		for(j = 0; j <= 125; j++)
		{
			fscanf(fsaFile, "%d", &lookupTable[i][j]);
		}
	}
	fclose(fsaFile);

	return;
}
