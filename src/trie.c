#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "trie.h"

#define CHILD_COUNT (UINT8_MAX + 1)

typedef struct trie_t
{
    bool accepts;
    uint32_t value;
    struct trie_t *children[CHILD_COUNT];
} trie_t;

trie_t *trie_make()
{
    trie_t *t = calloc(1, sizeof(trie_t));
    return t;
}

void trie_destroy(trie_t *t)
{
    if (t == NULL)
        return;

    for (int i = 0; i < CHILD_COUNT; i++)
        trie_destroy(t->children[i]);
    free(t);
}

void trie_add_char(trie_t *t, char c)
{
    if (t->children[(uint8_t)c] == NULL)
        t->children[(uint8_t)c] = trie_make();
}

void trie_add_word(trie_t *t, const char *word)
{
    while (word[0] != '\0')
    {
        uint8_t c = word[0];
        trie_add_char(t, c);
        t = t->children[c];
        word++;
    }

    t->accepts = true;
}

trie_t *trie_search_word(trie_t *t, const char *word)
{
    while (word[0] != '\0')
    {
        uint8_t c = word[0];
        if (t->children[c] == NULL)
            return NULL;
        t = t->children[c];
        word++;
    }

    return t->accepts ? t : NULL;
}

int trie_depth(trie_t *t)
{
    if (t == NULL)
        return 0;

    int child_max_depth = 0;
    for (int i = 0; i < CHILD_COUNT; i++)
    {
        int d = trie_depth(t->children[i]);
        if (d > child_max_depth)
            child_max_depth = d;
    }

    return 1 + child_max_depth;
}

bool trie_enum_language(
    trie_t *t, int cursor, char *prefix,
    bool (*action)(const char *word, uint32_t *value))
{
    if (t == NULL)
        return false;

    if (t->accepts)
        if (action(prefix, &t->value))
            return true;

    bool broken = false;
    for (int i = 0; i < CHILD_COUNT; i++)
    {
        prefix[cursor] = (uint8_t)i;
        broken = trie_enum_language(t->children[i], cursor + 1, prefix, action);
        if (broken)
            break;
    }
    prefix[cursor] = '\0';

    return broken;
}

bool trie_iter_language(
    trie_t *t,
    bool (*action)(const char *word, uint32_t *value))
{
    if (action == NULL)
        return false;

    int depth = trie_depth(t);
    char *prefix = calloc(depth, sizeof(char));
    bool result = trie_enum_language(t, 0, prefix, action);
    free(prefix);
    return result;
}
