// atggg3 - Andrew Goebel - codegen - 12/10/12
#ifndef TOKEN_H
#define TOKEN_H
#define SCAN_BUFFER_SIZE 50

//define token structure
typedef struct {
	char sym[10];
	char selection[SCAN_BUFFER_SIZE];
	int line;
} tok;

#endif
