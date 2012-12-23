// atggg3 - Andrew Goebel - codegen - 12/10/12
#ifndef SEMANTIC_H
#define SEMANTIC_H
#define MAX_STACK_SIZE 200
#define MAX_APT_NODE_SPAN_WIDTH 5

//STACK RELATED DATA TYPES
typedef struct
{
	int type;				//type of element on stack (VAR or MARKER).
	char* data;				//if type is VAR then data holds the name of the variable.
}VElement;

//the stack scope variables and scope markers (i.e. whenever a BEGIN block is found, a MARKER tag is pushed on the stack, marking the beginning of a new local scope)
typedef struct
{
	VElement* vars[MAX_STACK_SIZE];		//stack is represented as a static array of at most 200 elements.
	int line[MAX_STACK_SIZE];		//an array specifying the line where each element on stack was found.
	int topIndex;				//index of the element located at the top of the stack.
}VStack;

//ANNOTATED PARSE TREE RELATED STRUCTURES
//node attributes
typedef struct
{
	char* token;				//the grammar symbol.
	char* name;				//variable name, otherwise NULL
	char* value;				//variable value, otherwise NULL
	int line;
}APTAttributes;

typedef struct APTNodeTag
{
	APTAttributes attr;			//node attributes;
	struct APTNodeTag** children;	//all children nodes with child of index 0 being the child first met
	int numChildren;
}APTNode;


//STACK RELATED FUNCTIONS
VElement* initMarker();				//creates the MARKER element which will be constant throughout the program.
void initStack(VStack* s);
VElement* createVElement(char *data);		//creates a marker element associated to a variable and places the variable name inside.

void clearAll(VStack* s);
int isStackEmpty(VStack* s);
int isStackFull(VStack* s);

void push(VElement* e, VStack* s, int lineNum);
VElement* pop(VStack* s);

int findInLocalScope(char* varName, VStack* s);	//searches for the 1st occurence of varName in the stack from the top until a marker is found (local scope).
int findInAllScopes(char* varName, VStack* s); 	//searches for the 1st occurence of varName in the stack from the top to the bottom of the stack.
int finInPreviousScopes(char* varName, VStack* s);

void displayStack(VStack* s);			//displays the stack (for DEBUG purposes only)

//ANNOTATED PARSE TREE RELATED METHODS
APTNode* createIdAPTNode(char* token, char* name, char* value, int line);
APTNode* createNonIdAPTNode(char* token);
void addChildNode(APTNode* parent, APTNode* child);
void displayPreOrderAPT(APTNode* root, int indent);

char* topElement(VStack* s);
#endif
