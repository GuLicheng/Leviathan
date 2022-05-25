#pragma once

#define GenerateIteratorInterface(IteratorT, ConstIteratorT, Reverse, Self) \
    using iterator = IteratorT; \
    using const_iterator = ConstIteratorT; \
    using reverse_iterator =  std::reverse_iterator<iterator>; \
    using const_reverse_iterator = std::reverse_iterator<const_iterator>; \
    static_assert(std::is_constructible_v<ConstIteratorT, IteratorT>); \
    const_iterator begin() const noexcept { return const_cast<Self&>(*this).begin(); } \
    const_iterator end() const noexcept { return const_cast<Self&>(*this).end(); } \
    const_iterator cbegin() const noexcept { return this->begin(); } \
    const_iterator cend() const noexcept { return this->end(); } \
    reverse_iterator rbegin() noexcept requires (Reverse) { return std::make_reverse_iterator(this->end()); } \
    reverse_iterator rend() noexcept requires (Reverse) { return std::make_reverse_iterator(this->begin()); } \
    const_reverse_iterator rbegin() const noexcept requires (Reverse) { return const_cast<Self&>(*this).rbegin(); } \
    const_reverse_iterator rend() const noexcept requires (Reverse) { return const_cast<Self&>(*this).rend(); } \
    const_reverse_iterator rcbegin() const noexcept requires (Reverse) { return this->rbegin(); } \
    const_reverse_iterator rcend() const noexcept requires (Reverse) { return this->rend(); }


template <typename IteratorT, typename ConstIteratorT, typename Derived, bool Reverse>
struct container_iterator_interface
{
    using self_type = container_iterator_interface<IteratorT, ConstIteratorT, Derived, Reverse>;

    using iterator = IteratorT;
    using const_iterator = ConstIteratorT;
    using reverse_iterator =  std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static_assert(std::is_constructible_v<ConstIteratorT, IteratorT>);

    /*
        Derived::begin()
        Derived::end()
    */

    const_iterator begin() const noexcept 
    { return const_cast<Derived&>(static_cast<const Derived&>(*this)).begin(); }
    
    const_iterator end() const noexcept 
    { return const_cast<Derived&>(static_cast<const Derived&>(*this)).end(); }
    
    const_iterator cbegin() const noexcept 
    { return begin(); }

    const_iterator cend() const noexcept 
    { return end(); }

    reverse_iterator rbegin() noexcept requires (Reverse)
    { return std::make_reverse_iterator(const_cast<Derived&>(static_cast<const Derived&>(*this)).end()); }

    reverse_iterator rend() noexcept requires (Reverse)
    { return std::make_reverse_iterator(const_cast<Derived&>(static_cast<const Derived&>(*this)).begin()); }

    const_reverse_iterator rbegin() const noexcept requires (Reverse) 
    { return const_cast<self_type&>(*this).rbegin(); }
    
    const_reverse_iterator rend() const noexcept requires (Reverse)
    { return const_cast<self_type&>(*this).rend(); }

    const_reverse_iterator rcbegin() const noexcept requires (Reverse) 
    { return rbegin(); }
    
    const_reverse_iterator rcend() const noexcept requires (Reverse) 
    { return rend(); }

};


