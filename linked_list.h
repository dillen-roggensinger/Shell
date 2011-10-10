#ifndef LINKED_LIST_H_INCLUDE
#define LINKED_LIST_H_GUARD

struct node {
	char *dir;
	struct node *next;
};

/*Adds the dir to the front of the list*/
void pushn(struct node **head, char *dir);

/*Return and remove the front of the list*/
char *popn(struct node **head);

/*Check for duplicates and add to the end of the list. Return 0
if successful (no duplicates) or -1 otherwise.*/
int addn(struct node **head, char *dir);

/*Find the dir in the list and remove it. Return 0 if successful
(dir found) or -1 otherwise.*/
int removen(struct node **head, char *dir);

/*Clear and free all entries*/
void clear(struct node **head);

#endif /*LINKED_LIST_H_INCLUDED*/
