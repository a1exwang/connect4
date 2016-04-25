#include "ai.h"
#include <ctime>
#include <cassert>
#include "PatternMatcher.h"
#include <iostream>
#include <windows.h>
#define TRUE_RANDOMN

using namespace std;

int Node::m = -1;
int Node::n = -1;
int Node::noLine = -1; 
int Node::noColumn = -1;


static uint32_t x = 123456789, y = 362436069, z = 521288629;

uint32_t shabbyRandom() {
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

char BOARD_TRANSLATION_TABLE[] = "_XO-N";

constexpr uint32_t MAX_RANDOM_INT = (2 << 30);
class RouletteRandom {
public:
	RouletteRandom(int n, double *values) :count(n) {
		double sum = 0;
		begins[0] = 0;

		for (auto i = 0; i < n-1; ++i) {
			sum += values[i];
			begins[i + 1] = sum;
		}

		sum += values[n - 1];
		begins[n] = sum;

		auto factor = MAX_RANDOM_INT / sum;
		for (auto i = 0; i <= n; ++i) {
			begins[i] *= factor;
		}
	}

	int rand() const {
		uint32_t r = shabbyRandom() % MAX_RANDOM_INT;
		for (int i = 1; i <= count; ++i) {
			if (begins[i] > r) {
				return i-1;
			}
		}
		return -1;
	}
private:
	int32_t count;
	double begins[MAX_LINES+1];
};

constexpr double K1 = 3;
constexpr double K2 = 100;
int64_t myCounter = 0;
double confidenceUpper(int child, double score, int times, int totalTimes) {
	double a = score / times * K1;
	double b = sqrt(2 * log(times) / totalTimes) * K2;

	myCounter++;
	//if (myCounter % 5000 == 0)
	//	cout << "child " << child << ", a=" << a << ", b=" << b << endl;

	return a + b;
}

void Node::ai(int &out_line, int &out_column, int** board, int* top) {
	auto startTime = GetTickCount();

#ifdef _DEBUG
	cout << "sizeof(Node) " << sizeof(Node) << endl;
#endif
	// build node from board
	Node *node = createNode();
	for (int i = 0; i < Node::m; ++i) {
		for (int j = 0; j < Node::n; ++j) {
			node->set(i, j, BOARD_TRANSLATION_TABLE[board[i][j]]);
		}
	}
	for (int j = 0; j < Node::n; ++j) {
		node->top[j] = top[j];
	}
	node->player = Other;
	node->line = node->col = -1;

	// create children
	auto childrenCount = 0;
	Node *children[MAX_COLUMNS];
	double score[MAX_COLUMNS] = { DEFAULT_SCORE };
	int times[MAX_COLUMNS];
	int childColumns[MAX_COLUMNS];
	int totalTimes = 0;
	for (auto i = 0; i < Node::n; ++i) {
		auto *child = createNode();
		*child = *node;
		if (child->becomeChild(i)) {
			children[childrenCount] = child;
			childColumns[childrenCount] = i;
			childrenCount++;
		}
		else {
			destroy(child);
		}
	}

	// basic sim
	for (auto i = 0; i < childrenCount; ++i) {
		for (auto j = 0; j < MONTE_CARLO_TIMES_BASIC; ++j) {
			score[i] += children[i]->monteCarlo();
		}
		times[i] = MONTE_CARLO_TIMES_BASIC;
	}
	totalTimes += childrenCount * MONTE_CARLO_TIMES_BASIC;

	// calculate rates and confidence upper bound
	double rates[MAX_COLUMNS];
	double confidenceValue[MAX_COLUMNS] = { 0 };
	for (auto i = 0; i < childrenCount; ++i) {
		//auto child = Node::root->getChild(i);
		auto child = children[i];
		rates[i] = static_cast<double>(score[i]) / times[i];
		confidenceValue[i] = confidenceUpper(i, score[i], times[i], totalTimes);
	}
	
	// monte carlo simulate until time is up
	while (true) {
		auto currentTime = GetTickCount();
		if (currentTime - startTime > MONTE_CARLO_TIME_MILLIS)
			break;

		auto random = RouletteRandom(childrenCount, confidenceValue);
		auto c = random.rand();
		auto child = children[c];
		for (int k = 0; k < MONTE_CARLO_TIMES_EXTENDED_ONCE; ++k) {
			score[c] += children[c]->monteCarlo();
		}
		times[c] += MONTE_CARLO_TIMES_EXTENDED_ONCE;
		totalTimes += MONTE_CARLO_TIMES_EXTENDED_ONCE;
		confidenceValue[c] = confidenceUpper(c, score[c], times[c], totalTimes);
	}

	double max = -1;

	for (auto i = 0; i < childrenCount; ++i) {
		auto child = children[i];
		double rate = static_cast<double>(score[i]) / times[i];
		if (rate > max) {
			max = rate;
			out_column = childColumns[i];
		}

#ifdef _DEBUG
		cout << "\tcolumn: " << static_cast<int>(child->col) << ", score: "
			<< score[i] << ", times: " << times[i] <<", winrate: " << 2*(rate-0.5) << endl;
#endif
	}

	out_line = top[out_column];
#ifdef _DEBUG
	auto t2 = GetTickCount();
	cout << "time: " << t2 - startTime << "ms" << endl;
#endif
}

#define SHABBY_RANDOME
int Node::randomTop() const {
	int column = -1;
	do {
#if defined(TRUE_RANDOMN) && !defined(SHABBY_RANDOME)
		column = rand() % Node::n;
#elif defined SHABBY_RANDOME
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
int Node::getNaOfPie(int x, int y) {
	return y < Node::m - x ? y : Node::m - x;
}
int Node::getNaOfPieCount(int l, int c)  {
	int A = Node::m > Node::n ? Node::n : Node::m;
	int x1 = l + c + 1;
	int x2 = Node::m + Node::n - l - c - 1;
	return x1 < A ? (x1 < x2 ? x1 : x2) : (A < x2 ? A : x2);
}

int Node::getPieOfNa(int l, int c) {
	return l < c ? l : c;
}
int Node::getPieOfNaCount(int l, int c)  {
	int A = Node::m > Node::n ? Node::n : Node::m;
	int x1 = l - c + Node::n - 1 + 1;
	int x2 = Node::m + Node::n - (l - c + Node::n - 1) - 1;
	return x1 < A ? (x1 < x2 ? x1 : x2) : (A < x2 ? A : x2);
}

bool Node::otherWin(const int x, const int y) const {
	
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

bool Node::selfWin(const int x, const int y) const {
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

Player Node::getWinner() const {

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

#include "TreeAllocator.h"
Node *Node::createNode() {
	return TreeAllocator::get()->allocateNode()->init();
}

void Node::destroy(const Node *node) {
	TreeAllocator::get()->recycleNode(node);
}

Node* Node::init() {
	player = None;
	line = -1;
	col = -1;
#ifdef _DEBUG
	depth = 1;
#endif
	return this;
}

Player Node::otherPlayer() const {
	assert(player == Self || player == Other);
	if (player == Self) {
		return Other;
	}
	else {
		return Self;
	}
}

bool Node::becomeChild(int c) {
	if (top[c] < 0)
		return false;

	Player p = otherPlayer();
	int l = top[c];
	//board[l][c] = p;
	set(l, c, p);

	top[c]--;
	if (Node::noLine == l && Node::noColumn == col) {
		top[c]--;
	}

	line = l;
	col = c;
	player = p;

#ifdef _DEBUG
	depth++;
#endif
	return true;
}

double Node::monteCarlo() const {
	auto *newNode = createNode();
	*newNode = *this;
	auto ret = newNode->monteCarloSimOnceDestroy();
	destroy(newNode);
	return ret;
}

double Node::monteCarloSimOnceDestroy() {

	auto winner = getWinner();
	if (winner == None) {
		while (true) {
			auto selectedColumn = randomTop();
			if (becomeChild(selectedColumn))
				return monteCarloSimOnceDestroy();
			else
				return 0;
		}

	}
	else {
		if (winner == Self) 
			return SCORE_SELF_WIN;
		else if (winner == Other) 
			return SCORE_OTHER_WIN;
		else
			return SCORE_TIE;
	}
}
