#include <stdio.h>

#include "heap.h"

// 打一行数组，方便和下面的输出对照。
static void print_array(const int *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main(void) {
    printf("=== 堆数据结构测试 ===\n\n");

    const int values[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    const size_t n = sizeof(values) / sizeof(values[0]);
    int value;

    // 最小堆。容量先给 0，顺便看看自动扩容。
    printf("【最小堆测试】\n插入序列: ");
    print_array(values, n);

    Heap *min_heap = heap_create(0, MIN_HEAP);
    if (!min_heap) {
        fprintf(stderr, "创建最小堆失败\n");
        return 1;
    }
    for (size_t i = 0; i < n; i++) {
        heap_push(min_heap, values[i]);
    }
    heap_print(min_heap);
    heap_print_tree(min_heap);

    printf("依次弹出: ");
    while (heap_pop(min_heap, &value)) {
        printf("%d ", value);
    }
    printf("\n\n");
    heap_destroy(min_heap);

    // 同一组数据换成最大堆。
    printf("【最大堆测试】\n插入序列: ");
    print_array(values, n);

    Heap *max_heap = heap_create(4, MAX_HEAP);
    if (!max_heap) {
        fprintf(stderr, "创建最大堆失败\n");
        return 1;
    }
    for (size_t i = 0; i < n; i++) {
        heap_push(max_heap, values[i]);
    }
    heap_print(max_heap);
    heap_print_tree(max_heap);

    printf("依次弹出: ");
    while (heap_pop(max_heap, &value)) {
        printf("%d ", value);
    }
    printf("\n\n");
    heap_destroy(max_heap);

    // 拿现成数组建堆，这一步是 O(n)。
    const int source[] = {3, 1, 6, 5, 2, 4};
    const size_t source_n = sizeof(source) / sizeof(source[0]);

    printf("【Floyd 建堆测试】\n原始数组: ");
    print_array(source, source_n);

    Heap *built = heap_build(source, source_n, MIN_HEAP);
    if (!built) {
        fprintf(stderr, "建堆失败\n");
        return 1;
    }
    heap_print(built);
    heap_print_tree(built);
    heap_destroy(built);

    // 原地排序，升序降序各来一遍。
    int sort_arr[] = {64, 34, 25, 12, 22, 11, 90};
    const size_t sort_n = sizeof(sort_arr) / sizeof(sort_arr[0]);

    printf("【堆排序测试】\n原始数组: ");
    print_array(sort_arr, sort_n);

    heap_sort(sort_arr, sort_n, true);
    printf("升序排序: ");
    print_array(sort_arr, sort_n);

    heap_sort(sort_arr, sort_n, false);
    printf("降序排序: ");
    print_array(sort_arr, sort_n);
    printf("\n");

    // 删中间的元素，看看补位上来的那个会不会破坏堆序。
    printf("【删除操作测试】\n");
    Heap *remove_heap = heap_create(4, MIN_HEAP);
    if (!remove_heap) {
        fprintf(stderr, "创建测试堆失败\n");
        return 1;
    }
    for (size_t i = 0; i < n; i++) {
        heap_push(remove_heap, values[i]);
    }
    heap_print(remove_heap);

    int removed;
    if (heap_remove_at(remove_heap, 3, &removed)) {
        printf("删除下标 3 的元素: %d\n", removed);
        heap_print(remove_heap);
        heap_print_tree(remove_heap);
    }
    heap_destroy(remove_heap);

    printf("=== 所有测试完成 ===\n");
    return 0;
}
