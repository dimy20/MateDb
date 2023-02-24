#pragma once

#define ERROR(s) do { \
	fprintf(stderr, "Error %s\n", s); \
}while(0);

#define DIE(s) do { \
	fprintf(stderr, "Error %s\n", s); \
	exit(1); \
}while(0);
