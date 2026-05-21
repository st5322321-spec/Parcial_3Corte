#include "input.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

// ============================================================
//  input.cpp  –  raw non-blocking terminal input
// ============================================================

static struct termios s_orig;

void input_init() {
    tcgetattr(STDIN_FILENO, &s_orig);
    struct termios raw = s_orig;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    // Make stdin non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void input_restore() {
    tcsetattr(STDIN_FILENO, TCSANOW, &s_orig);
    // Restore blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

bool input_available() {
    char buf;
    return read(STDIN_FILENO, &buf, 1) == 1
           ? (ungetc(buf, stdin), true) : false;
}

char input_getchar() {
    char c = 0;
    read(STDIN_FILENO, &c, 1);
    return c;
}

Action input_parse(char c) {
    Action a;
    switch (c) {
        case 'w': case 'W': a.move   = Dir::Up;    break;
        case 's': case 'S': a.move   = Dir::Down;  break;
        case 'a': case 'A': a.move   = Dir::Left;  break;
        case 'd': case 'D': a.move   = Dir::Right; break;
        case 'e': case 'E': a.pickup = true;        break;
        case 'r': case 'R': a.drop   = true;        break;
        case 'u': case 'U': a.use    = true;        break;
        case 'f': case 'F': a.attack = true;        break;
        case 'p': case 'P': a.pause  = true;        break;
        case 'q': case 'Q': a.quit   = true;        break;
        default: break;
    }
    return a;
}
