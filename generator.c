// atggg3 - Andrew Goebel - codegen - 12/10/12
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "semantic.h"
#include "generator.h"

#define DEBUG 0
#define COMMENTS 0

extern VElement* MARKER;
extern const int MARKER_TYPE;
extern const int VAR_TYPE;
extern VStack SS;
char name[20];
char globalTemp[20];

VStack resStack;
VStack opStack;
VStack distinctVarNames;
VElement* ZERO;

//used to avoid errors when working with NULL strings in printf statements
char* D(char* data)
{
	if (data == NULL)
	{
		return "NULL";
	}

	return data;
}

//returns 1 if the string denoted by operator is a "+", "-", "*" or "/" token
int isOperator(char* operator)
{
	if (operator == NULL)
	{
		return 0;
	}

	if (!strcmp(operator, "<PLUStk>") || !strcmp(operator, "<MINUStk>") ||
	    !strcmp(operator, "<STARtk>") || !strcmp(operator, "<SLASHtk>"))
	{
		return 1;
	}

	return 0;
}

//used to implement arithmetic operator precedence; a higher value indicates a higher precedence
int getPrecedence(char* operator)
{
	if (!strcmp(operator, "<PLUStk>") || !strcmp(operator, "<MINUStk>"))
	{
		return 1;
	}

	if (!strcmp(operator, "<STARtk>") || !strcmp(operator, "<SLASHtk>"))
	{
		return 2;
	}
}

//A = pop(RESULT_STACK), B = ACC, OP = pop(OPERATOR_STACK), computes A OP B
void performOperationWithACC(FILE* out)
{
	char name[20];

	//get OP
	char* op = pop(&opStack)->data;

	if (DEBUG) {printf("performOperationWithACC(): OP_POP(%s)\n", D(op));}

	//get A
	VElement* A = pop(&resStack);

	if (DEBUG) {printf("performOperationWithACC(): RES_POP(%s)\n", D(A->data));}
	if (DEBUG) {printf("performOperationWithACC(): A = %s, op = %s, ACC\n", D(A->data), op);}

	//analyze OP and produce instruction in ACC
	if (!strcmp(op, "<PLUStk>"))
	{
		fprintf(out, "\tADD\t%s\n", A->data);
	}else if (!strcmp(op, "<MINUStk>"))
	{
		//avoid creating a new temporary variable and substitute SUB X with ADD -X
		fprintf(out, "\tMULT\t-1\n");
		fprintf(out, "\tADD\t%s\n", A->data);
	}else if (!strcmp(op, "<STARtk>"))
	{
		fprintf(out, "\tMULT\t%s\n", A->data);
	}else if (!strcmp(op, "<SLASHtk>"))
	{
		//can not avoid creating a new variable
		strcpy(name, newName(VAR));
		fprintf(out, "\tSTORE\t%s\n", name);
		fprintf(out, "\tLOAD\t%s\n", A->data);
		fprintf(out, "\tDIV\t%s\n", name);
	}
}

//process all operators from the OPERATOR_STACK, going from top to the bottom  up to the first "(" operator or an empty stack, whichever comes first
void reduceParanthesis(FILE* out, int withLPar)
{
	char name[20];

	if (isStackEmpty(&opStack))
	{
		if (DEBUG) {printf("reduceParanthesis(): OP stack is empty\n");}
		return;
	}

	char* topOp = topElement(&opStack);
	if (!isOperator(topOp))
	{
		if (DEBUG) {printf("reduceParanthesis(): OP_POP(%s)\n", D(topOp));}
		pop(&opStack);
		return;
	}
	if (DEBUG) {printf("reduceParanthesis(): topOp = %s\n", D(topOp));}

	//1. take the number at the top of the RESULT_STACK
	VElement* temp = pop(&resStack);
	if (DEBUG) {printf("reduceParanthesis(): RES_POP(%s)\n", D(temp->data));}

	//2. load it into ACC
	fprintf(out, "\tLOAD\t%s\n", temp->data);
	while (isOperator(topOp))
	{
		if (DEBUG)
		{
			printf("reduceParanthesis(): OP stack is: \n"); displayStack(&opStack);
			printf("reduceParanthesis(): RES stack is: \n"); displayStack(&resStack);
		}

		//3. perform operation between ACC, number on top of RESULT_STACK using operator on top of OPERATOR_STACK; store result in ACC
		performOperationWithACC(out);

		topOp = topElement(&opStack);
	}//repeat until empty stack or "(" is encountered

	if (withLPar)
	{
		//remove left paranthesis and store ACC on RESULT_STACK
		strcpy(name, newName(VAR));
		fprintf(out, "\tSTORE\t%s\n", name);
		if (DEBUG) {printf("reduceParanthesis(): RES_PUSH(%s)\n", D(name));}

		push(createVElement(name), &resStack, -1);

		if (DEBUG) {printf("reduceParanthesis(): OP_POP(%s)\n", D(topOp));}

		pop(&opStack);
	}
}

//put crtOp on stack respecting operator precedence; i.e. * can be pushed on top of + or - but not the other way round
void putOperatorOnStack(char* crtOp, FILE* out)
{
	char name[20];
	char* topOp = topElement(&opStack);

	if (DEBUG) {printf("putOperatorOnStack(%s): crtOp = %s topOp = %s\n", D(crtOp), D(crtOp), D(topOp));}

	//BASE CASE: left paranthesis; push and resume
	if (!isOperator(topOp))
	{
		if (DEBUG) {printf("putOperatorOnStack(%s): OP_PUSH(%s)\n", D(crtOp), D(crtOp));}
		push(createVElement(crtOp), &opStack, -1);
		return;
	}

	int opPerformed = 0;
	while (isOperator(topOp))
	{
		//If the precedence of the operator which needs to be put on the stack crtOp is lower than the precedence of the operator
		//on the top of the stack, topOp, perform the operation denoted by topOp and the top two numbers from the RESULT_STACK.
		//An optimization step is performed by using the ACC, thus avoiding the need to create extra temporary variables.
		if (getPrecedence(crtOp) <= getPrecedence(topOp))
		{
			if (!opPerformed)
			{
				//in the very first iteration load number from RESULT_STACK into ACC
				VElement* temp = pop(&resStack);
				if (DEBUG) {printf("putOperatorOnStack(): RES_POP(%s)\n", D(temp->data));}
				fprintf(out, "\tLOAD\t%s\n", temp->data);
			}

			if (DEBUG) {printf("putOperatorOnStack(%s): getPrecedence(%s) <= getPrecedence(%s)\n", D(crtOp), D(crtOp), D(topOp));}
			//perform operation topOp between ACC and the number on the top of the RESULT_STACK
			performOperationWithACC(out);
			opPerformed = 1;
		}else{
			if (DEBUG) {printf("putOperatorOnStack(%s): getPrecedence(%s) > getPrecedence(%s)\n", D(crtOp), D(crtOp), D(topOp));}
			break;
		}
		//fetch next operand
		topOp = topElement(&opStack);
	}

	if (opPerformed)
	{
		//store result from ACC into temporary variable
		strcpy(name, newName(VAR));
		fprintf(out, "\tSTORE\t%s\n", name);
		if (DEBUG) {printf("putOperatorOnStack(): RES_PUSH(%s)\n", D(name));}
		push(createVElement(name), &resStack, -1);
	}

	if (DEBUG) {printf("putOperatorOnStack(%s): OP_PUSH(%s)\n", D(crtOp), D(crtOp));}
	push(createVElement(crtOp), &opStack, -1);
}

//evaluates arithmetic expressions using the Shunting-yard algorithm
void evaluateExpression(APTNode* node, FILE* out)
{
	//BASE CASE
	if (node == NULL)
	{
		return;
	}

	if (DEBUG) {printf("evaluateExpression(%s) - %d children\n", D(node->attr.token), node->numChildren);}

	if (node->numChildren == 0)
	{
		//<IDtk> || <Numbertk>
		//PUSH on RESULT_STACK
		if (!strcmp(node->attr.token, "<IDtk>"))
		{
			if (DEBUG) {printf("evaluateExpression(%s): RES_PUSH(%s)\n", D(node->attr.token), D(node->attr.name));}
			push(createVElement(node->attr.name), &resStack, -1);
		}else if (!strcmp(node->attr.token, "<Numbertk>"))
		{
			if (DEBUG) {printf("evaluateExpression(%s): RES_PUSH(%s)\n", D(node->attr.token), D(node->attr.value));}
			push(createVElement(node->attr.value), &resStack, -1);
		}
	}else if (node->numChildren == 1)
	{
		//<T> || <R>
		//move deeper into the tree
		evaluateExpression(node->children[0], out);
	}
	else if (node->numChildren == 2)
	{
		//<MINUStk> <F>
		//ADD 0 to the RESULT_STACK and "-" to the OPERATOR_STACK
		if (DEBUG) {printf("evaluateExpression(%s): RES_PUSH(ZERO)\n", D(node->attr.token));}
		push(ZERO, &resStack, -1);
		if (DEBUG) {printf("evaluateExpression(%s): OP_PUSH(%s)\n", D(node->attr.token), D(node->children[0]->attr.token));}
		push(createVElement(node->children[0]->attr.token), &opStack, -1);
		//move deeper into the tree
		evaluateExpression(node->children[1], out);
	}else if (node->numChildren == 3)
	{
		//<LPARtk> <EXPR> <RPARtk>
		if (!strcmp(node->children[0]->attr.token, "<LPARtk>"))
		{
			if (DEBUG) {printf("evaluateExpression(%s): BEGIN (.)\n", D(node->attr.token));}
			if (DEBUG) {printf("evaluateExpression(%s): OP_PUSH(%s)\n", D(node->attr.token), D(node->children[0]->attr.token));}
			//add "(" on OPERATOR STACK
			push(createVElement(node->children[0]->attr.token), &opStack, -1);
			//move deeper into the tree
			evaluateExpression(node->children[1], out);
			if (DEBUG) {printf("HERE1!\n");}
			//solve paranthesis: process all operators still on the stack that have been added by evaluateExpression
			reduceParanthesis(out, 1);
		}else{
			if (DEBUG) {printf("evaluateExpression(%s): BEGIN %s %s %s\n", D(node->attr.token), D(node->children[0]->attr.token), D(node->children[1]->attr.token), D(node->children[2]->attr.token));}
			//<F> * <T> || <F> / <T> || <T> + <E> || <T> - <E>
			//move deeper into the left subtree
			evaluateExpression(node->children[0], out);
			//put operator on stack taking account of precedence
			putOperatorOnStack(node->children[1]->attr.token, out);
			//move deeper into the right subtree
			evaluateExpression(node->children[2], out);
			if (DEBUG) {printf("evaluateExpression(%s): END %s %s %s\n", D(node->attr.token), D(node->children[0]->attr.token), D(node->children[1]->attr.token), D(node->children[2]->attr.token));}
		}
	}
}

//creates new label or variable name
char* newName(nameType what)
{
	if (what == VAR)
	{
		sprintf(name, "V%d", varCntr++);    /* generate a new label as V0, V1, etc */
	}
	else
	{
		sprintf(name, "L%d", labelCntr++);            /* new lables as L0, L1, etc */
	}

	return (name);
}

//remove local duplicate variables, reconstruct previous scope "frame";
void popExistingVars(APTNode* node, VStack* scopeStack, FILE* out)
{
	if (node == NULL)
	{
		return;
	}

	if (DEBUG) {printf("popExistingVars(): node: token = %s name = %s value = %s \n", D(node->attr.token), D(node->attr.name), D(node->attr.value));}

	int idIndex;
	int nextIndex;

	//<TYPE> <ID> <MVARS>
	if (!strcmp(node->attr.token, "<VARtk>"))
	{
		idIndex = 1;
		nextIndex = 2;
	}else if (!strcmp(node->attr.token, "<MVARS>"))
	{
		idIndex = 0;
		nextIndex = 1;
	}

	popExistingVars(node->children[nextIndex], scopeStack, out);

	//IF VARIABLE ALREADY EXISTS
	if (findInPreviousScopes(node->children[idIndex]->attr.name, scopeStack) != -1)
	{
		//GENERATE ASM INSTRUCTIONS
		if (COMMENTS) fprintf(out, "//REBUILD VALUE OF %s\n", node->children[idIndex]->attr.name);
		fprintf(out, "\tSTACKR\t0\n");
		fprintf(out, "\tSTORE\t%s\n", node->children[idIndex]->attr.name);
		fprintf(out, "\tPOP\t\n");
	}

	pop(scopeStack);
}

//If a variable that exists in the enclosing scope is redeclared with the same name in the inner scope, its current value is pushed on the stack.
//This emulates a scope "frame" similar to the stack frame used in procedural calls
void pushExistingVars(APTNode* node, VStack* scopeStack, FILE* out)
{
	if (node == NULL)
	{
		return;
	}

	if (DEBUG) {printf("pushExistingVars(): node: token = %s name = %s value = %s \n", D(node->attr.token), D(node->attr.name), D(node->attr.value));}

	int idIndex;
	int nextIndex;

	//<TYPE> <ID> <MVARS>
	if (!strcmp(node->attr.token, "<VARtk>"))
	{
		idIndex = 1;
		nextIndex = 2;
	}else if (!strcmp(node->attr.token, "<MVARS>"))
	{
		idIndex = 0;
		nextIndex = 1;
	}

	if (findInAllScopes(node->children[idIndex]->attr.name, &distinctVarNames) == -1)
	{
		push(createVElement(node->children[idIndex]->attr.name), &distinctVarNames, 0);
	}

	//IF VARIABLE ALREADY EXISTS
	if (findInPreviousScopes(node->children[idIndex]->attr.name, scopeStack) != -1)
	{
		//GENERATE ASM INSTRUCTIONS TO PUSH CURRENT VALUE ON STACK
		if (DEBUG) { printf("EXISTS %s\n", D(node->children[idIndex]->attr.name)); }
		if (COMMENTS) fprintf(out, "//SAVE VALUE OF %s\n", node->children[idIndex]->attr.name);
		fprintf(out, "\tPUSH\t\n");
		fprintf(out, "\tLOAD\t%s\n", node->children[idIndex]->attr.name);
		fprintf(out, "\tSTACKW\t0\n");
	}else{
		if (DEBUG) { printf("NOT EXISTS %s\n", D(node->children[idIndex]->attr.name)); }
	}

	VElement* var = createVElement(node->children[idIndex]->attr.name);
	push(var, scopeStack, 0);

	if (DEBUG) { printf("added to scope stack %s\n", D(node->children[idIndex]->attr.name)); }

	pushExistingVars(node->children[nextIndex], scopeStack, out);
}

//
int isNumberOrID(APTNode* node)
{
	if (node == NULL)
	{
		return 0;
	}

	if (DEBUG) { printf("isNumberOrID(): %s %d", D(node->attr.token), node->numChildren); }

	if (!strcmp(node->attr.token, "<IDtk>"))
	{
		if (DEBUG) {printf(" %s\n", D(node->attr.name));}
		strcpy(globalTemp, node->attr.name);
		return 1;
	}else if (!strcmp(node->attr.token, "<Numbertk>"))
	{
		if (DEBUG) {printf(" %s\n", D(node->attr.value));}
		strcpy(globalTemp, node->attr.value);
		return 1;
	}else
	{
		printf("\n");
	}

	if (node->numChildren != 1)
	{
		return 0;
	}

	return isNumberOrID(node->children[0]);
}

void evaluateConditionResult(APTNode* node, char* label, FILE* out)
{
	if (!strcmp(node->children[1]->attr.token, "<LTtk>"))
	{
		fprintf(out, "\tBRZPOS\t%s\n", label);
	}else if (!strcmp(node->children[1]->attr.token, "<LTEQtk>"))
	{
		fprintf(out, "\tBRPOS\t%s\n", label);
	}else if (!strcmp(node->children[1]->attr.token, "<GTtk>"))
	{
		fprintf(out, "\tBRZNEG\t%s\n", label);
	}else if (!strcmp(node->children[1]->attr.token, "<GTEQtk>"))
	{
		fprintf(out, "\tBRNEG\t%s\n", label);
	}else if (!strcmp(node->children[1]->attr.token, "<EQEQtk>"))
	{
		fprintf(out, "\tBRPOS\t%s\n",label);
		fprintf(out, "\tBRNEG\t%s\n",label);
	}else{ //NOT operator
		fprintf(out, "\tBRZERO\t%s\n",label);
	}
}

void recGen(APTNode* node, FILE* out)
{
	char label[20], label2[20], argR[20], localTemp[20];
	if (node == NULL)
	{
		return;
	}

	if (DEBUG) {printf("recGen(): node: token = %s name = %s value = %s \n", D(node->attr.token), D(node->attr.name), D(node->attr.value));}

	//read <IDtk> from stack
	if (!strcmp(node->attr.token, "<Numbertk>"))
	{
		if (DEBUG) { printf("<Numbertk>\n"); }
		fprintf(out, "\tLOAD\t%s\n", node->attr.value);
	}else if (!strcmp(node->attr.token, "<IDtk>"))
	{
		if (DEBUG) { printf("<IDtk>\n"); }
		if (COMMENTS) fprintf(out, "//ACCESS VALUE OF <IDtk>\n");
		//1. get value of <IDtk>->name from stack and store in ACC
		fprintf(out, "\tLOAD\t%s\n", node->attr.name);
	}else if (!strcmp(node->attr.token, "<INtk>"))
	{
		if (DEBUG) { printf("<INtk>\n"); }
		if (COMMENTS) fprintf(out, "//READ VARIABLE %s\n", node->children[1]->attr.name);
		//2. read to that variable
		fprintf(out, "\tREAD\t%s\n", node->children[1]->attr.name);
	}else if (!strcmp(node->attr.token, "<OUTtk>"))
	{
		if (DEBUG) { printf("<OUTtk>\n"); }
		if (COMMENTS) fprintf(out, "//PRINT STATEMENT\n");
		// 0      1
		//<PRINTFtk> <EXPR>
		//1. EVALUATE RHS OPERATOR AND STORE IN ACC
		recGen(node->children[1], out);
		//2. GENERATE NEW VARIABLE NAME
		strcpy(argR, newName(VAR));
		//3. STORE
		fprintf(out, "\tSTORE\t%s\n", argR);
		//4. WRITE
		fprintf(out, "\tWRITE\t%s\n", argR);
	}else if (!strcmp(node->attr.token, "<ASSIGNtk>"))
	{
		if (DEBUG) { printf("<ASSIGNtk>\n"); }
		if (COMMENTS) fprintf(out, "//ASSIGN STATEMENT\n");
		//LHS EQ RHS
		//1. EVALUATE RHS OPERATOR AND STORE IN ACC
		recGen(node->children[2], out);
		//2. STORE IN LHS VARIABLE
		fprintf(out, "\tSTORE\t%s\n", node->children[0]->attr.name);
	}else if (!strcmp(node->attr.token, "<IFtk>"))
	{
		if (DEBUG) { printf("<IFtk>\n"); }
		if (COMMENTS) fprintf(out, "//IF STATEMENT\n");
		//      0     1     2        3
		//IF [<EXPR> <RO> <EXPR>] <BLOCK>
		//    LHS    RO    RHS     BLOCK
		//1. EVALUATE RHS IN ACC
		recGen(node->children[2], out);

		//2. CREATE NEW VARIABLE NAME
		strcpy(argR, newName(VAR));

		//3. STORE
		fprintf(out, "\tSTORE\t%s\n", argR);

		//4. EVALUATE LHS IN ACC
		recGen(node->children[0], out);

		//5. SUB VARIABLE IN 3
		fprintf(out, "\tSUB\t%s\n", argR);

		//6. GENERATE LABEL:
		strcpy(label, newName(LABEL));

		//7. ANALYZE <RO> AND JUMP TO LABEL
		evaluateConditionResult(node, label, out);

		//8. EXPLORE <BLOCK>
		recGen(node->children[3], out);

		//9. WRITE NOOP STATEMENT WITH LABEL
		fprintf(out, "%s:\tNOOP\n", label);
	}else if (!strcmp(node->attr.token, "<LOOPtk>"))
	{
		if (DEBUG) { printf("<LOOPtk>\n"); }
		if (COMMENTS) fprintf(out, "//LOOP STATEMENT\n");
		//      0     1     2        3
		//LOOP [<EXPR> <RO> <EXPR>] <BLOCK>
		//1. CREATE A LABEL FOR THE LOOP
		strcpy(label, newName(LABEL));
		fprintf(out, "%s:\tNOOP\n", label);

		//2. EVALUATE RHS IN ACC
		recGen(node->children[2], out);

		//3. CREATE NEW VARIABLE NAME
		strcpy(argR, newName(VAR));

		//4. STORE
		fprintf(out, "\tSTORE\t%s\n", argR);

		//5. EVALUATE LHS IN ACC
		recGen(node->children[0], out);

		//6. SUB VARIABLE IN 3
		fprintf(out, "\tSUB\t%s\n", argR);

		//7. GENERATE LABEL FOR THE BLOCK:
		strcpy(label2, newName(LABEL));

		//8. ANALYZE <RO> AND JUMP TO LABEL
		evaluateConditionResult(node, label2, out);

		//9. EXPLORE <BLOCK>
		recGen(node->children[3], out);

		//10. UNCONDITIONAL JUMP TO LOOP LABEL
		fprintf(out, "\tBR\t%s\n", label);

		//11. WRITE NOOP STATEMENT WITH LABEL
		fprintf(out, "%s:\tNOOP\n", label2);
	}
	else if (!strcmp(node->attr.token, "<EXPR>"))
	{
		if (DEBUG) { printf("<EXPR>\n"); }
		if (COMMENTS) fprintf(out, "//EXPR STATEMENT\n");
		if (DEBUG) { printf("recGen(): clearAll(resStack)\n"); }
		clearAll(&resStack);
		if (DEBUG) { printf("recGen(): clearAll(opStack)\n"); }
		clearAll(&opStack);
		if (DEBUG) { printf("recGen(): evaluateExpression()\n"); }

		//move deeper in the expression tree
		evaluateExpression(node, out);
		//reduce any operators which were not processed by traversing the expression tree
		reduceParanthesis(out, 0);

		if (!isStackEmpty(&resStack))
		{
			fprintf(out, "\tLOAD\t%s\n", pop(&resStack)->data);
		}
	}else if (!strcmp(node->attr.token, "<BLOCKtk>"))
	{
		if (DEBUG) { printf("BEGIN <BLOCKtk>\n"); }

		if (node->numChildren == 1)
		{
			recGen(node->children[0], out);
			return;
		}

		//ADD MARKER
		push(MARKER, &SS, 0);

		//SAVE VALUE OF VARIABLES WHICH ARE REDECLARED
		pushExistingVars(node->children[0], &SS, out);

		//MOVE DEEPER INTO THE TREE
		recGen(node->children[1], out);

		//REBUILD PREVIOUS CONTEXT
		popExistingVars(node->children[0], &SS, out);

		//REMOVE MARKER
		VElement* ve = pop(&SS);

		if (DEBUG) { printf("END <BLOCKtk>\n"); }
	}else
	{
		if (DEBUG) { printf("<OTHER>\n"); }
		//TRAVERSE TREE NORMALLY
		int i;

		for(i = 0; i < node->numChildren; i++)
		{
			recGen(node->children[i], out);
		}

		if (!strcmp(node->attr.token, "<MSTAT>") || !strcmp(node->attr.token, "<STATtk>") && (node->numChildren > 0))
		{
			//fprintf(out, "\n");
		}
	}
}

/* Entry point for the generator    */
/*   listed in st subtree                                                    */
void generate(APTNode *root, FILE *out)
{
	int i;

	if (root == NULL)
	{
		printf("error: APT node can not be NULL!");
		exit(0);
	}


	clearAll(&SS);

	//used for processing <MINUStk> <F> expressions
	ZERO = createVElement("0");

	//used for computing expressions
	initStack(&opStack);
	initStack(&resStack);

	//list of all program variable names
	initStack(&distinctVarNames);

	//<VARtk> node not null?
	if (root->numChildren == 2)
	{
		//ADD INITIAL MARKER
		push(MARKER, &SS, 0);
		if (DEBUG) { printf("root has 2 children \n"); }

		//ADD VARIABLES TO LOCAL SCOPE
		pushExistingVars(root->children[0], &SS, out);

		//TRAVERSE TREE
		recGen(root->children[1], out);

		//REBUILD PREVIOUS CONTEXT
		popExistingVars(root->children[0], &SS, out);

		VElement* ve = pop(&SS);
	}else{
		//<VARtk> IS NULL
		if (DEBUG) { printf("root has 1 child \n"); }
		//TRAVERSE TREE
		recGen(root->children[0], out);
	}

	fprintf(out, "\tSTOP\n");

	while(!isStackEmpty(&distinctVarNames))          // allocate storage for program variables
	{
		VElement* v = pop(&distinctVarNames);
		fprintf(out, "%s\t0\n", v->data);
	}

	for (i = 0; i < varCntr; i++)      // allocate space for temporary variables
	{
		fprintf(out, "V%d\t0\n", i);
	}
}

