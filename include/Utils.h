#pragma once

#define NO_ERROR 0x0
#define REG_INVALID_NAME 0x1

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
