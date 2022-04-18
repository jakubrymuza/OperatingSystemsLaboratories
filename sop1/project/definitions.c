#include "definitions.h"

// zdjecie z kolejki
int popErr(errNode_t** head, char** retSrc)
{
	if (head == NULL) return -1; // nie powinno sie zdarzyc
	
	errNode_t* prev = NULL;
	errNode_t* iter = *head;
	
	while(iter->next != NULL)
	{
		prev = iter;
		iter = iter->next;
	}
	
	*retSrc = iter->errSrc;
	errCode_t code = iter->errCode;
	if (prev == NULL)	
		*head = NULL; 
	else prev->next = NULL;
	
	free(iter);
	return code;
}
// wpisanie do kolejki
void push(errNode_t** head, char* src, errCode_t code)
{
	errNode_t* newNode = malloc(sizeof(errNode_t));
	
	if (newNode == NULL) ERR("malloc");
	
	newNode->errSrc = src;
	newNode->errCode = code;
	newNode->next = *head;
	*head = newNode; 
}
