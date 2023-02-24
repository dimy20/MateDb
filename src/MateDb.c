#include "MateDb.h"

#define MATEDEF inline static

extern HashTable * commandsTable;

MATEDEF void MateDb_SetBreakpointAt(Session * debugger, intptr_t addr);
MATEDEF void MateDb_DisableBreakPointAt(Session * debugger, intptr_t addr);
//MATEDEF void MateDb_DisableBreakPointId(Session * debugger, uint8_t id);
MATEDEF void MateDb_InfoRegisters(Session * debugger);

MATEDEF void MateDb_DestroySession(Session * s);
MATEDEF void MateDb_RunSession(Session * s);

static const RegisterDescriptor rdTable[NUM_REGISTERS] = {
    {r15, 15, "r15" },
    {r14, 14, "r14" },
    {r13, 13, "r13" },
    {r12, 12, "r12" },
    {rbp, 6, "rbp" },
    {rbx, 3, "rbx" },
    {r11, 11, "r11" },
    {r10, 10, "r10" },
    {r9,  9, "r9" },
    {r8,  8, "r8" },
    {rax, 0, "rax" },
    {rcx, 2, "rcx" },
    {rdx, 1, "rdx" },
    {rsi, 4, "rsi" },
    {rdi, 5, "rdi" },
    {orig_rax, -1, "orig_rax" },
    {rip, -1, "rip" },
    {cs, 51, "cs" },
    {rflags, 49, "eflags" },
    {rsp, 7, "rsp" },
    {ss, 52, "ss" },
    {fs_base, 58, "fs_base" },
    {gs_base, 59, "gs_base" },
    {ds, 53, "ds" },
    {es, 50, "es" },
    {fs, 54, "fs" },
    {gs, 55, "gs" },
};

void MateDb_Init(){
	MateDb_InitCommands();
};

void MateDb_Quit(){
	MateDb_QuitCommands();
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

MATEDEF void MateDb_ExecuteCmd(Session * s, const char * line){
	assert(s != NULL);
	assert(line != NULL);

	size_t len;
	char ** strings = split(line, ' ', &len);
	char * cmdName = strings[0];

	Command cmd = HashTable_Get(commandsTable, cmdName);
	printf("->> %d\n", cmd);
	if(cmd == NO_ELEM){
		char errorString[512] = {0};
		snprintf(errorString, sizeof(errorString), "Uknown command: %s\n", cmdName);
		ERROR(errorString);
		goto exit;
	};

	switch(cmd){
		case CMD_BREAK:
			{
				char * stringAddr = strings[1];
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
			MateDb_InfoRegisters(s);
			break;
		case CMD_QUIT:
			//TODO: Right now debugee executes anyway, prevent that from happening
			//and exit the program directly
			s->running = 0;
			break;
	};


	exit:
		for(size_t i = 0; i < len; i++){
			free(strings[i]);
		}
		free(strings);;
};

MATEDEF void MateDb_RunSession(Session * s){ // wait for change of state
	s->running = 1;

	int waitStatus;
	waitpid(s->pid, &waitStatus, 0);

	printf("Debuggin process %d \n", s->pid);
	char * line;
	while(s->running && (line = linenoise("test> ")) != NULL){
		MateDb_ExecuteCmd(s, line);

		linenoiseHistoryAdd(line);
		linenoiseFree(line);
	}

}

MATEDEF void MateDb_InfoRegisters(Session * s){
	struct user_regs_struct regs;
	errno = 0;
	long res = ptrace(PTRACE_GETREGS, s->pid, NULL, &regs);
	if(res < 0 && errno != 0){
		DIE(strerror(errno))
	};

	typedef unsigned long long int lli;
	for(size_t i = 0; i < NUM_REGISTERS; i++){
		lli registerValue  = ((lli *)&regs)[i];
		printf("%s -> %llx\n", rdTable[i].regName, registerValue);
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
