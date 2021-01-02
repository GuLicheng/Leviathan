/*
        for matrix mul matrix
        test matrix : n * n, element type: int, mode:debug
        traditional:
                mul: 200,341ms(n = 1000)
                         200,656ms(n = 1000)
                         1725,090ms(n = 2000)

        transpose before mul:
                transpose: 149.34ms
                                   144.675ms
                                   640.538ms(n = 2000)

                mul: 188,527ms
                         184,984ms
                         1489,990ms(n = 2000)
*/

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <ranges>
#include <iterator>
#include <type_traits>
#include <concepts>

#include "type_list.hpp"

namespace leviathan::detail {

template <typename T, typename = void>
struct is_container : std::is_array<T> {};

template <typename T, typename = void>
struct is_iterator : std::false_type {};

template <typename T>
struct is_iterator<T, std::enable_if_t<!std::is_same_v<
                          void, typename std::iterator_traits<T>::value_type>>>
    : std::true_type {};

template <typename T>
struct is_container<
    T, std::enable_if_t<!std::is_same_v<void, typename T::iterator>>>
    : std::true_type {};
// something to be implented

// another implement to is_iterator_v
// this struct is unnecessary
struct _NoTypeDumleviathan {};
// we don't need to implement it
template <typename T>
constexpr std::enable_if_t<true, typename std::iterator_traits<T>::value_type>
is_iterator_fn(T &&);
// if template deduced failed this function will returns _NoTypeDumleviathan
_NoTypeDumleviathan is_iterator_fn(...);

template <typename T>
constexpr bool is_iterator_v =
    !std::is_same_v<_NoTypeDumleviathan, decltype(is_iterator_fn(std::declval<T>()))>;
/*                         ---test----
constexpr auto test1 = is_iterator_v<std::vector<int>::iterator>;  true
constexpr auto test2 = is_iterator_v<int>;  false
constexpr auto test3 = is_iterator_v<double*>; true, we assume pointer is
random_access_iterator
*/

template <typename T, typename U>
struct common_type
    : std::enable_if<true,
                     decltype(true ? std::declval<T>() : std::declval<U>())> {};

}  // namespace leviathan::detail

namespace leviathan {

template <typename T>
constexpr bool is_container_v = ::leviathan::detail::is_container<T>::value;

// we can find std::_Is_iterator_v in <xutility>
// it use void_t<T> {type = void}
template <typename T>
constexpr bool is_iterator_v = ::leviathan::detail::is_iterator<T>::value;

}  // namespace leviathan



namespace leviathan {


namespace detail
{
    
}

template <typename T> 
class matrix;

namespace detail {
template <typename T>
struct is_matrix : std::false_type {};

template <typename T>
struct is_matrix<matrix<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_matrix_v = detail::is_matrix<std::decay_t<T>>::value;

}  // namespace detail

// the type T of matrix usually be integer or double, hardly complex or fraction
#define DEBUG_TEST 0
template <typename T = double>
class matrix {
#if DEBUG_TEST
    inline static int COPY = 0;
    inline static int MOVE = 0;
    static void showCopy() {
        ++COPY;
        std::cout << "Copy :" << COPY << std::endl;
    }
    static void showMove() {
        ++MOVE;
        std::cout << "Move :" << MOVE << std::endl;
    }

#endif  // DEBUG_TEST

#if DEBUG_TEST

#define ShowCopy() showCopy()
#define ShowMove() showMove()

#else

#define ShowCopy()
#define ShowMove()

#endif

   public:
    using value_type = T;
    using array_t = std::vector<T>;

   public:
    matrix() {
        this->_col = this->_row = 1;
        this->_arr.emplace_back(value_type{});
    }

    matrix(size_t row, size_t col) : _row{row}, _col{col} {
        _arr.resize(_row * _col);
#if DEBUG_TEST
        std::cout << "matrix(size_t, size_t) called\n";
#endif  // DEBUG_TEST
    }

    template <typename Container, typename decay_type = std::decay_t<Container>,
              typename = std::enable_if_t<is_container_v<decay_type>>>
    explicit matrix(Container&& c) {
        this->_arr.resize(std::size(c));
        if constexpr (std::is_rvalue_reference_v<decltype(c)>) {
            ShowMove();
            std::move(std::begin(c), std::end(c), this->_arr.begin());
        } else {
            ShowCopy();
            std::copy(std::begin(c), std::end(c), this->_arr.begin());
        }
        this->_row = 1;
        this->_col = this->_arr.size();
    }

    template <typename Iter, typename = std::enable_if_t<is_iterator_v<Iter>>>
    matrix(Iter _First, Iter _Last) {
        size_t cnt = 0;
        for (; _First != _Last; ++_First, ++cnt) {
            this->_arr.emplace_back(*_First);
        }
        this->_row = 1;
        this->_col = this->_arr.size();
    }

    matrix(const matrix& rhs) : _row{rhs._row}, _col{rhs._col}, _arr{rhs._arr} {
        ShowCopy();
    }

    matrix(matrix&& rhs) noexcept
        : _row{rhs._row}, _col{rhs._col}, _arr{std::move(rhs._arr)} {
        ShowMove();
    }

    matrix& operator=(const matrix& rhs) {
        ShowCopy();
        if (this != std::addressof(rhs)) {
            this->_arr = rhs._arr;
            this->_col = rhs._col;
            this->_row = rhs._row;
        }
        return *this;
    }

    matrix& operator=(matrix&& rhs) noexcept {
        ShowMove();
        if (this != std::addressof(rhs)) {
            this->_arr.swap(rhs._arr);
            std::swap(_col, rhs._col);
            std::swap(_row, rhs._row);
        }
        return *this;
    }

    ~matrix() = default;

    const value_type& operator()(size_t x, size_t y) const noexcept {
        return _arr[get_address(x, y)];
    }

    value_type& operator()(size_t x, size_t y) noexcept {
        return _arr[get_address(x, y)];
    }

    inline size_t row() const noexcept { return _row; }
    inline size_t col() const noexcept { return _col; }
    inline std::pair<size_t, size_t> dimension() const noexcept {
        return {_row, _col};
    }
    
    inline size_t size() const noexcept 
    { return this->_arr.size(); }
    
    constexpr size_t rank() const noexcept 
    { return 2; };
    
    const std::vector<value_type>& date() const noexcept 
    { return this->_arr; }

    matrix transpose() const noexcept 
    {
        matrix m{_col, _row};
        for (size_t i = 0; i < _row; ++i)
            for (size_t j = 0; j < _col; ++j) 
                m(j, i) = this->operator()(i, j);
        return m;
    }

    friend std::ostream& operator<<(std::ostream& os, const matrix& m) {
        os << '[' << std::endl;
        auto& arr = m.date();
        auto col = m.col();
        for (int i = 0; i < arr.size(); ++i) {
            if (i % col != 0) std::cout << ", ";
            std::cout << arr[i];
            if (i % col == col - 1) std::cout << std::endl;
        }
        os << ']';
        return os;
    }

    
    void swap(matrix& rhs) noexcept 
    {
        this->_arr.swap(rhs._arr);
        std::swap(this->_col, rhs._col);
        std::swap(this->_row, rhs._row);
    }

    template <typename RType>
    matrix& operator+=(RType&& rhs) {
        // RType can only be T or T&(such as int, int&)
        // is_matrix will decay RType
        if constexpr (is_matrix_v<RType>) {
            // assert row == rhs.row and col == rhs.col
            size_t len = this->size();
            for (size_t i = 0; i < len; ++i)
                this->_arr[i] += static_cast<value_type>(rhs._arr[i]);
        } else {
            // rhs is a scalar
            for (auto& e : this->_arr) 
                e += static_cast<value_type>(rhs);
        }
        return *this;
    }

    template <typename RType>
    matrix& operator-=(RType&& rhs) {
        // RType can only be T or T&(such as int, int&)
        // is_matrix will decay RType
        if constexpr (is_matrix_v<RType>) {
            // assert row == rhs.row and col == rhs.col
            size_t len = this->size();
            for (size_t i = 0; i < len; ++i)
                this->_arr[i] -= static_cast<value_type>(rhs._arr[i]);
        } else {
            // rhs is a scalar
            for (auto& e : this->_arr) e -= static_cast<value_type>(rhs);
        }
        return *this;
    }

    template <typename RType>
    matrix& operator/=(RType&& rhs) {
        // matrix cannot be divisor
        for (auto& e : this->_arr) e /= static_cast<value_type>(rhs);
        return *this;
    }

    template <typename RType>
    matrix& operator*=(RType&& rhs) {
        if constexpr (is_matrix_v<RType>) {
            // assert col == rhs.row
            matrix m{_row, rhs._col};
            auto t = rhs.transpose();

            for (size_t i = 0; i < _row; ++i)
                for (size_t j = 0; j < rhs._col; ++j) {
                    m(i, j) = value_type{};
                    for (size_t k = 0; k < _col; ++k)
                        m(i, j) += this->operator()(i, k) * t(j, k);
                }
            *this = std::move(m);
        } else {
            for (auto& e : this->_arr) e *= static_cast<value_type>(rhs);
        }
        return *this;
    }

   private:
    size_t get_address(size_t x, size_t y) const noexcept {
        return x * _col + y;
    }

    template <typename LType, typename RType>
    struct matrix_opr_helper;

    // matrix op matrix
    template <typename T1, typename T2>
    struct matrix_opr_helper<matrix<T1>, matrix<T2>> {
        template <typename LType, typename RType>
        static decltype(auto) add(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) {
                // if left is &&
                left += right;
                // when first call std::forward it will call move constrcution
                return std::forward<LType>(left);
            } else if constexpr (std::is_rvalue_reference_v<decltype(right)>) {
                // if right is &&
                right += left;
                return std::forward<RType>(right);
            } else {
                // if both of left and right is &
                auto m = left;
                m += right;
                return m;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) sub(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) {
                // if left is &&
                left -= right;
                return std::forward<LType>(left);
            } else if constexpr (std::is_rvalue_reference_v<decltype(right)>) {
                // if right is &&
                right -= left;
                return std::forward<RType>(right);
            } else {
                // if both of left and right is &
                auto m = left;
                m -= right;
                return m;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) mul(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) {
                // if left is &&
                // COUT_TEST(1);
                left *= right;
                return std::forward<LType>(left);
            } else if constexpr (std::is_rvalue_reference_v<decltype(right)>) {
                // if right is &&
                // COUT_TEST(2);
                right *= left;
                return std::forward<RType>(right);
            } else {
                // COUT_TEST(3);
                // if both of left and right is &
                auto m = left;
                m *= right;
                return m;
            }
        }
    };

    // matrix op scalar
    template <typename T1, typename ScalarType>
    struct matrix_opr_helper<matrix<T1>, ScalarType> {
        template <typename LType, typename RType>
        static decltype(auto) add(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) {
                left += right;
                return std::forward<LType>(left);
            } else {
                auto r = left;
                r += right;
                return r;
            }
        }
        template <typename LType, typename RType>
        static decltype(auto) sub(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) {
                left -= right;
                return std::forward<LType>(left);
            } else {
                auto r = left;
                r -= right;
                return r;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) mul(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) {
                left *= right;
                return std::forward<LType>(left);
            } else {
                auto r = left;
                r *= right;
                return r;
            }
        }
        template <typename LType, typename RType>
        static decltype(auto) div(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) {
                left /= right;
                return std::forward<LType>(left);
            } else {
                auto r = left;
                r /= right;
                return r;
            }
        }
    };

    // scalar op matrix
    template <typename ScalarType, typename T1>
    struct matrix_opr_helper<ScalarType, matrix<T1>> {
        template <typename LType, typename RType>
        static decltype(auto) add(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(right)>) {
                right += left;
                return std::forward<RType>(right);
            } else {
                auto r = right;
                r += left;
                return r;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) mul(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(right)>) {
                right *= left;
                return std::forward<RType>(right);
            } else {
                auto r = right;
                r *= left;
                return r;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) sub(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(right)>) {
                for (auto& e : right._arr) e = left - e;
                return std::forward<RType>(right);
            } else {
                auto r = right;
                for (auto& e : r._arr) e = left - e;
                return r;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) div(LType&& left, RType&& right) {
            if constexpr (std::is_rvalue_reference_v<decltype(right)>) {
                for (auto& e : right._arr) e = left / e;
                return std::forward<RType>(right);
            } else {
                auto r = right;
                for (auto& e : r._arr) e = left / e;
                return r;
            }
        }
    };

   public:
    // binary operation for +, -, *, /
    template <typename LType, typename RType>
    friend decltype(auto) operator+(LType&& left, RType&& right);

    template <typename LType, typename RType>
    friend decltype(auto) operator-(LType&& left, RType&& right);

    template <typename LType, typename RType>
    friend decltype(auto) operator*(LType&& left, RType&& right);

    template <typename LType, typename RType>
    friend decltype(auto) operator/(LType&& left, RType&& right);

   public:
    // help function
    template <typename U>
    friend matrix<U> make_unit_matrix(size_t row);

    template <typename U, typename... Args>
    friend matrix<U> make_matrix(size_t row, size_t col, Args&&... args);

   private:
    std::vector<value_type> _arr;
    size_t _row;
    size_t _col;
};

template <typename LType, typename RType>
decltype(auto) operator+(LType&& left, RType&& right) {
    using left_t = std::decay_t<LType>;
    using right_t = std::decay_t<RType>;
    if constexpr (is_matrix_v<LType>) {
        // if left is matrix
        return left_t::template matrix_opr_helper<
            left_t, right_t>::template add(std::forward<LType>(left),
                                           std::forward<RType>(right));
    } else {
        // if right is matrix
        return right_t::template matrix_opr_helper<
            left_t, right_t>::template add(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
}

template <typename LType, typename RType>
decltype(auto) operator-(LType&& left, RType&& right) {
    using left_t = std::decay_t<LType>;
    using right_t = std::decay_t<RType>;
    if constexpr (is_matrix_v<LType>) {
        // if left is matrix
        return left_t::template matrix_opr_helper<
            left_t, right_t>::template sub(std::forward<LType>(left),
                                           std::forward<RType>(right));
    } else {
        // if right is matrix
        return right_t::template matrix_opr_helper<
            left_t, right_t>::template sub(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
}

template <typename LType, typename RType>
decltype(auto) operator*(LType&& left, RType&& right) {
    using left_t = std::decay_t<LType>;
    using right_t = std::decay_t<RType>;
    if constexpr (is_matrix_v<LType>) {
        // if left is matrix

        return left_t::template matrix_opr_helper<
            left_t, right_t>::template mul(std::forward<LType>(left),
                                           std::forward<RType>(right));
    } else {
        // if right is matrix
        return right_t::template matrix_opr_helper<
            left_t, right_t>::template mul(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
}

template <typename LType, typename RType>
decltype(auto) operator/(LType&& left, RType&& right) {
    using left_t = std::decay_t<LType>;
    using right_t = std::decay_t<RType>;
    if constexpr (is_matrix_v<LType>) {
        // if left is matrix
        return left_t::template matrix_opr_helper<
            left_t, right_t>::template div(std::forward<LType>(left),
                                           std::forward<RType>(right));
    } else {
        // if right is matrix
        return right_t::template matrix_opr_helper<
            left_t, right_t>::template div(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
}

template <typename U>
matrix<U> make_unit_matrix(size_t row) {
    matrix<U> m{row, row};
    //  for each(x, x) in matrix, address = x * col + x = x * (col + 1)
    auto k = row + 1;
    auto len = m.size();
    for (size_t i = 0;; ++i) {
        size_t index = i * k;
        if (index >= len) break;
        m._arr[index] = static_cast<U>(1);
    }
    return m;
}

template <typename U, typename... Args>
matrix<U> make_matrix(size_t row, size_t col, Args&&... args) {
    // assert row * col == sizeof...(args)
    matrix<U> m{};
    m._col = col;
    m._row = row;
    m._arr.clear();
    m._arr.reserve(row * col);
    (m._arr.emplace_back(args), ...);
    return m;
}

}  // namespace detail
// matrix-chain-multiplication to be implemented

#endif  // !MATRIX_HPP