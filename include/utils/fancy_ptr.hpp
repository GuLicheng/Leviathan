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

template<typename T>
class FancyPtr {
    T* ptr = nullptr;
    FancyPtr(T *ptr, bool) : ptr(ptr) {}
public:
    using element_type = T;

    using difference_type = std::ptrdiff_t;
    using value_type = element_type;
    using pointer = element_type*;
    using reference = element_type&;
    using iterator_category = std::random_access_iterator_tag;

    FancyPtr() = default;
    FancyPtr(const FancyPtr&) = default;

    FancyPtr& operator=(const FancyPtr&) = default;

    static FancyPtr pointer_to(element_type& r) noexcept {
        return FancyPtr(std::addressof(r), true);
    }

    // allocator::pointer is convertible to allocator::const_pointer
    template<typename U = T, typename std::enable_if<std::is_const<U>::value, int>::type = 0>
    FancyPtr(const FancyPtr<typename std::remove_const<T>::type>& p) : ptr(p.operator->()  /* std::to_address(p) in C++20 */) {}

    // NullablePointer
    FancyPtr(std::nullptr_t) : FancyPtr() {}
    FancyPtr& operator=(std::nullptr_t) {
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
    FancyPtr& operator++() {
        ++ptr;
        return *this;
    }
    // InputIterator
    friend bool operator==(FancyPtr l, FancyPtr r) {
        return l.ptr == r.ptr;
    }
    friend bool operator!=(FancyPtr l, FancyPtr r) {
        return !(l == r);
    }
    FancyPtr operator++(int) {
        return FancyPtr(ptr++, true);
    }
    // BidirectionalIterator
    FancyPtr& operator--() {
        --ptr;
        return *this;
    }
    FancyPtr operator--(int) {
        return FancyPtr(ptr--, true);
    }
    // RandomAccessIterator
    FancyPtr& operator+=(difference_type n) {
        ptr += n;
        return *this;
    }
    friend FancyPtr operator+(FancyPtr p, difference_type n) {
        return p += n;
    }
    friend FancyPtr operator+(difference_type n, FancyPtr p) {
        return p += n;
    }
    FancyPtr& operator-=(difference_type n) {
        ptr -= n;
        return *this;
    }
    friend FancyPtr operator-(FancyPtr p, difference_type n) {
        return p -= n;
    }
    friend difference_type operator-(FancyPtr a, FancyPtr b) {
        return a.ptr - b.ptr;
    }
    reference operator[](difference_type n) const {
        return ptr[n];
    }
    friend bool operator<(FancyPtr a, FancyPtr b) {
        return std::less<pointer>(a.ptr, b.ptr);
    }
    friend bool operator> (FancyPtr a, FancyPtr b) { return b < a; }
    friend bool operator>=(FancyPtr a, FancyPtr b) { return !(a < b); }
    friend bool operator<=(FancyPtr a, FancyPtr b) { return !(b < a); }


#if defined(_LIBCPP_MEMORY)
    // Extra libc++ requirement (Since libc++ does `static_cast<FancyPtr<U>>(FancyPtr<T>())` sometimes)
    template<typename U> FancyPtr(FancyPtr<U> p) : ptr(static_cast<T*>(p.operator->())) {}
#elif defined(_GLIBCXX_MEMORY)
    // Extra libstdc++ requirement (Since libstdc++ uses raw pointers internally and tries to implicitly cast back
    // and also casts from pointers to different types)
    template<typename U> FancyPtr(FancyPtr<U> p) : ptr(static_cast<T*>(p.operator->())) {}
    FancyPtr(T *ptr) : FancyPtr(ptr, true) {}
    operator T*() { return ptr; }
#endif
};

// NullablePointer (Not strictly necessary because of implicit conversion from nullptr to FancyPtr)
template<typename T>
bool operator==(FancyPtr<T> p, std::nullptr_t) {
    return p == FancyPtr<T>();
}
template<typename T>
bool operator==(std::nullptr_t, FancyPtr<T> p) {
    return FancyPtr<T>() == p;
}
template<typename T>
bool operator!=(FancyPtr<T> p, std::nullptr_t) {
    return p != FancyPtr<T>();
}
template<typename T>
bool operator!=(std::nullptr_t, FancyPtr<T> p) {
    return FancyPtr<T>() != p;
}

template<>
class FancyPtr<void> {
    void* ptr = nullptr;
    FancyPtr(void *ptr, bool) : ptr(ptr) {}
public:
    using element_type = void;
    using pointer = void*;

    FancyPtr() = default;
    FancyPtr(const FancyPtr&) = default;
    template<typename T, typename std::enable_if<!std::is_const<T>::value, int>::type = 0>
    FancyPtr(FancyPtr<T> p) : ptr(static_cast<void*>(p.operator->())) {}

    FancyPtr& operator=(const FancyPtr&) = default;
    FancyPtr& operator=(std::nullptr_t) { ptr = nullptr; return *this; }

    pointer operator->() const {
        return ptr;
    }

    // static_cast<A::pointer>(vp) == p
    template<typename T>
    explicit operator FancyPtr<T>() {
        if (ptr == nullptr) return nullptr;
        return std::pointer_traits<FancyPtr<T>>::pointer_to(*static_cast<T*>(ptr));
    }
};

template<>
class FancyPtr<const void> {
    const void* ptr = nullptr;
    FancyPtr(const void *ptr, bool) : ptr(ptr) {}
public:
    using element_type = const void;
    using pointer = const void*;

    FancyPtr() = default;
    FancyPtr(const FancyPtr&) = default;
    template<typename T>
    FancyPtr(FancyPtr<T> p) : ptr(static_cast<const void*>(p.operator->())) {}

    FancyPtr& operator=(const FancyPtr&) = default;
    FancyPtr& operator=(std::nullptr_t) { ptr = nullptr; return *this; }

    pointer operator->() const {
        return ptr;
    }

    // static_cast<A::const_pointer>(cvp) == cp
    template<typename T>
    explicit operator FancyPtr<const T>() {
        if (ptr == nullptr) return nullptr;
        return std::pointer_traits<FancyPtr<const T>>::pointer_to(*static_cast<const T*>(ptr));
    }
};

template<typename T>
class TrivialAllocator {
public:
    using pointer = FancyPtr<T>;
    using value_type = T;

    TrivialAllocator() = default;

    template<typename Other>
    TrivialAllocator(const TrivialAllocator<Other> &) {}

    TrivialAllocator(const TrivialAllocator &alloc) = default;

    pointer allocate(size_t n) {
        std::allocator<T> alloc;
        return pointer::pointer_to(*std::allocator_traits<std::allocator<T>>::allocate(alloc, n*sizeof(T)));
    }
    void deallocate(pointer ptr, size_t n) {
        std::allocator<T> alloc;
        // std::to_address(ptr) instead of ptr.operator-> in C++20
        std::allocator_traits<std::allocator<T>>::deallocate(alloc, ptr.operator->(), n*sizeof(T));
    }

    bool operator==(const TrivialAllocator &) const { return true; }

    bool operator!=(const TrivialAllocator &) const { return false; }
};

// #include <list>

// template struct std::list<long double, TrivialAllocator<long double>>;

// int main() {
//     struct Test {};
//     using AllocT = std::allocator_traits<TrivialAllocator<long double>>;
//     static_assert(std::is_same<FancyPtr<long double>,std::pointer_traits<AllocT::pointer>::pointer>::value, "");
//     static_assert(std::is_same<FancyPtr<Test>, std::pointer_traits<AllocT::pointer>::rebind<Test>>::value, "");


//     std::list<long double, AllocT::allocator_type> list;
//     list.push_back(0);
//     for (auto i : list) { (void) i; }
//     list.pop_front();
// }
