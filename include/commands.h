#pragma once
#include "hash_table.h"

#define CMD_CONTINUE_STR "continue"
#define CMD_BREAK_STR "break"
#define CMD_INFO_STR "info"
#define CMD_QUIT_STR "quit"

typedef enum Command{
	CMD_CONTINUE,
	CMD_BREAK,
	CMD_INFO,
	CMD_QUIT
}Command;

void MateDb_InitCommands();
void MateDb_QuitCommands();
