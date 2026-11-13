#include "heap.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 容量至少给 4 个，省得第一次插入就 realloc。
#define V_MIN_CAPACITY ((size_t)4)
// 树形打印的缩进缓冲。每层 4 个字符，63 层够用了，再深本来也没法看。
#define V_TREE_PREFIX_SIZE ((size_t)256)

struct Heap {
    int *data;
    size_t size;
    size_t capacity;
    HeapType type;
};

// 二叉堆的下标换算。根节点是 0，左右孩子分别是 2i+1 和 2i+2。
static size_t parent_index(size_t i) {
    return (i - 1) / 2;
}

static size_t left_index(size_t i) {
    return 2 * i + 1;
}

static size_t right_index(size_t i) {
    return 2 * i + 2;
}

static void swap_int(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// 谁该排在前面。最小堆里小的优先，最大堆里大的优先。
static bool higher_priority(HeapType type, int a, int b) {
    return type == MIN_HEAP ? a < b : a > b;
}

// 新元素从末尾往上挪，直到不比父节点更优先。
static void sift_up(Heap *heap, size_t index) {
    while (index > 0) {
        size_t parent = parent_index(index);
        if (!higher_priority(heap->type, heap->data[index], heap->data[parent])) {
            break;
        }
        swap_int(&heap->data[index], &heap->data[parent]);
        index = parent;
    }
}

// 堆顶被换掉之后，从 index 往下找位置。
static void sift_down(Heap *heap, size_t index) {
    for (;;) {
        size_t best = index;
        size_t left = left_index(index);
        size_t right = right_index(index);

        if (left < heap->size &&
            higher_priority(heap->type, heap->data[left], heap->data[best])) {
            best = left;
        }
        if (right < heap->size &&
            higher_priority(heap->type, heap->data[right], heap->data[best])) {
            best = right;
        }
        if (best == index) {
            break;
        }

        swap_int(&heap->data[index], &heap->data[best]);
        index = best;
    }
}

// 保证容量至少有 min_capacity。翻倍扩容，摊还下来每次插入是 O(1)。
// 两个乘法都要防溢出：capacity * 2 和元素个数 * sizeof(int)。
static bool heap_reserve(Heap *heap, size_t min_capacity) {
    if (min_capacity <= heap->capacity) {
        return true;
    }

    size_t new_capacity = heap->capacity ? heap->capacity : V_MIN_CAPACITY;
    while (new_capacity < min_capacity) {
        // 翻倍翻过头就退回到刚好够用的值，避免绕回 0。
        if (new_capacity > SIZE_MAX / 2) {
            new_capacity = min_capacity;
            break;
        }
        new_capacity *= 2;
    }

    if (new_capacity > SIZE_MAX / sizeof(int)) {
        return false;
    }

    int *new_data = realloc(heap->data, new_capacity * sizeof(int));
    if (!new_data) {
        return false;
    }

    heap->data = new_data;
    heap->capacity = new_capacity;
    return true;
}

Heap *heap_create(size_t capacity, HeapType type) {
    if (type != MIN_HEAP && type != MAX_HEAP) {
        return NULL;
    }

    Heap *heap = malloc(sizeof(Heap));
    if (!heap) {
        return NULL;
    }

    heap->data = NULL;
    heap->size = 0;
    heap->capacity = 0;
    heap->type = type;

    if (!heap_reserve(heap, capacity)) {
        free(heap);
        return NULL;
    }
    return heap;
}

void heap_destroy(Heap *heap) {
    if (!heap) {
        return;
    }
    free(heap->data);
    free(heap);
}

size_t heap_size(const Heap *heap) {
    return heap ? heap->size : 0;
}

size_t heap_capacity(const Heap *heap) {
    return heap ? heap->capacity : 0;
}

bool heap_is_empty(const Heap *heap) {
    return !heap || heap->size == 0;
}

HeapType heap_type(const Heap *heap) {
    return heap ? heap->type : MIN_HEAP;
}

bool heap_push(Heap *heap, int value) {
    if (!heap) {
        return false;
    }
    if (!heap_reserve(heap, heap->size + 1)) {
        return false;
    }

    heap->data[heap->size] = value;
    heap->size++;
    sift_up(heap, heap->size - 1);
    return true;
}

bool heap_pop(Heap *heap, int *out_value) {
    if (!heap || heap->size == 0) {
        return false;
    }

    if (out_value) {
        *out_value = heap->data[0];
    }

    heap->size--;
    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        sift_down(heap, 0);
    }
    return true;
}

bool heap_peek(const Heap *heap, int *out_value) {
    if (!heap || heap->size == 0) {
        return false;
    }
    if (out_value) {
        *out_value = heap->data[0];
    }
    return true;
}

Heap *heap_build(const int *arr, size_t n, HeapType type) {
    if (n > 0 && !arr) {
        return NULL;
    }

    Heap *heap = heap_create(n, type);
    if (!heap) {
        return NULL;
    }

    if (n > 0) {
        memcpy(heap->data, arr, n * sizeof(int));
    }
    heap->size = n;

    // 从最后一个非叶子节点往前下沉。越靠近底层的节点挪得越少，总和是 O(n)。
    for (size_t i = n / 2; i-- > 0;) {
        sift_down(heap, i);
    }
    return heap;
}

bool heap_remove_at(Heap *heap, size_t index, int *out_value) {
    if (!heap || index >= heap->size) {
        return false;
    }

    if (out_value) {
        *out_value = heap->data[index];
    }

    heap->size--;
    if (index == heap->size) {
        return true;
    }

    // 拿最后一个元素补位。补上来的值可能比父节点更优先，也可能该往下走，
    // 先和父节点比一次，能省掉一次没必要的下沉。
    heap->data[index] = heap->data[heap->size];
    if (index > 0 &&
        higher_priority(heap->type, heap->data[index], heap->data[parent_index(index)])) {
        sift_up(heap, index);
    } else {
        sift_down(heap, index);
    }
    return true;
}

void heap_print(const Heap *heap) {
    if (!heap || heap->size == 0) {
        printf("Empty Heap\n");
        return;
    }

    printf("%s (size=%llu, capacity=%llu): ",
           heap->type == MIN_HEAP ? "MinHeap" : "MaxHeap",
           (unsigned long long)heap->size,
           (unsigned long long)heap->capacity);

    for (size_t i = 0; i < heap->size; i++) {
        printf("%d ", heap->data[i]);
    }
    printf("\n");
}

// 递归打印子树。prefix 是当前行的前导缩进，is_last 表示它是不是父节点的最后一个孩子。
// 不是最后一个孩子就要继续往下画竖线，否则下一层会错位。
static void print_subtree(const Heap *heap, size_t index, const char *prefix, bool is_last) {
    printf("%s%s%d\n", prefix, is_last ? "`-- " : "|-- ", heap->data[index]);

    size_t left = left_index(index);
    size_t right = right_index(index);
    if (left >= heap->size) {
        return;
    }

    char next_prefix[V_TREE_PREFIX_SIZE];
    int written = snprintf(next_prefix, sizeof(next_prefix), "%s%s",
                           prefix, is_last ? "    " : "|   ");
    if (written < 0 || (size_t)written >= sizeof(next_prefix)) {
        return;
    }

    print_subtree(heap, left, next_prefix, right >= heap->size);
    if (right < heap->size) {
        print_subtree(heap, right, next_prefix, true);
    }
}

void heap_print_tree(const Heap *heap) {
    if (!heap || heap->size == 0) {
        printf("Empty Heap\n");
        return;
    }

    printf("%s tree:\n", heap->type == MIN_HEAP ? "MinHeap" : "MaxHeap");
    printf("%d\n", heap->data[0]);

    size_t left = left_index(0);
    size_t right = right_index(0);
    if (left < heap->size) {
        print_subtree(heap, left, "", right >= heap->size);
        if (right < heap->size) {
            print_subtree(heap, right, "", true);
        }
    }
    printf("\n");
}

// 裸数组上的下沉。n 是当前有效长度，堆排序每轮都会把它缩短一格。
static void sift_down_array(int *arr, size_t n, size_t index, bool max_heap) {
    for (;;) {
        size_t best = index;
        size_t left = left_index(index);
        size_t right = right_index(index);

        if (left < n && (max_heap ? arr[left] > arr[best] : arr[left] < arr[best])) {
            best = left;
        }
        if (right < n && (max_heap ? arr[right] > arr[best] : arr[right] < arr[best])) {
            best = right;
        }
        if (best == index) {
            break;
        }

        swap_int(&arr[index], &arr[best]);
        index = best;
    }
}

void heap_sort(int *arr, size_t n, bool ascending) {
    if (!arr || n < 2) {
        return;
    }

    // 升序建大顶堆，每轮把最大值换到末尾；降序反过来。
    bool max_heap = ascending;
    for (size_t i = n / 2; i-- > 0;) {
        sift_down_array(arr, n, i, max_heap);
    }

    for (size_t end = n - 1; end > 0; end--) {
        swap_int(&arr[0], &arr[end]);
        sift_down_array(arr, end, 0, max_heap);
    }
}
