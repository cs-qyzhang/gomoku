#ifndef _FIVECHESS_H_
#define _FIVECHESS_H_

#include <sys/types.h>

/*
 * 使用位图来保存棋盘
 *
 * LSB 是第一排第一列
 */
typedef struct {
    u_int64_t max_board;
    u_int64_t min_board;
    int rounds;
} Board;

#define MAX_PLAYER  1
#define MIN_PLAYER  2
#define DRAW        3
#define NUL_PLAYER  0

void set_board_size(int board_size);
void init_board(int board_size);
int five_chess_play(int min_row, int min_col, int depth, int *row, int *col);

#endif
