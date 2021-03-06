#pragma once

#include <pebble.h>

typedef struct cells_size {
    uint16_t row;
    uint16_t column;
} CSize;

typedef enum cells_pattern {
    CP_None = 0,    // as clear
    CP_Clock,
    CP_Glider,
    CP_Saceship,    // Spaceship
    CP_RRntomino    // R-pentomino
    // You have to modify 'MAX_CPATTERN' value.
} CPattern;
#define MAX_CPATTERN    ((int)CP_RRntomino + 1)

typedef struct cells Cells;

Cells *cells_create(CSize size);
void cells_destroy(Cells *cells);
CSize cells_get_size(const Cells *cells);
bool cells_is_alive(const Cells *cells, uint16_t row, uint16_t column);
void cells_set_pattern(Cells *cells, CPattern pattern);
bool cells_evolution(Cells *cells);
