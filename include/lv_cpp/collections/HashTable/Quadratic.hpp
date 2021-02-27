#ifndef QUADRATIC_HASH_TABLE_H
#define QUADRATIC_HASH_TABLE_H

#include <algorithm>
#include <iostream>
#include <vector>

template <typename T, typename Hash = std::hash<T>>
class QuadraticHashTable {
    enum class Info : char { ACTIVE, EMPTY, DELETED };

    struct Entry {
        T elem;
        Info info;
        Entry(const T &x = T{}) : elem(x), info(Info::EMPTY) {}
        Entry(T &&x) : elem(std::move(x)), info(Info::EMPTY) {}
    };

    struct hash_iterator {
        typename std::vector<Entry>::iterator cur;
        typename std::vector<Entry>::iterator end;

        hash_iterator(const typename std::vector<Entry>::iterator start,
                      const typename std::vector<Entry>::iterator over)
            : cur{start}, end{over} {
            if (cur != end) {
                while (cur != end && cur->info != Info::ACTIVE) ++cur;
            }
        }

        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;

        const T &operator*() const { return cur->elem; }

        const T *operator->() const { return &(this->operator*()); }

        bool operator==(const hash_iterator &rhs) const {
            return this->cur == rhs.cur && this->end == rhs.end;
        }

        bool operator!=(const hash_iterator &rhs) const {
            return !this->operator==(rhs);
        }

        hash_iterator &operator++() {
            if (cur == end) return *this;

            while (++cur != end && cur->info != Info::ACTIVE)
                ;

            return *this;
        }

        hash_iterator operator++(int) {
            auto old = *this;
            ++*this;
            return old;
        }
    };

   public:
    explicit QuadraticHashTable(std::size_t capacity = DEFAULT_SIZE)
        : curSize{0}, hashfunc{} {
        table.resize(DEFAULT_SIZE);
    }
    QuadraticHashTable(const QuadraticHashTable &) = default;
    QuadraticHashTable(QuadraticHashTable &&) = default;
    QuadraticHashTable &operator=(const QuadraticHashTable &) = default;
    QuadraticHashTable &operator=(QuadraticHashTable &&) = default;
    ~QuadraticHashTable() = default;

    bool erase(const T &x) {
        auto index = getIndex(x);
        if (!isActive(index)) return false;

        table[index].info = Info::DELETED;
        --curSize;
        return true;
    }
    bool find(const T &x) { return isActive(getIndex(x)); }

    bool insert(const T &x) {
        auto index = getIndex(x);
        if (isActive(index)) return false;  // duplicate
        table[index].elem = x;
        table[index].info = Info::ACTIVE;

        if (++curSize > table.size() >> 1) rehash();
        return true;
    }
    bool insert(T &&x) {
        auto index = getIndex(x);
        if (isActive(index)) return false;  // duplicate
        table[index].elem = std::move(x);
        table[index].info = Info::ACTIVE;
        // cout << "I am called\n";
        if (++curSize > table.size() >> 1) rehash();
        return true;
    }

    void clear() {
        QuadraticHashTable other{};
        std::swap(other, *this);
    }
    auto size() const { return curSize; }
    bool empty() const { return curSize == 0; }

    /*void show() const {
            vector<T> v;
            for (auto i : table)
                    if (i.info == Info::ACTIVE)
                            v.emplace_back(i.elem);

            std::sort(begin(v), end(v));
            for (auto i : v)
                    cout << i << ' ';
    }*/

    using iterator = hash_iterator;

    iterator begin() { return {this->table.begin(), this->table.end()}; }

    iterator end() { return {this->table.end(), this->table.end()}; }

   private:
    constexpr static char DEFAULT_SIZE = 17;

    std::vector<Entry> table;
    std::size_t curSize;
    Hash hashfunc;

    void rehash() {
        std::vector<Entry> old = table;
        table.resize(getNextSize(old.size() << 1));
        for (auto &entry : table) entry.info = Info::EMPTY;
        curSize = 0;
        for (auto &entry : old)
            if (entry.info == Info::ACTIVE) insert(std::move(entry.elem));
    }

    //以下函数写在cpp文件里面更好
    std::size_t getNextSize(std::size_t x) {
        if (!(x & 1)) ++x;
        for (; !this->isPrime(x); x += 2)
            ;
        return x;
    }

    bool isPrime(int x) {
        for (std::size_t i = 2; i * i <= x; ++i) {
            if (x % i == 0) return false;
        }
        return true;
    }

    std::size_t getIndex(const T &x) const {
        std::size_t offset = 1;
        std::size_t index = this->hashfunc(x) % table.size();
        //发生冲突,单方向探测
        while (table[index].info != Info::EMPTY && table[index].elem != x) {
            index += offset;
            offset += 2;
            if (index >= table.size()) index -= table.size();
        }
        return index;
    }

    bool isActive(std::size_t pos) const {
        return table[pos].info == Info::ACTIVE;
    }
};

#endif  // !QUADRATIC_HASH_TABLE_H
