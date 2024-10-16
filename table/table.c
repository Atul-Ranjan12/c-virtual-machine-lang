#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../memory/memory.h"
#include "../object/object.h"
#include "table.h"

#define TABLE_MAX_LOAD 0.75

// initTable initializes the empty hash table
void initTable(Table *tb) {
  tb->count = 0;
  tb->capacity = 0;
  tb->entries = NULL;
}

// freeTable frees the current hash table
void freeTable(Table *tb) {
  FREE_ARRAY(Entry, tb->entries, tb->count);
  initTable(tb);
}

// findEntry function finds a value in the hash table
static Entry *findEntry(Entry *entries, int capacity, ObjString *key) {
  uint32_t index = key->hash % capacity;

  Entry *tombstone = NULL;

  for (;;) {
    Entry *entry = &entries[index];
    if (entry->key == NULL) {
      // Check if it is a tombstone
      if (IS_NIL(entry->value)) {
        // Enpty value
        return tombstone != NULL ? tombstone : entry;
      } else {
        // Found a tombstone
        if (tombstone == NULL)
          tombstone = entry;
      }
    }
    if (entry->key == key) {
      return entry;
    }
    // Did not find it
    index = (index + 1) % capacity;
  }
}

// adjustCapacity dynamically grows a hashmap
static void adjustCapacity(Table *table, int capacity) {
  Entry *entries = ALLOCATE(Entry, capacity);

  printf("Reached here\n");

  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];

    if (entry == NULL) {
      printf("Reaching here: NULL Pointer reference\n");
    }

    printf("Outside the if for %d\n", i);

    if (entry->key == NULL) {
      printf("%d\n", i);
      continue;
    }

    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

// tableSet sets a value on a table
bool tableSet(Table *table, ObjString *key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    // Adjust capacity here
    adjustCapacity(table, capacity);
  }

  Entry *entry = findEntry(table->entries, table->capacity, key);

  bool isNewKey = entry->key == NULL;

  if (isNewKey && IS_NIL(entry->value))
    table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

// function to copy one hash table into another
void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];

    if (entry->key == NULL)
      continue;

    tableSet(to, entry->key, entry->value);
  }
}

// function to get a value from a table
// gets the value for the key in tb and stores it in value
bool tableGet(Table *tb, ObjString *key, Value *value) {
  if (tb->count == 0)
    return false;

  Entry *entry = findEntry(tb->entries, tb->capacity, key);
  if (entry->key == NULL)
    return false;

  *value = entry->value;

  return true;
}

// tableDelete deletes a key from the table by adding
// a special tombstone
bool tableDelete(Table *tb, ObjString *key) {
  if (tb->count == 0)
    return false;

  Entry *entry = findEntry(tb->entries, tb->capacity, key);
  if (entry->key == NULL)
    return false;

  // Delete here
  entry->key = NULL;
  entry->value = BOOL_VAL(true);

  return true;
}

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash) {
  if (table->count == 0)
    return NULL;

  uint32_t index = hash % table->capacity;
  for (;;) {
    Entry *entry = &table->entries[index];
    if (entry->key == NULL) {
      // Stop if we find an empty non-tombstone entry.
      if (IS_NIL(entry->value))
        return NULL;
    } else if (entry->key->length == length && entry->key->hash == hash &&
               memcmp(entry->key->chars, chars, length) == 0) {
      // We found it.
      return entry->key;
    }

    index = (index + 1) % table->capacity;
  }
}
