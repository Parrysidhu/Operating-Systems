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
char **tokens, **tokenCommand;
char *line;
pid_t child_pid, pid1;

int status, status1;
int in_bg=0;
int i,j,p, flag1,flag2, flag3, flag4;
int in, out;
int fds[2];


void runsource(int pfds[]);
void rundest(int pfds[]);


void sighandler(int signal){
printf("\nterminating forground process\n");
return;
}

//void handler(){
//printf("terminating background process");
//}

//signal(SIGUSR1, handler);


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

assert( (tokenCommand = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);


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

    if(strcmp(this_token,"kill")==0){
    
    flag4=1;


    continue;
    }

    if (strcmp(this_token,"|")==0){
    flag3=1;
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


int run_command() {

if (strcmp( tokens[0], EXIT_STR ) == 0)
return EXIT_CMD;

return UNKNOWN_CMD;
}

int executeCommand(struct listjobs arr[50]){
    pipe(fds);
    if (pipe(fds) == -1) {
                perror("pipe");
                exit(1);
    }

    /*if(flag3==1){

    


    runsource(fds);
    rundest(fds);

    close(fds[0]);
    close(fds[1]);

    waitpid(pid1,&status1,0);

    flag3=0;
    }*/


    child_pid = fork();
    if(child_pid<0){
      printf("fork failed to execute");
    return 1;
    }
    else if(child_pid==0){

    if(flag1==1){

    int in= open(tokens[2],O_CREAT | O_RDONLY,777);
    if(in<0){
      printf("cannot open file");
    }
    tokens[2]=NULL;
    dup2(in,0);
    flag1=0;


    }

    if(flag2==1){
    int out= open(tokens[2],O_CREAT |O_WRONLY,777);
    if(out<0){
      printf("cannot open file");
    }
      tokens[2]=NULL;
      dup2(out,1);
      flag2=0;
    }

    close(in);
    close(out);

    if(execvp(tokens[0],tokens)<0){
    perror("execvp");
    exit(0);
  }



  }
  else if(child_pid>0) {

  if(in_bg==1){
  printf("\nprocess in background\n");



  for (i=0;i<50;i++){
  waitpid(child_pid,&status,WNOHANG);
  //arr[0].command = tokens[0];


  arr[i].pid = child_pid;
  }

  in_bg=0;

  }

  else
  {
  printf("\nprocess in foreground\n");
  signal(SIGINT,sighandler);

  waitpid(child_pid,&status,0);



  }

  }

  if(flag4==1)  {
    printf("\nkill signal\n");
    kill(atoi(tokens[0]),SIGTERM);
    flag4=0;
  }

  if(in_bg==2){
  //printf("%s with pid %d",tokens[0],child_pid);
    printf("\n command with pid %d \n",arr[0].pid);
    in_bg=0;
  }
}

int tokenizeCommand(char* line){
    int token_count = 0;
    int size = MAX_TOKENS;
    char *this_token;

    // this_token = strtok( line, "|");
    // printf("In tokenize\n");
    // if(this_token != NULL)
    //   printf("jjj %s\n", this_token);

    while ((this_token = strsep( &line, "|")) != NULL) {

      if (*this_token == '\0') continue;

      tokenCommand[token_count] = this_token;

      printf("Token %d: %s\n", token_count, tokenCommand[token_count]);

      token_count++;

      // if there are more tokens than space ,reallocate more space
      if(token_count >= size){
        size*=2;

        assert ( (tokenCommand = realloc(tokenCommand, sizeof(char*) * size)) != NULL);
      }
      //this_token = strtok(NULL, "|");

    }
    return token_count;
}

int read_command()
{

    // getline will reallocate if input exceeds max length
    assert( getline(&line, &MAX_LINE_LEN, fp) > -1);


    printf("Shell read this line: %s\n", line);

    int tokenCount = tokenizeCommand(line);

    return tokenCount;
}


int main()
{
    initialize();

    struct listjobs arr[50];

    do {
    int i = 0;
    printf("\nsh550> ");
    tokenCommand = realloc(tokenCommand, sizeof(char*) * MAX_TOKENS);
    int totalNoOfCommands = read_command();
    tokens = realloc(tokens, sizeof(char*) * MAX_TOKENS);
    for(int i = 0; i < totalNoOfCommands; i++){
        tokenize(tokenCommand[i]);
        executeCommand(arr);
    }
    // free(tokens);
    // if(tokens != NULL){
    //   printf("Token is not NULL ");
    // }
    // else{
    //   printf("Token is NULL")
    // }
} while( run_command() != EXIT_CMD   );
      return 0;
}

//void signal(int tokens[0]){


//}
void runsource(int pfds[]){

pid1=fork();

if(pid1<0){
printf("\nfork failed to execute\n");
}

if(pid1==0){
dup2(pfds[1],1);
close(pfds[0]);
tokens[2]=NULL;
if(execvp(tokens[0],tokens)<0){
perror("exec didnot execute");
exit(0);
}
}


}

void rundest(int pfds[]){

pid1= fork();

if(pid1<0){
printf("\nfork failed to execute\n");
}

if(pid1==0){
dup2(pfds[0],0);
close(pfds[1]);
tokens[0]=NULL;
tokens[1]=NULL;
if(execvp(tokens[0],tokens)<0){
perror("exec didnot execute");
exit(0);
}
}

}
