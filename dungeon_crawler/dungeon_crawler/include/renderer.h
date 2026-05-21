#pragma once
#include "types.h"

// ============================================================
//  renderer.h  –  terminal rendering with fog-of-war
//  Innovative feature: distance-based visibility (light radius)
// ============================================================

void render_frame(
    const Tile   grid[MAP_H][MAP_W],
    const Player &player,
    const Enemy  enemies[MAX_ENEMIES],
    const Item   items[MAX_ITEMS],
    const Particle particles[MAX_PARTICLES],
    GameState    state,
    int          tick
);

void render_hud (const Player &p, int tick, int enemiesLeft);
void render_gameover(bool victory, int score);
void render_pause();

// Spawn a particle effect at position (used by combat)
void particle_spawn(Particle particles[MAX_PARTICLES], int x, int y, char glyph, int ttl);
void particle_tick (Particle particles[MAX_PARTICLES]);

// Fog-of-war visibility check (innovative)
bool is_visible(const Player &player, int x, int y);
