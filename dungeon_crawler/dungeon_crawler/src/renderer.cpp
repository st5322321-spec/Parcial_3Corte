#include "renderer.h"
#include <cstdio>
#include <cstring>
#include <cmath>

// ============================================================
//  renderer.cpp  –  ANSI terminal renderer
//  Innovative: fog-of-war via circular light radius
// ============================================================

// ANSI colour helpers
#define COL_RESET   "\033[0m"
#define COL_WALL    "\033[90m"      // dark grey
#define COL_FLOOR   "\033[37m"      // light grey
#define COL_PLAYER  "\033[1;32m"    // bright green
#define COL_BAT     "\033[1;35m"    // magenta
#define COL_DRAGON  "\033[1;31m"    // bright red
#define COL_ITEM    "\033[1;33m"    // yellow
#define COL_EXIT    "\033[1;36m"    // cyan
#define COL_TRAP    "\033[31m"      // red
#define COL_DOOR    "\033[33m"      // orange-ish
#define COL_FOG     "\033[34m"      // blue (dim fog)
#define COL_PART    "\033[1;37m"    // white spark
#define COL_HP_FULL "\033[32m"
#define COL_HP_LOW  "\033[31m"

// Clear screen + move cursor home
static void clear_screen() { fputs("\033[H\033[J", stdout); }

// Fog-of-war: simple Euclidean distance check
bool is_visible(const Player &player, int x, int y) {
    int dx = x - player.pos.x;
    int dy = y - player.pos.y;
    return (dx * dx + dy * dy) <= (player.lightRadius * player.lightRadius);
}

// Particle helpers
void particle_spawn(Particle particles[MAX_PARTICLES], int x, int y, char glyph, int ttl) {
    Particle *p = particles;
    for (int i = 0; i < MAX_PARTICLES; ++i, ++p) {
        if (!p->active) {
            p->pos    = {x, y};
            p->glyph  = glyph;
            p->ttl    = ttl;
            p->active = true;
            return;
        }
    }
}

void particle_tick(Particle particles[MAX_PARTICLES]) {
    Particle *p = particles;
    for (int i = 0; i < MAX_PARTICLES; ++i, ++p)
        if (p->active && --p->ttl <= 0) p->active = false;
}

// Build a particle lookup table for O(1) glyph query
static char particle_at(const Particle particles[MAX_PARTICLES], int x, int y) {
    const Particle *p = particles;
    for (int i = 0; i < MAX_PARTICLES; ++i, ++p)
        if (p->active && p->pos.x == x && p->pos.y == y) return p->glyph;
    return 0;
}

static char enemy_glyph(EnemyType t) {
    return (t == EnemyType::Bat) ? 'b' : 'D';
}

void render_frame(
    const Tile   grid[MAP_H][MAP_W],
    const Player &player,
    const Enemy  enemies[MAX_ENEMIES],
    const Item   items[MAX_ITEMS],
    const Particle particles[MAX_PARTICLES],
    GameState    /*state*/,
    int          tick)
{
    clear_screen();

    // Title bar
    printf("\033[1;36m=== DUNGEON CRAWLER  [tick %04d] ===\033[0m\n", tick);

    for (int y = 0; y < MAP_H; ++y) {
        for (int x = 0; x < MAP_W; ++x) {

            bool visible = is_visible(player, x, y);

            // Player
            if (player.pos.x == x && player.pos.y == y) {
                printf(COL_PLAYER "@" COL_RESET);
                continue;
            }

            if (!visible) {
                // Outside light radius: show dimmed fog
                printf(COL_FOG "░" COL_RESET);
                continue;
            }

            // Particle effect takes priority
            char pc = particle_at(particles, x, y);
            if (pc) { printf(COL_PART "%c" COL_RESET, pc); continue; }

            // Enemies
            bool drawn = false;
            const Enemy *ep = enemies;
            for (int i = 0; i < MAX_ENEMIES; ++i, ++ep) {
                if (ep->active && ep->pos.x == x && ep->pos.y == y) {
                    const char *col = (ep->type == EnemyType::Dragon) ? COL_DRAGON : COL_BAT;
                    printf("%s%c" COL_RESET, col, enemy_glyph(ep->type));
                    drawn = true;
                    break;
                }
            }
            if (drawn) continue;

            // Items
            const Item *it = items;
            for (int i = 0; i < MAX_ITEMS; ++i, ++it) {
                if (it->active && it->pos.x == x && it->pos.y == y) {
                    char g = '?';
                    switch (it->type) {
                        case ItemType::Sword:  g = '/'; break;
                        case ItemType::Key:    g = 'k'; break;
                        case ItemType::Potion: g = 'p'; break;
                        default: break;
                    }
                    printf(COL_ITEM "%c" COL_RESET, g);
                    drawn = true;
                    break;
                }
            }
            if (drawn) continue;

            // Tiles
            Tile t = grid[y][x];
            switch (t) {
                case Tile::Wall:  printf(COL_WALL  "#" COL_RESET); break;
                case Tile::Floor: printf(COL_FLOOR "." COL_RESET); break;
                case Tile::Door:  printf(COL_DOOR  "+" COL_RESET); break;
                case Tile::Exit:  printf(COL_EXIT  "E" COL_RESET); break;
                case Tile::Trap:  printf(COL_TRAP  "^" COL_RESET); break;
                default:          printf(" ");                       break;
            }
        }
        printf("\n");
    }
}

void render_hud(const Player &p, int /*tick*/, int enemiesLeft) {
    // HP bar
    int barLen = 10;
    int filled = (p.hp * barLen) / p.maxHp;
    const char *hpCol = (p.hp > p.maxHp / 3) ? COL_HP_FULL : COL_HP_LOW;
    printf("\n%sHP[", hpCol);
    for (int i = 0; i < barLen; ++i) printf(i < filled ? "█" : "░");
    printf("] %d/%d" COL_RESET, p.hp, p.maxHp);

    // Inventory
    const char *heldName = "nothing";
    switch (p.held) {
        case ItemType::Sword:  heldName = "Sword";  break;
        case ItemType::Key:    heldName = "Key";    break;
        case ItemType::Potion: heldName = "Potion"; break;
        default: break;
    }
    printf("  \033[1;33mInventory: [%s]\033[0m", heldName);
    printf("  \033[1;36mScore: %d\033[0m", p.score);
    printf("  Enemies left: %d\n", enemiesLeft);

    // Controls
    printf("\033[90mMove: WASD | Attack: f | Pick: e | Drop: r | Use: u | Pause: p | Quit: q\033[0m\n");
}

void render_gameover(bool victory, int score) {
    clear_screen();
    if (victory) {
        printf("\033[1;32m");
        printf("  ╔══════════════════════════╗\n");
        printf("  ║   YOU ESCAPED THE DUNGEON  ║\n");
        printf("  ╚══════════════════════════╝\n");
    } else {
        printf("\033[1;31m");
        printf("  ╔══════════════════════════╗\n");
        printf("  ║     YOU HAVE DIED...       ║\n");
        printf("  ╚══════════════════════════╝\n");
    }
    printf(COL_RESET "  Final score: \033[1;33m%d\033[0m\n\n", score);
}

void render_pause() {
    printf("\033[1;33m\n  [ PAUSED – press p to resume ]\033[0m\n");
}
