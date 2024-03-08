#include <hashset.h>

void hashset_init(HashSet *set, int elem_size) {
    set->table = g_hash_table_new(g_direct_hash, g_direct_equal);
}

void hashset_add(HashSet *set, void *elem) {
    g_hash_table_insert(set->table, elem, elem);
}

void hashset_remove(HashSet *set, void *elem) {
    g_hash_table_remove(set->table, elem);
}

int hashset_contains(HashSet *set, void *elem) {
    return g_hash_table_contains(set->table, elem);
}

void hashset_free(HashSet *set) {
    g_hash_table_destroy(set->table);
}

