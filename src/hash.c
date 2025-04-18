/*
** hash.c - Hash type
**
** See Copyright Notice in hash.h
*/

#include "unxnote/hash.h"

#include <malloc.h>
#include <stdarg.h>
#include <string.h>

#define SCALE_FACTOR 2
#define GROWTH_FACTOR 2

typedef struct {
  char key[10];
  PT_HashValue value;
} HashEntry;

struct PT_Hash {
  HashEntry *entries;         // Hash slots
  size_t capacity;            // Size of ```entries``` array
  size_t size;                // number of elements in hash table
};

#define INITIAL_CAPACITY 16   // Most be greater then zero

static uint64_t hash_number(const char *key);
static const char *hash_set_entry(HashEntry *entries, size_t capacity,
				  const char *key, PT_HashValue value,
				  size_t *p_size);
static bool hash_expand(PT_Hash *table);

// --- API definitions

PT_HashIter pt_hash_iter(PT_Hash *table) {
  PT_HashIter it;
  it.value = (PT_HashValue){0};
  it._p_table = table;
  it._index = 0;
  return it;
}

bool
pt_hash_iter_next(PT_HashIter *it)
{
  PT_Hash *table = it->_p_table;
  while (it->_index < table->capacity) {
    size_t i = it->_index;
    it->_index++;
    if (table->entries[i].key[0] != '\0') {

      /* Found next non-empty bucket, update iterator key and value. */

      HashEntry entry = table->entries[i];
      it->key = entry.key;
      it->value = entry.value;
      return true;
    }
  }
  return false;
}

/*
 * Create a hash table
 */

PT_Hash *
pt_hash_new(const size_t n, ...)
{

  /* Allocate space for hash structure */

  PT_Hash *table = malloc(sizeof(PT_Hash));
  if (table == NULL)
    return NULL;

  table->size = 0;

  /* Allocate a minimal hash */

  table->capacity = n < (INITIAL_CAPACITY / SCALE_FACTOR) ? INITIAL_CAPACITY :
    (n * SCALE_FACTOR / INITIAL_CAPACITY + 1) * INITIAL_CAPACITY;
  table->entries = calloc(table->capacity, sizeof(HashEntry));
  if (table->entries == NULL) {
    free(table);
    return NULL;
  }

  /* Fill hash if any entries provided */

  if (n != 0) {
    va_list ap;
    va_start(ap, n);
    for (size_t i = 0; i < n; ++i) {
      char key[10] = {'\0'};
      strcpy(key, va_arg(ap, char*));
      PT_HashValue value = va_arg(ap, PT_HashValue);
      hash_set_entry(table->entries, table->capacity, key,
		     value, &table->size);
    }
    va_end(ap);
  }

  return table;
}

void
pt_hash_free(PT_Hash *table)
{
  free(table->entries);
  free(table);
}

const size_t
pt_hash_size(PT_Hash *table)
{
  return table->size;
}

/* Probing method */

#define OPEN_ADDRESSING(entries, capacity, ret, ...)		\
  while (entries[index].key[0] != '\0') {			\
    if (strcmp(key, entries[index].key) == 0) {			\
      __VA_ARGS__;						\
      *result = entries[index].value;				\
      return ret;						\
    }								\
    index++;							\
    if (index >= capacity)					\
      index = 0;						\
  }								\
  NULL

bool
pt_hash_hash_search(PT_Hash *table, const char *key, PT_HashValue *result)
{
  uint64_t hash = hash_number(key);
  size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

  /* Loop till an empty entry was found */

  OPEN_ADDRESSING(table->entries, table->capacity, true);

  /* No cookie found */

  return false;
}

const char *
pt_hash_set(PT_Hash *table, const char *key, PT_HashValue value)
{

  /* Expand table if load factor >= 0.5 */

  if (table->size >= table->capacity / SCALE_FACTOR)
    if (!hash_expand(table))
      return NULL;

  /* Set entry and update size */

  return hash_set_entry(table->entries, table->capacity, key, value,
			&table->size);
}

// --- Static definitions

# define FNV_offset_basis UINT64_C(14695981039346656037)
# define FNV_prime UINT64_C(1099511628211)

/**
 * Returns a FNV-1a hash
 * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
 */

static uint64_t
hash_number(const char *key)
{
  uint64_t hash = FNV_offset_basis;

  for (const char *p = &key[0]; *p; p++) {
    hash ^= (uint64_t)(const char)(*p);
    hash *= FNV_prime;
  }

  return hash;
}

/**
 * Helps ```pt_hash_set()``` to set a hash entry
 */

static const char *
hash_set_entry(HashEntry *entries, size_t capacity, const char *key,
	       PT_HashValue value, size_t *p_size)
{
  uint64_t hash = hash_number(key);
  size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

  /* Return if entry already exists; using linear probing */

  PT_HashValue *result;

  OPEN_ADDRESSING(entries, capacity, key, entries[index].value = value);

  strcpy(entries[index].key, key);
  entries[index].value = value;

  /* Return debug insert value */

  return key;
}

/**
 * Expands hash table to twice of it's current capacity
 */

static bool
hash_expand(PT_Hash *table) {

  /* Allocate new entries array */

  size_t new_capacity = table->capacity * GROWTH_FACTOR;
  if (new_capacity < table->capacity)
    return false;

  HashEntry *new_entries = calloc(new_capacity, sizeof (HashEntry));
  if (new_entries == NULL)
    return false;

  /* Initialize entries, move all non-empty to new array */

  for (size_t i = 0; i < table->capacity; i++) {
    HashEntry entry = table->entries[i];
    if (entry.key[0] != '\0')
      hash_set_entry(new_entries, new_capacity, entry.key, entry.value, NULL);
  }

  /* Free old entries array */

  free(table->entries);
  table->entries = new_entries;
  table->capacity = new_capacity;
  return true;
}
