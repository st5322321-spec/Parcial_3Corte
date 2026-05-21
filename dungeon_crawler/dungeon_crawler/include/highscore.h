#pragma once

// ============================================================
//  highscore.h  –  persistent high-score via plain text file
//  Innovative feature: score is saved/loaded across sessions
// ============================================================

static constexpr int HS_MAX = 5;

struct ScoreEntry {
    char name[16] = {};
    int  score    = 0;
};

void hs_load  (ScoreEntry table[HS_MAX], const char* path);
void hs_save  (ScoreEntry table[HS_MAX], const char* path, const char* name, int score);
void hs_print (const ScoreEntry table[HS_MAX]);
