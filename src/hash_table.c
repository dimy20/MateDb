#include "hash_table.h"
#include <assert.h>

static HTNode DELETED_NODE = {NULL, NO_ELEM};

static uint32_t isPrime(uint32_t num){
	if(num == 0 || num == 1) return 0;

	for(uint32_t i = 2; i <= num / 2; i++){
		if(num % i == 0) return 0;
	}

	return 1;
};

static uint32_t nextPrime(uint32_t num){
	while(!isPrime(num)) num++;
	return num;
};

static uint32_t hash1(const char * key, const int a, const uint32_t upper_bound){
	uint32_t hash = 0;
	size_t m = strlen(key);
	for(size_t i = 0; i < m; i++){
		hash += (uint32_t)pow(a, (double)(m - (i + 1))) * (uint8_t)key[i];
		hash %= upper_bound;
	}

	return hash;

};

// djb2 by Dan Bernstein
static uint32_t hash2(const char *str, const uint32_t upper_bound){
	uint32_t hash = 5381;
	int c;

	while ((c = *str++)){
		hash = ((hash << 5) + hash) + (uint32_t)c; /* hash * 33 + c */
	}

	hash %= upper_bound;
	return hash == 0 ? 1 : (uint32_t)hash;
}

static uint32_t findSpot(const char * key, uint32_t upper_bound, uint32_t collision_i){
	uint32_t h1 = hash1(key, 589, upper_bound);

	// probe for a new spot.
	uint32_t h2 = hash2(key, upper_bound);
	uint32_t next_index = (h1 + collision_i * h2) % upper_bound;

	return  next_index;
};

static HTNode * HTNode_Create(const char * key, int value){
	assert(key != NULL);

	HTNode * node = malloc(sizeof(HTNode));

	node->key = malloc(strlen(key) + 1);
	strncpy(node->key, key, strlen(key));
	node->key[strlen(key)] = '\0';

	node->value = value;
	return node;
};

static void HTNode_Destroy(HTNode * node){
	if(node != NULL && node != &DELETED_NODE){
		free(node->key);
		free(node);
	}
};

HashTable * HashTable_Create(uint32_t capacity){
	HashTable * table = malloc(sizeof(HashTable));

	table->capacity = nextPrime(capacity <= 0 ? DEFAULT_BASE_CAPACITY: capacity);
	table->size = 0;
	table->nodes = calloc(table->capacity, sizeof(HTNode *));

	return table;
}

static void HashTable_Resize(HashTable ** old_table, uint32_t new_capacity){
	if(new_capacity < DEFAULT_BASE_CAPACITY) return;

	HashTable * new_table = HashTable_Create(new_capacity);
	HTNode * curr_node;
	for(uint32_t i = 0; i < (*old_table)->capacity; i++){
		curr_node = (*old_table)->nodes[i];

		if(curr_node != NULL && curr_node != &DELETED_NODE){
			HashTable_Insert(&new_table, curr_node->key, curr_node->value);
		};
	};

	HashTable_Destroy(*old_table);
	*old_table = new_table;
};

void HashTable_Insert(HashTable ** table, const char * key, int value){
	float load_factor = (float)((*table)->size) / (float)(*table)->capacity;
	//printf("load factor : %f\n", load_factor);
	if(load_factor > LOAD_FACTOR_UP_THRESHOLD){
		HashTable_Resize(table, (*table)->capacity * 2);
	}

	size_t index = findSpot(key, (*table)->capacity, 0);
	uint32_t collision_i = 1; // assume first collision happened

	while((*table)->nodes[index] != NULL){
		HTNode * curr_node = (*table)->nodes[index];
		// if keys match this is not a collision, its a modify
		if(curr_node != &DELETED_NODE){
			if(strncmp(curr_node->key, key, strlen(curr_node->key)) == 0){
				curr_node->value = value;
				// copy value string
				/*
				if(strlen(curr_node->value) + 1 < strlen(value)){
					memcpy(curr_node->value, value, strlen(value));
					curr_node->value[strlen(value)] = '\0';
				}else{
					free(curr_node->value);
					curr_node->value = malloc(strlen(value) + 1);
					memcpy(curr_node->value, value, strlen(value));
					curr_node->value[strlen(value)] = '\0';

				}
				*/
				return;
			};
		}

		index = findSpot(key, (*table)->capacity, collision_i);
		collision_i++;
	};

	(*table)->nodes[index] = HTNode_Create(key, value);
	(*table)->size++;
};

int HashTable_Get(HashTable * table, const char * key){
	size_t index = findSpot(key, table->capacity, 0);
	uint32_t collision_i = 1; // assume first collision happened

	while(table->nodes[index] != NULL){
		HTNode * curr_node = table->nodes[index];
		// if keys match this is not a collision, its a modify
		if(curr_node != &DELETED_NODE){
			if(strncmp(curr_node->key, key, strlen(curr_node->key)) == 0){
				return curr_node->value;
			};
		}

		index = findSpot(key, table->capacity, collision_i);
		collision_i++;
	};

	return NO_ELEM;
};

void HashTable_Remove(HashTable ** table, const char * key){
	if(((*table)->size == 0) || (HashTable_Get(*table, key) == NO_ELEM)) return;

	float load_factor = (float)((*table)->size) / (float)(*table)->capacity;
	if(load_factor < LOAD_FACTOR_LOW_THRESHOLD){
		HashTable_Resize(table, (*table)->capacity / 2);
	}

	size_t index = findSpot(key, (*table)->capacity, 0);
	uint32_t collision_i = 1;

	while((*table)->nodes[index] != NULL){
		HTNode * curr_node = (*table)->nodes[index];
		if(curr_node != &DELETED_NODE && strncmp(curr_node->key, key, strlen(curr_node->key)) == 0){
			HTNode_Destroy(curr_node);
			(*table)->nodes[index] = &DELETED_NODE;
			break;
		};
		index = findSpot(key, (*table)->capacity, collision_i);
		collision_i++;
	};

	(*table)->size--;
};

void HashTable_Destroy(HashTable * table){
	for(size_t i = 0; i < table->capacity; i++){
		HTNode_Destroy(table->nodes[i]);
	}
	free(table->nodes);
	free(table);
};
