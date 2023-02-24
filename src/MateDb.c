#include "MateDb.h"

typedef struct MateContext{
	char ** cmdStrings;
	size_t argsLen;
}MateContext;

MateContext mateCtx;

#define UNKNOWN_COMMAND(cmd) do { \
	char _buff[512] = {0}; \
	snprintf(_buff, sizeof(_buff), "Uknown command: %s\n", cmd); \
	fprintf(stderr, "%s\n", _buff); \
} while(0);

#define MATEDEF inline static

extern HashTable * commandsTable;
extern const RegisterDescriptor rdTable[NUM_REGISTERS];

MATEDEF void MateDb_SetBreakpointAt(Session * s, intptr_t addr);
MATEDEF void MateDb_DisableBreakPointAt(Session * s, intptr_t addr);
//MATEDEF void MateDb_DisableBreakPointId(Session * debugger, uint8_t id);
MATEDEF void MateDb_InfoRegisterAll(Session * s);

MATEDEF void MateDb_DestroySession(Session * s);
MATEDEF void MateDb_RunSession(Session * s);

void MateDb_Init(){
	MateDb_InitCommands();
	Registers_Init();
};

void MateDb_Quit(){
	MateDb_QuitCommands();
	Registers_Quit();
};

MATEDEF void MateDb_SetBreakpointAt(Session * s, intptr_t addr){
	//printf("Setting breakpoint at %lx\n", *((uint64_t *)addr));
	if(s->breakCount >= MAX_BREAKPOINTS){
		return;
	};

	// TODO: maybe use a hashtable?
	for(size_t i = 0; i < s->breakCount; i++){
		if(s->breaks[i].addr == addr) return;
	}

	errno = 0;
	long data = ptrace(PTRACE_PEEKTEXT, s->pid, addr, NULL);

	if(data < 0 && errno != 0){
		DIE(strerror(errno));
	};

	uint8_t low = (uint8_t)(data & 0xff);
	uint8_t int3_Opcode = 0xcc;

	uint64_t modifiedInstruction = (uint64_t)((data & ~0xff) | int3_Opcode);

	errno = 0;
	if(ptrace(PTRACE_POKEDATA, s->pid, addr, modifiedInstruction) < 0){
		DIE(strerror(errno));
	};

	uint8_t id = (uint8_t)(s->breakCount + 1);

	Breakpoint b = {addr, low, id};
	s->breaks[s->breakCount++] = b;
}

MATEDEF void MateDb_DisableBreakPointAt(Session * s, intptr_t addr){
	assert(s != NULL);

	if(s->breakCount == 0) return;

	// TODO: make this a hashtable
	int found = 0;
	size_t i;
	for(i = 0; i < s->breakCount; i++){
		if(s->breaks[i].addr == addr){
			found = 1;
			break;
		}
	}
	if(!found) return;

	// restore original lower byte

	Breakpoint bp = s->breaks[i];

	errno = 0;
	long data = ptrace(PTRACE_PEEKDATA, s->pid, addr, NULL);
	if(data < 0){
		DIE(strerror(errno));
	};

	uint64_t originalData = (uint64_t)((data & ~0xff) | bp.savedByte);

	errno = 0;
	if(ptrace(PTRACE_POKEDATA, s->pid, addr, originalData) < 0){
		DIE(strerror(errno));
	};
}

MATEDEF Session * MateDb_CreateSession(const char * programName, int pid){
	Session * s = malloc(sizeof(Session));
	s->pid = pid;
	s->programName = programName;
	s->breakCount = 0;
	return s;
};

MATEDEF char ** split(const char * s, const char del, size_t * count){
	if(s == NULL){
		*count = 0;
		return NULL;
	}

	size_t n = strlen(s);
	size_t k = 1;
	char ** result;

	// how many strings will be in the result?
	for(size_t i = 0; i < n; i++){
		k += s[i] == del;
	}
	*count = k;

	result = malloc(sizeof(char *) * k);
	memset(result, 0, sizeof(char *) * k);


	size_t j = 0;
	size_t start = 0;
	for(size_t end = 0; end < n; end++){
		if(s[end] == del || end == n-1){
			if(end == n-1) end++;

			size_t len = end - start;

			char * newS = malloc(len + 1);
			strncpy(newS, s + start, len);
			newS[len] = '\0';

			result[j] = newS;

			start = end + 1;
			j++;
		};
	};

	return result;
}

MATEDEF void MateDb_Info(Session * s){
	if(strncmp("registers", mateCtx.cmdStrings[1], strlen("registers")) == 0){
		MateDb_InfoRegisterAll(s);
		return;
	}else{
		char errorString[128] = {0};
		snprintf(errorString, 512, "info %s", mateCtx.cmdStrings[1]);
		UNKNOWN_COMMAND(errorString);
	}
};

MATEDEF void MateDb_ExecuteCmd(Session * s){
	assert(s != NULL);

	char * cmdName = mateCtx.cmdStrings[0];

	Command cmd = HashTable_Get(commandsTable, cmdName);

	if(cmd == NO_ELEM){
		UNKNOWN_COMMAND(cmdName)
		return;
	};

	switch(cmd){
		case CMD_BREAK:
			{
				char * stringAddr = mateCtx.cmdStrings[1];
				if(!stringAddr) DIE("addres for break not provided.");
				intptr_t addr = strtol(stringAddr, NULL, 0);
				int offset = 0x114e;
				MateDb_SetBreakpointAt(s, addr + offset);
				break;
			}

		case CMD_CONTINUE:
			// continue execution
			ptrace(PTRACE_CONT, s->pid, NULL, NULL);
			// block thread intil tracee is signaled
			int waitStatus;
			waitpid(s->pid, &waitStatus, 0);
			break;
		case CMD_INFO:
			if(mateCtx.argsLen == 1){
				ERROR("No arguments provided for info.");
				break;
			}
			MateDb_Info(s);
			break;
		case CMD_QUIT:
			//TODO: Right now debugee executes anyway, prevent that from happening
			//and exit the program directly
			s->running = 0;
			break;
	};
};

MATEDEF void MateDb_RunSession(Session * s){ // wait for change of state
	s->running = 1;

	int waitStatus;
	waitpid(s->pid, &waitStatus, 0);

	char * line;
	printf("Debuggin process %d \n", s->pid);
	while(s->running && (line = linenoise("<Mate>")) != NULL){

		mateCtx.cmdStrings = split(line, ' ', &mateCtx.argsLen);

		if(mateCtx.argsLen > 0){
			MateDb_ExecuteCmd(s);
		}


		linenoiseHistoryAdd(line);
		linenoiseFree(line);

		for(size_t i = 0; i < mateCtx.argsLen; i++){
			free(mateCtx.cmdStrings[i]);
		}

		free(mateCtx.cmdStrings);;
	}

}

// info registers
MATEDEF void MateDb_InfoRegisterAll(Session * s){
	int readAll = 1;

	size_t i = 0;
	size_t count = NUM_REGISTERS;

	if(mateCtx.argsLen > 2){
		i = 2;
		count = mateCtx.argsLen;
		readAll = 0;
	}

	Registers_PumpValues(s->pid);

	for(; i < count; i++){
		const char * name;
		uint64_t value;

		if(readAll){
			name = rdTable[i].name;
		}else{
			name = mateCtx.cmdStrings[i];
		}
		if(Registers_Read(name, &value) < 0){
			char errorString[128] = {0};
			snprintf(errorString, 128, "info registers %s", name);
			UNKNOWN_COMMAND(errorString);
			return;
		}

		printf("%s -> %lx\n", name, value);
	};
};

MATEDEF void MateDb_DestroySession(Session * s){ free(s); };

void MateDb_StartSession(const char * prog){
	Session * s;
	pid_t pid = fork();

	s = MateDb_CreateSession(prog, pid);

	if(pid == 0){
		personality(ADDR_NO_RANDOMIZE);
		errno = 0;
		if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0){
			DIE(strerror(errno));
		}
		execl(prog, prog, NULL);
	}else{
		MateDb_RunSession(s);
		MateDb_DestroySession(s);
	}
}
