/*
    https://stackoverflow.com/questions/65645904/implementing-a-custom-allocator-with-fancy-pointers
    https://en.cppreference.com/mwiki/index.php?title=cpp%2Fconcept%2FAllocator&diff=89066&oldid=86365
*/
#include <cstddef>
#include <iterator>
#include <memory>
#include <functional>
#include <utility>
#include <type_traits>

namespace leviathan::alloc
{

template<typename T>
class fancy_ptr {
    T* ptr = nullptr;
    fancy_ptr(T *ptr, bool) : ptr(ptr) {}
public:
    using element_type = T;

    using difference_type = std::ptrdiff_t;
    using value_type = element_type;
    using pointer = element_type*;
    using reference = element_type&;
    using iterator_category = std::random_access_iterator_tag;

    fancy_ptr() = default;
    fancy_ptr(const fancy_ptr&) = default;

    fancy_ptr& operator=(const fancy_ptr&) = default;

    static fancy_ptr pointer_to(element_type& r) noexcept {
        return fancy_ptr(std::addressof(r), true);
    }

    // allocator::pointer is convertible to allocator::const_pointer
    template<typename U = T, typename std::enable_if<std::is_const<U>::value, int>::type = 0>
    fancy_ptr(const fancy_ptr<typename std::remove_const<T>::type>& p) : ptr(p.operator->()  /* std::to_address(p) in C++20 */) {}

    // NullablePointer
    fancy_ptr(std::nullptr_t) : fancy_ptr() {}
    fancy_ptr& operator=(std::nullptr_t) {
        ptr = nullptr;
        return *this;
    }
    explicit operator bool() const { return *this != nullptr; }

    // to_address, InputIterator
    pointer operator->() const {
        return ptr;
    }

    // Iterator
    element_type& operator*() const {
        return *ptr;
    }
    fancy_ptr& operator++() {
        ++ptr;
        return *this;
    }
    // InputIterator
    friend bool operator==(fancy_ptr l, fancy_ptr r) {
        return l.ptr == r.ptr;
    }
    friend bool operator!=(fancy_ptr l, fancy_ptr r) {
        return !(l == r);
    }
    fancy_ptr operator++(int) {
        return fancy_ptr(ptr++, true);
    }
    // BidirectionalIterator
    fancy_ptr& operator--() {
        --ptr;
        return *this;
    }
    fancy_ptr operator--(int) {
        return fancy_ptr(ptr--, true);
    }
    // RandomAccessIterator
    fancy_ptr& operator+=(difference_type n) {
        ptr += n;
        return *this;
    }
    friend fancy_ptr operator+(fancy_ptr p, difference_type n) {
        return p += n;
    }
    friend fancy_ptr operator+(difference_type n, fancy_ptr p) {
        return p += n;
    }
    fancy_ptr& operator-=(difference_type n) {
        ptr -= n;
        return *this;
    }
    friend fancy_ptr operator-(fancy_ptr p, difference_type n) {
        return p -= n;
    }
    friend difference_type operator-(fancy_ptr a, fancy_ptr b) {
        return a.ptr - b.ptr;
    }
    reference operator[](difference_type n) const {
        return ptr[n];
    }
    friend bool operator<(fancy_ptr a, fancy_ptr b) {
        return std::less<pointer>(a.ptr, b.ptr);
    }
    friend bool operator> (fancy_ptr a, fancy_ptr b) { return b < a; }
    friend bool operator>=(fancy_ptr a, fancy_ptr b) { return !(a < b); }
    friend bool operator<=(fancy_ptr a, fancy_ptr b) { return !(b < a); }


#if defined(_LIBCPP_MEMORY)
    // Extra libc++ requirement (Since libc++ does `static_cast<fancy_ptr<U>>(fancy_ptr<T>())` sometimes)
    template<typename U> fancy_ptr(fancy_ptr<U> p) : ptr(static_cast<T*>(p.operator->())) {}
#elif defined(_GLIBCXX_MEMORY)
    // Extra libstdc++ requirement (Since libstdc++ uses raw pointers internally and tries to implicitly cast back
    // and also casts from pointers to different types)
    template<typename U> fancy_ptr(fancy_ptr<U> p) : ptr(static_cast<T*>(p.operator->())) {}
    fancy_ptr(T *ptr) : fancy_ptr(ptr, true) {}
    operator T*() { return ptr; }
#endif
};

// NullablePointer (Not strictly necessary because of implicit conversion from nullptr to fancy_ptr)
template<typename T>
bool operator==(fancy_ptr<T> p, std::nullptr_t) {
    return p == fancy_ptr<T>();
}
template<typename T>
bool operator==(std::nullptr_t, fancy_ptr<T> p) {
    return fancy_ptr<T>() == p;
}
template<typename T>
bool operator!=(fancy_ptr<T> p, std::nullptr_t) {
    return p != fancy_ptr<T>();
}
template<typename T>
bool operator!=(std::nullptr_t, fancy_ptr<T> p) {
    return fancy_ptr<T>() != p;
}

template<>
class fancy_ptr<void> {
    void* ptr = nullptr;
    fancy_ptr(void *ptr, bool) : ptr(ptr) {}
public:
    using element_type = void;
    using pointer = void*;

    fancy_ptr() = default;
    fancy_ptr(const fancy_ptr&) = default;
    template<typename T, typename std::enable_if<!std::is_const<T>::value, int>::type = 0>
    fancy_ptr(fancy_ptr<T> p) : ptr(static_cast<void*>(p.operator->())) {}

    fancy_ptr& operator=(const fancy_ptr&) = default;
    fancy_ptr& operator=(std::nullptr_t) { ptr = nullptr; return *this; }

    pointer operator->() const {
        return ptr;
    }

    // static_cast<A::pointer>(vp) == p
    template<typename T>
    explicit operator fancy_ptr<T>() {
        if (ptr == nullptr) return nullptr;
        return std::pointer_traits<fancy_ptr<T>>::pointer_to(*static_cast<T*>(ptr));
    }
};

template<>
class fancy_ptr<const void> {
    const void* ptr = nullptr;
    fancy_ptr(const void *ptr, bool) : ptr(ptr) {}
public:
    using element_type = const void;
    using pointer = const void*;

    fancy_ptr() = default;
    fancy_ptr(const fancy_ptr&) = default;
    template<typename T>
    fancy_ptr(fancy_ptr<T> p) : ptr(static_cast<const void*>(p.operator->())) {}

    fancy_ptr& operator=(const fancy_ptr&) = default;
    fancy_ptr& operator=(std::nullptr_t) { ptr = nullptr; return *this; }

    pointer operator->() const {
        return ptr;
    }

    // static_cast<A::const_pointer>(cvp) == cp
    template<typename T>
    explicit operator fancy_ptr<const T>() {
        if (ptr == nullptr) return nullptr;
        return std::pointer_traits<fancy_ptr<const T>>::pointer_to(*static_cast<const T*>(ptr));
    }
};

template<typename T>
class trivial_allocator 
{
public:
    using pointer = fancy_ptr<T>;
    using value_type = T;

    trivial_allocator() = default;

    template<typename Other>
    trivial_allocator(const trivial_allocator<Other>&) {}

    trivial_allocator(const trivial_allocator&) = default;

    pointer allocate(size_t n) {
        std::allocator<T> alloc;
        return pointer::pointer_to(*std::allocator_traits<std::allocator<T>>::allocate(alloc, n*sizeof(T)));
    }
    void deallocate(pointer ptr, size_t n) {
        std::allocator<T> alloc;
        // std::to_address(ptr) instead of ptr.operator-> in C++20
        // std::allocator_traits<std::allocator<T>>::deallocate(alloc, ptr.operator->(), n*sizeof(T));
        std::allocator_traits<std::allocator<T>>::deallocate(alloc, std::to_address(ptr), n * sizeof(T));
    }

    bool operator==(const trivial_allocator &) const { return true; }

    // bool operator!=(const trivial_allocator &) const { return false; }
};

}

// #include <list>

// template struct std::list<long double, trivial_allocator<long double>>;

// int main() {
//     struct Test {};
//     using AllocT = std::allocator_traits<trivial_allocator<long double>>;
//     static_assert(std::is_same<fancy_ptr<long double>,std::pointer_traits<AllocT::pointer>::pointer>::value, "");
//     static_assert(std::is_same<fancy_ptr<Test>, std::pointer_traits<AllocT::pointer>::rebind<Test>>::value, "");


//     std::list<long double, AllocT::allocator_type> list;
//     list.push_back(0);
//     for (auto i : list) { (void) i; }
//     list.pop_front();
// }
