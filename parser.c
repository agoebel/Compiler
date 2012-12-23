// atggg3 - Andrew Goebel - codegen - 12/10/12
#include <stdlib.h>
#include "scanner.h"
#include "parser.h"
#include "token.h"

//TODO: CHECK FOR VARIABLE SCOPING
//TODO: CHECK FOR DECLARATION BEFORE USE

extern char* filename;
extern VElement* MARKER;
extern const int MARKER_TYPE;
extern const int VAR_TYPE;
extern VStack SS;

// <program>   ->      PROGRAM <var>  <block>  .
APTNode* PROGRAM(void)
{
	if (strcmp(currentTok.sym, "PROGRAMtk") == 0)
	{
		scanner();

		//BUILD <VAR> NODE
		APTNode* varNode = VAR();
		//BUILD <BLOCK> NODE
		APTNode* blockNode = BLOCK();
		if (strcmp(currentTok.sym, "DOTtk") == 0)
		{
			scanner();

			//BUILD <PROGRAM> NODE
			APTNode* programNode = createNonIdAPTNode("<PROGRAMtk>");
			addChildNode(programNode, varNode);
			addChildNode(programNode, blockNode);

			return programNode;
		} else errMsg("DOTtk");
	} else errMsg("PROGRAMtk");
}
//<block>       ->      BEGIN  <var> <stats> END
APTNode* BLOCK(void)
{
	if (strcmp(currentTok.sym, "BEGINtk") == 0)
	{
		scanner();

		//BUILD <VAR> NODE
		APTNode* varNode = VAR();
		APTNode* statsNode = STATS();
		if (strcmp(currentTok.sym, "ENDtk") == 0)
		{
			scanner();

			//BUILD <BLOCK> NODE
			APTNode* blockNode = createNonIdAPTNode("<BLOCKtk>");
			addChildNode(blockNode, varNode);
			addChildNode(blockNode, statsNode);

			return blockNode;
		} else errMsg("ENDtk");
	} else errMsg("BEGINtk");
}

// <var>         ->      empty | <type> ID <mvars> ;
APTNode* VAR(void)
{
	//HERE
	if ((strcmp(currentTok.sym, "INTtk") == 0) || (strcmp(currentTok.sym, "FLOATtk") == 0)) {
		//BUILD <TYPE> NODE
		APTNode* typeNode = TYPE();
		if (strcmp(currentTok.sym, "IDtk") == 0)
		{
			APTNode* idNode = createIdAPTNode("<IDtk>", currentTok.selection, NULL, currentTok.line);

			scanner();

			//BUILD <MVARS> NODE
			APTNode* mvarsNode = MVARS();
			if (strcmp(currentTok.sym, "SEMICOLtk") == 0)
			{
				scanner();

				//BUILD <VAR> NODE
				APTNode* varNode = createNonIdAPTNode("<VARtk>");
				addChildNode(varNode, typeNode);
				addChildNode(varNode, idNode);
				addChildNode(varNode, mvarsNode);

				return varNode;
			} else errMsg("SEMICOLtk");
		} else errMsg("IDtk");
	} else return NULL;
}

// <type>        ->     INT | FLOAT
APTNode* TYPE(void)
{
	if (strcmp(currentTok.sym, "INTtk") == 0)
	{
		scanner();
		return createNonIdAPTNode("<INTtk>");
	} else if (strcmp(currentTok.sym, "FLOATtk") == 0)
	{
		scanner();
		return createNonIdAPTNode("<FLOATtk>");
	} else errMsg("INTtk or FLOATtk");
}

// <mvars>     ->     empty | , ID <mvars>
APTNode* MVARS(void)
{
	//HERE
	if (strcmp(currentTok.sym, "COMMAtk") == 0)
	{
		scanner();
		if (strcmp(currentTok.sym, "IDtk") == 0)
		{
			//BUILD <ID> NODE
			APTNode* idNode = createIdAPTNode("<IDtk>", currentTok.selection, NULL, currentTok.line);

			scanner();

			//BUILD CHILD <MVARS> NODE
			APTNode* childMvarsNode = MVARS();

			//BUILD PARENT <MVARS> NODE
			APTNode* parentMvarsNode = createNonIdAPTNode("<MVARS>");
			addChildNode(parentMvarsNode, idNode);
			addChildNode(parentMvarsNode, childMvarsNode);

			return parentMvarsNode;
		} else errMsg("IDtk");
	} else return NULL;  // MVARS -> empty
}

// <expr>         ->      <T> + <expr> | <T> - <expr> | <T>
APTNode* EXPR(void)
{
	APTNode* tNode = T();
	APTNode* parentExprNode = createNonIdAPTNode("<EXPR>");
	addChildNode(parentExprNode, tNode);

	if (strcmp(currentTok.sym, "PLUStk") == 0)
	{
		addChildNode(parentExprNode, createNonIdAPTNode("<PLUStk>"));
		scanner();
		addChildNode(parentExprNode, EXPR());
	} else if (strcmp(currentTok.sym, "MINUStk") == 0)
	{
		addChildNode(parentExprNode, createNonIdAPTNode("<MINUStk>"));
		scanner();
		addChildNode(parentExprNode, EXPR());
	}

	return parentExprNode;
}

// <T>              ->      <F> * <T> | <F> / <T> | <F>
APTNode* T(void)
{
	APTNode* fNode = F();
	APTNode* childTNode = NULL;
	APTNode* opNode = NULL;

	APTNode* parentTNode = createNonIdAPTNode("<T>");
	addChildNode(parentTNode, fNode);

	if (strcmp(currentTok.sym, "STARtk") == 0)
	{
		opNode = createNonIdAPTNode("<STARtk>");
		addChildNode(parentTNode, opNode);

		scanner();

		childTNode = T();
		addChildNode(parentTNode, childTNode);
	} else if (strcmp(currentTok.sym, "SLASHtk") == 0)
	{
		opNode = createNonIdAPTNode("<SLASHtk>");
		addChildNode(parentTNode, opNode);

		scanner();

		childTNode = T();
		addChildNode(parentTNode, childTNode);
	}

	return parentTNode;
	//  T -> <F>
}

// <F>              ->      - <F> | <R>
APTNode* F(void)
{
	APTNode* parentFNode = createNonIdAPTNode("<F>");
	if (strcmp(currentTok.sym, "MINUStk") == 0)
	{
		APTNode* minusNode = createNonIdAPTNode("<MINUStk>");
		addChildNode(parentFNode, minusNode);

		scanner();

		APTNode* childFNode = F();
		addChildNode(parentFNode, childFNode);
	} else{
	       	APTNode* rNode = R(); //  F -> <R>
		addChildNode(parentFNode, rNode);
	}

	return parentFNode;
}

// <R>              ->      (<expr>) | ID | Number
APTNode* R(void)
{
	APTNode* rNode = createNonIdAPTNode("<R>");
	//HERE
	if (strcmp(currentTok.sym, "LPARtk") == 0)
	{
		APTNode* lparNode = createNonIdAPTNode("<LPARtk>");
		addChildNode(rNode, lparNode);

		scanner();

		APTNode* exprNode = EXPR();
		addChildNode(rNode, exprNode);
		if (strcmp(currentTok.sym, "RPARtk") == 0)
		{
			APTNode* rparNode = createNonIdAPTNode("<RPARtk>");
			addChildNode(rNode, rparNode);

			scanner();
			return rNode;
		} else errMsg("RPARtk");
	} else if (strcmp(currentTok.sym, "IDtk") == 0)
	{
		APTNode* idNode = createIdAPTNode("<IDtk>", currentTok.selection, NULL, currentTok.line);
		addChildNode(rNode, idNode);

		scanner();

		return rNode;
	} else if (strcmp(currentTok.sym, "NUMBERtk") == 0)
	{
		APTNode* numberNode = createIdAPTNode("<Numbertk>", NULL, currentTok.selection, currentTok.line);
		addChildNode(rNode, numberNode);

		scanner();

		return rNode;
	} else errMsg("LPARtk or IDtk or NUMBERtk");
}

// <stats>         ->      <stat>  <mStat>
APTNode* STATS(void)
{
	APTNode* statsNode = createNonIdAPTNode("<STATStk>");
	APTNode* statNode = STAT();
	APTNode* mstatNode = MSTAT();

	addChildNode(statsNode, statNode);
	addChildNode(statsNode, mstatNode);

	return statsNode;
}

// <mStat>       ->      empty | <stat>  <mStat>
APTNode* MSTAT(void)
{
	if ((strcmp(currentTok.sym, "SCANFtk") == 0) || (strcmp(currentTok.sym, "PRINTFtk") == 0) ||
		(strcmp(currentTok.sym, "IFtk") == 0) || (strcmp(currentTok.sym, "LOOPtk") == 0) ||
		(strcmp(currentTok.sym, "BEGINtk") == 0) || (strcmp(currentTok.sym, "IDtk") == 0))
	{
		//printf("MSTAT node with sym = %s and text = %s \n", currentTok.sym, currentTok.selection);
		APTNode* parentMstatNode = createNonIdAPTNode("<MSTAT>");
		APTNode* statNode = STAT();
		APTNode* childMstatNode = MSTAT();

		addChildNode(parentMstatNode, statNode);
		addChildNode(parentMstatNode, childMstatNode);

		return parentMstatNode;
	} else return NULL;
}

// <stat>           ->      <in> | <out> | <block> | <if> | <loop> | <assign>
APTNode* STAT(void)
{
	//printf("STAT node with sym = %s and text = %s \n", currentTok.sym, currentTok.selection);
	APTNode* parentStatNode = createNonIdAPTNode("<STATtk>");
	APTNode* childNode;
	if (strcmp(currentTok.sym, "SCANFtk") == 0)
	{
		childNode = IN();
	} else if (strcmp(currentTok.sym, "PRINTFtk") == 0)
	{
		childNode = OUT();
	} else if (strcmp(currentTok.sym, "IFtk") == 0)
	{
		childNode = IF();
	} else if (strcmp(currentTok.sym, "LOOPtk") == 0)
	{
		childNode = LOOP();
	} else if (strcmp(currentTok.sym, "IDtk") == 0)
	{
		childNode = ASSIGN();
	} else if (strcmp(currentTok.sym, "BEGINtk") == 0)
	{
		childNode = BLOCK();
	} else errMsg("STATEMENT tok");

	addChildNode(parentStatNode, childNode);
	return parentStatNode;
}

// <in>              ->      SCANF ID ;
APTNode* IN(void)
{
	//HERE
	if (strcmp(currentTok.sym, "SCANFtk") == 0)
	{
		APTNode* scanfNode = createNonIdAPTNode("<SCANFtk>");
		scanner();
		if (strcmp(currentTok.sym, "IDtk") == 0)
		{
			APTNode* idNode = createIdAPTNode("<IDtk>", currentTok.selection, NULL, currentTok.line);

			scanner();

			if (strcmp(currentTok.sym, "SEMICOLtk") == 0)
			{
				APTNode* inNode = createNonIdAPTNode("<INtk>");
				addChildNode(inNode, scanfNode);
				addChildNode(inNode, idNode);

				scanner();

				return inNode;
			} else errMsg("SEMICOLtk");
		} else errMsg("IDtk");
	} else errMsg("SCANFtk");
}

// <out>            ->      PRINTF  <expr>  ;
APTNode* OUT(void)
{
	if (strcmp(currentTok.sym, "PRINTFtk") == 0)
	{
		APTNode* scanfNode = createNonIdAPTNode("<PRINTFtk>");
		scanner();
		APTNode* exprNode = EXPR();
		if (strcmp(currentTok.sym, "SEMICOLtk") == 0)
		{
			APTNode* outNode = createNonIdAPTNode("<OUTtk>");
			addChildNode(outNode, scanfNode);
			addChildNode(outNode, exprNode);

			scanner();

			return outNode;
		} else errMsg("SEMICOLtk");
	} else errMsg("PRINTFtk");
}

// <if>               ->      IF [ <expr> <RO> <expr> ]  <block>
APTNode* IF(void)
{
	if (strcmp(currentTok.sym, "IFtk") == 0)
	{
		scanner();
		if (strcmp(currentTok.sym, "LBRACKtk") == 0)
		 {
			scanner();
			APTNode* lExprNode = EXPR();
			APTNode* roNode = RO();
			APTNode* rExprNode = EXPR();
			if (strcmp(currentTok.sym, "RBRACKtk") == 0)
			{
				scanner();

				APTNode* blockNode = BLOCK();

				APTNode* ifNode = createNonIdAPTNode("<IFtk>");
				addChildNode(ifNode, lExprNode);
				addChildNode(ifNode, roNode);
				addChildNode(ifNode, rExprNode);
				addChildNode(ifNode, blockNode);

				return ifNode;
			} else errMsg("RBRACKtk");
		} else errMsg("LBRACKtk");
	} else errMsg("IFtk");
}

// <loop>          ->      LOOP [ <expr> <RO> <expr> ] <block>
APTNode* LOOP(void)
{
	if (strcmp(currentTok.sym, "LOOPtk") == 0)
	{
		scanner();
		if (strcmp(currentTok.sym, "LBRACKtk") == 0)
		{
			scanner();
			APTNode* lExprNode = EXPR();
			APTNode* roNode = RO();
			APTNode* rExprNode = EXPR();
			if (strcmp(currentTok.sym, "RBRACKtk") == 0)
			{
				scanner();

				APTNode* blockNode = BLOCK();

				APTNode* loopNode = createNonIdAPTNode("<LOOPtk>");
				addChildNode(loopNode, lExprNode);
				addChildNode(loopNode, roNode);
				addChildNode(loopNode, rExprNode);
				addChildNode(loopNode, blockNode);

				return loopNode;
			} else errMsg("RBRACKtk");
		} else errMsg("LBRACKtk");
	} else errMsg("LOOPtk");
}

// <assign>       ->      ID = <expr> ;
APTNode* ASSIGN()
{
	//HERE
	if (strcmp(currentTok.sym, "IDtk") == 0)
	{
		APTNode* idNode = createIdAPTNode("<IDtk>", currentTok.selection, NULL, currentTok.line);

		scanner();
		if (strcmp(currentTok.sym, "EQUALtk") == 0)
		{
			APTNode* eqNode = createNonIdAPTNode("<EQUALtk>");
			scanner();
			APTNode* exprNode = EXPR();
			if (strcmp(currentTok.sym, "SEMICOLtk") == 0)
			{
				APTNode* assignNode = createNonIdAPTNode("<ASSIGNtk>");
				addChildNode(assignNode, idNode);
				addChildNode(assignNode, eqNode);
				addChildNode(assignNode, exprNode);

				scanner();

				return assignNode;
			} else errMsg("SEMICOLtk");
		} else errMsg("EQUALtk");
	} else errMsg("IDtk");
}

// <RO>            ->      <= | >= | == |  > | <  |  !
APTNode* RO(void)
{
	if (strcmp(currentTok.sym, "LTEQtk") == 0)
	{
		scanner();
		return createNonIdAPTNode("<LTEQtk>");
	} else if (strcmp(currentTok.sym, "GTEQtk") == 0)
	{
		scanner();
		return createNonIdAPTNode("<GTEQtk>");
	} else if (strcmp(currentTok.sym, "EQEQtk") == 0)
	{
		scanner();
		return createNonIdAPTNode("<EQEQtk>");
	} else if (strcmp(currentTok.sym, "GTtk") == 0)
	{
		scanner();
		return createNonIdAPTNode("<GTtk>");
	} else if (strcmp(currentTok.sym, "LTtk") == 0)
	{
		scanner();
		return createNonIdAPTNode("<LTtk>");
	} else if (strcmp(currentTok.sym, "NOTtk") == 0)
	{
		scanner();
		return createNonIdAPTNode("<NOTtk>");
	} else errMsg("RELOP tok");
}

// kick off the parser logic, determine success
APTNode* parse(void)
{
	APTNode* programNode = PROGRAM();
	if (strcmp(currentTok.sym, "EOFtk") == 0)
	{
		printf("Parse Completed Successfully.\n");
		return programNode;
	} else errMsg("eoftk");
	return NULL;
}

// catch errors
void errMsg(char *exp)
{
	printf("%s:%d: syntax error: %s should have come next, but %s was processed.\n", filename, currentTok.line, exp, currentTok.sym);
	exit(1);
}
