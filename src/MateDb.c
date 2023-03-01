#include "MateDb.h"
#include "Context.h"

Session * s;
extern Context ctx;

typedef void (*cmdAction)(Session * s);
static cmdAction cmdHandlers[NUM_COMMANDS];

#define MATEDEF inline static

extern HashTable * commandsTable;
extern const RegisterDescriptor rdTable[NUM_REGISTERS];

MATEDEF void MateDb_SetBreakpointAt(Session * s, intptr_t addr);
MATEDEF void MateDb_DisableBreakPointAt(Session * s, intptr_t addr);
//MATEDEF void MateDb_DisableBreakPointId(Session * debugger, uint8_t id);
MATEDEF void MateDb_InfoRegisterAll(Session * s);

MATEDEF void MateDb_DestroySession(Session * s);

MATEDEF void MateDb_Set(Session * s);

MATEDEF void MateDb_InfoMappings(Session * s);

MATEDEF void MateDb_Breakpoint(Session * s);
MATEDEF void MateDb_Continue(Session * s);
MATEDEF void MateDb_Info(Session * s);


void MateDb_Init(){
	/* MateDb */
	MateDb_InitCommands();

	cmdHandlers[CMD_SET] = &MateDb_Set;
	cmdHandlers[CMD_BREAK] = &MateDb_Breakpoint;
	cmdHandlers[CMD_INFO] = &MateDb_Info;
	cmdHandlers[CMD_CONTINUE] = &MateDb_Continue;

	Registers_Init();

	ctx.cmdStrings = NULL;
	ctx.argsLen = 0;
}

void MateDb_Quit(){
	MateDb_QuitCommands();
	Registers_Quit();

}

MATEDEF uint32_t MateDb_ReadMemory(pid_t pid, intptr_t addr, uint64_t ** buffer, size_t count){
	char filename[128] = {0};
	snprintf(filename, 128, "/proc/%d/mem", pid);
	FILE * f = fopen(filename, "r");

	if(f == NULL){
		ERROR(strerror(errno));
		return 1;
	}

	if(fseek(f, addr, SEEK_SET) < 0){
		ERROR(strerror(errno));
	}

	int fd = fileno(f);

	*buffer = malloc(count); // TODO: set a limit to this?

	ssize_t n = (int)read(fd, *buffer, count);

	if(n < 0){
		ERROR(strerror(errno));
		return ERROR_MEMORY_READ;
	};

	fclose(f);
	return NO_ERROR;
}

MATEDEF uint32_t MateDb_WriteMemory(pid_t pid, intptr_t addr, uint64_t value){
	UNIMPLEMENTED
}

MATEDEF void MateDb_Breakpoint(Session * s){
	char ** strings = ctx.cmdStrings;
	size_t len = ctx.argsLen;
	if(len < 2){
		ERROR_RET("Provide an adress to break at.");
	}

	char * end;
	intptr_t addr = strtol(strings[1], &end, 0);
	if(*end != '\0'){
		ERROR_RET("Bad address");
	}

	MateDb_SetBreakpointAt(s, addr);

}

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
}

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
	char ** strings = ctx.cmdStrings;
	size_t len = ctx.argsLen;

	if(len < 2)
		ERROR_RET("Not enough arguments for info");

	Command cmd = HashTable_Get(commandsTable, strings[1]);

	if(cmd == NO_ELEM){
		char errorString[128] = {0};
		snprintf(errorString, 128, "info %s", ctx.cmdStrings[1]);
		UNKNOWN_COMMAND(errorString);
	};

	if(cmd == CMD_REGISTERS){
		MateDb_InfoRegisterAll(s);
		return;
	};

	if(cmd == CMD_MEMORY){
		if(len < 3){
			ERROR_RET("info memory <mem-addr>\n");
		}

		char * end;
		intptr_t addr = strtol(strings[2], &end, 0);
		if(*end != '\0'){
			ERROR_RET("Bad address.\n");
		}

		uint64_t * data;
		MateDb_ReadMemory(s->pid, addr, &data, 1);
		printf("value at 0x%lx -> 0x%lx\n", addr, *data);

		free(data);

	};

	if(cmd == CMD_MAPPINGS){
		MateDb_InfoMappings(s);
	}
}

void MateDb_ExecuteCmd(){
	assert(s != NULL);

	ctx.cmdStrings = split(ctx.inputText, ' ', &ctx.argsLen);

	if(ctx.argsLen < 1) return;

	char * cmdName = ctx.cmdStrings[0];


	Command cmd = HashTable_Get(commandsTable, cmdName);

	if(cmd == NO_ELEM){
		UNKNOWN_COMMAND(cmdName)
		return;
	};

	if(cmd == CMD_QUIT){
		s->running = 0;
		return;
	};

	cmdAction fn = cmdHandlers[cmd];
	assert(fn != NULL && "NULL cmd function handler");

	fn(s);

	for(size_t i = 0; i < ctx.argsLen; i++){
		free(ctx.cmdStrings[i]);
	}
}

MATEDEF void MateDb_InfoRegisterAll(Session * s){
	int readAll = 1;

	size_t i = 0;
	size_t count = NUM_REGISTERS;

	if(ctx.argsLen > 2){
		i = 2;
		count = ctx.argsLen;
		readAll = 0;
	}

	Registers_PumpValues(s->pid);

	for(; i < count; i++){
		const char * name;
		uint64_t value;

		if(readAll){
			name = rdTable[i].name;
		}else{
			name = ctx.cmdStrings[i];
		}
		if(Registers_Read(name, &value) < 0){
			char errorString[128] = {0};
			snprintf(errorString, 128, "info registers %s", name);
			UNKNOWN_COMMAND(errorString);
			return;
		}

		printf("%s -> 0x%lx\n", name, value);
	};
}

MATEDEF void MateDb_DestroySession(Session * s){ free(s); }

void MateDb_StartSession(const char * prog, Run run){
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
		int waitStatus;
		waitpid(s->pid, &waitStatus, 0);

		printf("Debuggin process %d \n", s->pid);
		run();

		MateDb_DestroySession(s);
	}
}

MATEDEF void MateDb_Set(Session * s){
	char ** strings = ctx.cmdStrings;
	size_t len = ctx.argsLen;
	if(len < 2) ERROR_RET("Not enough arguments for set");

	Command cmd = HashTable_Get(commandsTable, strings[1]);

	if(cmd == NO_ELEM){
		char errorString[128] = {0};
		snprintf(errorString, 128, "Set %s", strings[1]);
		UNKNOWN_COMMAND(errorString);
	}

	if(cmd == CMD_REGISTERS){
		if(len < 4) ERROR_RET("No enough arguments for set registers");
		char * regName = strings[2];
		char * end;
		uint64_t value = strtoull(strings[3], &end, 0);
		if(*end != '\0'){
			char errLog[128] = {0};
			snprintf(errLog, 128, "Invalid value: %s\n", strings[3]);
			ERROR_RET(errLog);
		}

		uint32_t err;
		if((err = Registers_Write(s->pid, regName, value)) !=  0){
			if(err & REG_INVALID_NAME){
				char errLog[128] = {0};
				snprintf(errLog, 128,"%s is not a valid register name.\n", strings[2]);
				ERROR_RET(errLog);
			}
		};
	}

	if(cmd == CMD_MEMORY){
		UNIMPLEMENTED
	}
}

MATEDEF void MateDb_InfoMappings(Session * s){
	char filename[128] = {0};
	snprintf(filename, 128, "/proc/%d/maps", s->pid);
	printf("%s\n", filename);

    FILE * f = fopen(filename, "r");
	if(f == NULL){
		ERROR_RET(strerror(errno));
	};

	char buffer[256] = {0};

	while(fgets(buffer, 256, f)){
		printf("%s", buffer);

		memset(buffer, 0, 256);
	};

	fclose(f);
}

MATEDEF void MateDb_Continue(Session * s){
	ptrace(PTRACE_CONT, s->pid, NULL, NULL);
	// block thread intil tracee is signaled
	int waitStatus;
	waitpid(s->pid, &waitStatus, 0);
}
