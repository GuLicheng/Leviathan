#ifndef __COUNT_HPP__
#define __COUNT_HPP__


#include <ranges>


namespace leviathan
{

namespace ranges
{
#if 0
template <typename T, typename U>
class count_view : public std::ranges::view_interface<count_view<T, U>>
{
    using iota_t = decltype(::std::views::iota(std::declval<T>()), ::std::declval<U>());
private:
    
    // struct Sentinel;
    struct Iterator
    {
    private:
        decltype(::std::declval<iota_t>().begin()) current;
        friend count_view;
        count_view* base;
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        constexpr Iterator(count_view& parent, T init) 
            : base(std::addressof(parent)), current(std::move(init)) {
                std::cout << init << std::endl;
            };

        Iterator& operator++()
        {
            current += base->step;
        }

        Iterator operator++(int)
        {
            auto old = *this;
            ++ *this;
            return old;
        }

        T& operator*() noexcept
        {
            return this->current;
        }

        const T& operator*() const noexcept
        {
            return this->current;
        }

        friend constexpr bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept
        {
            // if cur >= last, we must end loop
            return lhs.current >= rhs.current;  
        }

        friend constexpr bool operator!=(const Iterator& lhs, const Iterator& rhs) noexcept
        {
            return !(lhs == rhs);
        }



    };

    iota_t m_iota;
    T step;

public:


    constexpr count_view(T first, T step, T last) 
        : m_iota(std::views::iota(first, last)), step(step) { }

    constexpr Iterator begin() noexcept
    {
        return {*this, *this->m_iota.begin()};
    }

    constexpr Iterator end() noexcept
    {
        return {*this, *this->m_iota.end()};
    }

};  
#endif
// template <typename T>
// count_view(T&&, T&&, T&&) -> count_view<::std::views::all_t<T>>;

template <typename T>
class count_view : public std::ranges::view_interface<count_view<T>>
{
public:
    T step;
    decltype(::std::views::iota(::std::declval<T>(), ::std::declval<T>())) m_iota;

    struct Iterator
    {
        count_view* base;
        T current;

        constexpr Iterator(count_view& parent, T init)
            : base(std::addressof(parent)), current(init) { }

        Iterator& operator++()
        {
            current += base->step;
        }

        Iterator operator++(int)
        {
            auto old = *this;
            ++ *this;
            return old;
        }

        T& operator*() noexcept
        {
            return this->current;
        }

        const T& operator*() const noexcept
        {
            return this->current;
        }

        friend constexpr bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept
        {
            // if cur >= last, we must end loop
            return lhs.current >= rhs.current;  
        }

        friend constexpr bool operator!=(const Iterator& lhs, const Iterator& rhs) noexcept
        {
            return !(lhs == rhs);
        }

    };
    constexpr count_view(T first, T last, T step)
        : m_iota(::std::views::iota(first, last)), step(step) { }

    

};


} // namespace ranges



namespace views
{

inline constexpr ::std::ranges::views::__adaptor::_RangeAdaptorClosure count = []<typename T>(T first, T last, T step)
{
    return ::leviathan::ranges::count_view{first, step, last};
};

}

}




#endif

