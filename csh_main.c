#define CSH_READ_LINE_BUFFERSIZE 1024
#define CSH_TOKEN_BUFFERSIZE 64
#define CSH_TOKEN_DELIMITER " \t\r\n\a"
#define GRUMPY_CAT_PATH "config/grumpycat.txt"
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
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>

/*
char getch(){
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
	
    if(tcgetattr(0, &old) < 0){
		perror("tcsetattr()");
	}
	
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
	
    if(tcsetattr(0, TCSANOW, &old) < 0){
		perror("tcsetattr ICANON");
	}
	
    if(read(0,&buf,1) < 0){
		perror("read()");
	}
	
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0){
		perror ("tcsetattr ~ICANON");
	}
	
    printf("%c",buf);
    return buf;
}
*/

const char *grumpy_taunts[] = {
	"Twinkle, twinkle little star, I want to hit you with a car", 
	"To hate or not to hate.\n\nThat is the... Nope, I hate it.", 
	"I love maths.\n\nIt makes people cry.", 
	"The enemies of my enemies...\n\n ...are my enemies.", 
	"If you're happy and you know it go to hell."
};

char *lastcommand = NULL;

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
int wat();
void print_grumpy_cat(char *path);
char *builtin_str[] = {
	"cd",
	"exit",
	"help",
	"getgrumpy",
	"wat"
};

int (*builtin_func[])(char **) = {
	&csh_cd,
	&csh_exit,
	&csh_help,
	&csh_get_grumpy,
	&wat
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
	print_grumpy_cat(GRUMPY_CAT_PATH);
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

int csh_get_grumpy(char **args){
	printNewlines(2);
	print_grumpy_cat(GRUMPY_CAT_PATH);
	printNewlines(2);
	unsigned short s = rand() % sizeof(grumpy_taunts) / sizeof(grumpy_taunts[0]);
	printf("%s", grumpy_taunts[s]);
	printNewlines(2);
	
	return 1;
}

int wat(){
	system("firefox https://www.destroyallsoftware.com/talks/wat");
	return 1;
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
char *auto_completion_generator(const char *text, int state){
    static int list_index;
	static int len;
    char *name;

    if(!state){
        list_index = 0;
        len = strlen(text);
    }

    while ((name = builtin_str[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

char **auto_completion(const char *text, int start, int end){
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, auto_completion_generator);
}

char *csh_read_line(void){
	int bufferSize;
	if(lastcommand == NULL){
		bufferSize = CSH_READ_LINE_BUFFERSIZE;
	}else{
		bufferSize = strlen(lastcommand) * sizeof(char);
	}
	int pos = 0;
	char *buffer = malloc(sizeof(char) * bufferSize);
	lastcommand = malloc(sizeof(char) * bufferSize);
	int c; //it's an int because EOF is an int, not a char
	
	if(!buffer){
		fprintf(stderr, "csh: buffer allocation error at line %d in %s, maaan!\n", __LINE__, __func__);
		exit(EXIT_FAILURE);
	}
	
	while(1){
		// get characters
		c = getchar();
		
		/* TODO: check for arrow up key to get last command
		printf("Code: %i, Char: %c\n", c, (char) c);
		size_t length = strlen(lastcommand);
		if(length * sizeof(char) > bufferSize){
			bufferSize = length * sizeof(char) + CSH_READ_LINE_BUFFERSIZE;
			buffer = realloc(buffer, bufferSize);
		}
		 
		memset(buffer, 0, length * sizeof(char));
		strcpy(buffer, lastcommand);
		*/
		
		if(c == EOF || c == '\n'){
			buffer[pos] = '\0';
			strcpy(lastcommand, buffer);
			return buffer;
			
		}else{
			buffer[pos] = c;
		}
		
		pos = pos + 1;
		
		if(pos >= bufferSize){
			bufferSize = bufferSize + CSH_READ_LINE_BUFFERSIZE;
			buffer = realloc(buffer, bufferSize);
			lastcommand = realloc(lastcommand, bufferSize);
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
	//rl_attempted_completion_function = auto_completion;
	//rl_bind_key('\t', rl_complete);
	char *username = getenv("USER");
	
	printf(GREEN ""); //configure text color
	
	do {
		printf("%s@ClemensLinuxKiste> ", username);
		line = csh_read_line();
		add_history(line);
		args = csh_split_line(line);
		status = csh_execute(args);
		
		free(line);
		free(args);
		
	} while(status);
}