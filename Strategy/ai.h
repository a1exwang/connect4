#pragma once
#include <vector>

typedef int** Matrix;

enum Player {
	None = 0,
	Other = 1,
	Self = 2,
	Tie = 3,
	No = 4
};
constexpr int MONTE_CARLO_TIMES = 10000;
constexpr int DEFAULT_SCORE = 0;
constexpr int SCORE_SELF_WIN = 1;
constexpr int SCORE_TIE = 0;
constexpr int SCORE_OTHER_WIN = -1;

constexpr int MAX_LINES = 13;
constexpr int MAX_COLUMNS = 13;

struct Node {
public:
	static void initRoot(bool opponentFirst, int, int col, const int *top);
	static void playerMove(int line, int column, Player player);
	static void ai(int &, int &);
	static int m, n, noLine, noColumn;
	static Node *root;
	static void resetAll();
public:
	static Node *createNode();
	static void destroy(const Node*);
	~Node();
	Node *findOrCreateByColumn(int colunm, bool addChild = true);
	//Player setPos(int line, int col, Player player);
	void updateScoreByChildren();
	void makeWinnerNode(Player winner);
	Player otherPlayer() const;
	//void monteCarloSim();
	int getTop(int column) const { return top[column]; }
	Player get(int line, int column) const {
		return board[line][column];
	}
	int getLine() const { return line; }
	int getColumn() const { return col; }
	int getChildrenCount() const { return childrenCount; }
	Node *getChild(int i) const { return children[i]; }
private:
	friend class TreeAllocator;
	Node();
	Node *findOrCreateByPattern();
	bool monteCarloSimOnce();
	int monteCarloSelectNextColumn() const;
	int confidenceSelectNextColomn() const;
	int randomTop() const;
	Node *randomChild() const;
	Player getWinner() const;
	bool otherWin(int, int) const;
	bool selfWin(int, int) const;
	void Node::cloneMatrix(Node &node) const;
public:
	Player board[MAX_LINES][MAX_COLUMNS];
	int top[MAX_COLUMNS];

	Player player;
	int line, col;
	int64_t score;
	int depth;
	int times;
	double winRate;

	Node* children[MAX_COLUMNS];
	int childrenCount;
	Node *parent;

	bool patternMatched;

	friend class __Initializer;
};