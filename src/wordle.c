#include <string.h>
#include "wordle.h"

wordle_encoded_t wordle_encode(wordle_decoded_t row)
{
    return row[0] * 1 + row[1] * 3 + row[2] * 9 + row[3] * 27 + row[4] * 81;
}

void wordle_decode(wordle_decoded_t dst, wordle_encoded_t code)
{
    for (int i = 0; i < 5; i++)
    {
        dst[i] = code % 3;
        code /= 3;
    }
}

void wordle_word_copy(wordle_word_t dst, const wordle_word_t src)
{
    strncpy(dst, src, WORDLE_WIDTH);
}

wordle_encoded_t wordle_play(
    const wordle_word_t guess,
    const wordle_word_t answer)
{
    wordle_word_t remain;
    wordle_word_copy(remain, answer);

    wordle_decoded_t row = {0};
    for (int i = 0; i < WORDLE_WIDTH; i++)
    {
        if (guess[i] == remain[i])
        {
            remain[i] = '\0';
            row[i] = CELL_GREEN;
            continue;
        }
    }

    for (int i = 0; i < WORDLE_WIDTH; i++)
    {
        for (int j = 0; j < WORDLE_WIDTH; j++)
        {
            if (remain[j] == guess[i])
            {
                remain[j] = '\0';
                row[i] = CELL_YELLOW;
                break;
            }
        }
    }

    return wordle_encode(row);
}
