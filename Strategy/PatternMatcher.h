#pragma once
#include <string>
#include <vector>
#include <memory>
#include "ai.h"
#include <functional>
#include "Point.h"
struct Node;

#define PATTERN_MAX_LINES 15
#define PATTERN_MAX_COLUMNS 15

enum {
	DONT_CARE = '#',
	ANYTHING = '*', 
	SELF = 'X',
	SELF_ANCHOR = 'x',
	OTHER = 'O',
	OTHER_ANCHOR = 'o',	// OTHER and last position
	EITHER = '?',		// SELF or OTHER but all ? must be the same
	OCCUPIED = '&',		// SELF or OTHER or NO
	NO = 'N',			// NO
	EMPTY = '_',		// must be empty
	INVALID = '!',		// invalid point (not initialized)
	TARGET = '$'		// EMPTY and this is the target to match
};


class Matcher {
public:
	enum PositionProperty {
		None = 0
	};	
	struct MatcherPoint {
		Player player;
		int line;
		int column;
		//PositionProperty property;
		MatcherPoint() :player(None), line(-1), column(-1) { }
	};
	struct MatchData {
		MatcherPoint points[PATTERN_MAX_LINES][PATTERN_MAX_COLUMNS];
		int targetLine, targetColumn;
		MatchData() :targetLine(-1), targetColumn(-1) { }
		//int lines, columns;
		//Player getData(int line, int column) const;
	};
public:
	static std::shared_ptr<MatchData> matchAllAndDoAction(const Node &node, int& line, int& column);
	Matcher(const std::string &pattern, std::function<void(const MatchData &matchData, int &x, int &y)> callback = nullptr);
	~Matcher();
	std::shared_ptr<Matcher::MatchData> matchWithAnchorEnabled(const Node &node);
	std::shared_ptr<Matcher::MatchData> matchAll(const Node &node);
	std::shared_ptr<Matcher::MatchData> matchOnce(const Node &node);

private:
	std::shared_ptr<Matcher::MatchData> matchWithAnchor(const Node &node, int myAnchorLine, int myAnchorColumn);

private:
	static Matcher allPatterns[];
private:
	char pattern[PATTERN_MAX_LINES][PATTERN_MAX_COLUMNS];
	int anchorLine, anchorColumn;
	std::vector<Point> anchors;
	int importantPositions;
	int targetLine, targetColumn;
	std::function<void(const MatchData &, int &x, int &y)> callback;
	//int lines, columns;
};

