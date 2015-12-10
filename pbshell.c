#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ncurses.h> 

/******************************* PBSH *********************************/
/*                                                                    */
/* Date:        September 8, 2015                                     */
/* Author:      Palash Bansal                                         */
/*              PalashBansal96@gmail.com                              */
/*              2014072                                               */
/* Purpose:	    Implementation of a simple shell for OS course        */
/*                                                                    */
/**********************************************************************/

// enum boolean{False=0, True=1};
// typedef char bool;

#define C_RED     "\x1b[31m"
#define C_GREEN   "\x1b[32m"
#define C_YELLOW  "\x1b[33m"
#define C_BLUE    "\x1b[34m"
#define C_MAGENTA "\x1b[35m"
#define C_CYAN    "\x1b[36m"
#define C_RESET   "\x1b[0m"

typedef struct command{
	int id;
	int timestamp;
	char* line;
	char** args;
	// int nargs; //no of args 
	char* infile;
	char* outfile;
	char* freeable;
	// struct command* next;
} command;


void sigIntHandler(); //Handle CTRL-C here
char* getInput();
command parseLine(char* line);
void printPrompt();
bool execIfBuiltIn(command cmd);
void freeNode(command cmd);
void execCommand(command cmd);

bool running = false;
int lastid = 0;
char *pwd;

FILE* history;

int main(int argc, char const *argv[]){
	FILE* fp;
	signal(SIGINT, sigIntHandler); // Handling CTRL-C
	int childPID;
	pwd = (char*)malloc(1024);
	lastid = 0;

	if(argc > 1){
		printf("No parameters are supported as of now, Sorry :(\n");
		exit(1);
	}

	printf(C_YELLOW "Welcome to PbSh: A (very) basic shell by Palash Bansal (2014072)\n" C_RESET);
	printf(C_BLUE "Type help for the list of possible commands\n" C_RESET);
	printf(C_RED "Type exit or send CTRL_Z to exit\n" C_RESET);

	running = true;
	while(running){
		printPrompt();
		char *line = getInput();
		command cmd = parseLine(line);
		if(*cmd.args==NULL) continue;
		history=fopen("history.txt", "a");
		fprintf(history, "%s\n", cmd.line);
		fclose(history);
		if(!(execIfBuiltIn(cmd))){
			childPID = fork();
			if(childPID==0){ //child process
				execCommand(cmd);
				perror("Could not execute command");
				exit(1);
			}else if(childPID<0){ //some error occurred
				perror("fork");
			}else{ //parent process
					wait(NULL);
			}
		}
		freeNode(cmd);
	}
	return 0;
}

void sigIntHandler(int param){}

char* getInput(){
	char* line = (char*) malloc(1024);
	fgets(line, 1024, stdin);
	line[strlen(line)-1]=0;
	return line;
}

command parseLine(char* line){
	char** args = (char**)malloc(sizeof(char*)*100);
	char* buffer = (char*) malloc(1024);
	char* infile = NULL, *outfile = NULL;
	strcpy(buffer, line);
	char *pch;
	pch = strtok(buffer, " ");
	bool quoteflag = false;
	int c=0;
	while(pch!=NULL){
		if(*pch==0) continue;
		if(quoteflag){
			strcat(args[c-1], " ");
			strcat(args[c-1], pch);
		}else{
			if(*pch=='>'){ // assuming of form cmd < file not cmd <file 
				pch = strtok(NULL, " ");
				if(pch == NULL) continue; // error handle later
				outfile = pch;
			}
			else if(*pch=='<'){
				pch = strtok(NULL, " ");
				if(pch == NULL) continue; // error handle later
				infile = pch;
			}
			else 
				args[c++] = pch;
		}
		if(*pch=='\"') quoteflag = true;
		if(pch[strlen(pch)-1]=='\"'&&pch[strlen(pch)-1]!='\\') quoteflag = false;   //takes care in case like echo "something \"bla bla\" else"
		pch = strtok(NULL, " ");
	}
	args[c] = (char*)NULL;
	command cmd;
	cmd.line = line;
	cmd.args = args;
	cmd.freeable = buffer;
	cmd.infile = infile;
	cmd.outfile = outfile;
	cmd.timestamp = 1;
	cmd.id = ++lastid;
	// for(c=0;args[c];c++)printf("%s\n", args[c]);		//for debugging
	return cmd;
}

void printPrompt(){ // To be extended, like this, [palash@palash-PC] - [\home\palash\Desktop] - [Thu Sep 10, 05:42] \n [$] >> 
	getcwd(pwd, 1024);
	fflush(0);
	printf(C_GREEN "%s\n" C_RESET, pwd);
	char host[1024]; 
	gethostname(host, 1024);
	printf(C_BLUE "%s@%s" C_RESET C_CYAN " $:> " C_RESET, getenv("USER"), host);
}

void freeNode(command cmd){
	free(cmd.line);
	free(cmd.args);
	free(cmd.freeable);
}

void execCommand(command cmd){
	if (cmd.infile){
		int infile = open(cmd.infile, O_RDONLY, 0);
		dup2(infile, STDIN_FILENO);
		close(infile);
	}
	if (cmd.outfile){
		int outfile = creat(cmd.outfile, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		dup2(outfile, STDOUT_FILENO);
		close(outfile);
	}
	execvp(cmd.args[0], cmd.args);
}

bool execIfBuiltIn(command cmd){
	if(!strcmp(cmd.args[0],"cd")){
		if(chdir(cmd.args[1])){
			perror("cd");
		}
		return true;
	}else if(!strcmp(cmd.args[0],"exit")){
		exit(0);
	}else if(!strcmp(cmd.args[0],"history")){
		FILE* outstream;
		if(cmd.outfile)
			outstream = fopen(cmd.outfile, "w");
		else
			outstream = stdout;  
		history=fopen("history.txt", "r");
		char buff[1024];
		int i=0;
		while(fgets(buff, 1024, history)){
			printf("%d\t%s", i++, buff);
		}
		return true;
	}else if(!strcmp(cmd.args[0],"hello")){
		printf(C_YELLOW "Welcome to PbSh: A (very) basic shell by Palash Bansal (2014072)\n" C_RESET);
		printf(C_BLUE "Type help for the list of possible commands\n" C_RESET);
		printf(C_RED "Type exit or send CTRL_Z to exit\n" C_RESET);
		return true;
	}else if(!strcmp(cmd.args[0],"help")){
		printf(C_YELLOW "Welcome to PbSh: A (very) basic shell by Palash Bansal (2014072)\n" C_RESET);
		printf(C_RED "Type exit or send CTRL_Z to exit\n" C_RESET);
		printf("\n");
		printf(C_MAGENTA "These shell commands are defined internally.  Type `help` to see this list.\n" C_RESET);
		printf("\n");
		printf(C_BLUE "cd:" C_RESET C_CYAN" to change the current working directory.\n" C_RESET);
		printf(C_BLUE "exit:" C_RESET C_CYAN" to exit the shell.\n" C_RESET);
		printf(C_BLUE "history:" C_RESET C_CYAN" to see the list of all the user commands executed earlier.\n"C_RESET);
		printf(C_BLUE "help:" C_RESET C_CYAN" to see this list.\n" C_RESET);
		printf(C_BLUE "hello:" C_RESET C_CYAN" to see the welcome message.\n" C_RESET);
		printf("\n");
		printf(C_GREEN "More commands coming soon, stay updated.\n" C_RESET);
		return true;
	}
	return false;
}