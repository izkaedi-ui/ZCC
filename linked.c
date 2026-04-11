int printf(const char *fmt, ...);
void *malloc(long size);

typedef struct Node {
    int val;
    struct Node *next;
} Node;

Node *push(Node *head, int val) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->val = val;
    n->next = head;
    return n;
}

void print_list(Node *head) {
    while (head) {
        printf("%d
", head->val);
        head = head->next;
    }
}

int main() {
    Node *head = (Node *)0;
    head = push(head, 10);
    head = push(head, 20);
    head = push(head, 30);
    head = push(head, 40);
    head = push(head, 50);
    print_list(head);
    return 0;
}
