#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/personality.h>

#include "MateDb.h"

int main(int argc, char ** argv){
	MateDb * db;
	if(argc < 2){
		DIE("Provide a program name as argument");
	}

	char * prog = argv[1];

	pid_t pid = fork();

	db = MateDb_Create(prog, pid);

	if(pid == 0){
		personality(ADDR_NO_RANDOMIZE);
		errno = 0;
		if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0){
			DIE(strerror(errno));
		}
		execl(prog, prog, NULL);
	}else{
		MateDb_Run(db);
		MateDb_Destroy(db);
	}

	return 0;
}
