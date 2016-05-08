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
constexpr int MONTE_CARLO_TIMES_BASIC = 10000;
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
		/*if((myLines[line] >> column) & 1)
			return Self;
		else if ((oppLines[line] >> column) & 1)
			return Other;
		else 
			return None;*/
		return board[line][column];
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
		board[line][column] = p;

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
		//board[line][column] = p;
	}
	double monteCarlo() const;
private:
	Node *init();
	friend class TreeAllocator;
	double monteCarloSimOnceDestroy();
	int randomTop() const;
	Player getWinner() const;
	bool otherWin(int, int) const;
	bool selfWin(int, int) const;
	bool becomeChild(int column);
public:
	Player board[MAX_LINES][MAX_COLUMNS];
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