#ifndef GRID_RECT_H
#define GRID_RECT_H



// The dimensions are in columns/rows thus the name "grid rectangle".
typedef struct GridRect_t {
    int row;
    int col;
    int max_row;
    int max_col;
}
GridRect;



#endif
