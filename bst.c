int printf(const char *fmt, ...);
void *malloc(long size);

typedef struct Node {
    int val;
    struct Node *left;
    struct Node *right;
} Node;

Node *new_node(int val) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->val = val;
    n->left = (Node *)0;
    n->right = (Node *)0;
    return n;
}

Node *insert(Node *root, int val) {
    if (!root) return new_node(val);
    if (val < root->val) root->left = insert(root->left, val);
    else if (val > root->val) root->right = insert(root->right, val);
    return root;
}

int search(Node *root, int val) {
    if (!root) return 0;
    if (val == root->val) return 1;
    if (val < root->val) return search(root->left, val);
    return search(root->right, val);
}

void inorder(Node *root) {
    if (!root) return;
    inorder(root->left);
    printf("%d ", root->val);
    inorder(root->right);
}

int main() {
    Node *root = (Node *)0;
    root = insert(root, 50);
    root = insert(root, 30);
    root = insert(root, 70);
    root = insert(root, 20);
    root = insert(root, 40);
    root = insert(root, 60);
    root = insert(root, 80);
    printf("inorder: ");
    inorder(root);
    printf("\n");
    printf("search 40: %d\n", search(root, 40));
    printf("search 55: %d\n", search(root, 55));
    return 0;
}
