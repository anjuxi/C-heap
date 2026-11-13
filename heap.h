#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stddef.h>

// 最小堆的父节点不大于子节点，最大堆反过来。
typedef enum {
    MIN_HEAP,
    MAX_HEAP
} HeapType;

// 不透明句柄。结构体定义在 heap.c 里，外部只拿指针，不碰内部字段。
typedef struct Heap Heap;

// 建一个堆。capacity 是初始容量，传 0 表示先不分配，第一次插入时再扩。
// 内存不足返回 NULL。
Heap *heap_create(size_t capacity, HeapType type);

// 用 arr 的前 n 个元素建堆，Floyd 算法，O(n)。
// 数据会拷贝一份，建完之后 arr 可以释放；arr 为 NULL 时 n 必须是 0。
Heap *heap_build(const int *arr, size_t n, HeapType type);

// 销毁堆。传 NULL 也安全。
void heap_destroy(Heap *heap);

// 插入一个元素。容量不够会自动扩，失败返回 false。
bool heap_push(Heap *heap, int value);

// 弹出堆顶，值写到 out_value 里。out_value 可以传 NULL。
// 堆为空或者 heap 是 NULL 时返回 false。
bool heap_pop(Heap *heap, int *out_value);

// 只看堆顶，不弹出。参数要求和 heap_pop 一样。
bool heap_peek(const Heap *heap, int *out_value);

// 删除下标 index 处的元素，值写到 out_value 里。out_value 可以传 NULL。
// index 越界或者 heap 是 NULL 时返回 false。
bool heap_remove_at(Heap *heap, size_t index, int *out_value);

// 以下四个是查询接口。heap 为 NULL 时分别返回 0、0、true、MIN_HEAP。
size_t heap_size(const Heap *heap);
size_t heap_capacity(const Heap *heap);
bool heap_is_empty(const Heap *heap);
HeapType heap_type(const Heap *heap);

// 调试用：一个按层序打印，一个打印成树。
void heap_print(const Heap *heap);
void heap_print_tree(const Heap *heap);

// 原地堆排序。ascending 为 true 升序，否则降序。
void heap_sort(int *arr, size_t n, bool ascending);

#endif
