#pragma once

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

#include "commands.h"
#include "Utils.h"
#include "linenoise.h"
#include "hash_table.h"

#define MAX_BREAKPOINTS 32
#define NUM_REGISTERS 27

/*Note: Im not supossed to be creating many instances of the debugger.
 *The debugger is supossed te be launched once.*/

typedef enum Register {
    rax, rbx, rcx, rdx,
	rdi, rsi, rbp, rsp,
    r8,  r9,  r10, r11, r12, r13, r14, r15,
    rip, rflags, cs,
    orig_rax, fs_base,
    gs_base,
    fs, gs, ss, ds, es
}Register;

typedef struct RegisterDescriptor{
	Register r;
	int dwarf;
	const char * regName;
}RegisterDescriptor;


typedef struct Breakpoint{
	intptr_t addr;
	uint8_t savedByte; // saves the byte that is modified by int3 opcode
	uint8_t id;
}Breakpoint;

typedef struct MateDb{
	const char * programName;
	pid_t pid;
	int running;

	// keep all breakpoints in this array
	Breakpoint breaks[MAX_BREAKPOINTS];
	size_t breakCount;

}MateDb;

MateDb * MateDb_Create(const char * programName, int pid);
void MateDb_Destroy(MateDb * debug);
void MateDb_Run(MateDb * debug);

/* Breakpoints functionality*/
void MateDb_SetBreakpointAt(MateDb * debugger, intptr_t addr);
void MateDb_DisableBreakPointAt(MateDb * debugger, intptr_t addr);
void MateDb_DisableBreakPointId(MateDb * debugger, uint8_t id);
/* Outputs the content of every register and their names*/
void MateDb_InfoRegisters(MateDb * debugger);
