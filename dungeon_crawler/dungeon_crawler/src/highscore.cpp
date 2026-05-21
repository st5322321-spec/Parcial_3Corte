#include "highscore.h"
#include <cstdio>
#include <cstring>

// ============================================================
//  highscore.cpp  –  read/write plain-text score file
// ============================================================

static void hs_clear(ScoreEntry table[HS_MAX]) {
    for (int i = 0; i < HS_MAX; ++i) {
        table[i].score = 0;
        table[i].name[0] = '\0';
    }
}

void hs_load(ScoreEntry table[HS_MAX], const char* path) {
    hs_clear(table);
    FILE *f = fopen(path, "r");
    if (!f) return;
    for (int i = 0; i < HS_MAX; ++i) {
        if (fscanf(f, "%15s %d\n", table[i].name, &table[i].score) != 2) break;
    }
    fclose(f);
}

// Insert a new entry and keep sorted descending
void hs_save(ScoreEntry table[HS_MAX], const char* path, const char* name, int score) {
    // Find insertion point
    int pos = HS_MAX;  // default: doesn't make the list
    for (int i = 0; i < HS_MAX; ++i) {
        if (score > table[i].score) { pos = i; break; }
    }
    if (pos == HS_MAX) { /* score too low, skip */ }
    else {
        // Shift down
        for (int i = HS_MAX - 1; i > pos; --i)
            table[i] = table[i-1];
        strncpy(table[pos].name, name, 15);
        table[pos].name[15] = '\0';
        table[pos].score = score;
    }
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < HS_MAX; ++i) {
        if (table[i].name[0] == '\0') break;
        fprintf(f, "%s %d\n", table[i].name, table[i].score);
    }
    fclose(f);
}

void hs_print(const ScoreEntry table[HS_MAX]) {
    printf("\033[1;33m=== HIGH SCORES ===\033[0m\n");
    for (int i = 0; i < HS_MAX; ++i) {
        if (table[i].name[0] == '\0') break;
        printf(" %d. %-15s  %d\n", i+1, table[i].name, table[i].score);
    }
    printf("\n");
}
