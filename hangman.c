#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_WORDS   10000
#define WORD_LEN    6       // 5 letters + null terminator
#define MAX_GUESSES 6

// -- Global storage --
char all_words[MAX_WORDS][WORD_LEN];
int  word_count = 0;

char pattern[WORD_LEN]  = "_____";     // current known letters
char guessed[26]       = {0};         // letters AI has tried
char wrong[26]         = {0};         // wrong guesses
int  guessed_count     = 0;
int  wrong_count       = 0;

// -- Helpers --
int is_guessed(char c) {
    for (int i = 0; i < guessed_count; i++)
        if (guessed[i] == c) return 1;
    return 0;
}

int is_win() {
    return strchr(pattern, '_') == NULL;
}

int is_loss() {
    return wrong_count >= MAX_GUESSES;
}

void update_pattern(const char *secret, char guess) {
    for (int i = 0; i < 5; i++)
        if (secret[i] == guess)
            pattern[i] = guess;
}

// Load words from a file (one 5-letter word per line)
void load_words(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open file");
        exit(1);
    }
    
    while (fscanf(f, "%5s", all_words[word_count]) == 1) {
        word_count++;
        if (word_count >= MAX_WORDS) break;
    }
    fclose(f);
}

// Filter out words that don't match `pattern` or contain any `wrong` letters
void filter_words(char possible[][WORD_LEN], int *count) {
    int new_count = 0;
    for (int i = 0; i < *count; i++) {
        int valid = 1;
        // must match known letters
        for (int j = 0; j < 5; j++) {
            if (pattern[j] != '_' && possible[i][j] != pattern[j]) {
                valid = 0;
                break;
            }
        }
        if (!valid) continue;
        // must not contain wrong guesses
        for (int j = 0; j < wrong_count; j++) {
            if (strchr(possible[i], wrong[j])) {
                valid = 0;
                break;
            }
        }
        if (!valid) continue;
        // safe copy even if overlapping
        if (new_count < MAX_WORDS) {
            memmove(possible[new_count], possible[i], WORD_LEN);
            new_count++;
        }
    }
    *count = new_count;
}

// Pick the highest-frequency unguessed letter
char get_best_guess(char possible[][WORD_LEN], int count) {
    int freq[26] = {0};

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 5; j++) {
            char c = possible[i][j];
            if (c >= 'a' && c <= 'z') {
                freq[c - 'a']++;
            }
        }
    }

    // force skip already guessed letters
    for (int i = 0; i < guessed_count; i++) {
        char g = guessed[i];
        if (g >= 'a' && g <= 'z') {
            freq[g - 'a'] = 0;
        }
    }

    int best_i = -1, best_f = -1;
    for (int i = 0; i < 26; i++) {
        if (freq[i] > best_f) {
            best_f = freq[i];
            best_i = i;
        }
    }

    return best_i >= 0 ? 'a' + best_i : '!';
}


int main() {
    // allocate candidate list on heap
    char (*possible)[WORD_LEN] = malloc(sizeof *possible * MAX_WORDS);
    if (!possible) {
        perror("malloc");
        return 1;
    }

    // load from file
    load_words("words.txt");
    if (word_count == 0) {
        fprintf(stderr, "No words loaded. Check words.txt\n");
        free(possible);
        return 1;
    }

    char secret[WORD_LEN];
    printf("Enter the secret 5-letter word (lowercase): ");
    if (scanf("%5s", secret) != 1) {
        printf("Invalid input\n");
        free(possible);
        return 1;
    }
    
    int found = 0;
    for (int i = 0; i < word_count; i++) {
        if (strcmp(secret, all_words[i]) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("Error: The word '%s' is not in the AI's dictionary!\n", secret);
        free(possible);
        return 1;
    }


    while (!is_win() && !is_loss()) {
        // build candidate list
        memcpy(possible, all_words, sizeof(all_words));
        int possible_count = word_count;
        filter_words(possible, &possible_count);

        // AI guess
        char guess = get_best_guess(possible, possible_count);

        // Skip if already guessed or invalid
        if (guess == '?' || is_guessed(guess)) {
            printf("AI has no new letters to guess. Giving up.\n");
            break;
        }

        guessed[guessed_count++] = guess;


        // update
        if (strchr(secret, guess))
            update_pattern(secret, guess);
        else
            wrong[wrong_count++] = guess;

        // show status
        printf("Pattern: %s\n", pattern);
        printf("Wrong (%d): ", wrong_count);
        for (int i = 0; i < wrong_count; i++) printf("%c ", wrong[i]);
        printf("\n");
    }

    if (is_win())
        printf("AI wins! Word: %s\n", secret);
    else
        printf("AI loses! Word was: %s\n", secret);

    free(possible);
    return 0;
}
