// 内存分配器 Allocator
#include <vector>
#include <list>
#include <deque>
#include <iostream>

template<typename _Ty>
struct Allocator_base {
    using value_type = _Ty;
};

template<typename _Ty>
struct Allocator_base<const _Ty> {
    using value_type = _Ty;
};

template<typename _Ty>
class Allocator : public Allocator_base<_Ty> {
public:
    //inner type of data
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;  //type of the minus of two pointers
    typedef _Ty* pointer;
    typedef _Ty& reference;
    typedef const _Ty& const_reference;
    typedef const _Ty* const_pointer;
    typedef Allocator_base<_Ty> _My_base;
    typedef typename _My_base::value_type value_type;

    template<typename _U>
    struct rebind {
        typedef Allocator<_U> other; // type_cast if the type is difference(type not unique)
    };

    Allocator() = default;
    Allocator(const Allocator&) = default;

    template<typename _otherAll>
    Allocator(const Allocator<_otherAll>&) noexcept {};

    ~Allocator() = default;

    //apply memory 
    pointer allocate(size_type num) {
        //------------------------------show information
        static int i = 0;
        ++i;
        std::cout << std::endl;
        std::cout << "the nums of allocate memory :" << num << std::endl;;
        std::cout << "------------------------------------------\n";
        std::cout << "allcation of room " << num << std::endl;
        //-----------------------------
        return (pointer)(::operator new(num * sizeof(_Ty)));
    }

    // construct obj in memory
    void construct(pointer p, const_reference value) {
        new (p)_Ty(value); // (one of overloads of operator new) placement new
    }

    //destory obj
    void destory(pointer p) {
        p->~Ty();
    }

    // relese memory
    void deallocate(pointer p, size_type size) {
        ::operator delete(p);
    }

};

// test code
template<typename T>
void print(T&& v) {
    // cout << "the capacity of container is " << v.capacity() << "\n";
    std::cout << "the size of container is  " << v.size() << std::endl;
    for (auto i : v)
        std::cout << i << ' ';
    std::cout << std::endl;
}

int main() {
    std::deque<int, Allocator<int>> ls{ 1,2,3 };
    for (int i = 0; i < 10; ++i) {
        ls.push_back(i);
    }
    print(ls);
    return 0;
}