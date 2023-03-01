#pragma once

#define _GNU_SOURCE

#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <assert.h>
#include <sys/personality.h>

#include "commands.h"
#include "Utils.h"
#include "linenoise.h"
#include "hash_table.h"
#include "Registers.h"

#define MAX_BREAKPOINTS 32
#define NUM_REGISTERS 27

typedef void (*Run)(void);

typedef struct Breakpoint{
	intptr_t addr;
	uint8_t savedByte; // saves the byte that is modified by int3 opcode
	uint8_t id;
}Breakpoint;

typedef struct Session{
	const char * programName;
	pid_t pid;
	int running;

	// keep all breakpoints in this array
	Breakpoint breaks[MAX_BREAKPOINTS];
	size_t breakCount;
}Session;

void MateDb_StartSession(const char * programName, Run run);
void MateDb_Init();
void MateDb_Quit();
void MateDb_ExecuteCmd();

