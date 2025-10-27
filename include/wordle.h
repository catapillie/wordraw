#ifndef WORDLE_H
#define WORDLE_H

#include <stdint.h>

#define WORDLE_WIDTH 5
#define WORDLE_HEIGHT 6

typedef enum
{
    CELL_BLACK,
    CELL_YELLOW,
    CELL_GREEN,
} wordle_cell_t;

typedef wordle_cell_t wordle_decoded_t[WORDLE_WIDTH];
typedef uint8_t wordle_encoded_t;

typedef char wordle_word_t[WORDLE_WIDTH];
void wordle_word_copy(wordle_word_t dst, const wordle_word_t src);

wordle_encoded_t wordle_encode(wordle_decoded_t);
void wordle_decode(wordle_decoded_t dst, wordle_encoded_t code);
wordle_encoded_t wordle_play(const wordle_word_t guess, const wordle_word_t answer);

#endif /* WORDLE_H */