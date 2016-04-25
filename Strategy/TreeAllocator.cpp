#include "TreeAllocator.h"
#include <cassert>
#include <iostream>
using namespace std;

TreeAllocator::TreeAllocator() {
	reset();
}

TreeAllocator* TreeAllocator::get() {
	return &theInstance;
}

TreeAllocator TreeAllocator::theInstance;

Node* TreeAllocator::allocateNode() {
	//return new Node;
	if (reusableNodesSize == 0) {
		if (currentSize > maxUsed)
			maxUsed = currentSize;
#ifdef _DEBUG
		if (maxUsed % 100 == 0)
			cout << "max used " << maxUsed << endl;
		if (currentSize >= totalSize)
			cout << "current size: " << currentSize << endl;
#endif
		if (currentSize >= totalSize) {
			assert(false);
		}

		auto ret = &buffer[currentSize];
		currentSize++;
		return ret;
	}
	else {
		--reusableNodesSize;
		auto node = reusableNodes[reusableNodesSize];
		reusableNodes[reusableNodesSize] = nullptr;
		//expandReusableNode(node);
		return node;
	}
}

void TreeAllocator::recycleNode(const Node* node) {
	/*for (auto i = 0; i < node->childrenCount; ++i) {
		if (node->children[i])
			recycleNode(node->children[i]);
	}
	delete node;
	return;*/
	reusableNodes[reusableNodesSize++] = const_cast<Node*>(node);
}

void TreeAllocator::destroy() {
	//Node::destroy(Node::root);
	TreeAllocator::theInstance.reset();
}


/*void TreeAllocator::expandReusableNode(Node *node) {
	auto childrenCount = node->childrenCount;//node->getChildrenCount();
	//cout << "expand node " << node << " count " << childrenCount << endl;
	for (auto i = 0; i < childrenCount; ++i) {
		//auto child = node->getChild(i);
		auto child = node->children[i];
		if (child)
			reusableNodes[reusableNodesSize++] = const_cast<Node*>(child);
	}
}*/

void TreeAllocator::reset() {
	memset(reusableNodes, 0, sizeof(reusableNodes));
	totalSize = TREE_ALLOCATOR_SIZE;
	currentSize = 0;
	reusableNodesSize = 0;
	maxUsed = 0;
}
