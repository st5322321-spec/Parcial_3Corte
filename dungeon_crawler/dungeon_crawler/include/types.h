#pragma once
#include <cstdint>

// ============================================================
//  Dungeon Crawler – types.h
//  All game-wide constants and plain-old-data types live here.
//  NO dynamic allocation anywhere in this file.
// ============================================================

// ---------- map dimensions ----------
static constexpr int MAP_W  = 40;
static constexpr int MAP_H  = 20;

// ---------- entity limits (static arrays) ----------
static constexpr int MAX_ENEMIES = 16;
static constexpr int MAX_ITEMS   = 8;
static constexpr int MAX_ROOMS   = 10;
static constexpr int MAX_PARTICLES = 32;   // visual fx – innovative feature

// ---------- tile types ----------
enum class Tile : char {
    Empty  = ' ',
    Wall   = '#',
    Door   = '+',
    Floor  = '.',
    Exit   = 'E',
    Trap   = '^'   // innovative: pressure traps
};

// ---------- direction ----------
enum class Dir { None, Up, Down, Left, Right };

// ---------- entity types ----------
enum class EnemyType { Bat, Dragon };

// ---------- item types ----------
enum class ItemType { None, Sword, Key, Potion };

// ---------- simple 2-D position ----------
struct Vec2 {
    int x = 0, y = 0;
};

// ---------- room descriptor ----------
struct Room {
    int x = 0, y = 0, w = 0, h = 0;
    bool active = false;
};

// ---------- player ----------
struct Player {
    Vec2      pos       = {1, 1};
    int       hp        = 10;
    int       maxHp     = 10;
    int       attack    = 2;
    int       score     = 0;
    bool      hasKey    = false;        // used for exit door logic
    ItemType  held      = ItemType::None;  // single-slot inventory
    bool      alive     = true;
    int       lightRadius = 4;         // innovative: fog-of-war radius
    int       invincibleTicks = 0;     // brief iframe after hit
};

// ---------- enemy ----------
struct Enemy {
    Vec2      pos       = {0, 0};
    EnemyType type      = EnemyType::Bat;
    int       hp        = 0;
    int       maxHp     = 0;
    int       attack    = 0;
    int       moveTimer = 0;   // ticks between moves (different per type)
    int       moveTick  = 0;   // countdown
    bool      active    = false;
    bool      alerted   = false;  // innovative: alert state – chase only when aware
    int       alertRange = 0;
};

// ---------- item on the floor ----------
struct Item {
    Vec2     pos    = {0, 0};
    ItemType type   = ItemType::None;
    bool     active = false;
};

// ---------- particle (visual effect) ----------
struct Particle {
    Vec2 pos    = {0, 0};
    char glyph  = '*';
    int  ttl    = 0;   // ticks to live
    bool active = false;
};

// ---------- game state machine ----------
enum class GameState {
    Playing,
    Victory,
    Defeat,
    Paused     // innovative: pause screen
};
