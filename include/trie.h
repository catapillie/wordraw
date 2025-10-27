#ifndef TRIE_H
#define TRIE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct trie_t trie_t;

trie_t *trie_make();
void trie_destroy(trie_t *t);
void trie_add_word(trie_t *t, const char *word);
trie_t *trie_search_word(trie_t *t, const char *word);
bool trie_iter_language(trie_t *t, bool (*action)(const char *word, uint32_t *value));

#endif /* TRIE_H */
