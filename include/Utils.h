#pragma once

#define NO_ERROR 0x0
#define REG_INVALID_NAME 0x1
#define ERROR_MEMORY_READ 0x2

#define ERROR(s) do { \
	fprintf(stderr, "Error %s\n", s); \
}while(0);

#define ERROR_RET(s) do { \
	ERROR(s); \
	return; \
}while(0);

#define DIE(s) do { \
	fprintf(stderr, "Error %s\n", s); \
	exit(1); \
}while(0);

#define UNKNOWN_COMMAND(cmd) do { \
	char _buff[512] = {0}; \
	snprintf(_buff, sizeof(_buff), "Uknown command: %s\n", cmd); \
	fprintf(stderr, "%s\n", _buff); \
} while(0);

#define UNIMPLEMENTED do { \
	fprintf(stderr, "UNIMPLEMENTED %s:%d\n", __FILE__, __LINE__); \
	exit(1); \
} while(0); 
