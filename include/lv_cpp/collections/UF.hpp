#pragma once

#include <map>
#include <type_traits>

template< class T >
class UF {
	template< class = void >
	struct hash_traits  : false_type {};
	template<>
    struct hash_traits<void_t<decltype(declval<hash<T>>().operator()(declval<T>()))>>
                        : true_type {};
	template< class Key, class Value >
	using map_t = conditional_t<hash_traits<>{}, unordered_map<Key, Value>, map<Key, Value>>;
	map_t<T, T> f;
	map_t<T, size_t> rank;
public:
	T operator[] ( T x ) {
		if ( !f.count(x)) {
			f[x] = x;
			rank[x] = 1;
		}
		return f[x] == x ? x : f[x] = ( *this )[f[x]];
	}

	void unite ( T x, T y ) {
		if ( rank[x] < rank[y] ) swap(x, y);
		rank[x] += rank[y];
		f[y] = x;
	}

	bool find_and_unite ( T x, T y ) {
		auto fx = ( *this )[x], fy = ( *this )[y];
		if ( fx != fy ) unite(fx, fy);
		return fx != fy;
	}

	auto size () { return size(f); }

	auto count () {
		size_t num = 0;
		for ( auto&& [x, fa] : f )
			if ( x == fa ) ++num;
		return num;
	}
};