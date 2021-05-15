#include <lv_cpp/io/console.hpp>
#include <lv_cpp/utils/iter.hpp>

#include <ranges>
#include <compare>

template <typename T>
struct count_iterator
    : leviathan::iterator_interface<count_iterator<T>, T, std::random_access_iterator_tag>
{
    T* value;
    // dereference, equal, next, prev, advance, distance

    constexpr count_iterator(T* init) : value{init}
    {
    }

    constexpr count_iterator() : value{nullptr}
    {
    }

    constexpr count_iterator(const count_iterator&) = default;
    constexpr count_iterator& operator=(const count_iterator&) = default;

    constexpr T& dereference() const noexcept
    {
        return *value;
    }

    constexpr auto equal_to(count_iterator const& rhs) const noexcept
    {
        return *value - *rhs.value;
    }


    constexpr count_iterator& next() noexcept
    {
        ++(*value);
        return *this;
    }

    constexpr count_iterator& prev() noexcept
    {
        --(*value);
        return *this;
    }

    constexpr count_iterator& advance(std::ptrdiff_t n) noexcept
    {
        (*value) += n; 
        return *this;
    }

    constexpr std::ptrdiff_t distance(const count_iterator& rhs) const noexcept
    {
        return rhs.value - value;
    }

};

int main()
{
    int x = 1, y = 2, z = 3;
    count_iterator<int> iter1{&x}, iter2{&y}, iter3{&z};
    using T = decltype(iter1);
    console::write_line("distance of x and y is {0}, and distance of x and z is {1}", iter2 - iter1, iter3 - iter1);
    console::write_line("bidirectional_iterator ? {0}", std::bidirectional_iterator<T>);
    console::write_line("random_access_iterator ? {0}", std::random_access_iterator<T>);

    console::write_line("iter1 = 1, iter2 = 2 and res is {0}", iter2 > iter2);
    console::write_line("iter1 = 1, iter2 = 2 and res is {0}", iter2 >= iter2);
    console::write_line("iter1 = 1, iter2 = 2 and res is {0}", iter2 < iter2);
    console::write_line("iter1 = 1, iter2 = 2 and res is {0}", iter2 <= iter2);
    console::write_line("iter1 = 1, iter2 = 2 and res is {0}", iter2 == iter2);
    console::write_line("iter1 = 1, iter2 = 2 and res is {0}", iter2 != iter2);

}