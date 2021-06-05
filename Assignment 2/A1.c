#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

//limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100

size_t MAX_LINE_LEN = 10000;


// builtin commands
#define EXIT_STR "exit"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99


FILE *fp; // file struct for stdin
char **tokens;
char *line;
pid_t child_pid;

int status;
int in_bg=0;
int i, flag1,flag2;
int in, out;

struct listjobs
{
	char command;
	int pid;
};



void initialize()
{

	// allocate space for the whole line
	assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// allocate space for individual tokens
	assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);

	// open stdin as a file pointer 
	assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);

}

void tokenize (char * string)
{
	int token_count = 0;
	int size = MAX_TOKENS;
	char *this_token;

	while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL) {

		if (*this_token == '\0') continue;
		if(strcmp(this_token,"&")==0){                                                                   
            		in_bg=1;
			continue;
		}
		
		if (strcmp(this_token,"listjobs")==0){
			in_bg=2;
			continue;
		}
		
		if (strcmp(this_token,"<")==0){
			flag1=1;
			
			continue;
		}
		
		if (strcmp(this_token,">")==0){
			flag2=1;
			
			continue;
		}
		
		
		
 
		tokens[token_count] = this_token;

		printf("Token %d: %s\n", token_count, tokens[token_count]);

		token_count++;

		// if there are more tokens than space ,reallocate more space
		if(token_count >= size){
			size*=2;

			assert ( (tokens = realloc(tokens, sizeof(char*) * size)) != NULL);

		}
	}
}

void read_command() 
{
	
	// getline will reallocate if input exceeds max length
	assert( getline(&line, &MAX_LINE_LEN, fp) > -1); 
	

	printf("Shell read this line: %s\n", line);
	
	

	tokenize(line);
}

int run_command() {

	if (strcmp( tokens[0], EXIT_STR ) == 0)
		return EXIT_CMD;

	return UNKNOWN_CMD;
}

int main()
{
	initialize();
	
	struct listjobs arr[50];
	

	do {
		printf("sh550> ");
		read_command();
		

		
		
		child_pid= fork();
		if(child_pid<0){
			printf("fork failed to execute");
			return 1;
			}
		else if(child_pid==0){
			
			
			
			/*if(flag1==1){
				FILE* inputFile = fopen(tokens[1], "r");
                    while (fgets(line, sizeof(line), inputFile) != NULL)
                    line[strlen(line)] = '\0';
                    fclose(inputFile);
			}*/
			if(flag1==1){
				
				int in= open(tokens[2],O_CREAT | O_RDONLY,777);
				tokens[2]=NULL;
				dup2(in,0);
				flag1=0;
				
				
			}
			
			if(flag2==1){
				int out= open(tokens[2],O_CREAT |O_WRONLY,777);
				tokens[2]=NULL;
				dup2(out,1);
				flag2=0;
			}
			
			close(in);
			close(out);
		
			execvp(tokens[0],tokens);
			perror("execvp");
			
			
			}
		else if(child_pid>0) {
			
				if(in_bg==1){
				printf("\nprocess in background\n");
				
				
				
				for (i=0;i<50;i++){
					waitpid(child_pid,&status,WNOHANG);
					//arr[0].command = tokens[0];
					arr[i].pid= child_pid;
					}
				
				in_bg=0;	
				
					}
				
				else
				{
				
				waitpid(child_pid,&status,0);	
			
				printf("process in foreground\n");
				}
			
			}
		
		if(in_bg==2){
				//printf("%s with pid %d",tokens[0],child_pid);
				printf("\n command with pid %d \n",arr[0].pid);
					}	
		
		
		
		
			
				
	} while( run_command() != EXIT_CMD );

	

      	return 0;
}	






	
