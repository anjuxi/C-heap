#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ========== 堆（Heap）数据结构实现 ==========
// 支持最小堆和最大堆，基于动态数组实现

typedef enum {
    MIN_HEAP,   // 最小堆：父节点 <= 子节点
    MAX_HEAP    // 最大堆：父节点 >= 子节点
} HeapType;

typedef struct {
    int* data;          // 存储元素的数组
    int size;           // 当前元素数量
    int capacity;       // 数组容量
    HeapType type;      // 堆类型
} Heap;

// ========== 辅助函数 ==========

// 获取父节点索引
#define PARENT(i) (((i) - 1) / 2)
// 获取左子节点索引
#define LEFT(i)   (2 * (i) + 1)
// 获取右子节点索引
#define RIGHT(i)  (2 * (i) + 2)

// 比较函数：根据堆类型决定比较方式
// 返回true表示a的优先级高于b（最小堆：a<b；最大堆：a>b）
static bool hasHigherPriority(Heap* h, int a, int b) {
    if (h->type == MIN_HEAP) {
        return a < b;
    } else {
        return a > b;
    }
}

// ========== 堆操作 ==========

// 创建堆
Heap* heapCreate(int capacity, HeapType type) {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    if (!h) return NULL;
    
    h->data = (int*)malloc(sizeof(int) * capacity);
    if (!h->data) {
        free(h);
        return NULL;
    }
    
    h->size = 0;
    h->capacity = capacity;
    h->type = type;
    return h;
}

// 销毁堆
void heapDestroy(Heap* h) {
    if (!h) return;
    free(h->data);
    free(h);
}

// 获取堆中元素数量
int heapSize(Heap* h) {
    return h ? h->size : 0;
}

// 判断堆是否为空
bool heapIsEmpty(Heap* h) {
    return !h || h->size == 0;
}

// 扩容
static bool heapExpand(Heap* h) {
    int newCapacity = h->capacity * 2;
    int* newData = (int*)realloc(h->data, sizeof(int) * newCapacity);
    if (!newData) return false;
    
    h->data = newData;
    h->capacity = newCapacity;
    return true;
}

// 向上调整（插入后维护堆性质）
static void heapifyUp(Heap* h, int index) {
    while (index > 0) {
        int parent = PARENT(index);
        // 如果当前节点优先级高于父节点，交换
        if (hasHigherPriority(h, h->data[index], h->data[parent])) {
            int temp = h->data[index];
            h->data[index] = h->data[parent];
            h->data[parent] = temp;
            index = parent;
        } else {
            break;
        }
    }
}

// 向下调整（删除后维护堆性质）
static void heapifyDown(Heap* h, int index) {
    while (true) {
        int left = LEFT(index);
        int right = RIGHT(index);
        int priority = index;  // 优先级最高的节点索引
        
        // 与左子节点比较
        if (left < h->size && hasHigherPriority(h, h->data[left], h->data[priority])) {
            priority = left;
        }
        // 与右子节点比较
        if (right < h->size && hasHigherPriority(h, h->data[right], h->data[priority])) {
            priority = right;
        }
        
        // 如果当前节点已经是优先级最高的，结束
        if (priority == index) break;
        
        // 交换
        int temp = h->data[index];
        h->data[index] = h->data[priority];
        h->data[priority] = temp;
        
        index = priority;
    }
}

// 插入元素
bool heapPush(Heap* h, int value) {
    if (!h) return false;
    
    // 检查是否需要扩容
    if (h->size >= h->capacity) {
        if (!heapExpand(h)) return false;
    }
    
    // 将新元素放到末尾
    h->data[h->size] = value;
    h->size++;
    
    // 向上调整
    heapifyUp(h, h->size - 1);
    return true;
}

// 弹出堆顶元素
bool heapPop(Heap* h, int* outValue) {
    if (!h || h->size == 0) return false;
    
    // 保存堆顶元素
    *outValue = h->data[0];
    
    // 将最后一个元素移到堆顶
    h->data[0] = h->data[h->size - 1];
    h->size--;
    
    // 向下调整
    if (h->size > 0) {
        heapifyDown(h, 0);
    }
    
    return true;
}

// 查看堆顶元素（不弹出）
bool heapPeek(Heap* h, int* outValue) {
    if (!h || h->size == 0) return false;
    *outValue = h->data[0];
    return true;
}

// 从数组构建堆（Floyd建堆算法，O(n)）
Heap* heapBuild(int* arr, int n, HeapType type) {
    Heap* h = heapCreate(n > 4 ? n : 4, type);
    if (!h) return NULL;
    
    // 复制数组
    for (int i = 0; i < n; i++) {
        h->data[i] = arr[i];
    }
    h->size = n;
    
    // 从最后一个非叶子节点开始向下调整
    for (int i = PARENT(n - 1); i >= 0; i--) {
        heapifyDown(h, i);
    }
    
    return h;
}

// 删除堆中指定索引的元素
bool heapRemoveAt(Heap* h, int index, int* outValue) {
    if (!h || index < 0 || index >= h->size) return false;
    
    *outValue = h->data[index];
    
    // 用最后一个元素替换
    h->data[index] = h->data[h->size - 1];
    h->size--;
    
    if (index < h->size) {
        // 尝试向上调整
        heapifyUp(h, index);
        // 尝试向下调整
        heapifyDown(h, index);
    }
    
    return true;
}

// 打印堆（按层序遍历）
void heapPrint(Heap* h) {
    if (!h || h->size == 0) {
        printf("Empty Heap\n");
        return;
    }
    
    printf("%s (size=%d, capacity=%d): ", 
           h->type == MIN_HEAP ? "MinHeap" : "MaxHeap",
           h->size, h->capacity);
    
    for (int i = 0; i < h->size; i++) {
        printf("%d ", h->data[i]);
    }
    printf("\n");
}

// 打印堆的树形结构
void heapPrintTree(Heap* h) {
    if (!h || h->size == 0) {
        printf("Empty Heap\n");
        return;
    }
    
    printf("\n%s Tree Structure:\n", h->type == MIN_HEAP ? "MinHeap" : "MaxHeap");
    
    int level = 0;
    int levelSize = 1;
    int index = 0;
    
    while (index < h->size) {
        // 打印当前层的缩进
        int indent = (h->size / (levelSize + 1)) * 2;
        for (int i = 0; i < indent / 2; i++) printf("  ");
        
        // 打印当前层的节点
        int nodesInThisLevel = levelSize;
        if (index + nodesInThisLevel > h->size) {
            nodesInThisLevel = h->size - index;
        }
        
        for (int i = 0; i < nodesInThisLevel; i++) {
            printf("%d", h->data[index++]);
            if (i < nodesInThisLevel - 1) {
                printf("  ");
            }
        }
        printf("\n");
        
        level++;
        levelSize *= 2;
    }
    printf("\n");
}

// 堆排序（原地排序，返回排序后的数组）
void heapSort(int* arr, int n, bool ascending) {
    // 根据排序方向选择堆类型
    HeapType type = ascending ? MAX_HEAP : MIN_HEAP;
    Heap* h = heapBuild(arr, n, type);
    if (!h) return;
    
    // 依次弹出堆顶元素
    for (int i = n - 1; i >= 0; i--) {
        heapPop(h, &arr[i]);
    }
    
    heapDestroy(h);
}

// ========== 测试代码 ==========

int main() {
    printf("=== 堆数据结构测试 ===\n\n");
    
    // 测试1：最小堆基本操作
    printf("【最小堆测试】\n");
    Heap* minHeap = heapCreate(4, MIN_HEAP);
    
    int values[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    int n = sizeof(values) / sizeof(values[0]);
    
    printf("插入序列: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", values[i]);
        heapPush(minHeap, values[i]);
    }
    printf("\n");
    
    heapPrint(minHeap);
    heapPrintTree(minHeap);
    
    printf("依次弹出: ");
    int val;
    while (heapPop(minHeap, &val)) {
        printf("%d ", val);
    }
    printf("\n\n");
    
    heapDestroy(minHeap);
    
    // 测试2：最大堆基本操作
    printf("【最大堆测试】\n");
    Heap* maxHeap = heapCreate(4, MAX_HEAP);
    
    printf("插入序列: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", values[i]);
        heapPush(maxHeap, values[i]);
    }
    printf("\n");
    
    heapPrint(maxHeap);
    heapPrintTree(maxHeap);
    
    printf("依次弹出: ");
    while (heapPop(maxHeap, &val)) {
        printf("%d ", val);
    }
    printf("\n\n");
    
    heapDestroy(maxHeap);
    
    // 测试3：Floyd建堆
    printf("【Floyd建堆测试】\n");
    int arr[] = {3, 1, 6, 5, 2, 4};
    int arrN = sizeof(arr) / sizeof(arr[0]);
    
    printf("原始数组: ");
    for (int i = 0; i < arrN; i++) printf("%d ", arr[i]);
    printf("\n");
    
    Heap* builtHeap = heapBuild(arr, arrN, MIN_HEAP);
    printf("建堆结果: ");
    heapPrint(builtHeap);
    heapPrintTree(builtHeap);
    heapDestroy(builtHeap);
    
    // 测试4：堆排序
    printf("【堆排序测试】\n");
    int sortArr[] = {64, 34, 25, 12, 22, 11, 90};
    int sortN = sizeof(sortArr) / sizeof(sortArr[0]);
    
    printf("原始数组: ");
    for (int i = 0; i < sortN; i++) printf("%d ", sortArr[i]);
    printf("\n");
    
    // 升序排序
    heapSort(sortArr, sortN, true);
    printf("升序排序: ");
    for (int i = 0; i < sortN; i++) printf("%d ", sortArr[i]);
    printf("\n");
    
    // 降序排序
    heapSort(sortArr, sortN, false);
    printf("降序排序: ");
    for (int i = 0; i < sortN; i++) printf("%d ", sortArr[i]);
    printf("\n\n");
    
    // 测试5：删除指定位置元素
    printf("【删除操作测试】\n");
    Heap* testHeap = heapCreate(8, MIN_HEAP);
    for (int i = 0; i < n; i++) {
        heapPush(testHeap, values[i]);
    }
    
    heapPrint(testHeap);
    
    int removed;
    if (heapRemoveAt(testHeap, 3, &removed)) {
        printf("删除索引3的元素: %d\n", removed);
        heapPrint(testHeap);
        heapPrintTree(testHeap);
    }
    
    heapDestroy(testHeap);
    
    printf("\n=== 所有测试完成 ===\n");
    return 0;
}
