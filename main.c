#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
	char **args = (char**)malloc(5*sizeof(char*));
	args[0] = strdup("xterm");
	args[1] = strdup("-T");
	args[2] = strdup("PB Shell");
	args[3] = strdup("./pbsh");
	args[4] = (char*)NULL;
	execvp(args[0], args);
}