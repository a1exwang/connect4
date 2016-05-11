#pragma once

#define MAX_BOARD_WIDTH		12
#define TIME_LIMIT			4600
#define MAX_SIMULATE_TIMES	(440 * 10000)

#define PLAYER_NONE		0
#define PLAYER_OTHER	1
#define PLAYER_SELF		2

#define WIN_SCORE		1
#define TIE_SCORE		0
#define LOSE_SCORE		-1

#define UCB_C (2 * 1.4142135623730950488016887242097)

typedef struct T_UCTNode {
	unsigned char player;
	unsigned char line, column;
	char board[MAX_BOARD_WIDTH][MAX_BOARD_WIDTH];
	struct T_UCTNode *children[MAX_BOARD_WIDTH];
	struct T_UCTNode *parent;

	unsigned char top[MAX_BOARD_WIDTH];
	unsigned char children_count;

	int simulate_times;
	double score;
	unsigned char is_terminate;
} UCTNode;

#ifdef __cplusplus
extern "C" {
#endif
	int uct_search(int **board, int m, int n, int no_x, int no_y, int last_x, int last_y, const int *top);
#ifdef __cplusplus
	}
#endif