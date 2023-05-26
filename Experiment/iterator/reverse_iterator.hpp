// #pragma once

// #include <iterator>

// template <typename Iterator>
// struct reverse_iterator
// {
//     using iterator_type = Iterator;
//     // using iterator_category = 
//     using iterator_concept = decltype([]{
//         if constexpr (std::random_access_iterator<Iterator>)    
//             return std::random_access_iterator_tag();
//         else
//             return std::bidirectional_iterator_tag();
//     }());
//     using value_type = std::iter_value_t<Iterator>;
//     using difference_type = std::iter_difference_t<Iterator>;
//     using pointer = typename std::iterator_traits<Iterator>::pointer;
//     using reference = std::iter_reference_t<Iterator>;

//     constexpr reverse_iterator();

//     constexpr explicit reverse_iterator(Iterator x);

//     template <typename U> 
//     constexpr reverse_iterator(const reverse_iterator<U>& u); 

//     template <typename U> 
//     constexpr reverse_iterator& operator=(const reverse_iterator<U>& u); 

//     constexpr Iterator base() const;

//     constexpr reference operator*() const;

//     constexpr pointer operator->() const requires true;

//     constexpr reverse_iterator& operator++();
//     constexpr reverse_iterator operator++(int);
//     constexpr reverse_iterator& operator--();
//     constexpr reverse_iterator operator--(int);

//     constexpr reverse_iterator operator+(difference_type n) const;
//     constexpr reverse_iterator& operator+=(difference_type n);
//     constexpr reverse_iterator operator-(difference_type n) const;
//     constexpr reverse_iterator& operator-=(difference_type n);
//     constexpr decltype(auto) operator[](difference_type n) const;

//     friend constexpr std::iter_rvalue_reference_t<Iterator>
//         iter_move(const reverse_iterator& i) noexcept();

//     template <std::indirectly_swappable<Iterator> Iterator2>
//     friend constexpr void iter_swap(const reverse_iterator& x, const reverse_iterator<Iterator2>& y) noexcept
//     {

//     }

//     Iterator m_current;
// };


