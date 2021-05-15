#ifndef __ITER_HPP__
#define __ITER_HPP__

#include <concepts>
#include <lv_cpp/meta/type_list.hpp>

namespace leviathan
{

    /*
        For derived iterator, you should provide follow method:
        dereference, equal_to, next, prev, advance, distance
    */

    template <typename Derived, 
            typename ValueType, 
            typename Category, 
            typename Reference = ValueType&, 
            typename Difference = std::ptrdiff_t>
    struct iterator_interface
    {

        static_assert(std::same_as<Derived, std::remove_cvref_t<Derived>>, "Derived should not be reference");

        using value_type = ValueType;
        using reference_type = Reference;
        using difference_type = Difference;
        using iterator_category = Category;

        constexpr reference_type operator*() const
        {
            return static_cast<Derived const*>(this)->dereference();
        }
        
        //  operator->() const; 

        constexpr reference_type operator[](difference_type n) const
        {
            Derived temp = this->derived();
            temp.advance(n);
            return temp.dereference();
        }

        constexpr Derived& operator++()
        {
            this->derived().next();
            return this->derived();
        }

        constexpr Derived operator++(int) requires (std::copyable<Derived>)
        {
            Derived temp = this->derived();
            ++ *this;
            return temp;
        }

        constexpr void operator++(int) 
        {
            (void)(this->derived().next());
        }

        constexpr Derived& operator--()
        {
            this->derived().prev();
            return this->derived();
        }

        constexpr Derived operator--(int)
        {
            Derived temp = this->derived();
            -- *this;
            return temp;
        }

        constexpr Derived& operator+=(difference_type n)
        {
            this->derived().advance(n);
            return this->derived();
        }

        constexpr Derived& operator-=(difference_type n)
        {
            this->derived().advance(-n);
            return this->derived();
        }

        constexpr Derived operator-(difference_type n) const
        {
            Derived temp = this->derived();
            temp -= n;
            return temp;
        }

        constexpr Derived operator+(difference_type n) const 
        {
            Derived temp = this->derived();
            temp += n;
            return temp;
        }

    private:
        constexpr Derived& derived() noexcept
        {
            return static_cast<Derived&>(*this);
        }

        constexpr const Derived& derived() const noexcept
        {
            return static_cast<const Derived&>(*this);
        }        
        
    };

    template <typename Iterator>
    concept iterator_derived = meta::is_instance<
        iterator_interface,
        iterator_interface<
            Iterator, 
            typename Iterator::value_type, 
            typename Iterator::iterator_category,
            typename Iterator::reference_type,
            typename Iterator::difference_type>
    >::value;

    template <typename DifferenceType, iterator_derived Iterator> 
    requires (std::convertible_to<typename Iterator::difference_type, DifferenceType>)
    constexpr auto operator+(DifferenceType n, const Iterator& iter)
    {
        return (iter + n);
    }

    template <typename DifferenceType, iterator_derived Iterator> 
    requires (std::convertible_to<typename Iterator::difference_type, DifferenceType>)
    constexpr auto operator-(DifferenceType n, const Iterator& iter)
    {
        return (iter - n);
    }

    template <iterator_derived Iterator> 
    constexpr auto operator-(const Iterator& lhs, const Iterator& rhs)
    {
        return lhs.distance(rhs);
    }


    template <typename Lhs, typename Rhs>
    concept binray_order = requires (const std::remove_cvref_t<Lhs>& lhs, const std::remove_cvref_t<Rhs>& rhs)
    {
        {lhs.equal_to(rhs)} -> std::convertible_to<bool>;
    };

    template <iterator_derived Iterator1, iterator_derived Iterator2>
    constexpr auto operator==(const Iterator1& lhs, const Iterator2& rhs)
    {
        if constexpr (binray_order<Iterator1, Iterator2>)
        {
            return lhs.equal_to(rhs) == 0;
        }
        else
        {
            return rhs.equal_to(lhs) == 0;
        }
    }

    template <iterator_derived Iterator1, iterator_derived Iterator2>
    constexpr auto operator!=(const Iterator1& lhs, const Iterator2& rhs)
    {
        return !(lhs == rhs);
    }

    template <iterator_derived Iterator1, iterator_derived Iterator2>
    requires (std::derived_from<typename Iterator1::iterator_category, std::random_access_iterator_tag> || std::derived_from<typename Iterator2::iterator_category, std::random_access_iterator_tag>)
    constexpr auto operator<(const Iterator1& lhs, const Iterator2& rhs)
    {
        if constexpr (binray_order<Iterator1, Iterator2>)
        {
            return lhs.equal_to(rhs) < 0;
        }
        else
        {
            return rhs.equal_to(lhs) >= 0;
        }
    }

    template <iterator_derived Iterator1, iterator_derived Iterator2>
    requires (std::derived_from<typename Iterator1::iterator_category, std::random_access_iterator_tag> || std::derived_from<typename Iterator2::iterator_category, std::random_access_iterator_tag>)
    constexpr auto operator<=(const Iterator1& lhs, const Iterator2& rhs)
    {
        return !(lhs > rhs);
    }

    template <iterator_derived Iterator1, iterator_derived Iterator2>
    requires (std::derived_from<typename Iterator1::iterator_category, std::random_access_iterator_tag> || std::derived_from<typename Iterator2::iterator_category, std::random_access_iterator_tag>)
    constexpr auto operator>(const Iterator1& lhs, const Iterator2& rhs)
    {
        return rhs < lhs;
    }

    
    template <iterator_derived Iterator1, iterator_derived Iterator2>
    requires (std::derived_from<typename Iterator1::iterator_category, std::random_access_iterator_tag> || std::derived_from<typename Iterator2::iterator_category, std::random_access_iterator_tag>)
    constexpr auto operator>=(const Iterator1& lhs, const Iterator2& rhs)
    {
        return !(lhs < rhs);
    }

}

#endif