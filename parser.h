// atggg3 - Andrew Goebel - codegen - 12/10/12
#ifndef PARSER_H
#define PARSER_H

#include "semantic.h"

// declare parser logic functions
APTNode* PROGRAM(void);
APTNode* BLOCK(void);
APTNode* VAR(void);
APTNode* TYPE(void);
APTNode* MVARS(void);
APTNode* EXPR(void);
APTNode* T(void);
APTNode* F(void);
APTNode* R(void);
APTNode* STATS(void);
APTNode* MSTAT(void);
APTNode* STAT(void);
APTNode* IN(void);
APTNode* OUT(void);
APTNode* IF(void);
APTNode* LOOP(void);
APTNode* ASSIGN(void);
APTNode* RO(void);

APTNode* parse(void);
void errMsg(char *);

#endif
