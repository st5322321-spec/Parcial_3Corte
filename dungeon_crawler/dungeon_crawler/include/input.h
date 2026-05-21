#pragma once
#include "types.h"

// ============================================================
//  input.h  –  non-blocking keyboard input (POSIX termios)
// ============================================================

void input_init ();
void input_restore();
bool input_available();
char input_getchar();

// Parse raw char into game action
struct Action {
    Dir      move    = Dir::None;
    bool     pickup  = false;
    bool     drop    = false;
    bool     use     = false;
    bool     attack  = false;
    bool     pause   = false;
    bool     quit    = false;
};

Action input_parse(char c);
