#pragma once
#include "hash_table.h"

#define CMD_CONTINUE_STR "continue"
#define CMD_BREAK_STR "break"
#define CMD_INFO_STR "info"
#define CMD_QUIT_STR "quit"
#define CMD_SET_STR "set"
#define CMD_REGISTERS_STR "registers"
#define CMD_MEMORY_STR "memory"

#define CMD_READ_STR "read"
#define CMD_WRITE_STR "write"

#define CMD_MAPPINGS_STR "mappings"

typedef enum Command{
	CMD_CONTINUE,
	CMD_BREAK,
	CMD_INFO,
	CMD_SET,
	CMD_REGISTERS,
	CMD_MEMORY,
	CMD_READ,
	CMD_WRITE, // todo
	CMD_QUIT,
	CMD_MAPPINGS,

	NUM_COMMANDS
}Command;

void MateDb_InitCommands();
void MateDb_QuitCommands();
