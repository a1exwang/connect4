#include "ai.h"
#include "Judge.h"
#include <memory>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cassert>
#include "PatternMatcher.h"
#include <iostream>
#define TRUE_RANDOMN

using namespace std;

int Node::m = -1;
int Node::n = -1;
int Node::noLine = -1; 
int Node::noColumn = -1;
Node *Node::root = nullptr;

void Node::cloneMatrix(Node &node) const {
	for (auto i = 0; i < m; ++i) {
		for (auto j = 0; j < n; ++j) {
			node.board[i][j] = board[i][j];
		}
	}
}

void Node::ai(int &out_line, int &out_column) {
	int selected_line = -1, selected_col = -1;
	srand(static_cast<unsigned int>(time(nullptr)));

	for (auto j = 0; j < 40000; ++j) {
		Node::root->monteCarloSimOnce();
	}
	cout << "round: " << (Node::root->depth+1)/2 << ", times: " << Node::root->times << endl;

	auto max = -1000.0;

	for (auto i = 0; i < Node::root->childrenCount; ++i) {
		auto child = Node::root->children[i];

		cout << "\tcolumn: " << child->getColumn() << ", score: "
			<< child->score << ", winrate: " << child->winRate << endl;

		if (child->winRate > max) {
			max = child->winRate;
			selected_line = child->line;
			selected_col = child->col;
		}
	}

	// select
	playerMove(selected_line, selected_col, Player::Self);

	out_line = selected_line;
	out_column = selected_col;
}

int Node::randomTop() const {
	int column = -1;
	do {
#ifdef TRUE_RANDOMN
		column = rand() % Node::n;
#else
		column++;
#endif
	} while (top[column] < 0);
	if (column < 0 || column >= Node::n) {
		throw "column out of bound";
	}
	return column;
}

Node* Node::randomChild() const {
	if (childrenCount <= 0)
		return nullptr;
#ifdef TRUE_RANDOMN
	return children[rand() % childrenCount];
#else
	return children[0];
#endif
}

bool Node::otherWin(const int x, const int y) const {
	//ºáÏò¼ì²â
	int i, j;
	int count = 0;
	for (i = y; i >= 0; i--)
		if (!(board[x][i] == 1))
			break;
	count += (y - i);
	for (i = y; i < n; i++)
		if (!(board[x][i] == 1))
			break;
	count += (i - y - 1);
	if (count >= 4) return true;

	//×ÝÏò¼ì²â
	count = 0;
	for (i = x; i < m; i++)
		if (!(board[i][y] == 1))
			break;
	count += (i - x);
	if (count >= 4) return true;

	//×óÏÂ-ÓÒÉÏ
	count = 0;
	for (i = x, j = y; i < m && j >= 0; i++, j--)
		if (!(board[i][j] == 1))
			break;
	count += (y - j);
	for (i = x, j = y; i >= 0 && j < n; i--, j++)
		if (!(board[i][j] == 1))
			break;
	count += (j - y - 1);
	if (count >= 4) return true;

	//×óÉÏ-ÓÒÏÂ
	count = 0;
	for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
		if (!(board[i][j] == 1))
			break;
	count += (y - j);
	for (i = x, j = y; i < m && j < n; i++, j++)
		if (!(board[i][j] == 1))
			break;
	count += (j - y - 1);
	if (count >= 4) return true;

	return false;
}

bool Node::selfWin(const int x, const int y) const {
	//ºáÏò¼ì²â
	int i, j;
	int count = 0;
	for (i = y; i >= 0; i--)
		if (!(board[x][i] == 2))
			break;
	count += (y - i);
	for (i = y; i < n; i++)
		if (!(board[x][i] == 2))
			break;
	count += (i - y - 1);
	if (count >= 4) return true;

	//×ÝÏò¼ì²â
	count = 0;
	for (i = x; i < m; i++)
		if (!(board[i][y] == 2))
			break;
	count += (i - x);
	if (count >= 4) return true;

	//×óÏÂ-ÓÒÉÏ
	count = 0;
	for (i = x, j = y; i < m && j >= 0; i++, j--)
		if (!(board[i][j] == 2))
			break;
	count += (y - j);
	for (i = x, j = y; i >= 0 && j < n; i--, j++)
		if (!(board[i][j] == 2))
			break;
	count += (j - y - 1);
	if (count >= 4) return true;

	//×óÉÏ-ÓÒÏÂ
	count = 0;
	for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
		if (!(board[i][j] == 2))
			break;
	count += (y - j);
	for (i = x, j = y; i < m && j < n; i++, j++)
		if (!(board[i][j] == 2))
			break;
	count += (j - y - 1);
	if (count >= 4) return true;

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
		return Player::Tie;

	if (player == Self) {
		if (selfWin(line, col))
			return Player::Self;
	}
	else {
		if (otherWin(line, col))
			return Player::Other;
	}
	return Player::None;
}

void Node::initRoot(bool opponentFirst, int thisLine, int col, const int *top) {
	auto node = new Node();
	node->col = col;
	node->player = opponentFirst ? Player::Other : Player::Self;
	node->parent = nullptr;
	for (auto i = 0; i < Node::m; ++i) {
		for (auto j = 0; j < Node::n; ++j) {
			node->board[i][j] = Player::None;
		}
	}
	node->board[Node::noLine][Node::noColumn] = Player::No;
	for (int i = 0; i < Node::n; ++i) {
		node->top[i] = top[i];
	}
	node->childrenCount = 0;
	node->score = 0;
	node->patternMatched = false;
	node->winRate = 0;
	node->times = 0;

	if (opponentFirst) {
		// no need to update top
		node->line = thisLine;
	}
	else {
		node->line = node->top[col];
		node->top[col]--;
		if (node->top[col] == Node::noLine && col == Node::noColumn)
			node->top[col]--;
	}
	node->board[node->line][node->col] = node->player;

	Node::root = node;
}

// update root
void Node::playerMove(int line, int column, Player player) {
	assert(root->otherPlayer() == player);

	Node *newRoot = nullptr;
	auto oldRoot = Node::root;
	for (auto i = 0; i < root->childrenCount; ++i) {
		if (root->children[i]->col == column) {
			newRoot = root->children[i];
			root->children[i] = nullptr;
			break;
		}
		else {
			//delete root->children[i];
		}
	}

	if (newRoot) {
		root = newRoot;
	}
	else {
		// not in children
		root = oldRoot->findOrCreateByColumn(column, false);
	}

	for (int i = 0; i < oldRoot->childrenCount; ++i) {
		if (oldRoot->children[i] != nullptr)
			delete oldRoot->children[i];
	}
	oldRoot->childrenCount = 0;
	delete oldRoot;
}

Node::Node() :
	parent(nullptr), player(None), line(-1), col(-1), score(-1), 
	childrenCount(0), depth(1), patternMatched(false) {
}

Node::~Node() {
	for (int i = 0; i < childrenCount; ++i) {
		delete children[i];
	}
}

Node* Node::findOrCreateByColumn(int thisCol, bool addChild) {
	for (int i = 0; i < childrenCount; ++i) {
		if (children[i]->col == thisCol) {
			if (addChild) {
				return children[i];
			}
			else {
				//children[i]->parent = nullptr;
				assert(false);
			}
		}
	}
	auto node = new Node(*this);
	node->depth = depth + 1;
	node->player = otherPlayer();
	node->line = -1;
	for (auto i = 0; i < Node::n; ++i) {
		node->top[i] = top[i];

	}

	if (node->top[thisCol] == Node::noLine && thisCol == Node::noColumn) {
		node->top[thisCol] -= 2;
	}
	else if (node->top[thisCol] >= 0) {
		node->top[thisCol]--;
	}
	else {
		delete node;
		return nullptr;
	}
	node->line = node->top[thisCol] + 1;

	assert(node->line >= 0);
	node->col = thisCol;
	cloneMatrix(*node);
	assert(node->board[node->line][node->col] == Player::None);
	node->board[node->line][node->col] = node->player;
	node->childrenCount = 0;
	node->score = DEFAULT_SCORE;
	node->patternMatched = false;
	node->times = 0;
	node->winRate = 0;

	if (addChild) {
		children[childrenCount] = node;
		childrenCount++;
		node->parent = this;
	}
	return node;
}

void Node::updateScoreByChildren() {
	score = 0;
	times = 0;
	for (int i = 0; i < childrenCount; ++i) {
		score += children[i]->score;
		times += children[i]->times;
	}
	//if ()
	winRate = (double)score / times;
}

void Node::makeWinnerNode(Player winner) {
	if (winner == Player::Self) {
		this->score = SCORE_SELF_WIN;// *depth;
		this->winRate = 1;
	}
	else if (winner == Player::Other) {
		this->score = SCORE_OTHER_WIN;// *depth;
		this->winRate = -1;
	}
	else {
		this->score = SCORE_TIE;
		this->winRate = 0;
	}
	this->times = 1;
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

Node* Node::findOrCreateByPattern() {
	
	if (childrenCount > 0)
		return nullptr;
	int selectedLine, selectedCol;
	auto m = Matcher::matchAllAndDoAction(*this, selectedLine, selectedCol);
	if (m) {
		return findOrCreateByColumn(selectedCol);
	}
	return nullptr;
}

bool Node::monteCarloSimOnce() {
	auto winner = getWinner();

	if (winner == Player::None) {
		// randomly select a top
		//vector<int> columnsTried;
		// TODO: should back trace
		for (int i = 0; i < 10; ++i) {
			auto selectedColumn = monteCarloSelectNextColumn();
			auto newNode = findOrCreateByColumn(selectedColumn);
			if (newNode->monteCarloSimOnce()) {
				updateScoreByChildren();
				break;
			}
		};
	}
	else {
		if (times > 0) {
			return false;
		}
		makeWinnerNode(winner);
	}
	return true;
}

int Node::monteCarloSelectNextColumn() const {
	return randomTop();
	//return confidenceSelectNextColomn();
}

#include <random>
int Node::confidenceSelectNextColomn() const {
	auto f = winRate;
	std::random_device rd;
	std::mt19937 gen(rd());

	// [0, 2]
	double *probArray = new double[Node::n];
	for (int _column = 0; _column < Node::n; ++_column) {
		probArray[_column] = 0;
	}
	for (int child = 0; child < childrenCount; ++child) {
		probArray[children[child]->col] = children[child]->winRate + 1;
	}
	for (int _column = 0; _column < Node::n; ++_column) {
		if (top[_column] >= 0 && probArray[_column] == 0)
			probArray[_column] = 1;
	}

	std::discrete_distribution<> d(probArray, probArray + Node::n);
	return d(gen);
}
