#pragma once
#include "hash_table.h"

#define CMD_CONTINUE_STR "continue"
#define CMD_BREAK_STR "break"
#define CMD_INFO_STR "info"
#define CMD_QUIT_STR "quit"
#define CMD_SET_STR "set"
#define CMD_REGISTERS_STR "registers"

typedef enum Command{
	CMD_CONTINUE,
	CMD_BREAK,
	CMD_INFO,
	CMD_SET,
	CMD_REGISTERS,
	CMD_QUIT
}Command;

void MateDb_InitCommands();
void MateDb_QuitCommands();
