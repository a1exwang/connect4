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


uint32_t Node::x = 123456789, Node::y = 362436069, Node::z = 521288629;

char BOARD_TRANSLATION_TABLE[] = "_XO-N";

#include <random>


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
#ifdef SHABBY_RANDOM
		uint32_t r = Node::shabbyRandom() % MAX_RANDOM_INT;
#else
		//uint32_t r = ::rand() % MAX_RANDOM_INT;
		std::random_device rd;     // only used once to initialise (seed) engine
		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
		std::uniform_int_distribution<uint32_t> uni(0, MAX_RANDOM_INT); // guaranteed unbiased
		uint32_t r = uni(rng);
#endif
		
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
#ifndef SHABBY_RANDOM
	srand(time(0));
#endif

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
		for (auto k = 0; k < MONTE_CARLO_TIMES_EXTENDED_ONCE; ++k) {
			score[c] += children[c]->monteCarlo();
		}
		times[c] += MONTE_CARLO_TIMES_EXTENDED_ONCE;
		totalTimes += MONTE_CARLO_TIMES_EXTENDED_ONCE;
		confidenceValue[c] = confidenceUpper(c, score[c], times[c], totalTimes);
	}

	double max = -1;
	for (auto i = 0; i < childrenCount; ++i) {
		auto child = children[i];
		auto rate = static_cast<double>(score[i]) / times[i];
		if (rate > max) {
			max = rate;
			out_column = childColumns[i];
		}

#ifdef _DEBUG
		cout << "\tcolumn: " << static_cast<int>(child->col) << ", score: "
			<< score[i] << ", times: " << times[i] <<", winrate: " << 2*(rate-0.5) << endl;
#endif
	}
	cout << "total times: " << totalTimes << endl;

	out_line = top[out_column];
#ifdef _DEBUG
	cout << "choose (" << out_line << ", " << out_column << ")" << endl;
#endif
#ifdef _DEBUG
	auto t2 = GetTickCount();
	cout << "time spent: " << t2 - startTime << "ms" << endl;
#endif
}




#include "TreeAllocator.h"
Node *Node::createNode() {
	return TreeAllocator::get()->allocateNode()->init();
	//return new Node();
}

void Node::destroy(const Node *node) {
	TreeAllocator::get()->recycleNode(node);
	//delete node;
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
	if (!(player == Self || player == Other))
		throw "otherPlayer()";
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
	if (Node::noLine == l && Node::noColumn == c){
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
	if (!(winner == None || winner == Self || winner == Other || winner == Tie))
		throw "monteCarloSimOnceDestroy";
	if (winner == None) {
		while (true) {
			auto selectedColumn = randomTop();
			if (becomeChild(selectedColumn))
				return monteCarloSimOnceDestroy();
			else
				throw "monteCarloSimOnceDestroy return";
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
