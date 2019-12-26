#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "FiveChess.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#define max(x, y)   ((x) < (y) ? (y) : (x))
#define min(x, y)   ((x) < (y) ? (x) : (y))

static int BOARD_SIZE = 6;
static u_int64_t *win_board;
static u_int64_t full_board;
static int win_cases;
static Board board;

#ifdef DEBUG
static void print_board(u_int64_t board)
{
    u_int64_t bitmap = 1;
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            printf("%d", (board & bitmap) ? 1 : 0);
            bitmap = bitmap << 1;
        }
        printf("\n");
    }
}
#endif

/* Recursively get nibble of a given number
  and map them in the array  */
static int count_set_bits(u_int64_t num)
{
    static int num_to_bits[16] = { 0, 1, 1, 2, 1, 2, 2, 3,
        1, 2, 2, 3, 2, 3, 3, 4 };

    int nibble = 0;
    if (0 == num)
        return num_to_bits[0];

    // Find last nibble
    nibble = num & 0xf;

    // Use pre-stored values to find count
    // in last nibble plus recursively add
    // remaining nibbles.
    return num_to_bits[nibble] + count_set_bits(num >> 4);
}

static int score_by_set_bits(int max_set_bits, int min_set_bits)
{
    if (max_set_bits + min_set_bits == 5)
        return 0;
    int set_bits = max_set_bits - min_set_bits;
    int bits = (set_bits < 0) ? -set_bits : set_bits;
    int ret;
    switch (bits) {
        case 1: ret = 1; break;
        case 2: ret = 5; break;
        case 3: ret = 30; break;
        case 4: ret = 300; break;
        case 5: ret = 100000; break;
        default: ret = 0; break;
    }
    return (set_bits < 0) ? -ret : ret;
}

static int score()
{
    int score = 0;
    for (int i = 0; i < win_cases; ++i) {
        score += score_by_set_bits(count_set_bits(board.max_board & win_board[i]),
                                   count_set_bits(board.min_board & win_board[i]));
    }

    return score;
}

static int is_terminal()
{
    /*if (board.rounds < 9)*/
        /*return NUL_PLAYER;*/

    for (int i = 0; i < win_cases; ++i) {
        if (count_set_bits(board.max_board & win_board[i]) == 5) {
            return MAX_PLAYER;
        }
        if (count_set_bits(board.min_board & win_board[i]) == 5) {
            return MIN_PLAYER;
        }
    }

    if ((board.max_board | board.min_board) == full_board) {
        return DRAW;
    }

    return NUL_PLAYER;
}

/*
 * Alpha-Beta 剪枝
 */
static int alpha_beta(int depth, int alpha, int beta, int player, int *row, int *col, int is_first)
{
    if (depth == 0 || is_terminal())
        return score();

    int value = (player == MAX_PLAYER) ? INT_MIN : INT_MAX;

    // 循环查找可以放置棋子的地方
    u_int64_t empty_board = ~(board.max_board | board.min_board);
    if (empty_board == 0) { // 无处可下
        return (player == MAX_PLAYER) ? INT_MAX : INT_MIN;
    }
    int pos = 0;
    for (u_int64_t bitmap = 1; bitmap != ((u_int64_t)0x1lu << BOARD_SIZE * BOARD_SIZE); bitmap = bitmap << 1, ++pos) {
        if (empty_board & bitmap) {  // 如果可以放置棋子
            int next_row, next_col;
            if (player == MAX_PLAYER) {
                board.rounds++;
                board.max_board |= bitmap;
                value = max(value, alpha_beta(depth - 1, alpha, beta, MIN_PLAYER, &next_row, &next_col, is_first + 1));
                if (value > alpha) {
                    alpha = value;
                    *row = pos / BOARD_SIZE;
                    *col = pos % BOARD_SIZE;
                }
                board.max_board &= ~bitmap;
                board.rounds--;
            } else {
                board.rounds++;
                board.min_board |= bitmap;
                value = min(value, alpha_beta(depth - 1, alpha, beta, MAX_PLAYER, &next_row, &next_col, is_first + 1));
                /*beta = min(value, beta);*/
                if (value < beta) {
                  beta = value;
                  *row = pos / BOARD_SIZE;
                  *col = pos % BOARD_SIZE;
                }
                board.min_board &= ~bitmap;
                board.rounds--;
            }

            if (alpha >= beta)  // 剪枝
                return (player == MAX_PLAYER) ? INT_MAX : INT_MIN;
        }
    }
    
    return value;
}

/*
 * API 接口
 */

void set_board_size(int board_size)
{
    BOARD_SIZE = board_size;
}

void init_board(int board_size)
{
    BOARD_SIZE = board_size;
    board.max_board = 0;
    board.min_board = 0;
    board.rounds = 0;

    full_board = ((u_int64_t)0x1lu << (BOARD_SIZE * BOARD_SIZE)) - 1;

    win_cases = ((BOARD_SIZE - 4) * BOARD_SIZE) + ((BOARD_SIZE - 4) * BOARD_SIZE) + ((BOARD_SIZE - 4) * (BOARD_SIZE - 4)) * 2;
    //          |           横向可能性        |   |          纵向可能性         |   |              斜向可能性                |

    win_board = (u_int64_t *)malloc(sizeof(u_int64_t) * win_cases);
    int pos = 0;

    // 横向五子
    for (int row = 0; row < BOARD_SIZE; row++) {
        u_int64_t bitmap = (u_int64_t)0x1flu << (row * BOARD_SIZE);
        for (int col = 0; col < BOARD_SIZE - 4; col++) {
            win_board[pos++] = bitmap;
            bitmap = bitmap << 1;
        }
    }
    // 纵向五子
    u_int64_t vertical_init_bitmap = 1;
    for (int i = 0; i < 4; ++i) {
        vertical_init_bitmap = vertical_init_bitmap << BOARD_SIZE;
        vertical_init_bitmap |= 1;
    }
    for (int row = 0; row < BOARD_SIZE - 4; ++row) {
        u_int64_t bitmap = vertical_init_bitmap << (row * BOARD_SIZE);
        for (int col = 0; col < BOARD_SIZE; ++col) {
            win_board[pos++] = bitmap;
            bitmap = bitmap << 1;
        }
    }
    // 左上-右下斜向五子
    u_int64_t leftup2rightdown_init_bitmap = 1;
    for (int i = 0; i < 4; ++i) {
        leftup2rightdown_init_bitmap = leftup2rightdown_init_bitmap << (BOARD_SIZE + 1);
        leftup2rightdown_init_bitmap |= 1;
    }
    for (int row = 0; row < BOARD_SIZE - 4; ++row) {
        u_int64_t bitmap = leftup2rightdown_init_bitmap << (row * BOARD_SIZE);
        for (int col = 0; col < (BOARD_SIZE - 4); ++col) {
            win_board[pos++] = bitmap;
            bitmap = bitmap << 1;
        }
    }
    // 右上-左下斜向五子
    u_int64_t rightup2leftdown_init_bitmap = 0x1;
    for (int i = 0; i < 4; ++i) {
        rightup2leftdown_init_bitmap = rightup2leftdown_init_bitmap << (BOARD_SIZE - 1);
        rightup2leftdown_init_bitmap |= 1;
    }
    rightup2leftdown_init_bitmap = rightup2leftdown_init_bitmap << 4;
    for (int row = 0; row < BOARD_SIZE - 4; ++row) {
        u_int64_t bitmap = rightup2leftdown_init_bitmap << (row * BOARD_SIZE);
        for (int col = 0; col < (BOARD_SIZE - 4); ++col) {
            win_board[pos++] = bitmap;
            bitmap = bitmap << 1;
        }
    }
#ifdef DEBUG
    for (int i = 0; i < win_cases; ++i) {
        printf("win_board[%d]: %lu\n", i, win_board[i]);
        print_board(win_board[i]);
        printf("\n");
    }
#endif
}

/*
 * 主函数
 *
 * @min_row: 对手 (MIN_PLAYER) 下的棋子所在的行
 * @min_col: 对手 (MIN_PLAYER) 下的棋子所在的列
 * @depth: 搜索时的最大层数
 * @row: 返回要下的棋子所在的行
 * @col: 返回要下的棋子所在的列
 *
 * @ret: 返回输赢状态 (NUL_PLAYER, MAX_PLAYER, MIN_PLAYER, DRAW)
 */
int five_chess_play(int min_row, int min_col, int depth, int *row, int *col)
{
    board.rounds++;
    board.min_board |= (u_int64_t)1lu << (min_row * BOARD_SIZE + min_col);
#ifdef DEBUG
    print_board(board.min_board);
    printf("\n");
#endif
    int win = is_terminal();
    if (win != NUL_PLAYER)
        return win;

    alpha_beta(depth, INT_MIN, INT_MAX, MAX_PLAYER, row, col, 1);
    board.max_board |= (u_int64_t)0x1lu << (*row * BOARD_SIZE + *col);
    board.rounds++;
#ifdef DEBUG
    print_board(board.max_board);
    printf("\n");
#endif
#ifdef DEBUG
    printf("end row: %d, col: %d, win: %d\n", *row, *col, win);
#endif
    return is_terminal();
}
