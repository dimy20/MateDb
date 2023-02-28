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
	HashTable_Insert(&commandsTable, CMD_SET_STR, CMD_SET);

	HashTable_Insert(&commandsTable, CMD_REGISTERS_STR, CMD_REGISTERS);
	HashTable_Insert(&commandsTable, CMD_MEMORY_STR, CMD_MEMORY);

	HashTable_Insert(&commandsTable, CMD_MAPPINGS_STR, CMD_MAPPINGS);

	commandsInitialized = 1;
}

void MateDb_QuitCommands(){
	HashTable_Destroy(commandsTable);
}
