#include <mutex>
#include <type_traits>
#include <iostream>
#include <optional>
#include <variant>

template <typename F>
class lazy
{
    using return_type = std::invoke_result_t<F>; 
private:
    F m_computation;
    // mutable return_type m_cache; // ...require default constructable
    mutable std::once_flag m_flag;
    mutable std::optional<return_type> m_cache;

public:

    lazy(F f) : m_computation{ std::move(f) } { }

    operator const return_type& () const 
    {
        std::call_once(m_flag, [this]() { 
            this->m_cache.emplace(this->m_computation());
        });
        return *m_cache;
    }

};

struct Foo
{
    Foo(int i) { }
    ~Foo() { std::cout << "des\n"; }
};



template <typename F>
class lazyV2
{
    using return_type = std::invoke_result_t<F>; 
private:
    F m_computation;
    mutable std::once_flag m_flag;
    mutable bool m_init = false;
    mutable std::aligned_storage_t<sizeof(return_type), std::alignment_of_v<return_type>> m_cache;

public:

    lazyV2(F f) : m_computation{ std::move(f) } { }

    operator const return_type& () const 
    {
        std::call_once(m_flag, [this]() { 
            ::new ((void*)std::addressof(this->m_cache)) return_type(this->m_computation());
            this->m_init = true;
        });
        return *(reinterpret_cast<return_type*>(&m_cache));
    }

    ~lazyV2() 
    {
        if (m_init)
            (reinterpret_cast<return_type*>(&m_cache))->~return_type();
    }

};

void test()
{
    lazyV2 add { []() { return 2; }};
    lazyV2 foo { []() { return Foo{1}; }};
    std::cout << (int) add << '\n';
    (void)(Foo)foo;
}

union Bar 
{
    int x;
    Foo f;

    Bar() : x{} { }
    ~Bar() { }
};

int main(int argc, char const *argv[])
{

    // test();

    Bar b;

    return 0;
}
