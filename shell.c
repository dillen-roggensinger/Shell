#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include "linked_list.h"

#define MAX_ARGS 100

struct node *stack;
struct node *p_list;

/*Reads stdin character by character copying them into a char*.
If the number of chars surpasses the size of the char*, it is
realloced to double the size. Returns the whole line with the
char* null terminated.*/
char *getline()
{
	int max = 10;
	int len = 0;
	int c;
	char *command;
	command = (char *) malloc(max + 1);
	if (command == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}
	while ((c = fgetc(stdin)) != '\n') {
		if (len == max) {
			max *= 2;
			command = realloc(command, max + 1);
		}
		if (command == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		command[len] = c;
		len++;
	}
	command[len] = '\0';
	return command;
}

/*Changes directories and prints any errors that may
occur in the process*/
int cd(char *dir)
{
	if (dir == NULL || strlen(dir) == 0) {
		fprintf(stderr, "Error: Argument list too short\n");
		return 0;
	}
	if (chdir(dir) != 0) {
		perror("Error");
		return 0;
	}
	return 1;
}

/*Adds the current directory to a stack and the uses
the cd(char* dir) method to change directories to the
directory given. Handles any errors that may occur,
returning 1 if successful.*/
int pushd(char *dir)
{
	char cwd[256];
	if (dir == NULL || strlen(dir) == 0) {
		fprintf(stderr, "Error: Argument list too short\n");
		return 0;
	}
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("Error");
		return -1;
	}
	if (cd(dir)) {
		pushn(&stack, cwd);
		return 1;
	}
	return 0;
}

/*Pops the head off the stack and uses the cd(char* dir)
method to change directories to the directory popped off.
Handles any errors that may occur, returning 1 if successful.*/
int popd()
{
	if (stack != NULL) {
		char *dir = popn(&stack);
		int val = cd(dir);
		free(dir);
		return val;
	}
	fprintf(stderr, "Error: Directory stack empty\n");
	return 0;
}

/*Prints the contents of the stack*/
void dirs()
{
	char cwd[1024];
	struct node *s_ptr = stack;
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		perror("Error");
	else
		printf("\t%s/\n", cwd);
	while (s_ptr != NULL) {
		printf("\t%s\n", s_ptr->dir);
		s_ptr = s_ptr->next;
	}
}

/*Check if a given string is a directory. Returns 1 if a directory,
0 if a file, -1 if it's nothing.*/
int is_dir(char *dir)
{
	struct stat buf;
	if (lstat(dir, &buf) == -1)
		return 0;
	if (S_ISDIR(buf.st_mode))
		return 1;
	return 0;
}

/*Prints the contents of the path list*/
void path()
{
	struct node *p_ptr = p_list;
	if (p_list == NULL)
		printf("\n");
	if (p_ptr != NULL) {
		printf("%s" , p_ptr->dir);
		while ((p_ptr = p_ptr->next) != NULL)
			printf(":%s", p_ptr->dir);
		printf("\n");
	}
}

/*Adds a directory to the path list. Handles any errors
and returns 1 if successful.*/
int p_path(char *dir)
{
	if (dir == NULL) {
		fprintf(stderr, "Error: Argument list too short\n");
		return 0;
	}
	if (!is_dir(dir)) {
		fprintf(stderr, "Error: No such file or directory\n");
		return 0;
	}
	if (addn(&p_list, dir) < 0) {
		fprintf(stderr, "Error: Path already exists\n");
		return 0;
	}
	return 1;
}

/*Removes a directory from the path list. Handles any errors
and returns 1 if successful.*/
int s_path(char *dir)
{
	if (dir == NULL || strlen(dir) == 0) {
		fprintf(stderr, "Error: Argument list too short\n");
		return 0;
	}
	if (removen(&p_list, dir) < 0) {
		fprintf(stderr, "Error: Path does not exist\n");
		return 0;
	}
	return 1;
}

/*Check all files in a directory for one matching the given command.
Return 1 if successfully found, 0 otherwise.*/
int check_dir(char *command, char *dir)
{
	int val = 0;
	DIR *pDIR;
	struct dirent *entry;
	pDIR = opendir(dir);
	if (pDIR == NULL)
		return val;
	entry = readdir(pDIR);
	while (entry != NULL) {
		if (strcmp(command, entry->d_name) == 0) {
			val = 1;
			break;
		}
		entry = readdir(pDIR);
	}
	if (closedir(pDIR) == -1)
		fprintf(stderr, "Error: Unable to close directory stream\n");
	return val;
}

/*Iterate through the p_list looking through each directory for a
file that matches the input command. Return the path of the directory
or NULL if none is found with the command.*/
char *get_path(char *command)
{
	if (p_list == NULL) {
		fprintf(stderr, "Error: No paths specified\n");
		return NULL;
	}
	struct node *ptr = p_list;
	while (ptr != NULL) {
		if (check_dir(command, ptr->dir))
			return ptr->dir;
		ptr = ptr->next;
	}
	fprintf(stderr, "Error: Command not found\n");
	return NULL;
}

/**Execute a command not implemented by the shell itself.*/
int exec_cmd(char *args[])
{
	int pid;
	char *dir = get_path(args[0]);
	if (dir == NULL)
		return 0;
	char *path = (char *) malloc(strlen(args[0]) + strlen(dir) + 1);
	if (path == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}
	strcpy(path, dir);
	strcat(path, args[0]);
	args[0] = path;
	pid = fork();
	if (pid == 0) {
		execv(args[0], args);
		perror("Error");
		return 0;
	}
	while (pid != wait(0))
		;
	return 1;
}

/*Determine if there are any more arguments continuing
with strtok. Print and error if there are and return 0.
Returns 1 otherwise.*/
int no_more_args()
{
	if (strtok(NULL, " ") != NULL) {
		fprintf(stderr, "Error: Argument list too long\n");
		return 0;
	}
	return 1;
}

/*Parse the string from stdin and determine what command
to execute.*/
void parse_args(char *params)
{
	char *command = strtok(params, " ");
	if (strcmp(command, "cd") == 0) {
		char *dir = strtok(NULL, " ");
		if (no_more_args())
			cd(dir);
	} else if (strcmp(command, "pushd") == 0) {
		char *dir = strtok(NULL, " ");
		if (dir == NULL)
			fprintf(stderr, "Error: No directory specified\n");
		else if (no_more_args())
			pushd(dir);
	} else if (strcmp(command, "popd") == 0) {
		if (no_more_args())
			popd();
	} else if (strcmp(command, "dirs") == 0) {
		if (no_more_args())
			dirs();
	} else if (strcmp(command, "path") == 0) {
		char *action = strtok(NULL, " ");
		char *dir = strtok(NULL, " ");
		if (action == NULL)
			path();
		else if (strcmp(action, "+") != 0 && strcmp(action, "-") != 0)
			fprintf(stderr, "Error: Invalid path command\n");
		else if (dir == NULL)
			fprintf(stderr, "Error: No directory specified\n");
		else if (strcmp(action, "+") == 0 && no_more_args())
			p_path(dir);
		else if (strcmp(action, "-") == 0 && no_more_args())
			s_path(dir);
		else
			fprintf(stderr, "Error: Argument list too long\n");
	} else {
		int i = 1;
		char *args[MAX_ARGS + 1];
		args[0] = command;
		while (i < MAX_ARGS + 1 && args[i-1] != NULL)
			args[i++] = strtok(NULL, " ");
		if (no_more_args())
			exec_cmd(args);
	}
}

int main(int argc, char *argv[])
{
	char cwd[1024];
	char *params;
	stack = NULL;
	p_list = NULL;
	while (1) {
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			perror("Error");
			printf("$: ");
		} else {
			printf("%s$: ", cwd);
		}
		params = getline();
		if (strcmp(params, "exit") == 0) {
			free(params);
			clear(&stack);
			clear(&p_list);
			return EXIT_SUCCESS;
		}
		parse_args(params);
		free(params);
	}
	return EXIT_FAILURE;
}
