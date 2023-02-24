#pragma once
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

#define DEFAULT_BASE_CAPACITY 100
#define LOAD_FACTOR_UP_THRESHOLD 0.7f
#define LOAD_FACTOR_LOW_THRESHOLD 0.1f
#define NO_ELEM INT_MIN

#ifdef __cplusplus
extern "C" {
#endif 
	typedef struct HTNode{
		char * key;
		int value;
	} HTNode;

	typedef struct HashTable{
		uint32_t capacity;
		uint32_t size;
		HTNode ** nodes;
	}HashTable;

	HashTable * HashTable_Create(uint32_t capacity);

	void HashTable_Destroy(HashTable * table);

	void HashTable_Insert(HashTable ** table, const char * key, int value);

	int HashTable_Get(HashTable * table, const char * key);

	void HashTable_Remove(HashTable ** table, const char * key);

#ifdef __cplusplus
}
#endif 


