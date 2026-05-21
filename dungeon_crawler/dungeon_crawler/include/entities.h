#pragma once
#include "types.h"

// ============================================================
//  entities.h  –  player, enemy and item logic
// ============================================================

// --- Player ---
void player_init  (Player &p, int startX, int startY);
void player_move  (Player &p, Dir d, const Tile grid[MAP_H][MAP_W]);
void player_pickup(Player &p, Item items[MAX_ITEMS]);
void player_drop  (Player &p, Item items[MAX_ITEMS]);
void player_use   (Player &p);   // use held item (potion)
void player_tick  (Player &p);   // update timers

// --- Enemies ---
void enemy_init(Enemy &e, EnemyType t, int x, int y);
void enemy_tick(Enemy &e,
                const Player &player,
                const Tile grid[MAP_H][MAP_W],
                Enemy enemies[MAX_ENEMIES]);

// Check & resolve player↔enemy collisions (returns damage dealt to player)
int  enemy_resolve_hits(Player &p, Enemy enemies[MAX_ENEMIES]);

// Check & resolve player attack on adjacent enemies
int  player_attack_enemies(Player &p, Enemy enemies[MAX_ENEMIES]);

// --- Items ---
void item_init(Item &it, ItemType t, int x, int y);
Item* item_at(Item items[MAX_ITEMS], int x, int y);   // returns pointer or nullptr

// --- Traps ---
bool trap_check(Player &p, const Tile grid[MAP_H][MAP_W]);   // innovative
