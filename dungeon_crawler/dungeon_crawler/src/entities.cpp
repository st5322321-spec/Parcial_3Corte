#include "entities.h"
#include "map.h"
#include <cstdlib>  // abs

// ============================================================
//  entities.cpp
// ============================================================

// ---- helpers -----------------------------------------------
static int sign(int v) { return (v > 0) - (v < 0); }

static bool pos_eq(Vec2 a, Vec2 b) { return a.x == b.x && a.y == b.y; }

static int manhattan(Vec2 a, Vec2 b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

// Check if position is occupied by another enemy
static bool enemy_at(const Enemy enemies[MAX_ENEMIES], int x, int y, int skipIdx) {
    const Enemy *e = enemies;
    for (int i = 0; i < MAX_ENEMIES; ++i, ++e)
        if (e->active && i != skipIdx && e->pos.x == x && e->pos.y == y)
            return true;
    return false;
}

// ---- Player ------------------------------------------------
void player_init(Player &p, int startX, int startY) {
    p.pos          = {startX, startY};
    p.hp           = 10;
    p.maxHp        = 10;
    p.attack       = 2;
    p.score        = 0;
    p.hasKey       = false;
    p.held         = ItemType::None;
    p.alive        = true;
    p.lightRadius  = 5;   // fog-of-war radius
    p.invincibleTicks = 0;
}

void player_move(Player &p, Dir d, const Tile grid[MAP_H][MAP_W]) {
    int nx = p.pos.x, ny = p.pos.y;
    switch (d) {
        case Dir::Up:    --ny; break;
        case Dir::Down:  ++ny; break;
        case Dir::Left:  --nx; break;
        case Dir::Right: ++nx; break;
        default: return;
    }
    if (map_is_walkable(grid, nx, ny))
        p.pos = {nx, ny};
}

void player_pickup(Player &p, Item items[MAX_ITEMS]) {
    if (p.held != ItemType::None) return;  // inventory full (single slot)
    Item *it = item_at(items, p.pos.x, p.pos.y);
    if (it) {
        p.held    = it->type;
        it->active = false;
        if (it->type == ItemType::Key) p.hasKey = true;
        p.score += 10;
    }
}

void player_drop(Player &p, Item items[MAX_ITEMS]) {
    if (p.held == ItemType::None) return;
    // Find free item slot
    Item *slot = items;
    for (int i = 0; i < MAX_ITEMS; ++i, ++slot) {
        if (!slot->active) {
            slot->active = true;
            slot->type   = p.held;
            slot->pos    = p.pos;
            p.held       = ItemType::None;
            if (slot->type == ItemType::Key) p.hasKey = false;
            return;
        }
    }
}

void player_use(Player &p) {
    if (p.held == ItemType::Potion) {
        p.hp = p.maxHp;    // full heal
        p.held = ItemType::None;
        p.score += 5;
    }
    if (p.held == ItemType::Sword) {
        p.attack = 4;      // upgrade attack permanently
        p.held   = ItemType::None;
    }
}

void player_tick(Player &p) {
    if (p.invincibleTicks > 0) --p.invincibleTicks;
}

// ---- Enemies -----------------------------------------------
void enemy_init(Enemy &e, EnemyType t, int x, int y) {
    e.pos    = {x, y};
    e.type   = t;
    e.active = true;
    e.alerted = false;
    if (t == EnemyType::Bat) {
        e.hp = e.maxHp = 3;
        e.attack    = 1;
        e.moveTimer = 2;   // fast
        e.alertRange = 6;
    } else {  // Dragon
        e.hp = e.maxHp = 8;
        e.attack    = 3;
        e.moveTimer = 4;   // slow but strong
        e.alertRange = 8;
    }
    e.moveTick = e.moveTimer;
}

void enemy_tick(Enemy &e,
                const Player &player,
                const Tile grid[MAP_H][MAP_W],
                Enemy enemies[MAX_ENEMIES])
{
    if (!e.active) return;

    // Alert check (innovative: enemies only chase when aware of player)
    int dist = manhattan(e.pos, player.pos);
    if (dist <= e.alertRange) e.alerted = true;

    if (--e.moveTick > 0) return;
    e.moveTick = e.moveTimer;

    if (!e.alerted) return;   // idle until alerted

    // Simple chase: move along axis with greater distance first
    int dx = player.pos.x - e.pos.x;
    int dy = player.pos.y - e.pos.y;

    int sx = sign(dx), sy = sign(dy);
    int nx = e.pos.x, ny = e.pos.y;

    if (abs(dx) >= abs(dy) && map_is_walkable(grid, e.pos.x + sx, e.pos.y))
        nx = e.pos.x + sx;
    else if (abs(dy) > 0 && map_is_walkable(grid, e.pos.x, e.pos.y + sy))
        ny = e.pos.y + sy;
    else if (map_is_walkable(grid, e.pos.x + sx, e.pos.y))
        nx = e.pos.x + sx;

    // Don't stack on another enemy
    int idx = (int)(&e - enemies);  // pointer arithmetic to get index
    if (!enemy_at(enemies, nx, ny, idx) && !(nx == player.pos.x && ny == player.pos.y)) {
        e.pos = {nx, ny};
    }
}

int enemy_resolve_hits(Player &p, Enemy enemies[MAX_ENEMIES]) {
    if (p.invincibleTicks > 0) return 0;
    int dmg = 0;
    Enemy *e = enemies;
    for (int i = 0; i < MAX_ENEMIES; ++i, ++e) {
        if (!e->active) continue;
        if (pos_eq(e->pos, p.pos)) {
            dmg += e->attack;
        }
    }
    if (dmg > 0) {
        p.hp -= dmg;
        p.invincibleTicks = 8;   // brief invincibility window
        if (p.hp <= 0) { p.hp = 0; p.alive = false; }
    }
    return dmg;
}

int player_attack_enemies(Player &p, Enemy enemies[MAX_ENEMIES]) {
    int killed = 0;
    Enemy *e = enemies;
    // Attack all enemies adjacent (8-directional) to player
    for (int i = 0; i < MAX_ENEMIES; ++i, ++e) {
        if (!e->active) continue;
        if (abs(e->pos.x - p.pos.x) <= 1 && abs(e->pos.y - p.pos.y) <= 1) {
            e->hp -= p.attack;
            if (e->hp <= 0) {
                e->active = false;
                p.score += (e->type == EnemyType::Dragon) ? 50 : 20;
                ++killed;
            }
        }
    }
    return killed;
}

// ---- Items --------------------------------------------------
void item_init(Item &it, ItemType t, int x, int y) {
    it.pos    = {x, y};
    it.type   = t;
    it.active = true;
}

Item* item_at(Item items[MAX_ITEMS], int x, int y) {
    Item *it = items;
    for (int i = 0; i < MAX_ITEMS; ++i, ++it)
        if (it->active && it->pos.x == x && it->pos.y == y)
            return it;
    return nullptr;
}

// ---- Traps (innovative) ------------------------------------
bool trap_check(Player &p, const Tile grid[MAP_H][MAP_W]) {
    if (grid[p.pos.y][p.pos.x] == Tile::Trap) {
        if (p.invincibleTicks == 0) {
            p.hp -= 2;
            p.invincibleTicks = 12;
            if (p.hp <= 0) { p.hp = 0; p.alive = false; }
            return true;
        }
    }
    return false;
}
