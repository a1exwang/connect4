#include "uct_allocator.h"
#include "uct.h"
#include <stdlib.h>
#include <assert.h>

static UCTNode *uct_buffer = NULL;
static int uct_count = 0;

static UCTNode **uct_reusable_nodes = NULL;
static int uct_reusable_count = 0;

void uct_allocator_init() {
	uct_buffer = malloc(UCT_ALLOCATOR_DEFAULT_SIZE * sizeof(UCTNode));
	uct_reusable_nodes = malloc(UCT_ALLOCATOR_DEFAULT_SIZE * sizeof(UCTNode*));
}
void uct_allocator_destroy(void) {
	free(uct_buffer);
	free(uct_reusable_nodes);
	uct_buffer = NULL;
	uct_reusable_nodes = NULL;
}

static void expand_node(UCTNode *node) {
	for (int i = 0; i < node->children_count; ++i) {
		uct_reusable_nodes[uct_reusable_count++] = node->children[i];
	}
}
struct T_UCTNode *uct_alloc() {
	assert(uct_count < UCT_ALLOCATOR_DEFAULT_SIZE);
	if (uct_reusable_count == 0) {
		return &uct_buffer[uct_count++];
	}
	else {
		UCTNode *node = uct_reusable_nodes[--uct_reusable_count];
		expand_node(node);
		return node;
	}
	//return malloc(sizeof(UCTNode));
}

void uct_recycle(struct T_UCTNode *node) {
	uct_reusable_nodes[uct_reusable_count++] = node;
	//free(node);
}
void uct_recycle_all(void) {
	uct_count = 0;
	uct_reusable_count = 0;
}