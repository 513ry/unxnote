/** hash.h
 * Copyright (c) 2024, Daniel Sierpiński All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - All advertising materials mentioning features or use of this software must
 *   display the following acknowledgement: This product includes software
 *   developed by the Daniel Sierpiński.
 * - Neither the name of the Daniel Sierpiński nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DANIEL SIERPIŃSKI AS IS AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL DANIEL SIERPIŃSKI BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * DESCRIPTION:
 * Unencrypted and fast scatter-storage addressing structure. Collisions are
 * resolved by linear probing, the older of the two colliding items, similar to
 * cash.
 *
 * PT_Hash extended to store user defined types
 */

#ifndef _PT_HASH_H
#define _PT_HASH_H

#include "unxnote/common.h"
#include "unxnote/hash_config.h"

/* The user should define HASH_UNION_TYPE before including this header */

#if !defined(HASH_UNION_TYPE)
# error "You must define DATA_UNION_TYPE before including data_struct.h"
#endif

typedef HASH_UNION_TYPE PT_HashValue;
typedef struct PT_Hash PT_Hash;

typedef struct {
  const char *key;
  PT_HashValue value;

  PT_Hash *_p_table;
  size_t _index;
} PT_HashIter;

/**
 * Returns a linear iterator
 * @param table - Hash table
 * @return Iterator of type PT_HashIter
 */
PT_HashIter pt_hash_iter(PT_Hash *table);

/**
 * Next iter element
 * @param it - Iterator of type PT_HashIter
 */
bool pt_hash_iter_next(PT_HashIter *it);

/**
 * Parse a file and create new hash
 * @param path - File path
 */
PT_Hash *pt_hash_from_file(const char *path);

/**
 * Initialize new hash
 */
PT_Hash *pt_hash_new(const size_t n, ...);

/**
 * Free hash memory
 * @param table - Hash table
 */
void pt_hash_free(PT_Hash *table);

/**
 * Number of entries in hash table
 * @param table - Hash table
 * @return size_t 
 */
const size_t pt_hash_size(PT_Hash *table);

/**
 * Sets keys and value of a hashes
 * @param table - Hash table
 * @param key - Hash key
 * @param value - Hash value
 * @return Constant key char *
 */
const char *pt_hash_set(PT_Hash *table, const char *key, PT_HashValue value);

/* Linear search by key --- UNDEFINED --- */

bool pt_hash_linear_search(PT_Hash *table, const char *key, PT_HashValue *result);

/* Hash search */

bool pt_hash_hash_search(PT_Hash *table, const char *key, PT_HashValue *result);

/* Choose the right search function */

#define pt_hash_get(table, key, result) pt_hash_hash_search(table, key, result)
#define pt_hash_search_by_key(table, key) pt_hash_search(table, key)

#endif // _PT_HASH_H
