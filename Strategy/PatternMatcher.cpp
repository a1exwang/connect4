#include "PatternMatcher.h"
#include "ai.h"
#include <cassert>
#include <memory>
#include <iostream>
using namespace std;
#define m(x) Matcher(x)

std::string R(const std::string &str) {
	std::string ret;
	for (auto c : str) {
		ret += c;
		ret += '\n';
	}
	return ret;
}

Matcher Matcher::allPatterns[] = {
	// 横竖斜的三子
	"$???","?$??","??$?","???$",
	R("$???"),

	/*"$\n"
	"&?\n"
	"&&?\n"
	"&&&?\n",

	"?\n"
	"&$\n"
	"&&?\n"
	"&&&?\n",

	"?\n"
	"&?\n"
	"&&$\n"
	"&&&?\n",

	"?\n"
	"&?\n"
	"&&?\n"
	"&&&$",

	"***?\n"
	"**?&\n"
	"*?&&\n"
	"$&&&",

	"***?\n"
	"**?&\n"
	"*$&&\n"
	"?&&&",

	"***?\n"
	"**$&\n"
	"*?&&\n"
	"?&&&",
	
	"***$\n"
	"**?&\n"
	"*?&&\n"
	"?&&&",*/
	// 两子必胜
	"_?$?_",
	//"&###&",

	"_??$",
	//"&__&"
};

std::shared_ptr<Matcher::MatchData> Matcher::matchAllAndDoAction(const Node &node, int& line, int& column) {
	for (auto p : allPatterns) {
		auto ret = p.matchOnce(node);
		if (ret) {
			if (p.callback) {
				p.callback(*ret, line, column);
			}
			else {
				line = ret->targetLine;
				column = ret->targetColumn;
			}
			return ret;
		}
	}
	return nullptr;
}

Matcher::Matcher(const std::string &strPattern, std::function<void(const MatchData &, int &x, int &y)> callback) 
	:anchorLine(-1), anchorColumn(-1), callback(callback), importantPositions(0) {
	for (auto i = 0; i < PATTERN_MAX_LINES; ++i) {
		for (auto j = 0; j < PATTERN_MAX_COLUMNS; ++j) {
			pattern[i][j] = DONT_CARE;
		}
	}

	auto line = 0, column = 0;
	for (auto c : strPattern) {
		switch(c) {
		case SELF_ANCHOR:
		case OTHER_ANCHOR:
			anchorLine = line;
			anchorColumn = column;
		case OTHER:
		case SELF:
		case ANYTHING:
		case EITHER:
		case OCCUPIED:
			anchors.push_back(Point(line, column));
		case TARGET:
			if (c == TARGET) {
				targetLine = line;
				targetColumn = column;
			}
		case NO:
		case EMPTY:
			importantPositions++;
		default: // DONT_CARE
			pattern[line][column++] = c;
			break;
		case '\n':
			line++;
			column = 0;
			break;
		}


		assert(line >= 0 && line < PATTERN_MAX_LINES &&
			column >= 0 && column < PATTERN_MAX_COLUMNS);
	}
}
Matcher::~Matcher() {
}

std::shared_ptr<Matcher::MatchData> Matcher::matchAll(const Node& node) {
	for (Point pt : anchors) {
		auto ret = matchWithAnchor(node, pt.x, pt.y);
		if (ret)
			return ret;
	}
	return nullptr;
}

std::shared_ptr<Matcher::MatchData> Matcher::matchWithAnchorEnabled(const Node& node) {
	return matchWithAnchor(node, anchorLine, anchorColumn);
}

std::shared_ptr<Matcher::MatchData> Matcher::matchOnce(const Node& node) {
	if (anchorLine >= 0) {
		return matchWithAnchorEnabled(node);
	}
	else {
		return matchAll(node);
	}
}
/*
std::shared_ptr<Matcher::MatchData> Matcher::matchWithAnchor(const Node& node, int myAnchorLine, int myAnchorColumn) {
	auto deltaLine = node.getLine() - myAnchorLine;
	auto deltaColumn = node.getColumn() - myAnchorColumn;

	auto ret = std::make_shared<MatchData>();

	auto matchCount = 0;
	auto eitherPlayer = None;
	for (auto i = 0; i < PATTERN_MAX_LINES; ++i) {
		for (auto j = 0; j < PATTERN_MAX_COLUMNS; ++j) {
			auto boardLine = deltaLine + i;
			auto boardColumn = deltaColumn + j;

			if (boardLine < 0 || boardLine >= Node::m || boardColumn < 0 || boardColumn >= Node::n) {
				switch(pattern[i][j]) {
				default:
					return nullptr;
				case ANYTHING:
				case OCCUPIED:
					matchCount++;
				case DONT_CARE:
					continue;
				}
				break;
			}
			auto currentPosition = node.get(boardLine, boardColumn);
			ret->points[i][j].line = boardLine;
			ret->points[i][j].column = boardColumn;
			ret->points[i][j].player = currentPosition;


			switch (pattern[i][j]) {
			case SELF:
			case SELF_ANCHOR:
				if (currentPosition != Player::Self)
					return nullptr;
				matchCount++;
				break;
			case OTHER:
			case OTHER_ANCHOR:
				if (currentPosition != Player::Other)
					return nullptr;
				matchCount++;
				break;
			case EMPTY:
			case TARGET:
				if (currentPosition != Player::None)
					return nullptr;
				matchCount++;
				if (pattern[i][j] == TARGET) {
					ret->targetLine = boardLine;
					ret->targetColumn = boardColumn;
					if (node.getTop(boardColumn) != boardLine)
						return nullptr;
				}
				break;
			case EITHER:
				if (eitherPlayer == Player::None) {
					if (currentPosition != Player::Self && currentPosition != Player::Other)
						return nullptr;
					eitherPlayer = currentPosition;
				}
				else {
					if (currentPosition != eitherPlayer)
						return nullptr;
				}
				cout << "round: " << node.depth << ' ' << "eitherplayer: " << eitherPlayer << endl;

				matchCount++;
				break;
			case OCCUPIED:
				if (currentPosition != Player::Self && currentPosition != Player::Other && currentPosition != Player::No)
					return nullptr;
				matchCount++;
				break;
			case NO:
				if (currentPosition != Player::No)
					return nullptr;
				matchCount++;
				break;
			case ANYTHING:
				matchCount++;
			default: // DONT_CARE
				break;
			}
			if (matchCount >= importantPositions) {
				return ret;
			}
		}
	}
	if (matchCount >= importantPositions)
		return ret;
	else
		return nullptr;
}
*/