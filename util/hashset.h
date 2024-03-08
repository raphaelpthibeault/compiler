#ifndef COMPILER_HASHSET_H
#define COMPILER_HASHSET_H

#include <glib.h>

typedef struct {
    GHashTable *table;
} HashSet;

void hashset_init(HashSet *set, int elem_size);
void hashset_add(HashSet *set, void *elem);
void hashset_remove(HashSet *set, void *elem);
int hashset_contains(HashSet *set, void *elem);
void hashset_free(HashSet *set);

#endif //COMPILER_HASHSET_H
