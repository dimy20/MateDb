#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/personality.h>

#include "Ui.h"
#include "Utils.h"

int main(int argc, char ** argv){
	if(argc < 2){
		DIE("Error: No program name provided.");
	}
	char * program = argv[1];
	UI_Init();
	UI_Run(program);
	UI_Quit();
	return 0;
}
