#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "linked_list.h"

/*Method to allocate a new pointer, copy the directory over and
add a trailing slash if there is none*/
char *copy_str(char *dir)
{
	char *dir_copy;
	int len = strlen(dir) + 1;
	if (dir[len - 2] != '/') {
		/*No slash so copy directory and add slash to end*/
		dir_copy = (char *) malloc(len + 1);
		if (dir_copy == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		strcpy(dir_copy, dir);
		dir_copy[len-1] = '/';
		dir_copy[len] = '\0';
	} else {
		/*Slash already there so just copy*/
		dir_copy = (char *) malloc(len);
		if (dir_copy == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		strcpy(dir_copy, dir);
	}
	return dir_copy;
}

void pushn(struct node **head, char *dir)
{
	char *dir_copy = copy_str(dir);
	struct node *entry = (struct node *) malloc(sizeof(struct node));
	if (entry == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}
	entry->dir = dir_copy;
	entry->next = (*head);
	(*head) = entry;
}

char *popn(struct node **head)
{
	char *dir = (*head)->dir;
	struct node *entry = *head;
	*head = (*head)->next;
	free(entry);
	return dir;
}

int addn(struct node **head, char *dir)
{
	struct node *h_ptr = (*head);
	char *dir_copy = copy_str(dir);
	/*Checks for duplicates*/
	while (h_ptr != NULL) {
		if (strcmp(h_ptr->dir, dir_copy) == 0) {
			free(dir_copy);
			return -1;
		}
		h_ptr = h_ptr->next;
	}
	free(dir_copy);
	pushn(head, dir);
	return 0;
}

int removen(struct node **head, char *dir)
{
	if ((*head) == NULL)
		return -1;
	char *dir_copy = copy_str(dir);
	struct node *h_ptr = (*head);
	/*Edge case where the head is the one to be removed*/
	if (strcmp((*head)->dir, dir_copy) == 0) {
		(*head) = (*head)->next;
		free(h_ptr->dir);
		free(h_ptr);
		free(dir_copy);
		return 0;
	}
	/*Find and remove the entry with dir*/
	while (h_ptr->next != NULL) {
		if (strcmp(h_ptr->next->dir, dir_copy) == 0) {
			struct node *node = h_ptr->next;
			h_ptr->next = h_ptr->next->next;
			free(node->dir);
			free(node);
			free(dir_copy);
			return 0;
		}
		h_ptr = h_ptr->next;
	}
	return -1;
}

void clear(struct node **head)
{
	if ((*head) == NULL)
		return;
	struct node *ptr = (*head);
	while (ptr != NULL) {
		(*head) = (*head)->next;
		free(ptr->dir);
		free(ptr);
		ptr = (*head);
	}
}
