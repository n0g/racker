struct bt_node {
	void *data;
	struct bt_node *left, *right;
};

struct bt_node* bt_new_node(void *data);
void bt_free_node(struct bt_node *node);
struct bt_node** bt_search(struct bt_node **tree,int (*compare)(const void *, const void *), void *data);
void bt_insert(struct bt_node **tree,int (*compare)(const void *, const void *), void *data);
void bt_delete(struct bt_node **node);
