/*********************************************************************
 *
 * avlmini.h - fast as linux's rbtree, but much smaller
 *
 * NOTE:
 * for more information, please see the readme file
 *
 *********************************************************************/
#ifndef _AVLMINI_H__
#define _AVLMINI_H__


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_NOT_STDDEF_H
#include <stddef.h>
#endif


/*====================================================================*/
/* GLOBAL MACROS                                                      */
/*====================================================================*/
#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif

/* you can change this by config.h or predefined macro */
#ifndef ASSERTION
#define ASSERTION(x) ((void)0)
#endif


/*====================================================================*/
/* avl_node - avl binary search tree                                  */
/*====================================================================*/
struct avl_node
{
	struct avl_node *left;
	struct avl_node *right;
	struct avl_node *parent;    /* pointing to node itself for empty node */
	int height;                 /* equals to 1 + max height in childs */
};

struct avl_root
{
	struct avl_node *node;		/* root node */
};


/*--------------------------------------------------------------------*/
/* NODE MACROS                                                        */
/*--------------------------------------------------------------------*/
#define AVL_LEFT    0        /* left child index */
#define AVL_RIGHT   1        /* right child index */

#define AVL_OFFSET(TYPE, MEMBER)    ((size_t) &((TYPE *)0)->MEMBER)

#define AVL_NODE2DATA(n, o)    ((void *)((size_t)(n) - (o)))
#define AVL_DATA2NODE(d, o)    ((struct avl_node*)((size_t)(d) + (o)))

#define AVL_ENTRY(ptr, type, member) \
	((type*)AVL_NODE2DATA(ptr, AVL_OFFSET(type, member)))

#define avl_node_init(node) do { ((node)->parent) = (node); } while (0)
#define avl_node_empty(node) ((node)->parent == (node))

#define AVL_LEFT_HEIGHT(node) (((node)->left)? ((node)->left)->height : 0)
#define AVL_RIGHT_HEIGHT(node) (((node)->right)? ((node)->right)->height : 0)


#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------*/
/* binary search tree - node manipulation                             */
/*--------------------------------------------------------------------*/

struct avl_node *avl_node_first(struct avl_root *root);
struct avl_node *avl_node_last(struct avl_root *root);
struct avl_node *avl_node_next(struct avl_node *node);
struct avl_node *avl_node_prev(struct avl_node *node);

void avl_node_replace(struct avl_node *victim, struct avl_node *newnode,
		struct avl_root *root);

static inline void avl_node_link(struct avl_node *node, struct avl_node *parent,
		struct avl_node **avl_link) {
	node->parent = parent;
	node->height = 0;
	node->left = node->right = NULL;
	avl_link[0] = node;
}

/* avl insert rebalance and erase */
void avl_node_post_insert(struct avl_node *node, struct avl_root *root);
void avl_node_erase(struct avl_node *node, struct avl_root *root);

/* tear down the whole tree */
struct avl_node* avl_node_tear(struct avl_root *root, struct avl_node **next);


/*--------------------------------------------------------------------*/
/* avl node templates                                                 */
/*--------------------------------------------------------------------*/

#define avl_node_find(root, what, compare_fn, res_node) do {\
		struct avl_node *__n = (root)->node; \
		(res_node) = NULL; \
		while (__n) { \
			int __hr = (compare_fn)(what, __n); \
			if (__hr == 0) { (res_node) = __n; break; } \
			else if (__hr < 0) { __n = __n->left; } \
			else { __n = __n->right; } \
		} \
	}   while (0)


#define avl_node_add(root, newnode, compare_fn, duplicate_node) do { \
		struct avl_node **__link = &((root)->node); \
		struct avl_node *__parent = NULL; \
		struct avl_node *__duplicate = NULL; \
		int __hr = 1; \
		while (__link[0]) { \
			__parent = __link[0]; \
			__hr = (compare_fn)(newnode, __parent); \
			if (__hr == 0) { __duplicate = __parent; break; } \
			else if (__hr < 0) { __link = &(__parent->left); } \
			else { __link = &(__parent->right); } \
		} \
		(duplicate_node) = __duplicate; \
		if (__duplicate == NULL) { \
			avl_node_link(newnode, __parent, __link); \
			avl_node_post_insert(newnode, root); \
		} \
	}   while (0)


/*====================================================================*/
/* avl_tree - easy interface                                          */
/*====================================================================*/

struct avl_tree
{
	struct avl_root root;		/* avl root */
	size_t offset;				/* node offset in user data structure */
	size_t size;                /* size of user data structure */
	size_t count;				/* node count */
	/* returns 0 for equal, -1 for n1 < n2, 1 for n1 > n2 */
	int (*compare)(const void *n1, const void *n2);
};


/* initialize avltree, use AVL_OFFSET(type, member) for "offset"
 * eg:
 *     avl_tree_init(&mytree, mystruct_compare,
 *          sizeof(struct mystruct_t), 
 *          AVL_OFFSET(struct mystruct_t, node));
 */
void avl_tree_init(struct avl_tree *tree,
		int (*compare)(const void*, const void*), size_t size, size_t offset);

void *avl_tree_first(struct avl_tree *tree);
void *avl_tree_last(struct avl_tree *tree);
void *avl_tree_next(struct avl_tree *tree, void *data);
void *avl_tree_prev(struct avl_tree *tree, void *data);

/* require a temporary user structure (data) which contains the key */
void *avl_tree_find(struct avl_tree *tree, const void *data);
void *avl_tree_nearest(struct avl_tree *tree, const void *data);

/* returns NULL for success, otherwise returns conflict node with same key */
void *avl_tree_add(struct avl_tree *tree, void *data);

void avl_tree_remove(struct avl_tree *tree, void *data);
void avl_tree_replace(struct avl_tree *tree, void *victim, void *newdata);

void avl_tree_clear(struct avl_tree *tree, void (*destroy)(void *data));




#ifdef __cplusplus
}
#endif


#endif

#include "avlmini.h"


/*====================================================================*/
/* Binary Search Tree                                                 */
/*====================================================================*/

struct avl_node *avl_node_first(struct avl_root *root)
{
	struct avl_node *node = root->node;
	if (node == NULL) return NULL;
	while (node->left) 
		node = node->left;
	return node;
}

struct avl_node *avl_node_last(struct avl_root *root)
{
	struct avl_node *node = root->node;
	if (node == NULL) return NULL;
	while (node->right) 
		node = node->right;
	return node;
}

struct avl_node *avl_node_next(struct avl_node *node)
{
	if (node == NULL) return NULL;
	if (node->right) {
		node = node->right;
		while (node->left) 
			node = node->left;
	}
	else {
		while (1) {
			struct avl_node *last = node;
			node = node->parent;
			if (node == NULL) break;
			if (node->left == last) break;
		}
	}
	return node;
}

struct avl_node *avl_node_prev(struct avl_node *node)
{
	if (node == NULL) return NULL;
	if (node->left) {
		node = node->left;
		while (node->right) 
			node = node->right;
	}
	else {
		while (1) {
			struct avl_node *last = node;
			node = node->parent;
			if (node == NULL) break;
			if (node->right == last) break;
		}
	}
	return node;
}

static inline void 
_avl_child_replace(struct avl_node *oldnode, struct avl_node *newnode, 
		struct avl_node *parent, struct avl_root *root) 
{
	if (parent) {
		if (parent->left == oldnode)
			parent->left = newnode;
		else 
			parent->right = newnode;
	}	else {
		root->node = newnode;
	}
}

static inline struct avl_node *
_avl_node_rotate_left(struct avl_node *node, struct avl_root *root)
{
	struct avl_node *right = node->right;
	struct avl_node *parent = node->parent;
	node->right = right->left;
	ASSERTION(node && right);
	if (right->left) 
		right->left->parent = node;
	right->left = node;
	right->parent = parent;
	_avl_child_replace(node, right, parent, root);
	node->parent = right;
	return right;
}

static inline struct avl_node *
_avl_node_rotate_right(struct avl_node *node, struct avl_root *root)
{
	struct avl_node *left = node->left;
	struct avl_node *parent = node->parent;
	node->left = left->right;
	ASSERTION(node && left);
	if (left->right) 
		left->right->parent = node;
	left->right = node;
	left->parent = parent;
	_avl_child_replace(node, left, parent, root);
	node->parent = left;
	return left;
}

void avl_node_replace(struct avl_node *victim, struct avl_node *newnode,
		struct avl_root *root)
{
	struct avl_node *parent = victim->parent;
	_avl_child_replace(victim, newnode, parent, root);
	if (victim->left) victim->left->parent = newnode;
	if (victim->right) victim->right->parent = newnode;
	newnode->left = victim->left;
	newnode->right = victim->right;
	newnode->parent = victim->parent;
	newnode->height = victim->height;
}


/*--------------------------------------------------------------------*/
/* avl - node manipulation                                            */
/*--------------------------------------------------------------------*/

static inline int AVL_MAX(int x, int y) 
{
	return (x < y)? y : x;
}

static inline void
_avl_node_height_update(struct avl_node *node)
{
	int h0 = AVL_LEFT_HEIGHT(node);
	int h1 = AVL_RIGHT_HEIGHT(node);
	node->height = AVL_MAX(h0, h1) + 1;
}

static inline struct avl_node *
_avl_node_fix_l(struct avl_node *node, struct avl_root *root)
{
	struct avl_node *right = node->right;
	int rh0, rh1;
	ASSERTION(right);
	rh0 = AVL_LEFT_HEIGHT(right);
	rh1 = AVL_RIGHT_HEIGHT(right);
	if (rh0 > rh1) {
		right = _avl_node_rotate_right(right, root);
		_avl_node_height_update(right->right);
		_avl_node_height_update(right);
		/* _avl_node_height_update(node); */
	}
	node = _avl_node_rotate_left(node, root);
	_avl_node_height_update(node->left);
	_avl_node_height_update(node);
	return node;
}

static inline struct avl_node *
_avl_node_fix_r(struct avl_node *node, struct avl_root *root)
{
	struct avl_node *left = node->left;
	int rh0, rh1;
	ASSERTION(left);
	rh0 = AVL_LEFT_HEIGHT(left);
	rh1 = AVL_RIGHT_HEIGHT(left);
	if (rh0 < rh1) {
		left = _avl_node_rotate_left(left, root);
		_avl_node_height_update(left->left);
		_avl_node_height_update(left);
		/* _avl_node_height_update(node); */
	}
	node = _avl_node_rotate_right(node, root);
	_avl_node_height_update(node->right);
	_avl_node_height_update(node);
	return node;
}

static inline void 
_avl_node_rebalance(struct avl_node *node, struct avl_root *root)
{
	while (node) {
		int h0 = (int)AVL_LEFT_HEIGHT(node);
		int h1 = (int)AVL_RIGHT_HEIGHT(node);
		int diff = h0 - h1;
		int height = AVL_MAX(h0, h1) + 1;
		if (node->height != height) {
			node->height = height;
		}	
		else if (diff >= -1 && diff <= 1) {
			break;
		}
		/* printf("rebalance %d\n", avl_value(node)); */
		if (diff <= -2) {
			node = _avl_node_fix_l(node, root);
		}
		else if (diff >= 2) {
			node = _avl_node_fix_r(node, root);
		}
		node = node->parent;
		/* printf("parent %d\n", (!node)? -1 : avl_value(node)); */
	}
}

void avl_node_post_insert(struct avl_node *node, struct avl_root *root)
{
	node->height = 1;
#if 0
	_avl_node_rebalance(node->parent, root);
#else
	for (node = node->parent; node; node = node->parent) {
		int h0 = (int)AVL_LEFT_HEIGHT(node);
		int h1 = (int)AVL_RIGHT_HEIGHT(node);
		int height = AVL_MAX(h0, h1) + 1;
		int diff = h0 - h1;
		if (node->height == height) break;
		node->height = height;
		/* printf("rebalance %d\n", avl_value(node)); */
		if (diff <= -2) {
			node = _avl_node_fix_l(node, root);
		}
		else if (diff >= 2) {
			node = _avl_node_fix_r(node, root);
		}
		/* printf("parent %d\n", (!node)? -1 : avl_value(node)); */
	}
#endif
}

void avl_node_erase(struct avl_node *node, struct avl_root *root)
{
	struct avl_node *child, *parent;
	ASSERTION(node);
	if (node->left && node->right) {
		struct avl_node *old = node;
		struct avl_node *left;
		node = node->right;
		while ((left = node->left) != NULL)
			node = left;
		child = node->right;
		parent = node->parent;
		if (child) {
			child->parent = parent;
		}
		_avl_child_replace(node, child, parent, root);
		if (node->parent == old)
			parent = node;
		node->left = old->left;
		node->right = old->right;
		node->parent = old->parent;
		node->height = old->height;
		_avl_child_replace(old, node, old->parent, root);
		ASSERTION(old->left);
		old->left->parent = node;
		if (old->right) {
			old->right->parent = node;
		}
	}
	else {
		if (node->left == NULL) 
			child = node->right;
		else
			child = node->left;
		parent = node->parent;
		_avl_child_replace(node, child, parent, root);
		if (child) {
			child->parent = parent;
		}
	}
	if (parent) {
		_avl_node_rebalance(parent, root);
	}
}


/* tear down the whole tree */
struct avl_node* avl_node_tear(struct avl_root *root, struct avl_node **next)
{
	struct avl_node *node = *next;
	struct avl_node *parent;
	if (node == NULL) {
		if (root->node == NULL) 
			return NULL;
		node = root->node;
	}
	/* sink down to the leaf */
	while (1) {
		if (node->left) node = node->left;
		else if (node->right) node = node->right;
		else break;
	}
	/* tear down one leaf */
	parent = node->parent;
	if (parent == NULL) {
		*next = NULL;
		root->node = NULL;
		return node;
	}
	if (parent->left == node) {
		parent->left = NULL;
	}	else {
		parent->right = NULL;
	}
	node->height = 0;
	*next = parent;
	return node;
}


/*====================================================================*/
/* avl_tree - easy interface                                          */
/*====================================================================*/

void avl_tree_init(struct avl_tree *tree,
	int (*compare)(const void*, const void*), size_t size, size_t offset)
{
	tree->root.node = NULL;
	tree->offset = offset;
	tree->size = size;
	tree->count = 0;
	tree->compare = compare;
}


void *avl_tree_first(struct avl_tree *tree)
{
	struct avl_node *node = avl_node_first(&tree->root);
	if (!node) return NULL;
	return AVL_NODE2DATA(node, tree->offset);
}

void *avl_tree_last(struct avl_tree *tree)
{
	struct avl_node *node = avl_node_last(&tree->root);
	if (!node) return NULL;
	return AVL_NODE2DATA(node, tree->offset);
}

void *avl_tree_next(struct avl_tree *tree, void *data)
{
	struct avl_node *nn;
	if (!data) return NULL;
	nn = AVL_DATA2NODE(data, tree->offset);
	nn = avl_node_next(nn);
	if (!nn) return NULL;
	return AVL_NODE2DATA(nn, tree->offset);
}

void *avl_tree_prev(struct avl_tree *tree, void *data)
{
	struct avl_node *nn;
	if (!data) return NULL;
	nn = AVL_DATA2NODE(data, tree->offset);
	nn = avl_node_prev(nn);
	if (!nn) return NULL;
	return AVL_NODE2DATA(nn, tree->offset);
}


/* require a temporary user structure (data) which contains the key */
void *avl_tree_find(struct avl_tree *tree, const void *data)
{
	struct avl_node *n = tree->root.node;
	int (*compare)(const void*, const void*) = tree->compare;
	int offset = tree->offset;
	while (n) {
		void *nd = AVL_NODE2DATA(n, offset);
		int hr = compare(data, nd);
		if (hr == 0) {
			return nd;
		}
		else if (hr < 0) {
			n = n->left;
		}
		else {
			n = n->right;
		}
	}
	return NULL;
}


void *avl_tree_nearest(struct avl_tree *tree, const void *data)
{
	struct avl_node *n = tree->root.node;
	struct avl_node *p = NULL;
	int (*compare)(const void*, const void*) = tree->compare;
	int offset = tree->offset;
	while (n) {
		void *nd = AVL_NODE2DATA(n, offset);
		int hr = compare(data, nd);
		p = n;
		if (hr == 0) {
			return nd;
		}
		else if (hr < 0) {
			n = n->left;
		}
		else {
			n = n->right;
		}
	}
	return (p)? AVL_NODE2DATA(p, offset) : NULL;
}


/* returns NULL for success, otherwise returns conflict node with same key */
void *avl_tree_add(struct avl_tree *tree, void *data)
{
	struct avl_node **link = &tree->root.node;
	struct avl_node *parent = NULL;
	struct avl_node *node = AVL_DATA2NODE(data, tree->offset);
	int (*compare)(const void*, const void*) = tree->compare;
	int offset = tree->offset;
	while (link[0]) {
		void *pd;
		int hr;
		parent = link[0];
		pd = AVL_NODE2DATA(parent, offset);
		hr = compare(data, pd);
		if (hr == 0) {
			return pd;
		}	
		else if (hr < 0) {
			link = &(parent->left);
		}
		else {
			link = &(parent->right);
		}
	}
	avl_node_link(node, parent, link);
	avl_node_post_insert(node, &tree->root);
	tree->count++;
	return NULL;
}


void avl_tree_remove(struct avl_tree *tree, void *data)
{
	struct avl_node *node = AVL_DATA2NODE(data, tree->offset);
	if (!avl_node_empty(node)) {
		avl_node_erase(node, &tree->root);
		node->parent = node;
		tree->count--;
	}
}


void avl_tree_replace(struct avl_tree *tree, void *victim, void *newdata)
{
	struct avl_node *vicnode = AVL_DATA2NODE(victim, tree->offset);
	struct avl_node *newnode = AVL_DATA2NODE(newdata, tree->offset);
	avl_node_replace(vicnode, newnode, &tree->root);
	vicnode->parent = vicnode;
}


void avl_tree_clear(struct avl_tree *tree, void (*destroy)(void *data))
{
	struct avl_node *next = NULL;
	struct avl_node *node = NULL;
	while (1) {
		void *data;
		node = avl_node_tear(&tree->root, &next);
		if (node == NULL) break;
		data = AVL_NODE2DATA(node, tree->offset);
		node->parent = node;
		tree->count--;
		if (destroy) destroy(data);
	}
	ASSERTION(tree->count == 0);
}

static inline int avl_node_compare(const void *n1, const void *n2)
{
	struct MyNode *x = (struct MyNode*)n1;
	struct MyNode *y = (struct MyNode*)n2;
	return x->key - y->key;
}

int main()
{
    struct avl_tree tree;
    int i;
    avl_tree_init(&tree, avl_node_compare, sizeof(int), 0);
    for (i = 0; i < 100; i++) {
        int *data = (int*)malloc(sizeof(int));
        *data = i;
        avl_tree_add(&tree, data);
    }
    for (i = 0; i < 100; i++) {
        int *data = (int*)avl_tree_find(&tree, &i);
        if (data) {
            printf("found %d\n", *data);
        }
    }
    avl_tree_clear(&tree, free);
    return 0;
}


