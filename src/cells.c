#include <pebble.h>
#include "cells.h"

#define ALIVE   (1)    // for Cells.data
#define DEAD    (0)

typedef struct cells {
    CSize size;
    uint16_t data_size;
    uint8_t *data;
    uint8_t *tmp_data;
} Cells;

#define MAX_FONT    (10)
#define FONT_ROW    (7)
#define FONT_COLUMN (3)

static uint8_t s_font[MAX_FONT][FONT_ROW * FONT_COLUMN] = {
    // 0
    {
        1,1,1,
        1,0,1,
        1,0,1,
        1,0,1,
        1,0,1,
        1,0,1,
        1,1,1,
    },
    // 1
    {
        1,1,0,
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0,
        1,1,1,
    },
    // 2
    {
        1,1,1,
        0,0,1,
        0,0,1,
        1,1,1,
        1,0,0,
        1,0,0,
        1,1,1,
    },
    // 3
    {
        1,1,1,
        0,0,1,
        0,0,1,
        1,1,1,
        0,0,1,
        0,0,1,
        1,1,1,
    },
    // 4
    {
        1,0,1,
        1,0,1,
        1,0,1,
        1,1,1,
        0,0,1,
        0,0,1,
        0,0,1,
    },
    // 5
    {
        1,1,1,
        1,0,0,
        1,0,0,
        1,1,1,
        0,0,1,
        0,0,1,
        1,1,1,
    },
    // 6
    {
        1,1,1,
        1,0,0,
        1,0,0,
        1,1,1,
        1,0,1,
        1,0,1,
        1,1,1,
    },
    // 7
    {
        1,1,1,
        0,0,1,
        0,0,1,
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0,
    },
    // 8
    {
        1,1,1,
        1,0,1,
        1,0,1,
        1,1,1,
        1,0,1,
        1,0,1,
        1,1,1,
    },
    // 9
    {
        1,1,1,
        1,0,1,
        1,0,1,
        1,1,1,
        0,0,1,
        0,0,1,
        1,1,1,
    }
};

#define ROUNDUP32BIT(n)    (((n) + 3) & ~3)

inline static uint8_t s_cell_get(const uint8_t *data, CSize size, int row, int column);
inline static void s_cell_set(uint8_t *data, CSize size, int row, int column, uint8_t life);
inline static int s_cells_num_alive(const uint8_t *data, CSize size, int row, int col);
static void s_cells_set_font(uint8_t *data, CSize size, int row, int col, int num);
static void s_math_cut_figure2(int num, int figure[2]);

#define CELL_GET(cells, sign, row, col)        s_cell_get((cells)->sign, (cells)->size, (row), (col))
#define CELL_SET(cells, sign, row, col, life)  s_cell_set((cells)->sign, (cells)->size, (row), (col), (life))
#define CELLS_NUM_ALIVE(cells, sign, row, col) s_cells_num_alive((cells)->sign, (cells)->size, (row), (col))
#define CELLS_SET_FONT(cells, sign, row, col, num)    s_cells_set_font((cells)->sign, (cells)->size, (row), (col), (num))

Cells *cells_create(CSize size) {
    Cells *cells = NULL;
    
    uint16_t data_size = ROUNDUP32BIT(sizeof(uint8_t) * (size.row * size.column));
    cells = malloc(ROUNDUP32BIT(sizeof(Cells)) + (data_size * 2));
    if (cells != NULL) {
        cells->size = size;
        cells->data_size = data_size;
        cells->data = &(((uint8_t*)cells)[ROUNDUP32BIT(sizeof(Cells))]);
        cells->tmp_data = &((cells->data)[data_size]);
    }
    return cells;
}

void cells_destroy(Cells *cells) {
    if (cells == NULL) {
        return;
    }
    free(cells);
}

CSize cells_get_size(const Cells *cells) {
    return cells->size;
}

bool cells_is_alive(const Cells *cells, uint16_t row, uint16_t column) {
    return CELL_GET(cells, data, row, column) == ALIVE ? true : false;
}

void cells_set_time(Cells *cells, time_t tim) {
    memset(cells->data, DEAD, cells->data_size);

#if 0 // Glider
    CELL_SET(cells, data, 0, 1, ALIVE);
    CELL_SET(cells, data, 1, 2, ALIVE);
    CellsELL_SET(cells, data, 2, 0, ALIVE);
    CELL_SET(cells, data, 2, 1, ALIVE);
    CELL_SET(cells, data, 2, 2, ALIVE);
#endif

    struct tm *ltim = localtime(&tim);

    int hour[2], min[2];
    s_math_cut_figure2(ltim->tm_hour, hour);
    s_math_cut_figure2(ltim->tm_min, min);

    CELLS_SET_FONT(cells, data, 5, ((FONT_COLUMN+1)*0)+0, hour[1]);
    CELLS_SET_FONT(cells, data, 5, ((FONT_COLUMN+1)*1)+0, hour[0]);
    CELLS_SET_FONT(cells, data, 5, ((FONT_COLUMN+1)*2)+1, min[1]);
    CELLS_SET_FONT(cells, data, 5, ((FONT_COLUMN+1)*3)+1, min[0]);
}

void cells_evolution(Cells *cells) {
    memcpy(cells->tmp_data, cells->data, cells->data_size);
    memset(cells->data, DEAD, cells->data_size);
    
    for (int row = 0; row < cells->size.row; row++) {
        for (int col = 0; col < cells->size.column; col++) {
            int num_alive = CELLS_NUM_ALIVE(cells, tmp_data, row, col);
            if (CELL_GET(cells, tmp_data, row, col) == DEAD) {
                if (num_alive == 3) {
                    CELL_SET(cells, data, row, col, ALIVE);
                }    
            } else {
                if ((num_alive == 2) || (num_alive == 3)) {
                    CELL_SET(cells, data, row, col, ALIVE);
                }
            }
        }
    }
}

inline static uint8_t s_cell_get(const uint8_t *data, CSize size, int row, int column) {
    if ((row < 0)
        || (column < 0)
        || (size.row <= row)
        || (size.column <= column)) {
        return DEAD;
    }
    return data[(row * size.column) + column];
}

inline static void s_cell_set(uint8_t *data, CSize size, int row, int column, uint8_t life){
    if ((row < 0)
        || (column < 0)
        || (size.row <= row)
        || (size.column <= column)) {
        return;
    }
    data[(row * size.column) + column] = life;
}

inline static int s_cells_num_alive(const uint8_t *data, CSize size, int row, int col) {
    int num = 0;

    for (int r = (row - 1); r <= (row + 1); r++) {
        for (int c = (col - 1); c <= (col + 1); c++) {
            num += s_cell_get(data, size, r, c);
        }
    }
    num -= s_cell_get(data, size, row, col);
    return num;
}

static void s_cells_set_font(uint8_t *data, CSize size, int row, int col, int num) {
    if(MAX_FONT <= num) {
        return;
    }
    uint8_t *font = s_font[num];

    for (int r = 0; r < FONT_ROW; r++) {
        for (int c = 0; c < FONT_COLUMN; c++) {
            s_cell_set(data, size, row+r, col+c, font[(r * FONT_COLUMN) + c]);
        }
    }
}

static void s_math_cut_figure2(int num, int figure[2]) {
    figure[0] = num % 10;
    figure[1] = (num / 10) % 10;
}