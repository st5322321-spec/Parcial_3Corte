#pragma once
#include "types.h"

// ============================================================
//  map.h  –  map grid, room placement, tile queries
// ============================================================

// The map is a flat static array of Tile values.
// Pointer arithmetic is used throughout for CPU-cache friendliness.

void map_init  (Tile grid[MAP_H][MAP_W]);
void map_place_rooms(Tile grid[MAP_H][MAP_W], Room rooms[MAX_ROOMS], int &roomCount);
void map_connect_rooms(Tile grid[MAP_H][MAP_W], const Room rooms[MAX_ROOMS], int roomCount);
void map_place_exit(Tile grid[MAP_H][MAP_W], const Room rooms[MAX_ROOMS], int roomCount);
void map_place_traps(Tile grid[MAP_H][MAP_W], const Room rooms[MAX_ROOMS], int roomCount);

bool map_is_walkable(const Tile grid[MAP_H][MAP_W], int x, int y);
bool map_in_bounds  (int x, int y);

// Returns pointer to tile for direct manipulation
Tile* map_tile_at(Tile grid[MAP_H][MAP_W], int x, int y);
