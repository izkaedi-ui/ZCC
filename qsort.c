int printf(const char *fmt, ...);

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int partition(int *arr, int lo, int hi) {
    int pivot = arr[hi];
    int i = lo - 1;
    int j;
    for (j = lo; j < hi; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[hi]);
    return i + 1;
}

void quicksort(int *arr, int lo, int hi) {
    if (lo < hi) {
        int p = partition(arr, lo, hi);
        quicksort(arr, lo, p - 1);
        quicksort(arr, p + 1, hi);
    }
}

int main() {
    int arr[10];
    int i;
    arr[0] = 38;
    arr[1] = 27;
    arr[2] = 43;
    arr[3] = 3;
    arr[4] = 9;
    arr[5] = 82;
    arr[6] = 10;
    arr[7] = 55;
    arr[8] = 17;
    arr[9] = 1;
    quicksort(arr, 0, 9);
    for (i = 0; i < 10; i++) {
        printf("%d
", arr[i]);
    }
    return 0;
}
