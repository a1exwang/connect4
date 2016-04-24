#pragma once

#include <vector>

typedef int** Matrix;

typedef char Player;
enum {
	None = 0,
	Other = 1,
	Self = 2,
	Tie = 3,
	No = 4
};
constexpr int MONTE_CARLO_TIMES = 120000;
constexpr int DEFAULT_SCORE = 0;
constexpr int SCORE_SELF_WIN = 1;
constexpr int SCORE_TIE = 0;
constexpr int SCORE_OTHER_WIN = -1;

constexpr int MAX_LINES = 13;
constexpr int MAX_COLUMNS = 13;

#pragma pack(push, 4)
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
	void set(int line, int column, Player p) {
		board[line][column] = p;
	}
	int getLine() const { return line; }
	int getColumn() const { return col; }
	int getChildrenCount() const { return childrenCount; }
	//Node *getChild(int i) const;
	//void setChild(int i, Node *child);
private:
	Node *init();
	friend class TreeAllocator;
	Node();
	Node *findOrCreateByPattern();
	bool monteCarloSimOnce();
	int monteCarloSimOnceDestroy();
	int monteCarloSelectNextColumn() const;
	int confidenceSelectNextColomn() const;
	int randomTop() const;
	Node *randomChild() const;
	Player getWinner() const;
	bool otherWin(int, int) const;
	bool selfWin(int, int) const;
	void Node::cloneMatrix(Node &node) const;
	double getWinRate() const {
		return static_cast<double>(score) / times;
	}

	void becomeChild(int column);
public:
	Player board[MAX_LINES][MAX_COLUMNS];
	char top[MAX_COLUMNS];

	Player player;
	int8_t line, col;
	int32_t score;
	int times;
#ifdef _DEBUG
	int depth;
#endif

	//double winRate;

	//uint32_t children[MAX_COLUMNS];
	Node *children[MAX_COLUMNS];
	int8_t childrenCount;
	//Node *parent;

	//bool patternMatched;

	friend class __Initializer;
};
#pragma pack(pop)