#include <sys/user.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "Registers.h"
#include "hash_table.h"
#include "Utils.h"

HashTable * regStrings;
struct user_regs_struct regs;

const RegisterDescriptor rdTable[NUM_REGISTERS] = {
    {r15, 15, "r15" },
    {r14, 14, "r14" },
    {r13, 13, "r13" },
    {r12, 12, "r12" },
    {rbp, 6,  "rbp" },
    {rbx, 3,  "rbx" },
    {r11, 11, "r11" },
    {r10, 10, "r10" },
    {r9,  9,  "r9" },
    {r8,  8,  "r8" },
    {rax, 0,  "rax" },
    {rcx, 2,  "rcx" },
    {rdx, 1,  "rdx" },
    {rsi, 4,  "rsi" },
    {rdi, 5,  "rdi" },
    {orig_rax, -1, "orig_rax" },
    {rip, -1, "rip" },
    {cs, 51,  "cs" },
    {eflags, 49, "eflags" },
    {rsp, 7, "rsp" },
    {ss, 52, "ss" },
    {fs_base, 58, "fs_base" },
    {gs_base, 59, "gs_base" },
    {ds, 53, "ds" },
    {es, 50, "es" },
    {fs, 54, "fs" },
    {gs, 55, "gs" },
};

void Registers_Init(){
	regStrings = HashTable_Create(NUM_REGISTERS);
	for(size_t i = 0; i < NUM_REGISTERS; i++){
		HashTable_Insert(&regStrings, rdTable[i].name, rdTable[i].r);
	};
}

void Registers_PumpValues(pid_t pid){
	errno = 0;
	long res = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
	if(res < 0 && errno != 0){
		DIE(strerror(errno))
	};
};

int Registers_Read(const char * rString, uint64_t * value){
	*value = 0;
	int r = HashTable_Get(regStrings, rString);

	if(r == NO_ELEM) return r;

	*value = ((uint64_t  *)&regs)[r];
	return 1;
};

void Registers_Quit(){
	HashTable_Destroy(regStrings);
}
