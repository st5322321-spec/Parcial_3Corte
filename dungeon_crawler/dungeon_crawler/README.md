# Dungeon Crawler – Parcial Final

## Integrantes
> Juan Sebastian Tovar Estrada

---

## Descripción
Dungeon-crawler en C++ inspirado en **Adventure (Atari 2600, 1980)**.  
El jugador (`@`) explora 10 habitaciones conectadas, evade trampas, derrota enemigos y busca la salida (`E`).

---

## Compilación

### Con Make (recomendado)
```bash
make          # compila
make run      # compila y ejecuta
make clean    # limpia binarios
```

### Con CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
./dungeon_crawler
```

### Requisitos
- Compilador C++17 (`g++` ≥ 7 o `clang++` ≥ 5)
- Sistema operativo: **Linux / macOS** (usa POSIX termios para input raw)
- Terminal con soporte ANSI (cualquier terminal moderno)

---

## Controles

| Tecla | Acción |
|-------|--------|
| W A S D | Mover |
| F | Atacar (8 direcciones) |
| E | Recoger objeto |
| R | Soltar objeto |
| U | Usar objeto del inventario |
| P | Pausar / Reanudar |
| Q | Salir |

---

## Condición de Victoria y Derrota
- **Victoria:** llegar al tile `E` (Exit) sin enemigos vivos, **o** con la llave (`k`) recogida.
- **Derrota:** HP llega a 0.

---

## Desarrollos Innovadores

### 1. Niebla de Guerra (Fog-of-War)
**¿Para qué?** Aumentar la tensión y el factor de exploración.  
**¿Por qué?** El juego original Adventure no la tenía; agregarla eleva la experiencia.  
**¿Cómo?** Se calcula la distancia euclidiana entre cada tile y la posición del jugador. Si `dx²+dy² > lightRadius²` el tile se renderiza como `░` (oscuro). El radio (`lightRadius = 5`) es un campo del struct `Player`.

### 2. Sistema de Alerta de Enemigos
**¿Para qué?** Los enemigos no persiguen al jugador desde el inicio; necesitan "verlo".  
**¿Por qué?** Evita que el juego sea imposible desde el primer frame.  
**¿Cómo?** Cada `Enemy` tiene un campo `alerted` y `alertRange`. En cada tick se calcula la distancia Manhattan. Si está en rango, `alerted = true` y el enemigo activa su IA de persecución.

### 3. Trampas en el Suelo (`^`)
**¿Para qué?** Añadir peligros pasivos que el jugador debe esquivar.  
**¿Por qué?** Enriquece el diseño de niveles y penaliza el movimiento descuidado.  
**¿Cómo?** Tiles especiales `Tile::Trap`. En cada tick se llama `trap_check()` que detecta si el jugador está sobre un trap y aplica 2 de daño con un periodo de invencibilidad para evitar daño continuo.

### 4. Efectos de Partículas
**¿Para qué?** Feedback visual de ataques y trampas.  
**¿Por qué?** Mejora notablemente la experiencia de usuario en un juego de terminal.  
**¿Cómo?** Arreglo estático `Particle particles[MAX_PARTICLES]`. Cada partícula tiene posición, glyph (`*`, `!`, `^`) y un TTL (ticks to live). `particle_tick()` decrementa TTL y desactiva las expiradas.

### 5. Tabla de Puntajes Persistente
**¿Para qué?** Competitividad entre sesiones y motivación de rejugabilidad.  
**¿Por qué?** Característica clásica de arcade que ningún dungeon-crawler debería omitir.  
**¿Cómo?** Al terminar la partida se escribe `scores.txt` con los 5 mejores puntajes usando `fopen/fprintf/fscanf` (sin heap). Se mantiene ordenado con un simple insertion-sort sobre el arreglo estático `ScoreEntry table[HS_MAX]`.

### 6. Pausa
**¿Para qué?** Control básico de flujo de juego.  
**¿Por qué?** Estándar en cualquier videojuego moderno; fácil de implementar y muy valorado.  
**¿Cómo?** El estado `GameState::Paused` congela la actualización de lógica pero sigue renderizando el frame actual con el mensaje de pausa superpuesto.

### 7. Ítems con Efectos Reales
**¿Para qué?** Sistema de progresión del jugador.  
**¿Por qué?** Da sentido estratégico al inventario de un solo slot.  
**¿Cómo?** Tres ítems (`Sword`, `Potion`, `Key`) con efectos distintos: la espada duplica el ataque permanentemente, la poción restaura HP completo, la llave es el requisito alternativo de victoria.

---

## Arquitectura del Código

```
dungeon_crawler/
├── CMakeLists.txt
├── Makefile
├── README.md
├── include/
│   ├── types.h       ← Tipos, constantes, structs (POD)
│   ├── map.h         ← Generación y consulta del mapa
│   ├── entities.h    ← Lógica de jugador, enemigos, ítems
│   ├── renderer.h    ← Renderizado ANSI + partículas
│   ├── input.h       ← Entrada de teclado (termios)
│   └── highscore.h   ← Tabla de puntajes persistente
└── src/
    ├── main.cpp      ← Game loop principal
    ├── map.cpp
    ├── entities.cpp
    ├── renderer.cpp
    ├── input.cpp
    └── highscore.cpp
```

### Uso de Punteros
- `map_tile_at()` retorna `Tile*` para modificación directa de tiles.
- `item_at()` retorna `Item*` o `nullptr` para búsqueda sin copia.
- `enemy_tick()` usa aritmética de punteros (`&e - enemies`) para calcular el índice del enemigo sin variable extra.
- Los arreglos estáticos se recorren con punteros incrementales en varios módulos.

### Restricciones de Memoria
- **Cero** uso de `new` / `delete`.
- **Cero** contenedores de la STL (`std::vector`, etc.).
- Todos los arreglos de entidades son estáticos con tamaño en tiempo de compilación.
