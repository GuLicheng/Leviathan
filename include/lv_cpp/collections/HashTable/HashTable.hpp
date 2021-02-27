#ifndef HAHSTABLE_H
#define HASHTABLE_H

#include "Quadratic.hpp"
#include "SeparateChain.hpp"

/**
 * Before C++17 we use "template<typename _Ky, typename _Hash = std::hash<_Ky>,
 *                          template <typename _K, typename _H> class _Container
 * = SeparateHashTable>"
 *
 * After C++17 we can use follower
 *
 * Or tempate<typename...> typename _Container = SeparateHashTable (see
 * DB/PriorityQueue.hpp)
 *
 *
 */

template <typename _Ky, typename _Hash = std::hash<_Ky>,
          template <typename, typename> typename _Container = SeparateHashTable>
class HashTable {
    _Container<_Ky, _Hash> c;

   public:
    HashTable() : c{} {}
    HashTable(const HashTable &) = default;
    HashTable(HashTable &&) = default;
    HashTable &operator=(const HashTable &) = default;
    HashTable &operator=(HashTable &&) = default;
    ~HashTable() = default;

    auto begin() { return c.begin(); }
    auto end() { return c.end(); }
    auto size() const { return c.size(); }
    bool empty() const { return c.empty(); }
    void clear() { c.clear(); }
    bool insert(const _Ky &x) { return c.insert(x); }
    bool insert(_Ky &&x) { return c.insert(std::move(x)); }
    bool find(const _Ky &x) { return c.find(x); }
    bool erase(const _Ky &x) { return c.erase(x); }
};

#endif  // !HAHSTABLE_H
