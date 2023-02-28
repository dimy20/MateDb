#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/personality.h>

#include "MateDb.h"
#include "hash_table.h"

int main(int argc, char ** argv){
	if(argc < 2){
		DIE("Error: No program name provided.");
	}
	char * prog = argv[1];
	MateDb_Init();
	MateDb_StartSession(prog);
	MateDb_Quit();
	return 0;
}
