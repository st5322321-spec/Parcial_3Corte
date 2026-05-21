#include "map.h"
#include <cstdlib>   // rand
#include <cstring>   // memset

// ============================================================
//  map.cpp
// ============================================================

void map_init(Tile grid[MAP_H][MAP_W]) {
    // Fill entire grid with walls using pointer arithmetic
    Tile* p = &grid[0][0];
    for (int i = 0; i < MAP_H * MAP_W; ++i, ++p)
        *p = Tile::Wall;
}

// Carve a rectangular room into the grid
static void carve_room(Tile grid[MAP_H][MAP_W], const Room &r) {
    for (int y = r.y; y < r.y + r.h; ++y)
        for (int x = r.x; x < r.x + r.w; ++x)
            grid[y][x] = Tile::Floor;
}

// Carve an L-shaped corridor between two centres
static void carve_corridor(Tile grid[MAP_H][MAP_W], Vec2 a, Vec2 b) {
    // Horizontal then vertical
    int sx = (a.x < b.x) ? 1 : -1;
    int sy = (a.y < b.y) ? 1 : -1;
    for (int x = a.x; x != b.x; x += sx)
        if (x > 0 && x < MAP_W - 1 && a.y > 0 && a.y < MAP_H - 1)
            grid[a.y][x] = Tile::Floor;
    for (int y = a.y; y != b.y; y += sy)
        if (b.x > 0 && b.x < MAP_W - 1 && y > 0 && y < MAP_H - 1)
            grid[y][b.x] = Tile::Floor;
}

void map_place_rooms(Tile grid[MAP_H][MAP_W], Room rooms[MAX_ROOMS], int &roomCount) {
    roomCount = 0;
    // Hard-coded room templates for deterministic but varied layout
    struct RoomDef { int x, y, w, h; };
    static const RoomDef defs[] = {
        { 1,  1,  8,  5},
        {12,  1,  7,  4},
        {22,  1,  8,  5},
        {32,  1,  7,  5},
        { 1,  8,  9,  5},
        {13,  8,  8,  6},
        {24,  8,  7,  5},
        {32,  9,  7,  5},
        { 5, 15,  8,  4},
        {20, 15, 10,  4}
    };
    static const int ROOM_COUNT = 10;
    for (int i = 0; i < ROOM_COUNT && roomCount < MAX_ROOMS; ++i) {
        Room &r = rooms[roomCount];
        r = {defs[i].x, defs[i].y, defs[i].w, defs[i].h, true};
        carve_room(grid, r);
        ++roomCount;
    }
}

void map_connect_rooms(Tile grid[MAP_H][MAP_W], const Room rooms[MAX_ROOMS], int roomCount) {
    // Connect each room to the next with a corridor
    for (int i = 0; i + 1 < roomCount; ++i) {
        const Room *a = &rooms[i];
        const Room *b = &rooms[i + 1];
        Vec2 ca = {a->x + a->w / 2, a->y + a->h / 2};
        Vec2 cb = {b->x + b->w / 2, b->y + b->h / 2};
        carve_corridor(grid, ca, cb);
        // Place a door at the midpoint of the corridor
        int mx = (ca.x + cb.x) / 2;
        int my = (ca.y + cb.y) / 2;
        if (mx > 0 && mx < MAP_W-1 && my > 0 && my < MAP_H-1)
            grid[my][mx] = Tile::Door;
    }
}

void map_place_exit(Tile grid[MAP_H][MAP_W], const Room rooms[MAX_ROOMS], int roomCount) {
    if (roomCount == 0) return;
    // Exit in the last room
    const Room &last = rooms[roomCount - 1];
    int ex = last.x + last.w / 2;
    int ey = last.y + last.h / 2;
    grid[ey][ex] = Tile::Exit;
}

void map_place_traps(Tile grid[MAP_H][MAP_W], const Room rooms[MAX_ROOMS], int roomCount) {
    // Place 1-2 traps in rooms 2..n-1 (skip start and end rooms)
    for (int i = 2; i < roomCount - 1; ++i) {
        const Room &r = rooms[i];
        // Place trap near corner of room
        int tx = r.x + 1;
        int ty = r.y + r.h - 2;
        if (tx < MAP_W && ty >= 0 && ty < MAP_H)
            grid[ty][tx] = Tile::Trap;
    }
}

bool map_in_bounds(int x, int y) {
    return x >= 0 && x < MAP_W && y >= 0 && y < MAP_H;
}

bool map_is_walkable(const Tile grid[MAP_H][MAP_W], int x, int y) {
    if (!map_in_bounds(x, y)) return false;
    Tile t = grid[y][x];
    return t == Tile::Floor || t == Tile::Door || t == Tile::Exit || t == Tile::Trap;
}

Tile* map_tile_at(Tile grid[MAP_H][MAP_W], int x, int y) {
    // Return raw pointer – callers can modify tile directly
    return &grid[y][x];
}
