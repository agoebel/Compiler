// atggg3 - Andrew Goebel - codegen - 12/10/12
#include "scanner.h"
#include "token.h"
// token array table
char arrTokens[][10] = {"IDtk", "BEGINtk", "THENtk", "ENDtk", "IFtk","INTtk", "LOOPtk", "VARtk", "VOIDtk", "DOtk","DUMMYtk", "FLOATtk", "SCANFtk", "PROGRAMtk", "PRINTFtk","EQUALtk", "EQEQtk", "COLONtk", "LTtk", "LTEQtk","GTtk", "GTEQtk", "NOTtk", "PLUStk", "MINUStk","STARtk", "SLASHtk", "MODtk", "DOTtk", "LPARtk","RPARtk", "COMMAtk", "LCURLYtk", "RCURLYtk", "SEMICOLtk","LBRACKtk", "RBRACKtk", "NUMBERtk"};
// scanner driver
void scanner(void)
{
    int m;
    int status = 0;
    char *p = currentTok.selection;
    for(m = 0; m < SCAN_BUFFER_SIZE; m++)
        currentTok.selection[m] = '\0';
    while (1)
    {
        if (feof(fsaTok))
        {
            strcpy(currentTok.sym, "EOFtk");
            strcpy(currentTok.selection, "NULL");
            currentTok.line--;
            return;
        }
        gchar = tolower(gchar);
	//printf("%c", gchar);
        status = lookupTable[status][gchar];
	//printf("status = %d\n", status);
        if (status < 0)
            break;
        while (gchar == '#')
        {
            gchar = getc(fsaTok);
//	    printf("%c", gchar);
            while (gchar != '#'){
                if (gchar == '\n')
                    currentTok.line++;
                gchar = getc(fsaTok);
//		printf("%c", gchar);
            }
            gchar = getc(fsaTok);
//	    printf("%c", gchar);
        }
        if (gchar == '\n')
        {
            currentTok.line++;
        }
        if (!(gchar == 32 || gchar ==9 || gchar == 10))
        {
           *p = gchar;
           p += 1;
        }
        gchar = getc(fsaTok);
//	printf("%c", gchar);
    }
    //printf("\n");
    status = (status * -1) - 1;
    strcpy(currentTok.sym, arrTokens[status]);
    //printf("currentTok.sym = %s\n", currentTok.sym);
    //printf("currentTok.selection = %s\n", currentTok.selection);
    return;
}
