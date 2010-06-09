#include <stdlib.h>

#include "bintree.h"

struct bt_node* bt_new_node(void *data) {
	struct bt_node *node = malloc(sizeof(struct bt_node));
	node->data = data;
	node->left = node->right = NULL;
	return node;
}

void bt_free_node(struct bt_node *node) {
	free(node);
}

struct bt_node** bt_search(struct bt_node **tree,int (*compare)(const void *,const void *),void *data) {
	/* didn't find element in tree */
	if(*tree == NULL) {
		return tree;
	}
	
	/* search in left/right subtree */
	int compare_result = compare((const void*) (*tree)->data,(const void*) data);
	if(compare_result < 0) {
		return bt_search(&(*tree)->left,compare,data);	
	} else if (compare_result > 0) {
		return bt_search(&(*tree)->right,compare,data);	
	}

	/* found element */
	return tree;
}

void bt_insert(struct bt_node **tree, int (*compare)(const void *, const void *), void *data) {
	/* look for element in tree */
	struct bt_node **node = bt_search(tree,compare,data);
	/* add element if it isn't in the tree */
	if(*node == NULL) {
		*node = bt_new_node(data);
	}
}

void bt_delete(struct bt_node **node) {
	struct bt_node *old_node = *node;
	/* case 1: node doesn't have any children */
	if((*node)->left == NULL && (*node)->right == NULL) {
		bt_free_node(old_node);
		return;
	}
	/* case 2: node has one child */
	if((*node)->left == NULL) {
		*node = (*node)->right;
		bt_free_node(old_node);
		return;
	} else if((*node)->right == NULL) {
		*node = (*node)->left;
		bt_free_node(old_node);
		return;
	}
	/* case 3: node has two children */
	/* find predecessor */
	struct bt_node **pred = &(*node)->left;
	while((*pred)->right != NULL) {
		pred = &(*pred)->right;
	}
	/* swap values */
	void *tmp = (*node)->data;
	(*node)->data = (*pred)->data;
	(*pred)->data = tmp;
	/* delete predecessor */
	bt_delete(pred);
}
