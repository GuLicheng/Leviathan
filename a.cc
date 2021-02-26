// #include <lv_cpp/io/console.hpp>

struct A
{
    void T(int);
    template <typename U>
    void Y(U);
};

struct B
{
    struct T
    {
        using type = int;
    };
    template <typename U>
    struct Y
    {
        using type = U;
    };
};

struct C : A, B
{
};

int main()
{
    C::T::type a;
    C::B::Y<int>::type b;
}
