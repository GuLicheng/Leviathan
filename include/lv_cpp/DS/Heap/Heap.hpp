#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include "BinaryHeap.hpp"
#include "BinomialHeap.hpp"
#include "PairingHeap.hpp"

/***
 *  这里未提供decreaseKey方法， 日后有机会完善此方法
 *  目前不支持lambda表达式作为Cmp， 但在以上某个堆中已经实现
 *  此堆不能保证在 Cmp != std::less<> or std::greater 情况下正常工作
 *  隔壁的hashtable也是一样,未来的balance tree可能依旧如此
 */

template <typename _Ky, typename Cmp = std::less<_Ky>,
          template <typename...> typename Container = BinaryHeap>
class Heap {
   public:
    Heap() : c{} {}
    Heap(const Heap &) = default;
    Heap(Heap &&) = default;
    Heap &operator=(const Heap &) = default;
    Heap &operator=(Heap &&) = default;
    ~Heap() = default;

    const _Ky &top() const { return c.top(); }
    bool empty() const { return c.empty(); }
    void push(const _Ky &x) { c.insert(x); }
    void push(_Ky &&x) { c.insert(std::move(x)); }
    void pop() { c.deleteMin(); }
    void clear() { c.clear(); }
    auto size() const { return c.size(); }

   private:
    Container<_Ky, Cmp> c;
};

#endif  // !PRIORITY_QUEUE_HPP
