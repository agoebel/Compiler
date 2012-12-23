// atggg3 - Andrew Goebel - codegen - 12/10/12
#include "semantic.h"
#include "token.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const int MARKER_TYPE = 0x1;
const int VAR_TYPE = 0x2;

const int DISPLAY_INDENT = 1; //CONTROLS HOW MANY WHITE SPACES ARE USED AS INDENT FOR DISPLAYING THE APT

VElement* MARKER;	//marker for block start
VStack SS;

extern char* filename;
extern tok currentTok;

int isStackEmpty(VStack* s)
{
	if (s->topIndex == -1)
	{
		return 1;
	}

	return 0;
}

int isStackFull(VStack* s)
{
	if (s->topIndex == MAX_STACK_SIZE)
	{
		return 1;
	}

	return 0;
}

void initStack(VStack* s)
{
	int i;

	for(i = 0; i < MAX_STACK_SIZE; i++)
	{
		s->vars[i] = NULL;
	}

	s->topIndex = -1;
}

VElement* initMarker()
{
	VElement* marker = (VElement*)malloc(1 * sizeof(VElement));
	marker->type = MARKER_TYPE;
	marker->data = NULL;

	return marker;
}

void push(VElement* element, VStack* s, int lineNum)
{
	if (isStackFull(s))
	{
		printf("%s:%d: syntax error: stack overflow error processing '%s'\n", filename, currentTok.line, currentTok.selection);
		exit(0);
	}

	s->topIndex++;
	s->vars[s->topIndex] = element;
	s->line[s->topIndex] = lineNum;
/*	if (element == MARKER)
	{
		printf("push(MARKER) : topIndex = %d\n", s->topIndex);
	}else{
		printf("push(%s) : topIndex = %d\n", element->data, s->topIndex);
	}*/
}

VElement* pop(VStack* s)
{
	if (isStackEmpty(s))
	{
		printf("%s:%d: syntax error: empty stack error \n", filename, currentTok.line);
		exit(0);
	}

	return s->vars[s->topIndex--];
}

void clearAll(VStack* s)
{
	if (isStackEmpty(s))
	{
		return;
	}

	int i;
	for(i = 0; i <= MAX_STACK_SIZE; i++)
	{
		s->vars[i] = NULL;
	}

	s->topIndex = -1;
}

void popCurrentScopeVars(VStack* s)
{
	VElement* ptr = pop(s);

	while(ptr->type != MARKER_TYPE && !isStackEmpty(s))
	{
		//printf("pop(%s)\n", ptr->data);
		//pop next element
		ptr = pop(s);
	}
}

VElement* createVElement(char* data)
{
	VElement* result = (VElement *)malloc(1 * sizeof(VElement));

	result->data = (char *)malloc(strlen(data)  + 1);
	strcpy(result->data, data);
	result->type = VAR_TYPE;

	return result;
}

void displayStack(VStack* s)
{
	printf("Stack is:\n");
	printf("s->top = %d\n", s->topIndex);
	int i = 0;
	for(i = 0; i <= s->topIndex; i++)
	{
		if (s->vars[i]->type == MARKER_TYPE)
		{
			printf("MARKER : %d\n", s->line[i]);
		}else{
			printf("%s : %d\n", s->vars[i]->data, s->line[i]);
		}
	}
}

APTNode* createIdAPTNode(char* token, char* name, char* value, int line)
{
	APTNode* res = (APTNode *)malloc(1 * sizeof(APTNode));
	res->children = (APTNode **)malloc(MAX_APT_NODE_SPAN_WIDTH * sizeof(APTNode *));

	int i;
	for(i = 0; i < MAX_APT_NODE_SPAN_WIDTH; i++)
	{
		res->children[i] = NULL;
	}

	res->attr.token = (char *)malloc(strlen(token) + 1);
	strcpy(res->attr.token, token);

	if (name != NULL)
	{
		res->attr.name = (char *)malloc(strlen(name) + 1);
		strcpy(res->attr.name, name);
	}else{
		res->attr.name = NULL;
	}

	if (value != NULL)
	{
		res->attr.value = (char *)malloc(strlen(value)  + 1);
		strcpy(res->attr.value, value);
	}else{
		res->attr.value = NULL;
	}

	res->numChildren = 0;
	res->attr.line = line;

	return res;
}

APTNode* createNonIdAPTNode(char* token)
{
	APTNode* res = (APTNode *)malloc(1 * sizeof(APTNode));
	res->children = (APTNode **)malloc(MAX_APT_NODE_SPAN_WIDTH * sizeof(APTNode *));

	int i;
	for(i = 0; i < MAX_APT_NODE_SPAN_WIDTH; i++)
	{
		res->children[i] = NULL;
	}

	res->attr.token = (char *)malloc(strlen(token) + 1);
	strcpy(res->attr.token, token);

	res->attr.name = NULL;
	res->attr.value = NULL;

	res->numChildren = 0;

	return res;
}

void addChildNode(APTNode* parent, APTNode* child)
{
	if (parent->numChildren >= MAX_APT_NODE_SPAN_WIDTH)
	{
		printf("%s:%d: parse error: invalid production rule\n", filename, currentTok.line);
		exit(0);
	}

	if (child == NULL)
	{
		return;
	}

	parent->children[parent->numChildren++] = child;
}

int findInLocalScope(char* varName, VStack* s)
{
	//printf("findInLocalScope: %s\n", varName);
	if (isStackEmpty(s))
	{
		return -1;
	}

	int i = s->topIndex;
	while ((s->vars[i] != MARKER) && (i > -1))
	{
		//printf("Examining %s\n", s->vars[i]->data);
		if (!strcmp(s->vars[i]->data, varName))
		{
			return s->line[i];
		}

		i--;
	}

	return -1;
}

int findInPreviousScopes(char* varName, VStack* s)
{
	if (isStackEmpty(s))
	{
		return -1;
	}

	int i;
	int markerPos = 0;
	for(i = s->topIndex; i >= 0; i--)
	{
		if (s->vars[i] == MARKER)
		{
			markerPos = i;
			break;
		}
	}

	if (markerPos == 0)
	{
		return -1;
	}

	for(i = markerPos - 1; i >= 0; i--)
	{
		if (s->vars[i] == MARKER)
		{
			continue;
		}

		if (!strcmp(s->vars[i]->data, varName))
		{
			return 1;
		}
	}

	return -1;
}

int findInAllScopes(char* varName, VStack* s)
{
	//printf("findInAllScopes: %s\n", varName);
	if (isStackEmpty(s))
	{
		return -1;
	}

	int i;
	for(i = s->topIndex; i >= 0; i--)
	{
		if (s->vars[i] == MARKER)
		{
			continue;
		}

		//printf("Examining %s\n", s->vars[i]->data);
		if (!strcmp(s->vars[i]->data, varName))
		{
			return s->line[i];
		}
	}

	return -1;
}

//NODE CHECK for VAR DECLARATION
void checkRedeclared(APTNode* var)
{
	//printf("checkRedeclarationSyntax()\n");
	int prevLineNo = findInLocalScope(var->attr.name, &SS);
	if (prevLineNo > 0)
	{
		printf("%s:%d: semantic error: redeclaration of '%s'; previous declaration on line %d ", filename, var->attr.line, var->attr.name, prevLineNo);
		//displayStack(&SS);
		exit(0);
	}
}

//NODE CHECK for VAR USE
void checkNotDeclared(APTNode* var)
{
	int prevLineNo = findInAllScopes(var->attr.name, &SS);
	if (prevLineNo < 0)
	{
		printf("%s:%d: semantic error: '%s' is undeclared (first use in this function)", filename, var->attr.line, var->attr.name);
		//displayStack(&SS);
		exit(0);
	}
}

//traverse SUBTREE in PREORDER for VAR DECLARATION
void checkVariableDeclarationsPreOrder(APTNode* root)
{
	//printf("VD(): %s\n", root->attr.token);

	if (!strcmp(root->attr.token, "<IDtk>"))
	{
		checkRedeclared(root);
		//printf("push(%s)\n", root->attr.name);
		push(createVElement(root->attr.name), &SS, root->attr.line);
		return;
	}

	int i;
	for(i = 0; i < root->numChildren; i++)
	{
		checkVariableDeclarationsPreOrder(root->children[i]);
	}
}

//traverse SUBTREE in PREORDER and check for VAR USE
void checkVariableUsePreOrder(APTNode* root)
{
	//printf("VU(): %s\n", root->attr.token);
	if (!strcmp(root->attr.token, "<IDtk>"))
	{
		checkNotDeclared(root);
		return;
	}

	int i;
	for(i = 0; i < root->numChildren; i++)
	{
		checkVariableUsePreOrder(root->children[i]);
	}
}

//main PREORDER tree traversal procedure:
//1. if the node is a VAR node, variable declarations are checked in the associated subtree
//2. if the node is a node which uses a variable, variable usage is checked in the associated subtree
//3. otherwise just advance in the tree
//the procedure handles BLOCK nodes as well by adding a stack marker when a block node is first met and removing it after it has been processed
void preOrderSemanticAnalysis(APTNode* root)
{
	int i;
	//printf("SA(): %s\n", root->attr.token);

	//ADD MARKER FOR LOCAL SCOPE
	if (!strcmp(root->attr.token, "<BLOCKtk>"))
	{
		//printf("push(MARKER)\n");
		//push local scope marker before traversing this node
		push(MARKER, &SS, -1);
	}



	for(i = 0; i < root->numChildren; i++)
	{
		if (!strcmp(root->attr.token, "<VARtk>"))
		{
			//CASE 1
			checkVariableDeclarationsPreOrder(root->children[i]);
		}else if (!strcmp(root->attr.token, "<INtk>") ||
			  !strcmp(root->attr.token, "<ASSIGNtk>") ||
			  !strcmp(root->attr.token, "<EXPRtk>") ||
			  !strcmp(root->attr.token, "<OUTtk>"))
		{
			//CASE 2
			checkVariableUsePreOrder(root->children[i]);
		}else{
			//CASE 3
			preOrderSemanticAnalysis(root->children[i]);
		}
	}

	//POP VARS IN LOCAL SCOPE AND ASSOCIATED VARS
	if (!strcmp(root->attr.token, "<BLOCKtk>"))
	{
		//remove all variables in current scope
		popCurrentScopeVars(&SS);
	}
}

void performSemanticAnalysis(APTNode* root)
{
	//push global scope marker on stack
	push(MARKER, &SS, -1);

	preOrderSemanticAnalysis(root);
}

void displayPreOrderAPT(APTNode* root, int indent)
{
	if (root == NULL)
	{
		return;
	}

	APTAttributes a = root->attr;
	//printf("indent = %d\n", indent);
	printf("%*s", indent, "");

	if (!strcmp(a.token, "<IDtk>") || !strcmp(a.token, "<Numbertk>"))
	{
		printf("%s :", a.token);
		if (a.name != NULL)
		{
			printf(" name = %s,", a.name);
		}else{
			printf(" name = (null),");
		}

		if (a.value != NULL)
		{
			printf(" value = %s,", a.value);
		}else{
			printf(" value = (null),");
		}
		printf("\n");
	}
	else{
		printf("%s\n", a.token);
	}

	int i = 0;
	for(i = 0; i < root->numChildren; i++)
	{
		displayPreOrderAPT(root->children[i], indent + DISPLAY_INDENT);
	}
}

char* topElement(VStack* s)
{
	if (isStackEmpty(s))
	{
		return NULL;
	}

	return (s->vars[s->topIndex]->data);
}

int findFirstStackOffset(char* name, VStack* s)
{
	printf("findFirstStackOffset(%s), stackTop = %d\n", name, s->topIndex);
	int i, sum;
	sum = 0;
	for (i = s->topIndex; i >= 0; i--)
	{
		printf("i = %d\n", i);
		//ignore marker elements
		if (s->vars[i] == MARKER)
		{

			printf("EXAMINIG MARKER\n");
			continue;
		}

		printf("EXAMINING %s\n", s->vars[i]->data);

		//is the current element the one we are lookin for?
		if (!strcmp(s->vars[i]->data, name))
		{
			return sum;
		}

		//return offset
		sum++;
	}

	//this line should not be reached
	printf("Unexepected error occured!\n");
	exit(0);

	return 2000;
}

