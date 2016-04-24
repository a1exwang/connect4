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
Node *Node::root = nullptr;

void Node::cloneMatrix(Node &node) const {
	memcpy(node.board, board, sizeof(board));
}

void Node::ai(int &out_line, int &out_column) {
	int selected_line = -1, selected_col = -1;
	srand(static_cast<unsigned int>(time(nullptr)));


#ifdef _DEBUG
	auto t1 = GetTickCount();
	cout << "sizeof(Node) " << sizeof(Node) << endl;
#endif

	for (auto j = 0; j < MONTE_CARLO_TIMES; ++j) {
		Node::root->monteCarloSimOnce();
	}
#ifdef _DEBUG
	cout << "round: " << (Node::root->depth+1)/2 << ", times: " << Node::root->times << endl;
#endif

	auto max = -1000.0;

	for (auto i = 0; i < Node::root->childrenCount; ++i) {
		//auto child = Node::root->getChild(i);
		auto child = Node::root->children[i];

#ifdef _DEBUG
		cout << "\tcolumn: " << child->getColumn() << ", score: "
			<< child->score << ", winrate: " << child->getWinRate() << endl;
#endif

		if (child->getWinRate() > max) {
			max = child->getWinRate();
			selected_line = child->line;
			selected_col = child->col;
		}
	}

	// select
	playerMove(selected_line, selected_col, Self);

	out_line = selected_line;
	out_column = selected_col;
#ifdef _DEBUG
	auto t2 = GetTickCount();
	cout << "time: " << t2 - t1 << endl;
#endif
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
	//return getChild(rand() % childrenCount);
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

void Node::initRoot(bool opponentFirst, int thisLine, int col, const int *top) {
	auto node = createNode();
	node->col = col;
	node->player = opponentFirst ? Other : Self;
	//node->parent = nullptr;
	for (auto i = 0; i < Node::m; ++i) {
		for (auto j = 0; j < Node::n; ++j) {
			node->board[i][j] = None;
		}
	}
	node->board[Node::noLine][Node::noColumn] = No;
	for (int i = 0; i < Node::n; ++i) {
		node->top[i] = top[i];
	}
	node->childrenCount = 0;
	node->score = 0;
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
	for (auto i = 0; i < oldRoot->childrenCount; ++i) {
		//if (oldRoot->getChild(i)->col == column) {
		//newRoot = oldRoot->getChild(i);
		//oldRoot->setChild(i, nullptr);
		if (oldRoot->children[i]->col == column) {
			newRoot = oldRoot->children[i];
			oldRoot->children[i] = nullptr;
			break;
		}
	}

	if (newRoot) {
		root = newRoot;
	}
	else {
		// not in children
		root = oldRoot->findOrCreateByColumn(column, false);
	}

	destroy(oldRoot);
}

void Node::resetAll() {
	if (Node::root)
		destroy(Node::root);
	Node::m = -1;
	Node::n = -1;
	Node::noLine = -1;
	Node::noColumn = -1;
	Node::root = nullptr;
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
	score = 0;
	times = 0;
	//winRate = 0;
	childrenCount = 0;
#ifdef _DEBUG
	depth = 1;
#endif
	return this;
}

Node::Node() :
	/*parent(nullptr),*/ player(None), line(-1), col(-1), score(-1), 
	childrenCount(0)/*, patternMatched(false) */{
#ifdef _DEBUG
	depth = 1;
#endif;
}

Node::~Node() {
	/*for (auto i = 0; i < childrenCount; ++i) {
		delete children[i];
	}*/
}

Node* Node::findOrCreateByColumn(int thisCol, bool addChild) {
	for (auto i = 0; i < childrenCount; ++i) {
		//if (getChild(i)->col == thisCol) {
		if (children[i]->col == thisCol) {
			if (addChild) {
				//return getChild(i);
				return children[i];
			}
			else {
				//children[i]->parent = nullptr;
				assert(false);
			}
		}
	}
	auto node = createNode();
	*node = *this;
#ifdef _DEBUG
	node->depth = depth + 1;
#endif
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
		destroy(node);
		return nullptr;
	}
	node->line = node->top[thisCol] + 1;

	if (node->line < 0) {
		assert(false);
	}
	node->col = thisCol;
	cloneMatrix(*node);
	if(node->board[node->line][node->col] != None) {
		assert(node->board[node->line][node->col] == None);
	}
	node->board[node->line][node->col] = node->player;
	node->childrenCount = 0;
	node->score = DEFAULT_SCORE;
	//node->patternMatched = false;
	node->times = 0;
	//node->winRate = 0;

	if (addChild) {
		//setChild(childrenCount, node);
		children[childrenCount] = node;
		childrenCount++;
		//node->parent = this;
	}
	return node;
}

void Node::updateScoreByChildren() {
	score = 0;
	times = 0;
	for (int i = 0; i < childrenCount; ++i) {
		//score += getChild(i)->score;
		//times += getChild(i)->times;
		score += children[i]->score;
		times += children[i]->times;
	}
	//if ()
	//winRate = static_cast<double>(score) / times;
}

void Node::makeWinnerNode(Player winner) {
	if (winner == Self) {
		this->score += SCORE_SELF_WIN;// *depth;
		//this->winRate = 1;
	}
	else if (winner == Other) {
		this->score += SCORE_OTHER_WIN;// *depth;
		//this->winRate = -1;
	}
	else {
		this->score += SCORE_TIE;
		//this->winRate = 0;
	}
	this->times += 1;
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

void Node::becomeChild(int column) {
	
}


int Node::monteCarloSimOnceDestroy() {
	auto winner = getWinner();

	if (winner == None) {
		auto selectedColumn = monteCarloSelectNextColumn();
		becomeChild(selectedColumn);
		return monteCarloSimOnceDestroy();
	}
	else {
		if (winner == Self) {
			return SCORE_SELF_WIN;
		}
		else if (winner == Other) {
			return SCORE_OTHER_WIN;						   
		}
		else {
			return SCORE_TIE;
		}
	}
}


bool Node::monteCarloSimOnce() {

	auto winner = getWinner();

	if (winner == None) {
		// randomly select a top
		//vector<int> columnsTried;
		// TODO: should back trace
		//for (int i = 0; i < 10; ++i) {
			auto selectedColumn = monteCarloSelectNextColumn();
			auto newNode = findOrCreateByColumn(selectedColumn);
			if (newNode->monteCarloSimOnce()) {
				updateScoreByChildren();
				//break;
			}
		//}
	}
	else {
		/*if (times > 0) {
			return false;
		}*/
		makeWinnerNode(winner);
	}
	return true;
}

int Node::monteCarloSelectNextColumn() const {
	return randomTop();
}
