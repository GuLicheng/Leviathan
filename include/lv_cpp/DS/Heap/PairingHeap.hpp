#ifndef PAIRING_HEAP_H
#define PAIRING_HEAP_H

#include <algorithm>
#include <iostream>
#include <vector>

template <typename T>
struct PairNode {
    T val;
    PairNode *leftChild;
    PairNode *nextSibling;
    PairNode *prev;

    PairNode(const T &x)
        : val{x}, leftChild{nullptr}, nextSibling{nullptr}, prev{nullptr} {}
    PairNode(T &&x)
        : val{std::move(x)},
          leftChild{nullptr},
          nextSibling{nullptr},
          prev{nullptr} {}
};

template <typename K, typename _Cmp = std::less<K>>
class PairingHeap {
    // 用上面的结点类替换下面这句代码
    using PairNode = PairNode<K>;

   public:
    PairingHeap() : curSize(0), root(nullptr) {}
    PairingHeap(const PairingHeap &rhs)
        : curSize{rhs.curSize}, root{clone(rhs.root)} {}
    PairingHeap(PairingHeap &&rhs) : curSize(rhs.curSize), root{rhs.root} {
        rhs.root = nullptr;
        rhs.curSize = 0;
    }
    // 让lambda表达式可以工作
    explicit PairingHeap(const _Cmp &c) : curSize(0), root(nullptr), cmp(c) {}

    PairingHeap &operator=(const PairingHeap &rhs) = default;
    PairingHeap &operator=(PairingHeap &&rhs) = default;
    ~PairingHeap() { clear(); }

    const K &top() const { return root->val; }
    void insert(const K &x) {
        PairNode *newNode = new PairNode{x};
        if (root == nullptr)
            root = newNode;
        else
            compareAndLink(root, newNode);
        ++curSize;
    }
    void insert(K &&x) {
        PairNode *newNode = new PairNode{std::move(x)};
        if (root == nullptr)
            root = newNode;
        else
            compareAndLink(root, newNode);
        ++curSize;
    }
    void deleteMin() {
        auto oldNode = root;
        if (root->leftChild == nullptr)
            root = nullptr;
        else
            root = combineSiblings(root->leftChild);
        --curSize;
        delete oldNode;
    }

    void clear() {
        clear(root);
        curSize = 0;
    }
    bool empty() const { return curSize == 0; }
    std::size_t size() const { return curSize; }

   private:
    std::size_t curSize;
    PairNode *root;
    _Cmp cmp;

    PairNode *clone(PairNode *r) const {
        if (r == nullptr) return nullptr;
        PairNode *newNode = new PairNode{r->val};
        if ((newNode->leftChild = clone(r->leftChild)) != nullptr)
            newNode->leftChild->prev = newNode;
        if ((newNode->nextSibling = clone(r->nextSibling)) != nullptr)
            newNode->nextSibling->prev = newNode;
        return newNode;
    }

    void clear(PairNode *&r) {
        if (r != nullptr) {
            clear(r->leftChild);
            clear(r->nextSibling);
            delete r;
            r = nullptr;
        }
    }

    //合并结点并且让first指向堆顶
    void compareAndLink(PairNode *&first, PairNode *second) {
        // std::count << "compare\n";
        if (second == nullptr) return;
        if (cmp(second->val, first->val)) {
            second->prev = first->prev;
            first->prev = second;
            first->nextSibling = second->leftChild;
            if (first->nextSibling != nullptr) first->nextSibling->prev = first;
            second->leftChild = first;
            first = second;
        } else {
            second->prev = first;
            first->nextSibling = second->nextSibling;
            if (first->nextSibling != nullptr) first->nextSibling->prev = first;
            second->nextSibling = first->leftChild;
            if (second->nextSibling != nullptr)
                second->nextSibling->prev = second;
            first->leftChild = second;
        }
    }

    //合并一排结点（一层上的）并返回最后的根节点
    PairNode *combineSiblings(PairNode *firstSibling) {
        if (firstSibling->nextSibling == nullptr) return firstSibling;

        // Allocate the array
        static std::vector<PairNode *> treeArray(5);

        // Store the subtrees in an array
        std::size_t numSiblings = 0;
        for (; firstSibling != nullptr; ++numSiblings) {
            if (numSiblings == treeArray.size())
                treeArray.resize(numSiblings * 2);
            treeArray[numSiblings] = firstSibling;
            // treeArray.emplace_back(firstSibling);
            firstSibling->prev->nextSibling = nullptr;
            firstSibling = firstSibling->nextSibling;
        }
        treeArray.emplace_back(nullptr);

        std::size_t i = 0;
        for (; i + 1 < numSiblings; i += 2)
            compareAndLink(treeArray[i], treeArray[i + 1]);

        auto j = i - 2;

        if (j == numSiblings - 3)
            compareAndLink(treeArray[j], treeArray[j + 2]);
        for (; j >= 2; j -= 2) compareAndLink(treeArray[j - 2], treeArray[j]);
        return treeArray[0];
    }
};

#endif  // !PAIRING_HEAP_H
