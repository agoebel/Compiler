// atggg3 - Andrew Goebel - codegen - 12/10/12
#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
// declarations for scanner
void scanner(void);
void gettable(void);
int lookupTable[76][126];
tok currentTok;
FILE *fsaTok;
char gchar;

#endif
