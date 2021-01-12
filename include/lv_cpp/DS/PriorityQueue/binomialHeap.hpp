#ifndef BINOMIAL_HEAP
#define BINOMIAL_HEAP

#include <algorithm>
#include <iostream>
#include <vector>

template <typename T>
struct BinomialNode {
    T val;
    BinomialNode *leftChild;
    BinomialNode *nextSibling;
    BinomialNode(const T &data, BinomialNode *left, BinomialNode *next)
        : val{data}, leftChild{left}, nextSibling{next} {}
    BinomialNode(T &&data, BinomialNode *left, BinomialNode *next)
        : val{std::move(data)}, leftChild{left}, nextSibling{next} {}
};

template <typename K, typename Cmp = std::less<K>>
class BinomialHeap {
    using BinomialNode = BinomialNode<K>;

   public:
    BinomialHeap() : curSize{0}, trees(DEFAULT_TREES), cmp{} {
        for (auto &root : trees) root = nullptr;
    }

    // 重载参数为const Cmp&的构造函数时， 此构造函数可能需要发生改动(同步Cmp)
    explicit BinomialHeap(const K &x) : curSize{1}, trees(1), cmp{} {
        trees[0] = new BinomialNode{x, nullptr, nullptr};
    }

    BinomialHeap(const BinomialHeap &rhs)
        : trees(rhs.trees.size()), curSize{rhs.curSize} {
        for (std::size_t i = 0; i < rhs.trees.size(); ++i) {
            trees[i] = clone(rhs.trees[i]);
        }
    }
    BinomialHeap(BinomialHeap &&rhs)
        : trees{std::move(rhs.trees)}, curSize{rhs.curSize} {}

    // 此构造函数使得()和{}两种构造方式有所不同
    BinomialHeap(const std::initializer_list<K> &ls) {
        BinomialHeap();
        for (const auto &i : ls) push(i);
    }

    ~BinomialHeap() { clear(); }

    BinomialHeap &operator=(const BinomialHeap &rhs) {
        BinomialHeap copy{rhs};
        std::swap(*this, copy);
        return *this;
    }
    BinomialHeap &operator=(BinomialHeap &&rhs) {
        std::swap(curSize, rhs.curSize);
        std::swap(trees, rhs.trees);
        return *this;
    }

    // 清空堆
    void clear() {
        for (auto &root : trees) clear(root);
        curSize = 0;
    }

    //是否为空
    bool empty() const { return curSize == 0; }

    // 获取堆顶元素
    const K &top() const {
        return trees[findMinIndex()]->val;  // assert(curSize == 0)
    }

    //获取元素个数
    std::size_t size() const { return curSize; }
    //合并堆
    void merge(BinomialHeap &rhs) {
        if (this == &rhs)  // 避免重名
            return;

        curSize += rhs.curSize;

        if (curSize > capacity()) {  // 扩容
            auto oldNumTrees = trees.size();
            auto newNumTrees = std::max(trees.size(), rhs.trees.size()) + 1;
            trees.resize(newNumTrees);
            for (std::size_t i = oldNumTrees; i < newNumTrees; ++i)
                trees[i] = nullptr;  //对新的部分进行初始化
        }

        BinomialNode *carry = nullptr;  // 上一次合并过来的树, 一开始为空

        //逐一合并， 类比二进制加法， 逐位相加
        for (std::size_t i = 0, j = 1; j <= curSize; ++i, j <<= 1) {
            // 获得两棵树对应位置的结点
            // t1->thisNode t2->rhsNode
            BinomialNode *t1 = trees[i];
            BinomialNode *t2 = i < rhs.trees.size() ? rhs.trees[i] : nullptr;

            // 二进制 000-111 第一位是carry, 然后是t2 ,最后是t1
            char whichCase = t1 == nullptr ? 0 : 1;
            whichCase += t2 == nullptr ? 0 : 2;
            whichCase += carry == nullptr ? 0 : 4;

            switch (whichCase) {
                    // 000 and 001
                case 0: /*non trees*/
                case 1: /*only this*/
                    break;

                    // 010
                case 2: /*only rhs*/
                    trees[i] = t2;
                    rhs.trees[i] = nullptr;
                    break;
                case 3:  // 011 this and rhs but no carry
                    carry = combineTrees(t1, t2);
                    trees[i] = rhs.trees[i] = nullptr;
                    break;

                case 4:  // 100 only carry
                    trees[i] = carry;
                    carry = nullptr;
                    break;

                case 5:  // 101 carry and this
                    carry = combineTrees(carry, t1);
                    trees[i] = nullptr;
                    break;

                case 6:  // 110 carry and rhs
                    carry = combineTrees(carry, t2);
                    rhs.trees[i] = nullptr;
                    break;

                case 7:  // 111 carry and rhs and this --> all
                    trees[i] = carry;
                    carry = combineTrees(t1, t2);
                    rhs.trees[i] = nullptr;
                    break;
                default:
                    break;
            }
        }
        // 不用的东西归0
        for (auto &root : rhs.trees) root = nullptr;
        rhs.curSize = 0;
    }

    //插入元素
    void insert(const K &x) {
        BinomialHeap oneItem(x);
        //在合并的时候要注意保持Cmp的类型相同，在clear()等操作中也要注意
        // oneItem.cmp = this->cmp;
        merge(oneItem);
    }
    void insert(K &&x) {
        BinomialHeap oneItem(std::move(x));
        // oneItem.cmp = this->cmp;
        merge(oneItem);
    }

    //删除堆顶元素
    void deleteMin() {
        auto minIndex = findMinIndex();
        BinomialNode *oldRoot = trees[minIndex];
        BinomialNode *deletedTree = oldRoot->leftChild;
        delete oldRoot;

        //构建新的堆
        BinomialHeap deletedHeap;
        deletedHeap.trees.resize(minIndex + 1);
        deletedHeap.curSize = (1 << minIndex) - 1;
        for (int j = minIndex - 1; j >= 0; --j) {
            deletedHeap.trees[j] = deletedTree;
            deletedTree = deletedTree->nextSibling;
            deletedHeap.trees[j]->nextSibling = nullptr;
        }

        //新的堆合并过来
        trees[minIndex] = nullptr;
        curSize -= deletedHeap.curSize + 1;
        merge(deletedHeap);
    }

    // auto show() const { return trees.size(); }

   private:
    const static int DEFAULT_TREES = 1;

    std::vector<BinomialNode *> trees;
    std::size_t curSize;
    Cmp cmp;

    std::size_t capacity() const { return (1 << (trees.size())) - 1; }

    void clear(BinomialNode *&r) {
        if (r != nullptr) {
            clear(r->leftChild);
            clear(r->nextSibling);
            delete r;
            r = nullptr;
        }
    }

    BinomialNode *clone(BinomialNode *r) const {
        if (r == nullptr) return nullptr;
        return new BinomialNode{r->val, clone(r->leftChild),
                                clone(r->nextSibling)};
    }

    // 找到最小值对应的rank
    std::size_t findMinIndex() const {
        std::size_t i, index;
        for (i = 0; trees[i] == nullptr; ++i)
            ;
        // 顺序遍历获取最小值
        for (index = i; i < trees.size(); ++i)
            if (trees[i] != nullptr && cmp(trees[i]->val, trees[index]->val))
                index = i;
        return index;
    }

    //合并同样大小的两棵树
    BinomialNode *combineTrees(BinomialNode *t1, BinomialNode *t2) {
        if (cmp(t2->val, t1->val)) return combineTrees(t2, t1);  //让小的在前面
        t2->nextSibling = t1->leftChild;
        t1->leftChild = t2;
        return t1;
    }
};

#endif  // !BINOMIAL_HEAP
