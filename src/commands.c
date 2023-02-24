#include "commands.h"
#include <stdio.h>

static int commandsInitialized = 0;
HashTable * commandsTable;

void MateDb_InitCommands(){
	if(commandsInitialized) return;
	/*Build the commands table*/
	commandsTable = HashTable_Create(0);

	HashTable_Insert(&commandsTable, CMD_CONTINUE_STR, CMD_CONTINUE);
	HashTable_Insert(&commandsTable, CMD_BREAK_STR, CMD_BREAK);
	HashTable_Insert(&commandsTable, CMD_INFO_STR, CMD_INFO);
	HashTable_Insert(&commandsTable, CMD_QUIT_STR, CMD_QUIT);

	commandsInitialized = 1;
};

void MateDb_QuitCommands(){
	HashTable_Destroy(commandsTable);
};
