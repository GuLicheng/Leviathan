#include <lv_cpp/utils/iter.hpp>
#include <lv_cpp/io/console.hpp>

#include <ranges>
#include <compare>

template <typename T>
struct count_iterator
    : leviathan::iterator_interface<count_iterator<T>, T, std::random_access_iterator_tag>
{
    T* value;
    // dereference, equal, next, prev, advance, distance

    count_iterator(T* init) : value{init}
    {
    }

    count_iterator() : value{nullptr}
    {
    }

    count_iterator(const count_iterator&) = default;
    count_iterator& operator=(const count_iterator&) = default;

    T& dereference() const noexcept
    {
        return *value;
    }

    bool equal(count_iterator const& rhs) const noexcept
    {
        return *value - *rhs.value;
    }

    auto operator<=>(count_iterator const& rhs) const noexcept
    {
        std::cout << "called\n";
        return *value <=> *rhs.value;
    }


    count_iterator& next() noexcept
    {
        ++(*value);
        return *this;
    }

    count_iterator& prev() noexcept
    {
        --(*value);
        return *this;
    }

    count_iterator& advance(std::ptrdiff_t n) noexcept
    {
        (*value) += n; 
        return *this;
    }

    std::ptrdiff_t distance(const count_iterator& rhs) const noexcept
    {
        return rhs.value - value;
    }

};

int main()
{
    int x = 1, y = 2, z = 3;
    count_iterator<int> iter1{&x}, iter2{&y}, iter3{&z};
    using T = decltype(iter1);
    auto iter = 3 + iter1;
    console::write_line("distance of x and y is {0}, and distance of x and z is {1}", iter2 - iter1, iter3 - iter1);
    console::write_line("bidirectional_iterator ? {0}", std::bidirectional_iterator<T>);
    console::write_line("random_access_iterator ? {0}", std::random_access_iterator<T>);
}