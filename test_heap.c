#include <stdbool.h>
#include <stdio.h>

#include "heap.h"

// 测试函数统一返回 0 表示通过，返回 1 表示失败。
// 断言挂了就打印表达式和行号，然后直接退出当前用例。
#define CHECK(expr)                                                          \
    do {                                                                     \
        if (!(expr)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

// 空堆、NULL 参数，以及容量为 0 时的第一次插入。
static int test_empty_heap(void) {
    int value = 0;

    CHECK(heap_size(NULL) == 0);
    CHECK(heap_capacity(NULL) == 0);
    CHECK(heap_is_empty(NULL));
    CHECK(heap_type(NULL) == MIN_HEAP);
    CHECK(!heap_push(NULL, 1));
    CHECK(!heap_pop(NULL, &value));
    CHECK(!heap_peek(NULL, &value));
    CHECK(!heap_remove_at(NULL, 0, &value));

    Heap *heap = heap_create(0, MIN_HEAP);
    CHECK(heap != NULL);
    CHECK(heap_size(heap) == 0);
    CHECK(heap_capacity(heap) == 0);
    CHECK(heap_is_empty(heap));
    CHECK(!heap_pop(heap, &value));
    CHECK(!heap_peek(heap, &value));

    CHECK(heap_push(heap, 42));
    CHECK(!heap_is_empty(heap));
    CHECK(heap_size(heap) == 1);
    CHECK(heap_capacity(heap) > 0);
    CHECK(heap_peek(heap, &value));
    CHECK(value == 42);
    CHECK(heap_pop(heap, &value));
    CHECK(value == 42);
    CHECK(heap_is_empty(heap));

    heap_destroy(heap);
    heap_destroy(NULL);
    return 0;
}

// 最小堆弹出来的顺序必须是非递减的。
static int test_min_heap_order(void) {
    const int values[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    const size_t n = sizeof(values) / sizeof(values[0]);

    Heap *heap = heap_create(2, MIN_HEAP);
    CHECK(heap != NULL);
    CHECK(heap_type(heap) == MIN_HEAP);

    for (size_t i = 0; i < n; i++) {
        CHECK(heap_push(heap, values[i]));
    }
    CHECK(heap_size(heap) == n);

    int previous = 0;
    for (size_t i = 0; i < n; i++) {
        int value;
        CHECK(heap_pop(heap, &value));
        if (i > 0) {
            CHECK(value >= previous);
        }
        previous = value;
    }
    CHECK(heap_is_empty(heap));

    heap_destroy(heap);
    return 0;
}

// 最大堆反过来，非递增。
static int test_max_heap_order(void) {
    const int values[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    const size_t n = sizeof(values) / sizeof(values[0]);

    Heap *heap = heap_create(4, MAX_HEAP);
    CHECK(heap != NULL);
    CHECK(heap_type(heap) == MAX_HEAP);

    for (size_t i = 0; i < n; i++) {
        CHECK(heap_push(heap, values[i]));
    }

    int previous = 0;
    for (size_t i = 0; i < n; i++) {
        int value;
        CHECK(heap_pop(heap, &value));
        if (i > 0) {
            CHECK(value <= previous);
        }
        previous = value;
    }

    heap_destroy(heap);
    return 0;
}

// Floyd 建堆，顺便看看原数组有没有被改坏。
static int test_build(void) {
    const int values[] = {3, 1, 6, 5, 2, 4};
    const size_t n = sizeof(values) / sizeof(values[0]);

    Heap *min_heap = heap_build(values, n, MIN_HEAP);
    CHECK(min_heap != NULL);
    CHECK(heap_size(min_heap) == n);

    int previous = 0;
    for (size_t i = 0; i < n; i++) {
        int value;
        CHECK(heap_pop(min_heap, &value));
        if (i > 0) {
            CHECK(value >= previous);
        }
        previous = value;
    }
    heap_destroy(min_heap);

    Heap *max_heap = heap_build(values, n, MAX_HEAP);
    CHECK(max_heap != NULL);
    previous = 0;
    for (size_t i = 0; i < n; i++) {
        int value;
        CHECK(heap_pop(max_heap, &value));
        if (i > 0) {
            CHECK(value <= previous);
        }
        previous = value;
    }
    heap_destroy(max_heap);

    Heap *empty = heap_build(NULL, 0, MIN_HEAP);
    CHECK(empty != NULL);
    CHECK(heap_is_empty(empty));
    heap_destroy(empty);

    CHECK(heap_build(NULL, 1, MIN_HEAP) == NULL);

    const int original[] = {3, 1, 6, 5, 2, 4};
    for (size_t i = 0; i < n; i++) {
        CHECK(values[i] == original[i]);
    }
    return 0;
}

// 删中间、删堆顶、越界下标。
static int test_remove_at(void) {
    Heap *heap = heap_create(4, MIN_HEAP);
    CHECK(heap != NULL);
    for (int i = 1; i <= 9; i++) {
        CHECK(heap_push(heap, i));
    }

    int removed = 0;
    CHECK(heap_remove_at(heap, 3, &removed));
    CHECK(removed >= 1 && removed <= 9);
    CHECK(heap_size(heap) == 8);
    CHECK(!heap_remove_at(heap, 8, &removed));
    CHECK(!heap_remove_at(heap, (size_t)-1, &removed));

    // 剩下的 8 个值应该刚好是 1..9 里去掉被删的那个。
    bool seen[10] = {false};
    int previous = 0;
    for (int i = 0; i < 8; i++) {
        int value;
        CHECK(heap_pop(heap, &value));
        CHECK(value >= previous);
        CHECK(value >= 1 && value <= 9);
        CHECK(!seen[value]);
        seen[value] = true;
        previous = value;
    }
    CHECK(!seen[removed]);

    int count = 0;
    for (int i = 1; i <= 9; i++) {
        if (seen[i]) {
            count++;
        }
    }
    CHECK(count == 8);
    heap_destroy(heap);

    Heap *top_heap = heap_create(0, MAX_HEAP);
    CHECK(top_heap != NULL);
    CHECK(heap_push(top_heap, 10));
    CHECK(heap_push(top_heap, 20));
    CHECK(heap_push(top_heap, 30));

    int top = 0;
    CHECK(heap_remove_at(top_heap, 0, &top));
    CHECK(top == 30);
    CHECK(heap_size(top_heap) == 2);
    CHECK(heap_peek(top_heap, &top));
    CHECK(top == 20);
    heap_destroy(top_heap);
    return 0;
}

// 升序、降序、重复值、负数，再加空数组和单元素。
static int test_sort(void) {
    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    const int expected[] = {11, 12, 22, 25, 34, 64, 90};
    const size_t n = sizeof(arr) / sizeof(arr[0]);

    heap_sort(arr, n, true);
    for (size_t i = 0; i < n; i++) {
        CHECK(arr[i] == expected[i]);
    }

    heap_sort(arr, n, false);
    for (size_t i = 0; i < n; i++) {
        CHECK(arr[i] == expected[n - 1 - i]);
    }

    int duplicates[] = {5, -1, 5, 0, -1, 3};
    const int expected_duplicates[] = {-1, -1, 0, 3, 5, 5};
    heap_sort(duplicates, 6, true);
    for (size_t i = 0; i < 6; i++) {
        CHECK(duplicates[i] == expected_duplicates[i]);
    }

    heap_sort(NULL, 0, true);
    int single = 7;
    heap_sort(&single, 1, false);
    CHECK(single == 7);
    return 0;
}

// 插 1000 个随机数，检查扩容和整体有序性。
static int test_growth(void) {
    const size_t n = 1000;
    Heap *heap = heap_create(0, MIN_HEAP);
    CHECK(heap != NULL);

    unsigned int seed = 12345u;
    for (size_t i = 0; i < n; i++) {
        seed = seed * 1103515245u + 12345u;
        int value = (int)((seed >> 16) % 2001u) - 1000;
        CHECK(heap_push(heap, value));
    }
    CHECK(heap_size(heap) == n);
    CHECK(heap_capacity(heap) >= n);

    int previous = 0;
    for (size_t i = 0; i < n; i++) {
        int value;
        CHECK(heap_pop(heap, &value));
        if (i > 0) {
            CHECK(value >= previous);
        }
        previous = value;
    }
    CHECK(heap_is_empty(heap));

    heap_destroy(heap);
    return 0;
}

int main(void) {
    // 名字和函数放在一起，加用例的时候只改这张表。
    const struct {
        const char *name;
        int (*run)(void);
    } tests[] = {
        {"空堆与非法参数", test_empty_heap},
        {"最小堆弹出顺序", test_min_heap_order},
        {"最大堆弹出顺序", test_max_heap_order},
        {"Floyd 建堆", test_build},
        {"按位置删除", test_remove_at},
        {"原地堆排序", test_sort},
        {"扩容与大量数据", test_growth},
    };
    const size_t count = sizeof(tests) / sizeof(tests[0]);
    size_t failed = 0;

    for (size_t i = 0; i < count; i++) {
        if (tests[i].run() == 0) {
            printf("[通过] %s\n", tests[i].name);
        } else {
            printf("[失败] %s\n", tests[i].name);
            failed++;
        }
    }

    if (failed == 0) {
        printf("全部 %llu 项测试通过\n", (unsigned long long)count);
        return 0;
    }

    printf("%llu / %llu 项测试失败\n",
           (unsigned long long)failed, (unsigned long long)count);
    return 1;
}
