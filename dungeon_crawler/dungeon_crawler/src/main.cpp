#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>   // usleep

#include "types.h"
#include "map.h"
#include "entities.h"
#include "renderer.h"
#include "input.h"
#include "highscore.h"

// ============================================================
//  main.cpp  –  game loop
//
//  Innovative features implemented:
//    1. Fog-of-war (circular light radius around player)
//    2. Enemy alert system (enemies only chase once they spot you)
//    3. Pressure traps on the map floor
//    4. Particle effects for combat hits
//    5. Persistent high-score table (saved to disk)
//    6. Pause screen
//    7. Potion & sword items with actual stat effects
// ============================================================

static constexpr int TICK_US = 100000;  // 100 ms per tick (10 FPS)

// ---- static game storage (NO heap allocation) ---------------
static Tile     s_grid   [MAP_H][MAP_W];
static Room     s_rooms  [MAX_ROOMS];
static Enemy    s_enemies[MAX_ENEMIES];
static Item     s_items  [MAX_ITEMS];
static Particle s_parts  [MAX_PARTICLES];
static Player   s_player;
static ScoreEntry s_scores[HS_MAX];
static int      s_roomCount = 0;

// Count active enemies
static int count_enemies() {
    int n = 0;
    const Enemy *e = s_enemies;
    for (int i = 0; i < MAX_ENEMIES; ++i, ++e)
        n += e->active ? 1 : 0;
    return n;
}

// Check victory condition: player on Exit tile with no enemies OR has key
static bool check_victory() {
    Tile *t = map_tile_at(s_grid, s_player.pos.x, s_player.pos.y);
    return *t == Tile::Exit && (count_enemies() == 0 || s_player.hasKey);
}

static void spawn_enemies() {
    // Bats in rooms 1-4, Dragons in rooms 5-9
    struct Spawn { int roomIdx; EnemyType type; };
    static const Spawn placements[] = {
        {1, EnemyType::Bat},  {2, EnemyType::Bat},
        {3, EnemyType::Bat},  {4, EnemyType::Bat},
        {5, EnemyType::Dragon}, {6, EnemyType::Dragon},
        {7, EnemyType::Bat},  {8, EnemyType::Dragon},
        {9, EnemyType::Dragon},{3, EnemyType::Bat}
    };
    int n = sizeof(placements) / sizeof(placements[0]);
    for (int i = 0; i < n && i < MAX_ENEMIES; ++i) {
        int ri = placements[i].roomIdx;
        if (ri >= s_roomCount) ri = s_roomCount - 1;
        const Room &r = s_rooms[ri];
        // Place at offset inside room to avoid player start
        int ex = r.x + r.w - 2;
        int ey = r.y + 1;
        enemy_init(s_enemies[i], placements[i].type, ex, ey);
    }
}

static void spawn_items() {
    // Fixed item placements across rooms
    struct IPlacement { int roomIdx; ItemType type; };
    static const IPlacement ip[] = {
        {2, ItemType::Sword},
        {4, ItemType::Potion},
        {6, ItemType::Key},
        {8, ItemType::Potion}
    };
    int n = sizeof(ip) / sizeof(ip[0]);
    for (int i = 0; i < n && i < MAX_ITEMS; ++i) {
        int ri = ip[i].roomIdx;
        if (ri >= s_roomCount) ri = s_roomCount - 1;
        const Room &r = s_rooms[ri];
        item_init(s_items[i], ip[i].type, r.x + 2, r.y + 2);
    }
}

static void game_init() {
    // Zero-init all static arrays via pointer scan (demonstrate pointer usage)
    for (int i = 0; i < MAX_ENEMIES; ++i) s_enemies[i] = Enemy{};
    for (int i = 0; i < MAX_ITEMS;   ++i) s_items[i]   = Item{};
    for (int i = 0; i < MAX_PARTICLES;++i) s_parts[i]  = Particle{};

    map_init(s_grid);
    map_place_rooms(s_grid, s_rooms, s_roomCount);
    map_connect_rooms(s_grid, s_rooms, s_roomCount);
    map_place_exit(s_grid, s_rooms, s_roomCount);
    map_place_traps(s_grid, s_rooms, s_roomCount);

    // Player starts at centre of first room
    const Room &start = s_rooms[0];
    player_init(s_player, start.x + start.w / 2, start.y + start.h / 2);

    spawn_enemies();
    spawn_items();
}

// Get player name for high-score entry
static void read_name(char *buf, int maxLen) {
    // Restore blocking stdin for fgets
    input_restore();
    printf("Enter your name: ");
    fflush(stdout);
    if (fgets(buf, maxLen, stdin)) {
        // Strip newline
        for (int i = 0; buf[i]; ++i)
            if (buf[i] == '\n') { buf[i] = '\0'; break; }
    }
    input_init();
}

int main() {
    srand((unsigned)time(nullptr));

    hs_load(s_scores, "scores.txt");

    printf("\033[1;36m");
    printf("  ╔═══════════════════════════════════════╗\n");
    printf("  ║       DUNGEON CRAWLER  v1.0            ║\n");
    printf("  ║  Inspired by Adventure (Atari 2600)    ║\n");
    printf("  ╚═══════════════════════════════════════╝\n");
    printf("\033[0m");
    printf("  Controls: WASD=Move  f=Attack  e=Pickup  r=Drop  u=Use  p=Pause  q=Quit\n\n");
    hs_print(s_scores);
    printf("  Press ENTER to start...\n");
    getchar();

    input_init();
    game_init();

    GameState state = GameState::Playing;
    int tick = 0;

    while (state != GameState::Defeat && state != GameState::Victory) {

        // --- Input ---
        char c = 0;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            Action act = input_parse(c);

            if (act.quit) break;

            if (act.pause) {
                state = (state == GameState::Paused)
                        ? GameState::Playing
                        : GameState::Paused;
            }

            if (state == GameState::Playing) {
                if (act.move   != Dir::None) player_move(s_player, act.move, s_grid);
                if (act.pickup) player_pickup(s_player, s_items);
                if (act.drop)   player_drop(s_player, s_items);
                if (act.use)    player_use(s_player);
                if (act.attack) {
                    int killed = player_attack_enemies(s_player, s_enemies);
                    if (killed > 0)
                        particle_spawn(s_parts, s_player.pos.x, s_player.pos.y, '*', 3);
                }
            }
        }

        if (state == GameState::Playing) {
            // --- Enemy AI tick ---
            Enemy *ep = s_enemies;
            for (int i = 0; i < MAX_ENEMIES; ++i, ++ep)
                enemy_tick(*ep, s_player, s_grid, s_enemies);

            // --- Resolve combat ---
            int dmg = enemy_resolve_hits(s_player, s_enemies);
            if (dmg > 0)
                particle_spawn(s_parts, s_player.pos.x, s_player.pos.y, '!', 3);

            // --- Traps ---
            if (trap_check(s_player, s_grid))
                particle_spawn(s_parts, s_player.pos.x, s_player.pos.y, '^', 2);

            // --- Player timers ---
            player_tick(s_player);
            particle_tick(s_parts);

            // --- Win / lose ---
            if (!s_player.alive) state = GameState::Defeat;
            if (check_victory()) state = GameState::Victory;

            ++tick;
        }

        // --- Render ---
        render_frame(s_grid, s_player, s_enemies, s_items, s_parts, state, tick);
        render_hud(s_player, tick, count_enemies());
        if (state == GameState::Paused) render_pause();

        usleep(TICK_US);
    }

    input_restore();

    bool won = (state == GameState::Victory);
    render_gameover(won, s_player.score);

    // Save high score
    char name[16] = "Player";
    read_name(name, 16);
    hs_save(s_scores, "scores.txt", name, s_player.score);
    printf("\n");
    hs_print(s_scores);

    printf("\nThanks for playing!\n");
    return 0;
}
