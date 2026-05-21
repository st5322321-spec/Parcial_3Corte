# Parcial_3Corte
# Dungeon Crawler – Parcial Final

> Videojuego desarrollado en C++ como proyecto final de curso.  
> Inspirado en **Adventure (Atari 2600, 1980)** con el objetivo de optimizar el uso de RAM al mínimo, usando exclusivamente memoria estática.

---

## Integrantes

| Nombre | Contribución |
|--------|-------------|
| **Juan Sebastian Tovar Estrada** | Diseño del mapa, sistema de enemigos, lógica de combate, sistema de alerta de enemigos, trampas |
| **Kaleth Mena** | Renderizador ANSI, fog-of-war, sistema de partículas, inventario, high-score persistente, pausa |

Ambos integrantes participaron de forma equitativa (50/50) en el diseño de la arquitectura general, la definición de tipos en `types.h`, la integración del game loop principal en `main.cpp` y las pruebas funcionales del juego.

---

## Descripción del Juego

El jugador (`@`) despierta atrapado en una mazmorra de 10 habitaciones. Su objetivo es encontrar la salida (`E`) derrotando enemigos en el camino, o consiguiendo la llave (`k`) que abre la puerta de escape directamente.

El dungeon es oscuro — solo puedes ver lo que ilumina tu antorcha. Los murciélagos y dragones patrullan las salas en silencio hasta que te detectan. El suelo está lleno de trampas. Cada decisión cuenta.

---

## Compilación

### Opción A — Make (recomendado)

```bash
# Compilar
make

# Compilar y ejecutar directamente
make run

# Limpiar binarios y scores
make clean
```

### Opción B — CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
./dungeon_crawler
```

### Requisitos del sistema

| Requisito | Detalle |
|-----------|---------|
| Compilador | `g++` ≥ 7 o `clang++` ≥ 5 con soporte C++17 |
| Sistema operativo | Linux o macOS (usa POSIX `termios` para input de teclado en tiempo real) |
| Terminal | Cualquier terminal moderna con soporte ANSI/UTF-8 (gnome-terminal, iTerm2, Alacritty, etc.) |

> **Nota Windows:** El juego usa `termios.h` que es exclusivo de sistemas POSIX. En Windows se puede ejecutar mediante **WSL2** (Windows Subsystem for Linux).

---

## Controles

| Tecla | Acción |
|-------|--------|
| `W` `A` `S` `D` | Mover al personaje (arriba / izquierda / abajo / derecha) |
| `F` | Atacar — golpea a todos los enemigos adyacentes en las 8 direcciones |
| `E` | Recoger el objeto que hay en el suelo bajo el jugador |
| `R` | Soltar el objeto que lleva el jugador al suelo |
| `U` | Usar el objeto del inventario (tomar poción, equipar espada) |
| `P` | Pausar / Reanudar la partida |
| `Q` | Salir del juego |

---

## Objetos del Inventario

El jugador solo puede cargar **un objeto a la vez** (inventario de un solo slot).

| Símbolo | Objeto | Efecto al usar (`U`) |
|---------|--------|----------------------|
| `/` | Espada | Duplica el ataque del jugador de 2 → 4 permanentemente |
| `p` | Poción | Restaura todos los puntos de vida al máximo |
| `k` | Llave | Permite escapar por la salida `E` sin eliminar todos los enemigos |

---

## Enemigos

| Símbolo | Tipo | HP | Ataque | Velocidad | Rango de alerta |
|---------|------|-----|--------|-----------|-----------------|
| `b` | Murciélago (Bat) | 3 | 1 | Rápido (cada 2 ticks) | 6 tiles |
| `D` | Dragón (Dragon) | 8 | 3 | Lento (cada 4 ticks) | 8 tiles |

Los enemigos permanecen **inactivos** hasta que el jugador entra en su rango de alerta — en ese momento activan la persecución y no la abandonan.

---

## Condiciones de Victoria y Derrota

**Victoria** — se cumple cualquiera de estas dos condiciones:
- Llegar al tile `E` (salida) habiendo eliminado **todos** los enemigos, o
- Llegar al tile `E` con la **llave** (`k`) en el inventario.

**Derrota** — los puntos de vida del jugador llegan a **0** (por golpes de enemigos o trampas).

---

## Puntuación

| Evento | Puntos |
|--------|--------|
| Eliminar un murciélago | +20 |
| Eliminar un dragón | +50 |
| Recoger un objeto | +10 |
| Usar una poción | +5 |

Al terminar la partida (victoria o derrota) se registra el puntaje en una **tabla de high-scores** persistente guardada en `scores.txt`. Se mantienen los 5 mejores puntajes históricos.

---

## Mapa y Simbología

```
# → Pared          . → Suelo         + → Puerta
E → Salida         ^ → Trampa        @ → Jugador
b → Murciélago     D → Dragón        / → Espada
p → Poción         k → Llave         ░ → Zona en oscuridad (niebla)
```

---

## Desarrollos Innovadores

### 1. Niebla de Guerra (Fog-of-War)

**¿Para qué se implementó?**  
Para aumentar la tensión y el factor de exploración. El jugador no puede ver lo que hay más allá de su radio de luz, lo que obliga a moverse con cautela.

**¿Por qué se consideró necesario?**  
El juego original Adventure (Atari 1980) mostraba todo el mapa completo. Agregarle visibilidad limitada moderniza la experiencia y añade un componente estratégico que no existía.

**¿Cómo se llevó a cabo su implementación?**  
En cada frame, antes de renderizar un tile, se calcula la distancia euclidiana entre ese tile y la posición del jugador:

```cpp
bool is_visible(const Player &player, int x, int y) {
    int dx = x - player.pos.x;
    int dy = y - player.pos.y;
    return (dx * dx + dy * dy) <= (player.lightRadius * player.lightRadius);
}
```

Si la distancia supera el radio (`lightRadius = 5`, almacenado en el struct `Player`), el tile se renderiza como el carácter `░` en color azul oscuro. No se usa ninguna estructura de heap — todo el cálculo es en tiempo de render.

---

### 2. Sistema de Alerta de Enemigos

**¿Para qué se implementó?**  
Para que los enemigos tengan un comportamiento más realista e interesante: patrullan su área y solo persiguen al jugador cuando lo detectan.

**¿Por qué se consideró necesario?**  
Sin este sistema, todos los enemigos correrían hacia el jugador desde el inicio, haciendo el juego frustrante e injugable. La alerta le da al jugador la posibilidad de planear su ruta.

**¿Cómo se llevó a cabo su implementación?**  
Cada `Enemy` tiene dos campos: `alerted` (bool) y `alertRange` (int). En cada tick de IA se calcula la distancia Manhattan entre el enemigo y el jugador:

```cpp
int dist = manhattan(e.pos, player.pos);
if (dist <= e.alertRange) e.alerted = true;
```

Una vez alertado, el estado es permanente (no se "desalerta"). Los murciélagos tienen rango 6 y los dragones rango 8. Mientras no estén alertados, los enemigos simplemente no se mueven.

---

### 3. Trampas en el Suelo (`^`)

**¿Para qué se implementó?**  
Para añadir peligros pasivos e independientes de los enemigos, que penalicen el movimiento descuidado y enriquezcan el diseño del dungeon.

**¿Por qué se consideró necesario?**  
Solo tener enemigos como fuente de daño hace el juego predecible. Las trampas obligan al jugador a prestar atención al suelo y a moverse con intención, especialmente en los pasillos oscuros.

**¿Cómo se llevó a cabo su implementación?**  
Se agregó `Tile::Trap` al enum de tipos de tile. Al inicio se colocan trampas en esquinas específicas de las habitaciones intermedias (no en la inicial ni en la final). En cada tick del game loop se llama `trap_check()`:

```cpp
bool trap_check(Player &p, const Tile grid[MAP_H][MAP_W]) {
    if (grid[p.pos.y][p.pos.x] == Tile::Trap) {
        if (p.invincibleTicks == 0) {
            p.hp -= 2;
            p.invincibleTicks = 12;  // previene daño continuo
            ...
        }
    }
}
```

El campo `invincibleTicks` actúa como un periodo de invencibilidad (iframe) que evita que el jugador pierda HP en cada tick mientras permanece sobre la trampa.

---

### 4. Efectos de Partículas Visuales

**¿Para qué se implementó?**  
Para dar retroalimentación visual inmediata al jugador cuando ocurren eventos importantes como ataques, daño recibido o trampas activadas.

**¿Por qué se consideró necesario?**  
En un juego de terminal sin sonido ni animaciones, el feedback visual es crucial para que el jugador entienda qué está pasando. Sin esto, el combate se siente "muerto".

**¿Cómo se llevó a cabo su implementación?**  
Se creó un arreglo estático `Particle particles[MAX_PARTICLES]` (32 slots). Cada partícula tiene posición, un carácter glyph (`*` para ataque, `!` para daño, `^` para trampa) y un TTL (ticks to live):

```cpp
void particle_spawn(Particle particles[MAX_PARTICLES], int x, int y, char glyph, int ttl) {
    for (int i = 0; i < MAX_PARTICLES; ++i)
        if (!particles[i].active) {
            particles[i] = {x, y, glyph, ttl, true};
            return;
        }
}
```

En cada frame, las partículas activas sobreescriben el tile correspondiente en el render. `particle_tick()` decrementa el TTL y desactiva las que lleguen a 0. Todo sin heap.

---

### 5. Tabla de High-Scores Persistente

**¿Para qué se implementó?**  
Para guardar los mejores puntajes entre sesiones, añadir competitividad y motivar la rejugabilidad.

**¿Por qué se consideró necesario?**  
Es una característica icónica de los juegos arcade clásicos. Sin ella, el puntaje obtenido no tiene ningún valor más allá de la sesión actual.

**¿Cómo se llevó a cabo su implementación?**  
Se usa un arreglo estático `ScoreEntry table[HS_MAX]` (5 entradas) con `fopen/fprintf/fscanf` para leer y escribir el archivo `scores.txt`. Al finalizar la partida se pide el nombre del jugador y se inserta en la tabla manteniendo el orden descendente mediante insertion-sort sobre el arreglo:

```cpp
void hs_save(ScoreEntry table[HS_MAX], const char* path, const char* name, int score) {
    // Encontrar posición de inserción
    for (int i = 0; i < HS_MAX; ++i)
        if (score > table[i].score) { /* insertar y desplazar */ break; }
    // Escribir archivo
    FILE *f = fopen(path, "w");
    for (int i = 0; i < HS_MAX; ++i)
        fprintf(f, "%s %d\n", table[i].name, table[i].score);
    fclose(f);
}
```

---

### 6. Pantalla de Pausa

**¿Para qué se implementó?**  
Para permitir al jugador interrumpir la partida sin perder su estado de juego.

**¿Por qué se consideró necesario?**  
Es una funcionalidad básica de cualquier videojuego moderno. En un juego en tiempo real donde los enemigos se mueven solos, la pausa es especialmente necesaria.

**¿Cómo se llevó a cabo su implementación?**  
Se agregó el estado `GameState::Paused` al enum de estados. Al presionar `P`, el game loop alterna entre `Playing` y `Paused`. Cuando está en pausa, el bloque de lógica (movimiento de enemigos, combate, trampas) se omite completamente, pero el render sigue ejecutándose mostrando el frame congelado con un mensaje superpuesto.

---

### 7. Ítems con Efectos Reales y Progresión del Jugador

**¿Para qué se implementó?**  
Para dar profundidad estratégica al inventario de un solo slot: cada objeto tiene un uso único que cambia cómo se juega el resto de la partida.

**¿Por qué se consideró necesario?**  
Un inventario vacío de significado no aporta nada. Al hacer que la elección de qué cargar tenga consecuencias reales (¿guardo la llave para escapar o la cambio por la poción?), se crea una tensión de decisión interesante.

**¿Cómo se llevó a cabo su implementación?**  
Tres tipos de ítem implementados en `player_use()`:

```cpp
void player_use(Player &p) {
    if (p.held == ItemType::Potion) {
        p.hp = p.maxHp;        // restaura HP completo
        p.held = ItemType::None;
    }
    if (p.held == ItemType::Sword) {
        p.attack = 4;          // upgrade permanente de ataque (2 → 4)
        p.held   = ItemType::None;
    }
    // La llave (Key) no se "usa" manualmente — se verifica automáticamente al llegar a E
}
```

---

## Arquitectura del Código

```
dungeon_crawler/
├── CMakeLists.txt          ← Configuración de build con CMake
├── Makefile                ← Configuración de build con Make
├── README.md               ← Este archivo
├── scores.txt              ← Generado automáticamente al jugar
├── include/
│   ├── types.h             ← Todos los tipos, constantes y structs POD del juego
│   ├── map.h               ← Interfaz de generación y consulta del mapa
│   ├── entities.h          ← Interfaz de lógica de jugador, enemigos e ítems
│   ├── renderer.h          ← Interfaz del renderizador ANSI y partículas
│   ├── input.h             ← Interfaz de captura de teclado en tiempo real
│   └── highscore.h         ← Interfaz de la tabla de puntajes persistente
└── src/
    ├── main.cpp            ← Game loop principal, inicialización, condiciones de fin
    ├── map.cpp             ← Generación de habitaciones, pasillos, trampas y salida
    ├── entities.cpp        ← IA de enemigos, movimiento del jugador, sistema de ítems
    ├── renderer.cpp        ← Renderizado frame a frame con fog-of-war y partículas
    ├── input.cpp           ← Input no bloqueante con termios (POSIX)
    └── highscore.cpp       ← Lectura/escritura de scores.txt con insertion-sort
```

### Uso Verificable de Punteros

El uso de punteros está presente en múltiples módulos, no solo de forma simbólica:

- **`map_tile_at()`** — retorna `Tile*` para permitir modificación directa del tile sin copiar.
- **`item_at()`** — retorna `Item*` o `nullptr`, patrón clásico de búsqueda con puntero nulo como centinela.
- **`enemy_tick()`** — usa aritmética de punteros `&e - enemies` para calcular el índice del enemigo dentro del arreglo sin necesitar una variable de índice extra.
- **`map_init()`** — recorre el grid completo mediante un puntero incrementado `Tile* p = &grid[0][0]` en lugar de índices dobles, demostrando acceso lineal a memoria contigua.
- **`player_pickup()`** — recibe el arreglo de ítems como puntero y trabaja directamente sobre él sin copias.

### Restricciones de Memoria Cumplidas

- ✅ **Cero** llamadas a `new` o `delete` en el game loop ni en ningún otro módulo.
- ✅ **Cero** contenedores de la STL (`std::vector`, `std::string`, `std::list`, etc.).
- ✅ Todos los arreglos de entidades tienen tamaño fijo definido en tiempo de compilación en `types.h`.
- ✅ El mapa es un arreglo bidimensional estático en stack: `Tile s_grid[MAP_H][MAP_W]`.
- ✅ Las cadenas de texto usan `char[]` de tamaño fijo (ej. `char name[16]`).
