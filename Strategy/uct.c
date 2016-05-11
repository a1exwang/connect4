#include "uct.h"
#include "fast_rand.h"
#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <intrin.h>
#include <time.h>
#include "uct_allocator.h"

static int M, N, NO_X, NO_Y;

char *player_string = "_XO12345";

void print_node(const UCTNode *node) {
	printf("node: (%d, %d), player: %d, no: (%d, %d)\n", node->line, node->column, node->player, NO_X, NO_Y);
	printf("children_count: %d\n", node->children_count);
	printf("simulate_times: %d, score: %f\n", node->simulate_times, node->score);
	printf("top: ");
	for (int i = 0; i < N; ++i) {
		printf("%d ", node->top[i]);
	}
	printf("\n");
	printf("\tboard:\n");
	for (int i = 0; i < M; ++i) {
		printf("\t");
		for (int j = 0; j < N; ++j)
			printf("%c ", player_string[node->board[i][j]]);
		printf("\n");
	}
}

UCTNode *node_init(const int **board, int line, int column, int player, const int *top) {
	UCTNode *node = uct_alloc();
	node->player = player;
	node->line = line;
	node->column = column;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			node->board[i][j] = board[i][j];
		}
	}

	for (int i = 0; i < MAX_BOARD_WIDTH; ++i) {
		node->children[i] = NULL;
	}
	node->children_count = 0;
	node->simulate_times = 0;
	node->score = 0;
	node->is_terminate = false;
	node->parent = NULL;

	for (int i = 0; i < N; ++i) {
		node->top[i] = top[i];
	}
	
	return node;
}

inline int self_win(const UCTNode *node) {
	int x = node->line;
	int y = node->column;
	//横向检测
	int i, j;
	int count = 0;
	for (i = y; i >= 0; i--)
		if (!(node->board[x][i] == 2))
			break;
	count += (y - i);
	for (i = y; i < N; i++)
		if (!(node->board[x][i] == 2))
			break;
	count += (i - y - 1);
	if (count >= 4) return true;

	//纵向检测
	count = 0;
	for (i = x; i < M; i++)
		if (!(node->board[i][y] == 2))
			break;
	count += (i - x);
	if (count >= 4) return true;

	//左下-右上
	count = 0;
	for (i = x, j = y; i < M && j >= 0; i++, j--)
		if (!(node->board[i][j] == 2))
			break;
	count += (y - j);
	for (i = x, j = y; i >= 0 && j < N; i--, j++)
		if (!(node->board[i][j] == 2))
			break;
	count += (j - y - 1);
	if (count >= 4) return true;

	//左上-右下
	count = 0;
	for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
		if (!(node->board[i][j] == 2))
			break;
	count += (y - j);
	for (i = x, j = y; i < M && j < N; i++, j++)
		if (!(node->board[i][j] == 2))
			break;
	count += (j - y - 1);
	if (count >= 4) return true;

	return false;
}

inline int other_win(const UCTNode *node) {
	int x = node->line;
	int y = node->column;
	//横向检测
	int i, j;
	int count = 0;
	for (i = y; i >= 0; i--)
		if (!(node->board[x][i] == 1))
			break;
	count += (y - i);
	for (i = y; i < N; i++)
		if (!(node->board[x][i] == 1))
			break;
	count += (i - y - 1);
	if (count >= 4) return true;

	//纵向检测
	count = 0;
	for (i = x; i < M; i++)
		if (!(node->board[i][y] == 1))
			break;
	count += (i - x);
	if (count >= 4) return true;

	//左下-右上
	count = 0;
	for (i = x, j = y; i < M && j >= 0; i++, j--)
		if (!(node->board[i][j] == 1))
			break;
	count += (y - j);
	for (i = x, j = y; i >= 0 && j < N; i--, j++)
		if (!(node->board[i][j] == 1))
			break;
	count += (j - y - 1);
	if (count >= 4) return true;

	//左上-右下
	count = 0;
	for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
		if (!(node->board[i][j] == 1))
			break;
	count += (y - j);
	for (i = x, j = y; i < M && j < N; i++, j++)
		if (!(node->board[i][j] == 1))
			break;
	count += (j - y - 1);
	if (count >= 4) return true;

	return false;
}

inline int is_tie(const UCTNode *node) {
	for (int i = 0;i < N; ++i) {
		if (node->top[i] > 0)
			return 0;
	}
	return 1;
}

inline int node_is_terminate(const UCTNode *node) {
	/*if (node->player == PLAYER_OTHER) {
		return other_win(node) || is_tie(node);
	}
	else {
		return self_win(node) || is_tie(node);
	}*/
	return node->is_terminate;
}
inline UCTNode *node_dup(const UCTNode *node) {
	UCTNode *ret = uct_alloc();
	memcpy(ret, node, sizeof(UCTNode));
	ret->children_count = 0;
	ret->score = 0;
	ret->simulate_times = 0;
	return ret;
}
inline int other_player(int player) {
	if (player == PLAYER_SELF)
		return PLAYER_OTHER;
	else if (player == PLAYER_OTHER)
		return PLAYER_SELF;
	else
		__debugbreak();
	return 0;
}

inline void node_set(UCTNode *node, int column, int player) {
	if(node->top[column] <= 0) {
		__debugbreak();
	}
	node->top[column] --;
	node->board[node->top[column]][column] = player;
	node->player = player;
	node->line = node->top[column];
	node->column = column;

	if (column == NO_Y && node->top[column] - 1 == NO_X)
		node->top[column]--;
}

// 随机选一个未扩展的子节点扩展, 如果没有该种节点, 返回NULL
UCTNode *expand(UCTNode *node) {
	if (node->children_count == N)
		return NULL;

	// 对于column i来说, 如果new_children[i] == 1, 则column i已经搜索过
	int new_chilren[MAX_BOARD_WIDTH] = { 0 };
	for (int i = 0; i < node->children_count; ++i) {
#ifdef _DEBUG
		if (new_chilren[node->children[i]->column] == 1)
			__debugbreak();
#endif
		new_chilren[node->children[i]->column] = 1;
	}

	// cols保存所有没搜索过的column i
	int cols[MAX_BOARD_WIDTH] = { -1 };
	int new_children_count = 0;
	for (int i = 0; i < N; ++i) {
		// 该位置未扩展, 并且该位置可着子
		if (new_chilren[i] == 0 && node->top[i] > 0) {
			cols[new_children_count++] = i;
		}
	}
	if (new_children_count > N - node->children_count)
		__debugbreak();
	if (new_children_count == 0)
		return NULL;
	int r = fast_rand(new_children_count);

	UCTNode *child = node_dup(node);
	node_set(child, cols[r], other_player(node->player));
	node->children[node->children_count++] = child;
	child->parent = node;
	return child;
}

inline UCTNode *uct_best_child(const UCTNode *node) {
	double max = -1e100;
	int max_i = -1;

	//return node->children[fast_rand(node->children_count)];
	for (int i = 0; i < node->children_count; ++i) {
		double value = (node->children[i]->score + UCB_C * sqrt(log(node->simulate_times))) / 
			node->children[i]->simulate_times;
		//printf("rate: %f, y: %f\n", value, y);
		//value += y;

		if (value > max) {
			max = value;
			max_i = i;
		}
	}
#ifdef _DEBUG
	//if (max_i < 0)
	//	__debugbreak();
#endif

	return node->children[max_i];
}
// 用ucb算法选择一个孩子节点
inline UCTNode *best_child(const UCTNode *node) {
	double max = -1e100;
	int max_i = -1;
	for (int i = 0; i < node->children_count; ++i) {
		double value = node->children[i]->score / node->children[i]->simulate_times;

#ifdef _DEBUG
		printf("\tchild: %d, score: %f, times: %d, rate: %f\n", 
			node->children[i]->column,
			node->children[i]->score, 
			node->children[i]->simulate_times,
			node->children[i]->score / node->children[i]->simulate_times);
#endif

		if (value > max) {
			max = value;
			max_i = i;
		}
	}
	if (max_i < 0) 
		__debugbreak();
	
	return node->children[max_i];
}

UCTNode *uct_tree_expand(UCTNode *node) {
	UCTNode *current = node;
	while (!node_is_terminate(current)) {
		UCTNode *exp = expand(current);
		if (exp)
			return exp;
		else
			current = uct_best_child(current);
	}
	return current;
}

// 更新该节点的所有祖先节点的score和times
inline void update_score(UCTNode *node, double score) {
	UCTNode *current = node;
	while (current->parent) {
		current->parent->simulate_times++;

		score = -score;
		current->parent->score += score;
	
		current = current->parent;
	}
}

// 根据column创建node的子节点, 如果该column无法着子, 那么返回NULL
UCTNode *create_child_by_column(UCTNode *node, int column) {
	if (node->top[column] <= 0)
		return NULL;
	
	UCTNode *child = node_dup(node);
	node_set(child, column, other_player(node->player));

	return child;
}

// 对某个节点进行一次蒙特卡洛模拟, 并且更新该节点的score, times, 不影响node的其他属性
double monte_carlo_simulate_once(UCTNode *node) {
	node->simulate_times++;

	if (node->player == PLAYER_SELF && self_win(node)) {
		node->is_terminate = true;
		node->score += WIN_SCORE;
		return WIN_SCORE;
	}
	else if (node->player == PLAYER_OTHER && other_win(node)) {
		node->is_terminate = true;
		node->score += WIN_SCORE;
		return WIN_SCORE;
	}
	else if (is_tie(node)) {
		node->is_terminate = true;
		node->score += TIE_SCORE;
		return TIE_SCORE;
	}

	// select a child
	UCTNode *child = NULL;
	do {
		int r = fast_rand(N);
		child = create_child_by_column(node, r);
	} while (!child);

	// set node
	// recur
	double score = -monte_carlo_simulate_once(child);
	uct_recycle(child);

	// update score and times
	node->score += score;
	return score;
}

// 递归删除某个node和他的所有子节点
void node_destroy(UCTNode *node) {
	for (int i = 0; i < node->children_count; ++i) {
		node_destroy(node->children[i]);
	}
	uct_recycle(node);
}

int uct_search(int **board, int m, int n, int no_x, int no_y, int last_x, int last_y, const int *top) {
	srand((unsigned)time(0));
	M = m;
	N = n;
	NO_X = no_x;
	NO_Y = no_y;

	UCTNode *root = node_init(board, last_x, last_y, PLAYER_OTHER, top);
#ifdef _DEBUG
	print_node(root);
#endif
	DWORD start_time = GetTickCount();

	int j = 0;
	double total_score = 0;

//#ifdef _DEBUG
//	for (int i = 0; i < MAX_SIMULATE_TIMES; ++i) {
//#else
	while (GetTickCount() - start_time < TIME_LIMIT && j < MAX_SIMULATE_TIMES) {
//#endif
		// node = tree expand node
		UCTNode *node = uct_tree_expand(root);
		
		// monte carlo simulate
		double score = monte_carlo_simulate_once(node);
		update_score(node, score);
		j++;
	}

	const UCTNode *child = best_child(root);
	int ret = child->column;
#ifdef _DEBUG
	printf("select %d, rate: %f\n", child->column, child->score / child->simulate_times);
#endif

	//node_destroy(root);
	uct_recycle_all();

	printf("sim times: %d, time elapsed %d\n", j, GetTickCount() - start_time);
	return ret;
}
