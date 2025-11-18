#include <stddef.h>
#include <stdint.h>
#include <string.h>  // Used for messing with strings
#include <stdio.h>		// I used it mostly for printing to stdout
#include <stdlib.h>		// Stuff like malloc and free

// Table size
#define TOTAL_SIZE 10000

/*
 * entry_t is a pointer to memory allocated for the table
 * or something like that
 * */
typedef struct entry_t
{
	char *key;
	char *value;
	struct entry_t *next;
} entry_t;

/*
 * This defines the specific hashtable or something
 * like that
 * */
typedef struct
{
	entry_t **entries;
	size_t size;
	size_t count;
} ht_t;

/*
 * This returns a hashtable and allocates space
 * for it and the entries
 *
 * It also set all the values in the entries to NULL
 *  -But beware, trying to access memory that is NULL
 *  -will result in a seg fault
 * */
ht_t *create()
{
	// Initialize the hashmap
	ht_t *hashmap = malloc(sizeof(ht_t));
	hashmap->size = TOTAL_SIZE;
	hashmap->count = 0;

	// Initialize all entries in the hashmap
	hashmap->entries = malloc(sizeof(entry_t) * hashmap->size);

	// Set all entries to NULL
	for (int i = 0; i < TOTAL_SIZE; ++i)
	{
		hashmap->entries[i] = NULL;
	}

	return hashmap;
}


/*
 * I did the literal barebones minimal with this hash function
 * but it could definitely be improved upon
 *
 * I literally just added the decmial codes for each of the characters
 * and added them up
 *
 * They are far better ways to do this that allow minimal collisions
 * -------------------------------
 *  Flash forward later to now i adopted a method for hashing that
 *  makes the hashes way more unique
 *  ----------------------------------------
 *  I also used uint64_t cus 32bit types truncated stuff before
 * */
uint64_t hash(const char *key)
{
	uint64_t hash = 1469598103934665603UL;
	int c;

	while ((c = *key++))
	{
		hash ^= c;
		hash *= 1099511628211UL;
	}

	return hash;
}

/*
 * This block allocates new memory for the new size
 * It sets all values to NULL
 * It then updates the hashes to fit into the new size
 *
 * It then checks all values in the old entries and copies
 * it into the new entries
 *
 * It then free the old entries and replaces it with the new
 * entries
 *
 * Then it updates the size of the entry
 * */

void ht_resize(ht_t *hashmap, int new_size)
{
	entry_t **entry = malloc(sizeof(entry_t*) * new_size);
	for (int i = 0; i < new_size; i++) {
		entry[i] = NULL;
	}

	for (int i = 0; i < hashmap->size; i++)
	{
		if (hashmap->entries[i] != NULL)
		{
			int new_index = hash(hashmap->entries[i]->key) % new_size;
			entry[new_index] = hashmap->entries[i];
		}
	}

	free(hashmap->entries);
	hashmap->entries = entry;
	hashmap->size = new_size;
}


/*
 * After getting the index of the entry you want to add info to,
 * this funtion alocates memory for the key and value info and stores
 * the strings
 *
 * Think of it like This
 * 	You allocate space for 10000 entries, not that this entries have
 * 	a key, value property that you have to allocate memory for to use
 *
 * That is what this funtion does - It allocates memory for the key, value
 * at the index of entries you want to change
 * */
entry_t *ht_pair(const char *key, const char *value)
{
	// This creates a new entry and will set it to the value of the entry
	// we want to modify
	entry_t *entry = malloc(sizeof(entry_t*)); // allocates space
	entry->key = malloc(strlen(key) + 1);			// allocates space for the strings
	entry->value = malloc(strlen(value) + 1);
	/*
	 * This bit here copies the info you want into the entry
	 * */
	strcpy(entry->key, key);
	strcpy(entry->value, value);

	// Return the entry
	return entry;
}

/*
 * Step one of this gets the hash of the key
 * If the entry at that index == NULL it immediately copies the values
 * into the entry
 *
 * If the entry is not equal to NULL, it compares the keys.
 * If the keys are the same, it replaces the value.
 *
 * It the keys are different, it increments the index by until it finds
 * a key that is equal or a NULL entry to replace.
 *
 * While increasing indexes it also checks if it is at the edge of the array,
 * so that it can start looking from the beginning, instead of going past the array
 * and causing another seg fault.
 *
 * */

void ht_set(ht_t *hashmap, const char *key, const char *value)
{
	unsigned int slot = hash(key) % hashmap->size;  // Retrieves the index

	entry_t **entry = &hashmap->entries[slot];  // Points to it for easier access
	//
	if ((double)hashmap->count / hashmap->size > 0.7)
	{
		ht_resize(hashmap, hashmap->size * 2);
	}

	// If entry is Null replace immediately
	if (*entry == NULL)
	{
		hashmap->entries[slot] = ht_pair(key, value);
		hashmap->count++;
		return;
	}
	else {
		//Checks until it reaches a Null pointer
		//
		//Also be sure to replace values with hashmap->entries[slot]->key = ...
		//	and not entries->key = ... as this will cause problems
		while (hashmap->entries[slot] != NULL)
		{
			if (strcmp((*entry)->key, key) == 0)  //compares if values are the same
			{
				free(*entry); // Removes the values
				hashmap->entries[slot] = ht_pair(key, value);  // Replaces the old values
				hashmap->count++;
				return; // Stops the loop here
			}
			// If keys are not the same, increments the index and checks again
			else {
				if (slot == hashmap->size - 1)
				{
					slot = 0;
					printf("%d\n", (int)hashmap->size);
				}
				slot++;
			}
		}
		// If it comes across a NULL pointer, it puts the value there
		hashmap->entries[slot] = ht_pair(key, value);
		hashmap->count++;
	}
}

/*
 * So uhh the get funtion is pretty much almost the same as set
 * it just prints the value if it is found
 * It also tells the user if the value exists
 * */

void ht_get(ht_t *hashmap, const char *key)
{
	// Get the hash
	unsigned int slot = hash(key) % hashmap->size;

	// Checks if the entry exists and prints it
	entry_t *entry = hashmap->entries[slot];
	if (entry == NULL)
	{
		printf("Uhhm sorry that doesnt exit\n");
	}
	else {
		while (hashmap->entries[slot] != NULL)
		{
			if (strcmp(hashmap->entries[slot]->key, key) == 0)
			{
				printf("%s\n", hashmap->entries[slot]->value);
				return;
			}
			else
			{
				if (slot == TOTAL_SIZE - 1)
				{
					slot = 0;
				}
				slot++;
			}
		}
		printf("Sorry man but that isn't in the list\n");
	}
}

void ht_deactivate(entry_t ***entry, const char *key, int slot)
{
	(*entry)[slot]->value = "Deleted";
}

void ht_del(ht_t *hashmap, const char *key)
{
	unsigned int slot = hash(key) % hashmap->size;

	entry_t *entry = hashmap->entries[slot];
	while (hashmap->entries[slot] != NULL)
	{
		if (strcmp(hashmap->entries[slot]->key, key) == 0)
		{
			ht_deactivate(&hashmap->entries, key, slot);
			return;
		}
		else {
			if (slot == TOTAL_SIZE - 1)
			{
				slot = 0;
			}
			slot++;
		}
	}
	printf("That doesnt exist man\n");
}

void ht_dump(ht_t *hashmap)
{
	int i = 0;
	while (i < hashmap->size) {
		if (hashmap->entries[i] != NULL)
		{
			printf("slot[%d]:			%s - %s\n", i, hashmap->entries[i]->key, hashmap->entries[i]->value);
		}
		i++;
	}
}

int main(int argc, char **argv)
{
	// Initialision
	ht_t *hashmap = create();
	ht_set(hashmap, "College 1", "Portsmouth college");  // Set the values
	ht_set(hashmap, "College 2", "UTC");  // Set the values
	ht_set(hashmap, "College 3", "Havant College");  // Set the values
	ht_set(hashmap, "Another College", "Fareham College");  // Set the values
	ht_del(hashmap, "College 1");
	ht_dump(hashmap);
	//
	return 0;
}
