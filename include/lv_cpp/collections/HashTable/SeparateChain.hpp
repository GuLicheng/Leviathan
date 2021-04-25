#ifndef SEPARATE_HASH_TABLE
#define SEPARATE_HASH_TABLE

#include <algorithm>
#include <iterator>
#include <list>
#include <vector>

// static int hash(int h) {
//	// This function ensures that hashCodes that differ only by
//	// constant multiples at each bit position have a bounded
//	// number of collisions (approximately 8 at default load factor).
//	// replace Mod with operator (h & size - 1)
//	h ^= (h >> 20) ^ (h >> 12);
//	return h ^ (h >> 7) ^ (h >> 4);
//}

template <typename _Ky, typename Hash = std::hash<_Ky>>
class SeparateHashTable {
   public:
    struct hash_iterator {
        using value_type = _Ky;
        using pointer = _Ky *;
        using reference = _Ky &;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;

        std::size_t index;                      // 索引
        SeparateHashTable *ptable;              // 表的指针
        typename std::list<_Ky>::iterator cur;  // 当前链表的迭代器

        hash_iterator(SeparateHashTable *ptable, std::size_t index,
                      typename std::list<_Ky>::iterator plist)
            : ptable{ptable}, index{index}, cur{plist} {}

        const _Ky &operator*() { return *cur; }

        const _Ky *operator->() { return &(operator*()); }

        bool operator==(const hash_iterator &rhs) {
            //不是一个表cur不可能相等
            if (index == -1 && rhs.index == -1) return true;
            return index == rhs.index ? cur == rhs.cur : false;
        }

        bool operator!=(const hash_iterator &rhs) { return !(operator==(rhs)); }

        hash_iterator &operator++() {
            if (index == -1)  // 不可以前进了
                return *this;
            ++cur;
            if (cur == ptable->table[index].end()) {
                nextIndex();
            }
            return *this;
        }

        hash_iterator operator++(int) {
            auto old = *this;
            ++*this;
            return old;
        }

       private:
        void nextIndex() {
            for (int i = index + 1; i < ptable->table.size(); ++i) {
                if (!ptable->table[i].empty()) {
                    index = i;
                    cur = this->ptable->table[i].begin();
                    return;
                }
            }
            index = -1;  // max
        }
    };
    using iterator = hash_iterator;
    // using value_type = _Ky;
    const static char DEFAULT_SIZE = 8;

    SeparateHashTable(std::size_t capacity = DEFAULT_SIZE) : curSize{0} {
        table.resize(capacity);
    }
    SeparateHashTable(const SeparateHashTable &) = default;
    SeparateHashTable(SeparateHashTable &&) = default;
    SeparateHashTable &operator=(const SeparateHashTable &) = default;
    SeparateHashTable &operator=(SeparateHashTable &&) = default;
    ~SeparateHashTable() = default;

    auto size() const { return curSize; }
    auto empty() const { return curSize == 0; }

    void clear() {  // 未测试
        SeparateHashTable other{};
        std::swap(*this, other);
    }

    bool insert(const _Ky &x) {
        auto index = getIndex(x);
        for (const auto &elem : table[index])
            if (elem == x) return false;

        table[index].push_front(x);
        ++curSize;
        if (curSize > table.size() << 1) rehash();

        return true;
    }
    bool insert(_Ky &&x) {
        auto index = getIndex(x);

        for (const auto &elem : table[index])
            if (elem == x) return false;

        table[index].push_front(std::move(x));
        ++curSize;
        if (curSize > table.size() << 1) rehash();

        return true;
    }
    bool find(const _Ky &x) {
        auto index = getIndex(x);
        for (auto it = table[index].begin(); it != table[index].end(); ++it) {
            if (*it == x) return true;
        }
        return false;
    }
    bool erase(const _Ky &x) {
        auto index = getIndex(x);
        for (auto it = table[index].begin(); it != table[index].end(); ++it) {
            if (*it == x) {
                table[index].erase(it);
                --curSize;
                return true;
            }
        }
        return false;
    }

    hash_iterator begin() {
        for (std::size_t i = 0; i < table.size(); ++i) {
            if (table[i].size())
                return hash_iterator{this, i, this->table[i].begin()};
        }
        return this->end();
    }

    hash_iterator end() {
        return hash_iterator{this, (std::size_t)(0) - 1,
                             this->table[0].begin()};
    }

    // void show() const { cout << table.capacity() << endl; }

   private:
    std::vector<std::list<_Ky>> table;
    std::size_t curSize;
    Hash hashfunc;

    auto getIndex(const _Ky &x) {
        std::size_t res = hashfunc(x);
        std::size_t loc = res & table.size() - 1;
        return loc;
    }

    void rehash() {
        // cout << "i am called\n";
        SeparateHashTable newHashTable{table.capacity() << 1};
        for (const auto &bucket : this->table)
            for (const auto &elem : bucket)
                newHashTable.insert(std::move(elem));
        std::swap(*this, newHashTable);
    }
};

#endif  // !SEPARATE_HASH_TABLE
