#pragma once
#include "ai.h"

constexpr uint32_t INVALID_NODE_INDEX = 0xFFFFFFFF;

constexpr int TREE_ALLOCATOR_SIZE = 1 * 10000;

class TreeAllocator {
public:
	static TreeAllocator *get();
	TreeAllocator();

	Node *allocateNode();
	Node *get(uint32_t index) {
		if (index == INVALID_NODE_INDEX)
			return nullptr;
		return &buffer[index];
	}
	uint32_t getIndex(Node *node) {
		if (!node)
			return INVALID_NODE_INDEX;
		return node - buffer;
	}
	void recycleNode(const Node *node);
	static void destroy();
protected:
	static TreeAllocator theInstance;
	void reset();
private:
	Node* reusableNodes[TREE_ALLOCATOR_SIZE];
	int reusableNodesSize;
	int maxUsed;

	Node buffer[TREE_ALLOCATOR_SIZE];
	int currentSize;
	int totalSize;
};

