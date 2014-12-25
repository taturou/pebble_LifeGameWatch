#include <pebble.h>
#include "cells.h"
#include "font.h"

#define ALIVE   (1)    // for Cells.data
#define DEAD    (0)

typedef struct cells {
    CSize size;
    uint16_t data_size;
    uint8_t *data;
} Cells;

#define DATA        (0)
#define GEN(n)      (n)    // 1..6
#define TEMP        (7)

#define ROUNDUP32BIT(n)    (((n) + 3) & ~3)

static void s_cells_set_pattern_clock(Cells *cells);

inline static uint8_t s_cell_get(const Cells *cells, int bit, int row, int column);
inline static void s_cell_set(Cells *cells, int bit, int row, int column, uint8_t life);
inline static int s_cells_num_alive(const Cells *cells, int bit, int row, int col);
static void s_cells_draw_font(Cells *cells, int bit, int offset_row, int offset_col, const CFont *font);
static void s_math_cut_figure2(int num, int figure[2]);
static void s_cells_rotate(Cells *cells);
static bool s_cells_is_evolution(const Cells *cells);

Cells *cells_create(CSize size) {
    Cells *cells = NULL;
    
    uint16_t data_size = sizeof(uint8_t) * (size.row * size.column);
    cells = malloc(ROUNDUP32BIT(sizeof(Cells)) + data_size);
    if (cells != NULL) {
        cells->size = size;
        cells->data_size = data_size;
        cells->data = &(((uint8_t*)cells)[ROUNDUP32BIT(sizeof(Cells))]);
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
    return s_cell_get(cells, DATA, row, column) == ALIVE ? true : false;
}

void cells_set_pattern(Cells *cells, CPattern pattern) {
    memset(cells->data, 0x00, cells->data_size);

    switch (pattern) {
    case CP_None:
        break;
    case CP_Clock:
        s_cells_set_pattern_clock(cells);
        break;
    case CP_Glider:
         s_cells_draw_font(cells, DATA, 1, 1, &font_pattern_glider);
        if ((font_pattern_glider.size.column * 4) <= cells->size.column) {
            s_cells_draw_font(cells,
                              DATA,
                              rand() % 2 == 0 ? 1 : 2,
                              cells->size.column - font_pattern_glider2.size.column - 1,
                              &font_pattern_glider2);
        }
        break;
    case CP_LWSaceship:
         s_cells_draw_font(cells, DATA, 3, 1, &font_pattern_lwspaceship);
        break;
    case CP_RRntomino:
        s_cells_draw_font(cells, DATA, cells->size.row / 2, cells->size.column / 2, &font_pattern_pentomino);
        break;
    default:
        break;
    }
}

bool cells_evolution(Cells *cells) {
    s_cells_rotate(cells);
    
    for (int row = 0; row < cells->size.row; row++) {
        for (int col = 0; col < cells->size.column; col++) {
            int num_alive = s_cells_num_alive(cells, TEMP, row, col);
            if (s_cell_get(cells, TEMP, row, col) == DEAD) {
                if (num_alive == 3) {
                    s_cell_set(cells, DATA, row, col, ALIVE);
                }    
            } else {
                if ((num_alive == 2) || (num_alive == 3)) {
                    s_cell_set(cells, DATA, row, col, ALIVE);
                }
            }
        }
    }
    
    return s_cells_is_evolution(cells);
}

static void s_cells_set_pattern_clock(Cells *cells) {
    time_t tim = time(NULL);
    struct tm *ltim = localtime(&tim);

    int hour[2], min[2];
    s_math_cut_figure2(ltim->tm_hour, hour);
    s_math_cut_figure2(ltim->tm_min, min);

    CSize offset;
    offset.row = (cells->size.row / 2) - (font_number[0].size.row / 2);

    // HH
    offset.column = ((cells->size.column / 2) - 2) - font_number[hour[0]].size.column;
    s_cells_draw_font(cells, DATA, offset.row, offset.column, &font_number[hour[0]]);

    offset.column -= font_number[hour[0]].size.column + 1;
    s_cells_draw_font(cells, DATA, offset.row, offset.column, &font_number[hour[1]]);

    // MM
    offset.column = ((cells->size.column / 2) + 2);
    s_cells_draw_font(cells, DATA, offset.row, offset.column, &font_number[min[1]]);

    offset.column += font_number[min[1]].size.column + 1;
    s_cells_draw_font(cells, DATA, offset.row, offset.column, &font_number[min[0]]);

    // :
    offset.row = ((cells->size.row / 2) - (font_number[0].size.row / 2)) + 2;
    offset.column = (cells->size.column / 2);
    s_cell_set(cells, DATA, offset.row, offset.column, ALIVE);

    offset.row = ((cells->size.row / 2) + (font_number[0].size.row / 2)) - 2;
    offset.column = (cells->size.column / 2);
    s_cell_set(cells, DATA, offset.row, offset.column, ALIVE);
}

inline static int s_cell_calc_data_index(const CSize *size, int row, int column) {
    if (row < 0) {
        row = size->row + row;
    } else if (size->row <= row) {
        row = row - size->row;
    } else {
        // do nothing
    }
    if (column < 0) {
        column = size->column + column;
    } else if (size->column <= column) {
        column = column - size->column;
    } else {
        // do nothing
    }
    return (row * size->column) + column;
}

inline static uint8_t s_cell_get(const Cells *cells, int bit, int row, int column) {
    return (cells->data[s_cell_calc_data_index(&cells->size, row, column)] >> bit) & 0x01;
}

inline static void s_cell_set(Cells *cells, int bit, int row, int column, uint8_t life) {
    if (life == ALIVE) {
        cells->data[s_cell_calc_data_index(&cells->size, row, column)] |= (0x01 << bit);
    } else {
        cells->data[s_cell_calc_data_index(&cells->size, row, column)] &= ~(0x01 << bit);
    }
}

inline static int s_cells_num_alive(const Cells *cells, int bit, int row, int col) {
    int num = 0;

    for (int r = (row - 1); r <= (row + 1); r++) {
        for (int c = (col - 1); c <= (col + 1); c++) {
            num += s_cell_get(cells, bit, r, c);
        }
    }
    num -= s_cell_get(cells, bit, row, col);
    return num;
}

static void s_cells_draw_font(Cells *cells, int bit, int offset_row, int offset_col, const CFont *font) {
    for (int r = 0; r < font->size.row; r++) {
        for (int c = 0; c < font->size.column; c++) {
            s_cell_set(cells, bit, r+offset_row, c+offset_col, font->data[(r * font->size.column) + c]);
        }
    }
}

static void s_math_cut_figure2(int num, int figure[2]) {
    figure[0] = num % 10;
    figure[1] = (num / 10) % 10;
}

static void s_cells_rotate(Cells *cells) {
    int max = cells->size.row * cells->size.column;
    
    for (int i = 0; i < max; i++) {
        cells->data[i] <<= 1;
        if (((cells->data[i] >> GEN(1)) & 0x01) == ALIVE) {
            cells->data[i] |= (0x01 << TEMP);
        } else {
            cells->data[i] &= ~(0x01 << TEMP);
        }
    }
}

static bool s_cells_is_evolution(const Cells *cells) {
    bool evolution = false;
    int max = cells->size.row * cells->size.column;

    for (int gen = 1; gen <= 6; gen++) {
        evolution = false;
        for (int i = 0; i < max; i++) {
            uint8_t d = cells->data[i];
            if (((d >> DATA) & 0x01) != ((d >> GEN(gen)) & 0x01)) {
                evolution = true;
                break;
            }
        }
        if (evolution == false) {
            break;
        }
    }
    return evolution;
}
