#pragma once

#include <vector>
#include <iostream>
typedef int** Matrix;

typedef char Player;



enum {
	None = '_',
	Other = 'X',
	Self = 'O',
	Tie = '-',
	No = 'N'
};
#define TRUE_RANDOM
#define SHABBY_RANDOM
constexpr int MONTE_CARLO_TIMES_BASIC = 10;
constexpr int MONTE_CARLO_TIMES_EXTENDED_ONCE = 1;
constexpr int MONTE_CARLO_TIME_MILLIS = 3000;

constexpr double DEFAULT_SCORE = 0.5;

constexpr double SCORE_SELF_WIN = 1;
constexpr double SCORE_TIE = 0.5;
constexpr double SCORE_OTHER_WIN = 0;

constexpr int MAX_LINES = 13;
constexpr int MAX_COLUMNS = 13;

struct Node {
public:
	//static void initRoot(bool opponentFirst, int, int col, const int *top);
	static void ai(int &, int &, int** board, int* top);
	static int m, n, noLine, noColumn;
	//static Node *root;
	//static void resetAll();
public:
	static Node *createNode();
	static void destroy(const Node*);
	Player otherPlayer() const;
	int getTop(int column) const { return top[column]; }

	static int Node::getNaOfPie(int x, int y) {
		return y < Node::m - x ? y : Node::m - x;
	}
	static int Node::getNaOfPieCount(int l, int c) {
		int A = Node::m > Node::n ? Node::n : Node::m;
		int x1 = l + c + 1;
		int x2 = Node::m + Node::n - l - c - 1;
		return x1 < A ? (x1 < x2 ? x1 : x2) : (A < x2 ? A : x2);
	}
	static int Node::getPieOfNa(int l, int c) {
		return l < c ? l : c;
	}
	static int Node::getPieOfNaCount(int l, int c) {
		int A = Node::m > Node::n ? Node::n : Node::m;
		int x1 = l - c + Node::n - 1 + 1;
		int x2 = Node::m + Node::n - (l - c + Node::n - 1) - 1;
		return x1 < A ? (x1 < x2 ? x1 : x2) : (A < x2 ? A : x2);
	}
	Player get(int line, int column) const {
		if((myLines[line] >> column) & 1)
			return Self;
		else if ((oppLines[line] >> column) & 1)
			return Other;
		else 
			return None;
		//return board[line][column];
	}
	void set(int line, int column, Player p) {
		if (!(p == Self || p == Other || p == None))
			throw "set";
		int pie = line + column;
		int naOfPie = getNaOfPie(line, column);
		int na = line - column + Node::n - 1;
		int pieOfNa = getPieOfNa(line, column);
		//std::cout << "pie " << pie << ", na" << na << std::endl;
		//board[line][column] = p;

		if (p == Self) {
			myLines[line]		|= 1 << column;
			myColumns[column]	|= 1 << line;
			myPie[pie]			|= 1 << naOfPie;
			myNa[na]			|= 1 << pieOfNa;

			oppLines[line]		&= ~(1 << column);
			oppColumns[column]	&= ~(1 << line);
			oppPie[pie]			&= ~(1 << naOfPie);
			oppNa[na]			&= ~(1 << pieOfNa);
		}
		else if (p == Other) {
			oppLines[line]		|= 1 << column;
			oppColumns[column]	|= 1 << line;
			oppPie[pie]			|= 1 << naOfPie;
			oppNa[na]			|= 1 << pieOfNa;

			myLines[line]		&= ~(1 << column);
			myColumns[column]	&= ~(1 << line);
			myPie[pie]			&= ~(1 << naOfPie);
			myNa[na]			&= ~(1 << pieOfNa);
		}
		else {
			myLines[line]		&= ~(1 << column);
			myColumns[column]	&= ~(1 << line);
			myPie[pie]			&= ~(1 << naOfPie);
			myNa[na]			&= ~(1 << pieOfNa);

			oppLines[line]		&= ~(1 << column);
			oppColumns[column]	&= ~(1 << line);
			oppPie[pie]			&= ~(1 << naOfPie);
			oppNa[na]			&= ~(1 << pieOfNa);
		}
	}
	double monteCarlo() const;
	static uint32_t shabbyRandom() {
		uint32_t t;
		x ^= x << 16;
		x ^= x >> 5;
		x ^= x << 1;

		t = x;
		x = y;
		y = z;
		z = t ^ x ^ y;

		return z;
	}
private:
	Node *init();
	friend class TreeAllocator;
	double monteCarloSimOnceDestroy();

	static uint32_t x, y, z;



	int randomTop() const {
		int column = -1;
		do {
#if defined(TRUE_RANDOM) && !defined(SHABBY_RANDOM)
			column = rand() % Node::n;
#elif defined SHABBY_RANDOM
			column = shabbyRandom() % Node::n;
#else
			column++;
#endif
		} while (top[column] < 0);
		if (column < 0 || column >= Node::n) {
			throw "column out of bound";
		}
		return column;
	}
	bool otherWin(const int x, const int y) const {

		/*int i, j, count;
		// 横向
		count = 0;
		for (i = y; i >= 0; i--)
		if (!(get(x, i) == Other))
		break;
		count += (y - i);
		for (i = y; i < Node::n; i++)
		if (!(get(x, i) == Other))
		break;
		count += (i - y - 1);
		if (count >= 4) return true;

		// 纵向
		count = 0;
		for (i = x; i >= 0; i--)
		if (!(get(i, y) == Other))
		break;
		count += (x - i);
		for (i = x; i < Node::m; i++)
		if (!(get(i, y) == Other))
		break;
		count += (i - x - 1);
		if (count >= 4) return true;

		//左下-右上
		count = 0;
		for (i = x, j = y; i < m && j >= 0; i++, j--)
		if (!(get(i, j) == Other))
		break;
		count += (y - j);
		for (i = x, j = y; i >= 0 && j < n; i--, j++)
		if (!(get(i, j) == Other))
		break;
		count += (j - y - 1);
		if (count >= 4) return true;

		//左上-右下
		count = 0;
		for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
		if (!(get(i, j) == Other))
		break;
		count += (y - j);
		for (i = x, j = y; i < m && j < n; i++, j++)
		if (!(get(i, j) == Other))
		break;
		count += (j - y - 1);
		if (count >= 4) return true;*/

		for (auto i = y - 3; i <= y; i++) {
			if (i >= 0 && i < Node::n && ((oppLines[x] >> i) & 0xF) == 0xF)
				return true;
		}

		for (auto i = x - 3; i <= x; i++) {
			if (i >= 0 && i < Node::m && ((oppColumns[y] >> i) & 0xF) == 0xF)
				return true;
		}
		auto pie = x + y;
		auto na = x - y + Node::n - 1;
		auto naOfPie = getNaOfPie(x, y);
		auto naOfPieCount = getNaOfPieCount(x, y);
		for (auto j = naOfPie - 3; j <= naOfPie; j++) {
			if (j >= 0 && j < naOfPieCount && ((oppPie[pie] >> j) & 0xF) == 0xF)
				return true;
		}
		auto pieOfNa = getPieOfNa(x, y);
		auto pieOfNaCount = getPieOfNaCount(x, y);
		for (auto j = pieOfNa - 3; j <= pieOfNa; j++) {
			if (j >= 0 && j < pieOfNaCount && ((oppNa[na] >> j) & 0xF) == 0xF)
				return true;
		}
		return false;
	}

	bool selfWin(const int x, const int y) const {
		/*
		int i, j, count;

		// 横向
		count = 0;
		for (i = y; i >= 0; i--)
		if (!(get(x, i) == Self))
		break;
		count += (y - i);
		for (i = y; i < Node::n; i++)
		if (!(get(x, i) == Self))
		break;
		count += (i - y - 1);
		if (count >= 4) return true;


		// 纵向
		count = 0;
		for (i = x; i >= 0; i--)
		if (!(get(i, y) == Self))
		break;
		count += (x - i);
		for (i = x; i < Node::m; i++)
		if (!(get(i, y) == Self))
		break;
		count += (i - x - 1);
		if (count >= 4) return true;

		//左下-右上
		count = 0;
		for (i = x, j = y; i < m && j >= 0; i++, j--)
		if (!(get(i, j) == Self))
		break;
		count += (y - j);
		for (i = x, j = y; i >= 0 && j < n; i--, j++)
		if (!(get(i, j) == Self))
		break;
		count += (j - y - 1);
		if (count >= 4) return true;

		//左上-右下
		count = 0;
		for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
		if (!(get(i, j) == Self))
		break;
		count += (y - j);
		for (i = x, j = y; i < m && j < n; i++, j++)
		if (!(get(i, j) == Self))
		break;
		count += (j - y - 1);
		if (count >= 4) return true;*/


		for (auto i = y - 3; i <= y; i++) {
			if (i >= 0 && i < Node::n && ((myLines[x] >> i) & 0xF) == 0xF)
				return true;
		}

		for (auto i = x - 3; i <= x; i++) {
			if (i >= 0 && i < Node::m && ((myColumns[y] >> i) & 0xF) == 0xF)
				return true;
		}

		auto pie = x + y;
		auto na = x - y + Node::n - 1;

		auto naOfPie = getNaOfPie(x, y);
		auto naOfPieCount = getNaOfPieCount(x, y);
		for (auto j = naOfPie - 3; j <= naOfPie; j++) {
			if (j >= 0 && j < naOfPieCount && ((myPie[pie] >> j) & 0xF) == 0xF)
				return true;
		}
		auto pieOfNa = getPieOfNa(x, y);
		auto pieOfNaCount = getPieOfNaCount(x, y);
		for (auto j = pieOfNa - 3; j <= pieOfNa; j++) {
			if (j >= 0 && j < pieOfNaCount && ((myNa[na] >> j) & 0xF) == 0xF)
				return true;
		}
		return false;
	}

	Player getWinner() const {

		// is tie
		bool tie = true;
		for (int i = 0; i < Node::n; i++)
		{
			if (top[i] >= 0)
			{
				tie = false;
				break;
			}
		}

		if (tie)
			return Tie;

		if (player == Self) {
			if (selfWin(line, col))
				return Self;
		}
		else {
			if (otherWin(line, col))
				return Other;
		}
		return None;
	}

	bool becomeChild(int column);
public:
	//Player board[MAX_LINES][MAX_COLUMNS];
	uint16_t myLines[MAX_LINES];
	uint16_t myColumns[MAX_LINES];
	uint32_t myPie[MAX_LINES + MAX_COLUMNS + 1];
	uint32_t myNa[MAX_LINES + MAX_COLUMNS + 1];

	uint16_t oppLines[MAX_LINES];
	uint16_t oppColumns[MAX_LINES];
	uint32_t oppPie[MAX_LINES + MAX_COLUMNS + 1];
	uint32_t oppNa[MAX_LINES + MAX_COLUMNS + 1];

	char top[MAX_COLUMNS];

	Player player;
	char line, col;
#ifdef _DEBUG
	int depth;
#endif

	friend class __Initializer;
};