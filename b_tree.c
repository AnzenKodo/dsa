// https://github.com/amit-davidson/btree

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 4
#define MIN 4/2

typedef struct Node {
    int keys[MAX];
    struct Node *child[MAX + 1];
    int count;
    bool leaf;
} Node;

Node *create_node(bool leaf) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->leaf = leaf;
    node->count = 0;

    for (int i = 0; i < MAX; i++) {
        node->keys[i] = 0;
        node->child[i] = NULL;
    }

    node->child[MAX] = NULL;
    return node;
}

void split_child(Node *x, int i, Node **y) {
    Node *z = create_node((*y)->leaf);
    z->count = MIN - 1;

    for (int j = 0; j < MIN - 1; j++) {
        z->keys[j] = (*y)->keys[j + MIN];
    }

    if (!(*y)->leaf) {
        for (int j = 0; j < MIN; j++) {
            z->child[j] = (*y)->child[j + MIN];
        }
    }

    (*y)->count = MIN - 1;

    for (int j = x->count; j >= i+1; j--)
        x->child[j + 1] = x->child[j];

    x->child[i + 1] = z;

    for (int j = x->count - 1; j >= i; j--)
        x->keys[j + 1] = x->keys[j];

    x->keys[i] = (*y)->keys[MIN - 1];

    x->count += 1;
}

void insert_non_full(Node *node, int key) {
    int i = node->count - 1;

    if (node->leaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }

        node->keys[i + 1] = key;
        node->count += 1;
    } else {
        while (i >= 0 && key < node->keys[i]) i--;

        i++;

        if (node->child[i]->count == MAX) {
            split_child(node, i, &(node->child[i]));

            if (key > node->keys[i]) i++;
        }
        insert_non_full(node->child[i], key);
    }
}

void insert_key(int key, Node **root) {
    if (*root == NULL) {
        *root = create_node(true);
        (*root)->keys[0] = key;
        (*root)->count = 1;
    } else {
        if ((*root)->count == MAX) {
            Node *node = create_node(false);
            node->child[0] = *root;
            split_child(node, 0, root);
            insert_non_full(node, key);
            *root = node;
        } else insert_non_full(*root, key);
    }
}

void print_tree(Node *root) {
    if (root != NULL) {
        for (int i = 0; i < root->count; i++) {
            print_tree(root->child[i]);
            printf("%d ", root->keys[i]);
        }
        print_tree(root->child[root->count]);
    }
}

int main(void) {
    Node *root = NULL;
    insert_key(10, &root);
    insert_key(10, &root);
    insert_key(20, &root);
    insert_key(5, &root);
    insert_key(6, &root);
    insert_key(12, &root);
    insert_key(30, &root);
    insert_key(7, &root);
    insert_key(3, &root);

    print_tree(root);

    return 0;
}
