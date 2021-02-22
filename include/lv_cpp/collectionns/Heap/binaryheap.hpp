#include <vector>

template <typename T>
class Minheap {
   private:
    using Container = std::vector<T>;
    Container arr;
    unsigned _size;
    void percolateDown(unsigned hole) {
        unsigned child;
        T tmp = std::move(arr[hole]);
        for (; (hole << 1) <= _size; hole = child) {
            //将hole下移
            child = hole << 1;
            //将child定位在hole的两个孩子中较大的那个位置
            // child 的最大取值是_size，如果child != size，则说明child <
            // _size，那么child <= _size,不可能越界
            if (child != _size && arr[child + 1] < arr[child]) {
                ++child;  // child指向俩个孩子的较小者
            }
            if (arr[child] < tmp) {
                //孩子的值比较小，则将孩子的值移动上来
                arr[hole] = std::move(arr[child]);
            } else
                break;  //不满足说明找到了最终位置
        }               //循环结束的位置则是最终位置
        arr[hole] = std::move(tmp);
    }  //总的过程：孩子一个个上移

    void buildHeap() {
        for (int i = _size >> 1; i > 0; --i) {
            percolateDown(i);
            //从最后一个节点的父亲开始逐个建堆,这是个下虑的过程
        }
    }

   public:
    explicit Minheap(unsigned capacity = 100) : arr(capacity + 1), _size(0) {}
    explicit Minheap(const Container &rhs)
        : arr(rhs.size() + 10), _size(rhs.size()) {
        for (int i = 0; i < rhs.size(); ++i) {
            arr[i + 1] = rhs[i];
        }
        buildHeap();
    }
    bool empty() const { return _size == 0; }

    const T &findMin() const {
        return arr[1];  // assert(empty())
    }
    void insert(const T &x) {
        //自动扩容
        arr.emplace_back(x);
        int hole = ++_size;
        //生成了一个临时变量
        // x并非右值，而且我们在这里并不是直接移动x
        T copy = x;

        //把父亲移动下来，自己就上去了
        // Percolate up
        arr[0] = std::move(copy);
        for (; x < arr[hole / 2]; hole /= 2)
            arr[hole] = std::move(arr[hole / 2]);
        arr[hole] = std::move(arr[0]);
    }

    void insert(T &&x) {
        //自动扩容
        arr.emplace_back(x);

        auto hole = ++_size;
        //这个是一个右值， 无需再生成临时变量
        //可见移动函数比较高效
        // percolate up
        //这这里hole大于一是因为arr[0]此时此刻的元素值可能会影响判断条件，
        //在上一个insert方法中arr[0]的值与x相同，当循环执行到0处的时候
        //在逻辑上循环可以正常结束
        for (; hole > 1 && x < arr[hole >> 1]; hole >>= 1) {
            //把父亲移动下来，自己就上去了
            arr[hole] = std::move(arr[hole >> 1]);
        }
        arr[hole] = std::move(x);
    }

    void deleteMin() {
        // assert(empty())
        arr[1] = std::move(arr[_size--]);
        //把最后一个元素拿上来再下虑下去
        percolateDown(1);
    }
    void deleteMin(T &x) {
        // x不用const因为修改了x
        //用x去接这个最小的元素，并删除最小元素
        // assert(empty())
        x = std::move(arr[1]);
        arr[1] = std::move(arr[_size--]);
        percolateDown(1);
        //最后一个元素拿上来再下滤
    }

    void clear() {
        _size = 0;  //逻辑上清空即可
        //若这个堆很大还是有必要调用arr.clear()，清理内存
    }
    //在OS中需要一些特殊的借口，比如不断的修改进程的优先级，
    //这个时候需要提供一下借口去寻找特定的进程并对优先级进行修改 decreaseKey(p,
    // △)减少优先级 increaseKey(p, △)增加优先级 remove终止进程
};

#ifndef BINARYHEAP_H
#define BINARYHEAP_H

#include <iostream>
#include <vector>

template <typename T, typename Cmp = std::less<T>>
class BinaryHeap {
   public:
    explicit BinaryHeap(std::size_t capacity = 8)
        : curSize{0}, item(capacity + 1), cmp{} {}
    BinaryHeap(const BinaryHeap &) = default;
    BinaryHeap(BinaryHeap &&) = default;
    BinaryHeap &operator=(const BinaryHeap &) = default;
    BinaryHeap &operator=(BinaryHeap &&) = default;
    ~BinaryHeap() = default;

    bool empty() const { return curSize == 0; }
    std::size_t size() const { return curSize; }
    const T &top() const {
        return item[1];  // assert(!empty())
    }

    void insert(const T &x) {
        // std::cout << "move\n";
        item.emplace_back(x);
        auto hole = ++curSize;  //当前元素所在位置
        T copy = x;

        item[0] = std::move(copy);
        //调整位置, 上虑, 每次hole更新为其父节点
        for (; cmp(x, item[hole >> 1]); hole >>= 1)
            item[hole] = std::move(item[hole >> 1]);
        item[hole] = std::move(item[0]);
    }
    void insert(T &&x) {
        item.emplace_back(std::move(x));
        auto hole = ++curSize;

        for (; hole > 1 && cmp(x, item[hole >> 1]); hole >>= 1)
            item[hole] = std::move(item[hole >> 1]);
        item[hole] = std::move(x);
    }
    void deleteMin() {
        item[1] = std::move(item[curSize--]);
        percolateDown(1);
    }
    void clear() {
        item.resize(1);
        curSize = 0;
    }

    /*void show() const {
            for (auto i : item)
                    std::cout << i << ' ';
    }
    */

   private:
    std::vector<T> item;  // 0号位置用作哨兵
    std::size_t curSize;
    Cmp cmp;

    void percolateDown(std::size_t hole) {
        std::size_t child;
        T tmp = std::move(item[hole]);
        for (; hole << 1 <= curSize; hole = child) {
            child = hole << 1;
            //选取俩个孩子中较大的
            if (child != curSize && cmp(item[child + 1], item[child])) ++child;

            if (cmp(item[child], tmp))
                item[hole] = std::move(item[child]);
            else
                break;
        }
        item[hole] = std::move(tmp);
    }

    //由于不提供初始化列表，不使用
    void buildHeap() {
        //从第一个非叶子结点开始逐个下移即可 --> O(N)
        for (size_t i = curSize >> 1; i > 0; --i) {
            percolateDown(i);
        }
    }
};

#endif  // !BINARYHEAP_H
