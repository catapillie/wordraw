#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>

#include "trie.h"
#include "wordle.h"

trie_t *read_word_list(const char *path)
{
    FILE *f = fopen(path, "r");

    char word[6];
    word[5] = '\0';

    trie_t *words = trie_make();
    while (fscanf(f, "%5s\n", word) == 1)
        trie_add_word(words, word);

    fclose(f);
    return words;
}

static Texture font;
static Texture icons[4];
static Texture icon_x;

void draw_char(char c, int x, int y, int scale, Color color)
{
    if (c < 'a' || c > 'z')
        return;

    int i = (int)(c - 'a');
    Rectangle source = {.x = i * 5, .y = 0, .width = 4, .height = 9};
    Rectangle dest = {.x = x, .y = y, .width = 4 * scale, .height = 9 * scale};
    DrawTexturePro(font, source, dest, (Vector2){0, 0}, 0.0, color);
}

static trie_t *answers = NULL;
static trie_t *guesses = NULL;

static char *answer;

bool populate_wordle_plays(const char *word, uint32_t *row)
{
    *row = wordle_play(word, answer);
    return false;
}

static wordle_encoded_t current_wanted_row;
static char *current_out_solution;
bool check_word_play(const char *word, uint32_t *row)
{
    if (*row == current_wanted_row)
    {
        strncpy(current_out_solution, word, 5);
        return true;
    }

    return false;
}

wordle_decoded_t board[WORDLE_HEIGHT] = {0};
char solution[WORDLE_HEIGHT][WORDLE_WIDTH] = {0};
bool success[WORDLE_HEIGHT] = {false};

void solve_row(int y)
{
    current_wanted_row = wordle_encode(board[y]);
    current_out_solution = solution[y];
    success[y] = trie_iter_language(guesses, check_word_play);
}

void solve()
{
    for (int y = 0; y < WORDLE_HEIGHT; y++)
        solve_row(y);
}

static trie_t *matching_guesses = NULL;
static bool found_matching_guess = false;
bool find_matching_pattern_with_answer(const char *guess, uint32_t *)
{
    if (wordle_play(guess, answer) == current_wanted_row)
    {
        trie_add_word(matching_guesses, guess);
        found_matching_guess = true;
    }
    return false;
}

bool log_trie(const char *word, uint32_t *)
{
    printf(" %s", word);
    return false;
}

bool log_answer_matching_pattern(const char *word, uint32_t *)
{
    answer = word;
    matching_guesses = trie_make();
    found_matching_guess = false;
    trie_iter_language(guesses, find_matching_pattern_with_answer);
    if (found_matching_guess)
    {
        printf("%s ->>", word);
        trie_iter_language(matching_guesses, log_trie);
        printf("\n");
    }
    trie_destroy(matching_guesses);
    return false;
}

int solve_pattern(const char pattern[])
{
    if (strlen(pattern) != 5)
    {
        fprintf(stderr, "ERROR: pattern must be five letters long; got '%s'.\n", pattern);
        return EXIT_FAILURE;
    }

    wordle_decoded_t row;
    for (int i = 0; i < WORDLE_WIDTH; i++)
    {
        switch (pattern[i])
        {
        case 'o':
            row[i] = CELL_BLACK;
            break;
        case 'y':
            row[i] = CELL_YELLOW;
            break;
        case 'g':
            row[i] = CELL_GREEN;
            break;
        default:
            fprintf(stderr, "ERROR: invalid pattern character '%c'.\n", pattern[i]);
            fprintf(stderr, "       pattern must only contain 'o', 'y' or 'g'; got '%s'.\n", pattern);
            return EXIT_FAILURE;
        }
    }

    current_wanted_row = wordle_encode(row);
    trie_iter_language(answers, log_answer_matching_pattern);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    answers = read_word_list("assets/answers.txt");
    guesses = read_word_list("assets/guesses.txt");

    if (argc != 2)
    {
        fprintf(stderr, "USAGE:\n");
        fprintf(stderr, "        ./a.out <answer>\n");
        fprintf(stderr, "        ./a.out ?<pattern>\n");
        return EXIT_FAILURE;
    }

    if (argv[1][0] == '?')
        return solve_pattern(argv[1] + 1);

    answer = argv[1];
    if (strlen(answer) != 5)
    {
        fprintf(stderr, "ERROR: answer must be a 5-letter word; got '%s'.\n", answer);
        return EXIT_FAILURE;
    }

    if (trie_search_word(answers, answer) == NULL)
    {
        fprintf(stderr, "ERROR: '%s' is not a valid Wordle answer.\n", answer);
        return EXIT_FAILURE;
    }

    trie_iter_language(guesses, populate_wordle_plays);
    solve();

    const int scale = 5;
    const int width = 16 * scale * 6;
    const int height = 16 * scale * 6;
    char title[50];
    sprintf(title, "Wordraw: draw on wordle [%s]", answer);
    SetTraceLogLevel(LOG_NONE);
    InitWindow(width, height, title);

    font = LoadTexture("assets/img/font.png");
    icons[CELL_BLACK] = LoadTexture("assets/img/black.png");
    icons[CELL_YELLOW] = LoadTexture("assets/img/yellow.png");
    icons[CELL_GREEN] = LoadTexture("assets/img/green.png");
    icons[3] = LoadTexture("assets/img/mask.png");
    icon_x = LoadTexture("assets/img/x.png");
    float cell_width = icons[0].width;
    float cell_height = icons[0].height;

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();

        int cx = (int)(mouse.x / (scale * cell_width));
        int cy = (int)(mouse.y / (scale * cell_height));
        int changed_row = -1;
        if ((0 <= cx && cx < WORDLE_WIDTH) && (0 <= cy && cy < WORDLE_HEIGHT))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                board[cy][cx] = (board[cy][cx] + 1) % 3;
                changed_row = cy;
            }
        }

        if (changed_row >= 0)
            solve_row(changed_row);

        BeginDrawing();
        {
            ClearBackground(BLACK);

            for (int y = 0; y < WORDLE_HEIGHT; y++)
            {

                if (!success[y])
                {
                    DrawTextureEx(
                        icon_x,
                        (Vector2){
                            .x = cell_width * scale * WORDLE_WIDTH,
                            .y = cell_height * scale * y,
                        },
                        0.0, scale, WHITE);
                }

                for (int x = 0; x < WORDLE_WIDTH; x++)
                {
                    Texture icon = icons[board[y][x]];
                    Vector2 topleft = {.x = scale * cell_width * x, .y = scale * cell_height * y};
                    Rectangle bounds = {
                        .x = topleft.x,
                        .y = topleft.y,
                        .width = scale * cell_width,
                        .height = scale * cell_height,
                    };
                    DrawTextureEx(icon, topleft, 0.0, scale, WHITE);
                    if (CheckCollisionPointRec(mouse, bounds))
                        DrawTextureEx(icons[3], topleft, 0.0, scale, ColorAlpha(WHITE, 0.25));

                    if (success[y])
                    {
                        draw_char(solution[y][x],
                                  scale * (6 + cell_width * x),
                                  scale * (4 + cell_height * y),
                                  scale, ColorAlpha(BLACK, 0.35));
                        draw_char(solution[y][x],
                                  scale * (6 + cell_width * x),
                                  scale * (3 + cell_height * y),
                                  scale, WHITE);
                    }
                }
            }
        }
        EndDrawing();
    }
    CloseWindow();

    trie_destroy(guesses);
    trie_destroy(answers);

    return EXIT_SUCCESS;
}
