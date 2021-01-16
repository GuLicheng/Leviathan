#pragma once

#include <map>
#include <type_traits>


template< class T >
class UF {
    template< class = void >
    struct Map {
        using f = map<T, T>;
        using rank = map<T, size_t>;
    };
    template<>
    struct Map<void_t<decltype(declval<hash<T>>().operator()(declval<T>()))>> {
        using f = unordered_map<T, T>;
        using rank = unordered_map<T, size_t>;
    };
    using f_t = typename Map<>::f;
    using rank_t = typename Map<>::rank;
    f_t f;
    rank_t rank;
public:
    T operator[](T x) {
        if (!f.count(x)) {
            f[x] = x;
            rank[x] = 1;
        }
        return f[x] == x ? x : f[x] = (*this)[f[x]];
    }
    void unite(int x, int y) {
        if (rank[x] < rank[y]) swap(x, y);
        rank[x] += rank[y];
        f[y] = x;
    }
    bool find_and_unite(int x, int y) {
        int fx = (*this)[x], fy = (*this)[y];
        if (fx != fy) unite(fx, fy);
        return fx != fy;
    }
    auto size() { return size(f); }
    auto count() {
        size_t num = 0;
        for (auto&& [x, fa] : f)
            if (x == fa) ++num;
        return num;
    }
};