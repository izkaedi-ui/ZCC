int printf(const char *fmt, ...);
void *malloc(long size);

typedef struct Node {
    int val;
    struct Node *next;
} Node;

Node *insert(Node *head, int val) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->val = val;
    n->next = head;
    return n;
}

Node *delete(Node *head, int val) {
    if (head->val == val) return head->next;
    head->next = delete(head->next, val);
    return head;
}

int search(Node *head, int val) {
    while (head) {
        if (head->val == val) return 1;
        head = head->next;
    }
    return 0;
}

void print_list(Node *head) {
    while (head) {
        printf("%d ", head->val);
        head = head->next;
    }
    printf("
");
}

int main() {
    Node *head = (Node *)0;
    head = insert(head, 10);
    head = insert(head, 20);
    head = insert(head, 30);
    head = insert(head, 40);
    head = insert(head, 50);
    printf("list: "); print_list(head);
    printf("search 30: %d
", search(head, 30));
    printf("search 99: %d
", search(head, 99));
    head = delete(head, 30);
    printf("after delete 30: "); print_list(head);
    head = delete(head, 50);
    printf("after delete 50: "); print_list(head);
    return 0;
}
