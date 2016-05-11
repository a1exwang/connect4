#pragma once

#define UCT_ALLOCATOR_DEFAULT_SIZE (450 * 10000)

#ifdef __cplusplus
extern "C" {
#endif
	void uct_allocator_init(void);
	void uct_allocator_destroy(void);

	struct T_UCTNode *uct_alloc();
	void uct_recycle(struct T_UCTNode *node);
	void uct_recycle_all(void);
#ifdef __cplusplus
}
#endif
