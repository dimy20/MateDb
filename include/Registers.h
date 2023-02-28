#pragma once

#include "Utils.h"
#include <sys/types.h>

typedef enum Register{
  r15, r14,     r13,    r12,
  rbp, rbx,     r11,    r10,
  r9,  r8,      rax,    rcx,
  rdx, rsi,     rdi,    orig_rax,
  rip, cs,      eflags, rsp,
  ss,  fs_base, gs_base, ds,
  es,  fs,      gs,
  NUM_REGISTERS,
}Register;

typedef struct RegisterDescriptor{
	Register r;
	int dwarf;
	const char * name;
}RegisterDescriptor;

void Registers_Init();
void Registers_Quit();

int Registers_Read(const char * rString, uint64_t * value);
uint32_t Registers_Write(pid_t pid, const char * regName, uint64_t value);

void Registers_PumpValues(pid_t pid);
