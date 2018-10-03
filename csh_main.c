#include "preprocessor_array.h"

#define GRUMPY_TAUNTS ("Twinkle, twinkle little star, I want to hit you with a car", "To hate or not to hate.\n\nThat is the... Nope, I hate it.", "I love maths.\n\nIt makes people cry.", "The enemies of my enemies...\n\n ...are my enemies.", "If you're happy and you know it go to hell.")
#define N_GRUMPY_TAUNTS 5
#define CSH_READ_LINE_BUFFERSIZE 1024
#define CSH_TOKEN_BUFFERSIZE 64
#define CSH_TOKEN_DELIMITER " \t\r\n\a"
#define LOGO_PATH "config/grumpycat.txt"
#define GREEN "\x1B[32m"
#define RED "\x1B[31m"
#define BLUE "\x1B[34m"
#define WHITE "\x1B[37m"
#define STD_COLOR GREEN

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 42
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void printNewlines(short n){
	while(n > 0){
		printf("\n");
		n = n - 1;
	}
}

// built-in shell commands
int csh_cd(char **args);
int csh_exit(char **args);
int csh_help(char **args);
int csh_get_grumpy();
void print_grumpy_cat(char *path);
char *builtin_str[] = {
	"cd",
	"exit",
	"help",
	"getgrumpy"
};

int (*builtin_func[])(char **) = {
	&csh_cd,
	&csh_exit,
	&csh_help,
	&csh_get_grumpy
};

int csh_num_builtins(){
	return sizeof(builtin_str) / sizeof(char *);
}

int csh_cd(char **args){
	if(args[1] == NULL){
		fprintf(stderr, "csh: expected argument to command \"cd\"\n");
	}else{
		if(chdir(args[1]) != 0){
			perror("csh");
		}
	}
	
	return 1;
}

int csh_help(char **args){
	printf("This is Clemens' Fancy Linux Shell (CSH).");
	printNewlines(2);
	print_grumpy_cat(LOGO_PATH);
	printNewlines(2);
	printf("You can enter a program name and hit enter.\n");
	printf("Or you can use the built-in commands:\n");
	for(int i = 0; i < csh_num_builtins(); ++i){
		printf("   %s\n", builtin_str[i]);
	}
	printf("\nPlease use \"man\" for info on other programs.\n");
	
	return 1;
}

int csh_exit(char **args){
	return 0;
}

int get_grumpy(char **args){
	// TODO: randomly choose a grumpy taunt :-)
}

void print_grumpy_cat(char *path){
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	
	fp = fopen(path, "r");
	if(fp == NULL){
		exit(EXIT_FAILURE);
	}
	
	while((read = getline(&line, &len, fp)) != -1){
		printf(WHITE "%s", line);
	}
	
	fclose(fp);
	if(line){
		free(line);
	}
	
	printf(STD_COLOR "");
}

// read and parse line
char *csh_read_line(void){
	int bufferSize = CSH_READ_LINE_BUFFERSIZE;
	int pos = 0;
	char *buffer = malloc(sizeof(char) * bufferSize);
	int c; //it's an int because EOF is an int, not a char
	
	if(!buffer){
		fprintf(stderr, "csh: buffer allocation error at line %d in %s, maaan!\n", __LINE__, __func__);
		exit(EXIT_FAILURE);
	}
	
	while(1){
		c = getchar();
		
		if(c == EOF || c == '\n'){
			buffer[pos] = '\0';
			return buffer;
		}else{
			buffer[pos] = c;
		}
		
		pos = pos + 1;
		
		if(pos >= bufferSize){
			bufferSize = bufferSize + CSH_READ_LINE_BUFFERSIZE;
			buffer = realloc(buffer, bufferSize);
			if(!buffer){
				fprintf(stderr, "csh: buffer allocation error at line %d in %s, maaan!\n", __LINE__, __func__);
				exit(EXIT_FAILURE);
			}
		}
	}
}

char **csh_split_line(char *line){
	int bufferSize = CSH_TOKEN_BUFFERSIZE;
	int pos = 0;
	char **tokens = malloc(bufferSize * sizeof(char *));
	char *token;
	
	if(!tokens){
		fprintf(stderr, "csh: buffer allocation error at line %d in %s, maaan!\n", __LINE__, __func__);
		exit(EXIT_FAILURE);
	}
	
	token = strtok(line, CSH_TOKEN_DELIMITER);
	
	while(token != NULL){
		tokens[pos] = token;
		pos = pos + 1;
		
		if(pos >= bufferSize){
			bufferSize = bufferSize + CSH_TOKEN_BUFFERSIZE;
			tokens = realloc(tokens, bufferSize * sizeof(char *));
			if(!tokens){
				fprintf(stderr, "csh: buffer allocation error at line %d in %s, maaan!\n", __LINE__, __func__);
				exit(EXIT_FAILURE);
			}
		}
		
		token = strtok(NULL, CSH_TOKEN_DELIMITER);
	}
	
	tokens[pos] = NULL;
	
	return tokens;
}

int csh_launch_process(char **args){
	pid_t pid;
	pid_t wpid;
	int status;
	
	pid = fork();
	if(pid == 0){
		// Child process
		// only give program name, let OS search for program in path
		if(execvp(args[0], args) == -1){
			perror("csh");
		}
		exit(EXIT_FAILURE); // exec only returns in case of error
		
	}else if(pid < 0){
		perror("csh");
		
	}else {
		// Parent process
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	
	return 1;
}

int csh_execute(char **args){
	if(args[0] == NULL){
		return 1;
	}
	
	for(int i = 0; i < csh_num_builtins(); ++i){
		if(strcmp(args[0], builtin_str[i]) == 0){
			return (*builtin_func[i])(args);
		}
	}
	
	return csh_launch_process(args);
}

void csh_main_loop(void){
	char *line;
	char **args;
	int status;
	printf(GREEN ""); //configure text color
	
	do {
		printf("> ");
		line = csh_read_line();
		args = csh_split_line(line);
		status = csh_execute(args);
		
		free(line);
		free(args);
		
	} while(status);
}