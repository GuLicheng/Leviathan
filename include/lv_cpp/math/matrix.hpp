#ifndef _MATRIX_HPP_
#define _MATRIX_HPP_

#include <iostream>
#include <algorithm>
#include <concepts>
#include <ranges>
#include <iterator>
#include <vector>
#include <lv_cpp/type_list.hpp>

namespace leviathan::numeric
{
template <typename T>
concept arithmetic = std::floating_point<T> || std::integral<T>;
// for complex to be continued

// 2D-matrix
template <arithmetic T>
class matrix; 

namespace detail 
{

template <typename T>
concept matrix = ::leviathan::meta::is_instance<leviathan::numeric::matrix, std::remove_cvref_t<T>>::value;

template <typename T>
concept scalar = arithmetic<std::remove_cvref_t<T>>;

template <typename T>
concept scalar_matrix = matrix<T> || scalar<T>;

}  // namespace detail

template <arithmetic T>
class matrix
{
#ifdef _DEBUG

    inline static int COPY = 0;
    inline static int MOVE = 0;
    static void showCopy() 
    {
        ++COPY;
        std::cout << "Copy :" << COPY << std::endl;
    }
    static void showMove() 
    {
        ++MOVE;
        std::cout << "Move :" << MOVE << std::endl;
    }

#endif // _DEBUG

#ifdef _DEBUG

#define ShowCopy() showCopy()
#define ShowMove() showMove()

#else

#define ShowCopy()
#define ShowMove()

#endif // _DEBUG

public:
    using value_type = T;
    using container_type = std::vector<value_type>;
    using self = matrix<value_type>;
    
    static_assert(std::is_same_v<container_type, std::remove_cvref_t<container_type>>);

    explicit matrix(size_t row = 1, size_t col = 1)
        : m_row{row}, m_col{col}
    {
        std::fill_n(std::back_inserter(this->m_arr), row * col, value_type{ });
    }

    matrix(const matrix& rhs) 
    noexcept(std::is_nothrow_copy_constructible_v<container_type>) 
        : m_row{rhs.m_row}, m_col{rhs.m_col}, m_arr{rhs.m_arr} 
    {  
        // std::cout << "Copy: ";
        // ShowCopy();
        // std::cout << std::is_nothrow_copy_constructible_v<container_type> << std::endl;
    }

    matrix(matrix&& rhs) 
    noexcept(std::is_nothrow_move_assignable_v<container_type> && noexcept(std::move(rhs.m_arr)))  
        : m_row{rhs.m_row}, m_col{rhs.m_col}, m_arr{std::move(rhs.m_arr)}
    {
        // ShowMove();
        // std::cout << "Move: ";
        // std::cout << (std::is_nothrow_move_assignable_v<container_type> && noexcept(std::move(rhs.m_arr))) << std::endl;
    }

    template <std::ranges::range Rng1, std::ranges::range... Rngs>
    matrix(Rng1&& rng1, Rngs&&... rngs)
    {
        static_assert(std::is_constructible_v<value_type, std::ranges::range_value_t<Rng1>>);
        static_assert((std::is_constructible_v<value_type, std::ranges::range_value_t<Rngs>> 
                        && ...));

        this->m_row = sizeof...(Rngs) + 1;
        this->m_col = std::ranges::size(rng1);
        if constexpr (std::is_rvalue_reference_v<decltype(rng1)>)
            std::ranges::move(rng1, std::back_inserter(this->m_arr));
        else 
            std::ranges::copy(rng1, std::back_inserter(this->m_arr));
        
        ((std::is_rvalue_reference_v<decltype(rngs)> ? 
                    std::ranges::move(rngs, std::back_inserter(this->m_arr)) : 
                    std::ranges::copy(rngs, std::back_inserter(this->m_arr))), ...);

        // sizeof each range must be equal
        // if (!(this->check_size(std::ranges::size(rng1), std::ranges::size(rngs)) && ...))
        // {
        //     std::cout << "error";
        // }
        // else 
        // {
        //     std::cout << "true";
        // } 
        // user should keep this
    }

    matrix(const std::initializer_list<std::initializer_list<value_type>>& ls)
    {
       this->m_row = ls.size();
       this->m_col = ls.begin()->size();
       for (const auto& l : ls)
           std::ranges::copy(l, std::back_inserter(this->m_arr));
       // the size of each list must equal
    }

    matrix& operator=(const matrix& rhs) 
    noexcept(std::is_nothrow_copy_constructible_v<container_type>) = default;

    matrix& operator=(matrix&& rhs) 
    noexcept(std::is_nothrow_move_assignable_v<container_type> 
                && noexcept(std::move(rhs.m_arr))) = default;
    
    template <std::ranges::range Rng>
    matrix& operator=(Rng&& rng)
    {
        using range_value_type = std::ranges::range_value_t<Rng>;
        static_assert(std::is_constructible_v<value_type, range_value_type>);
        this->m_col = std::ranges::size(rng);
        this->m_row = 1;
        this->m_arr.clear();
        if constexpr (std::is_rvalue_reference_v<decltype(rng)>) 
            std::ranges::move(rng, std::back_inserter(this->m_arr));
        else 
            std::ranges::copy(rng, std::back_inserter(this->m_arr));
        // for arithmic, it's the same 
        return *this; 
    }

    ~matrix() = default;

    auto row() const noexcept 
    { return this->m_row; }

    auto column() const noexcept
    { return this->m_col; }

    auto total() const noexcept 
    { return this->m_row * this->m_col; }
 
    std::pair<size_t, size_t> dimension() const noexcept
    { return {this->m_row, this->m_col}; }

    constexpr size_t rank() const noexcept
    { return 2; }

    friend std::ostream& operator<<(std::ostream& os, const matrix& m)
    {
        os << '[' << '\n';
        size_t idx = 0;
        for (auto&& e : m.m_arr)
        {
            std::cout.width(std::numeric_limits<value_type>::max_digits10 / 2);
            std::cout.fill(' ');
            std::cout << e << ',';
            idx ++;
            if (idx % m.column() == 0)
                std::cout << '\n';
        }
        return os << ']';
    }

    const value_type& operator()(size_t x, size_t y) 
    const noexcept(noexcept(std::declval<const container_type>().operator[](size_t{})))
    {
        const auto index = this->get_index(x, y);
        return this->m_arr[index];
    }

    value_type& operator()(size_t x, size_t y) 
    noexcept(noexcept(std::declval<container_type>().operator[](size_t{})))
    {
        const auto index = this->get_index(x, y);
        return this->m_arr[index];
    }

    matrix transpose() const 
    {
        matrix m{this->m_col, this->m_row};
        for (size_t i = 0; i < this->m_row; ++i)
            for (size_t j = 0; j < this->m_col; ++j)
            {
                m(j, i) = this->operator()(i, j);
            }
        return m;
    }

    template <detail::scalar_matrix RType>
    self& operator+=(RType&& rhs)
    {
        using _Type = std::remove_cvref_t<RType>;
        if constexpr (detail::scalar<_Type>)
        {
            for (auto& e : this->m_arr) 
                e += static_cast<value_type>(rhs);
        } 
        else
        {
            for (size_t i = 0; i < this->m_arr.size(); ++i)
            {
                this->m_arr[i] += static_cast<value_type>(rhs.m_arr[i]);
            }
        }
        return *this;
    }

    template <detail::scalar_matrix RType>
    self& operator-=(RType&& rhs)
    {
        using _Type = std::remove_cvref_t<RType>;
        if constexpr (detail::scalar<_Type>)
        {
            for (auto& e : this->m_arr) 
                e -= static_cast<value_type>(rhs);
        } 
        else
        {
            for (size_t i = 0; i < this->m_arr.size(); ++i)
            {
                this->m_arr[i] -= static_cast<value_type>(rhs.m_arr[i]);
            }
        }
        return *this;
    }

    template <detail::scalar RType>
    self& operator/=(RType&& rhs)
    {
        for (auto& e : this->m_arr)
            e /= rhs;
        return *this;
    }
    
    template <detail::scalar_matrix RType>
    self& operator*=(RType&& rhs)
    {
        using _Type = std::remove_cvref_t<RType>;
        if constexpr (detail::scalar<_Type>)
        {
            for (auto& e : this->m_arr) 
               e *= static_cast<value_type>(rhs);
        }
        else
        {
            matrix m{this->m_row, rhs.m_col};
            auto t = rhs.transpose();
            for (size_t i = 0; i < this->m_row; ++i)
                for (size_t j = 0; j < rhs.m_col; ++j) {
                    m(i, j) = value_type{};
                    for (size_t k = 0; k < this->m_col; ++k)
                        m(i, j) += this->operator()(i, k) * t(j, k);
                }
            *this = std::move(m);
        }
        return *this;
    }

    // binary operation for +, -, *, /
    template <detail::scalar_matrix LType, detail::scalar_matrix RType>
    friend decltype(auto) operator+(LType&& left, RType&& right);

    template <detail::scalar_matrix LType, detail::scalar_matrix RType>
    friend decltype(auto) operator-(LType&& left, RType&& right);

    template <detail::scalar_matrix LType, detail::scalar_matrix RType>
    friend decltype(auto) operator*(LType&& left, RType&& right);

    template <detail::scalar_matrix LType, detail::scalar_matrix RType>
    friend decltype(auto) operator/(LType&& left, RType&& right);

private:

    // for checking size
    // template <typename U> requires std::is_integral_v<U>
    // bool check_size(U a, U b) const noexcept 
    // { return a == b; }
    
    size_t get_index(size_t x, size_t y) const noexcept
    { return x * this->m_col + y; }
   
    container_type m_arr;
    size_t m_row;
    size_t m_col;

    template <typename LType, typename RType>
    struct matrix_opt_impl;

    // matrix op matrix
    template <arithmetic T1, arithmetic T2>
    struct matrix_opt_impl<matrix<T1>, matrix<T2>> 
    {
        template <typename LType, typename RType>
        static decltype(auto) add(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) 
            {
                left += right;
                return std::forward<LType>(left);
            } 
            else if constexpr (std::is_rvalue_reference_v<decltype(right)>) 
            {
                right += left;
                return std::forward<RType>(right);
            } 
            else 
            {
                auto m = left;
                m += right;
                return m;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) sub(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) 
            {
                left -= right;
                return std::forward<LType>(left);
            } 
            else if constexpr (std::is_rvalue_reference_v<decltype(right)>) 
            {
                right -= left;
                return std::forward<RType>(right);
            } 
            else 
            {
                auto m = left;
                m -= right;
                return m;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) mul(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) 
            {
                left *= right;
                return std::forward<LType>(left);
            } 
            else if constexpr (std::is_rvalue_reference_v<decltype(right)>) 
            {
                right *= left;
                return std::forward<RType>(right);
            } 
            else 
            {
                auto m = left;
                m *= right;
                return m;
            }
        }
    };

    // matrix op scalar
    template <arithmetic T1, typename ScalarType>
    struct matrix_opt_impl<matrix<T1>, ScalarType> 
    {
		template <typename LType, typename RType>
		static decltype(auto) add(LType &&left, RType &&right) 
        {
			if constexpr (std::is_rvalue_reference_v<decltype(left)>) 
            {
				left += right;
				return std::forward<LType>(left);
			} 
            else 
            {
				auto r = left;
				r += right;
				return r;
			}
		}
		template <typename LType, typename RType>
        static decltype(auto) sub(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) 
            {
                left -= right;
                return std::forward<LType>(left);
            } 
            else 
            {
                auto r = left;
                r -= right;
                return r;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) mul(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) 
            {
                left *= right;
                return std::forward<LType>(left);
            } 
            else 
            {
                auto r = left;
                r *= right;
                return r;
            }
        }
        
        template <typename LType, typename RType>
        static decltype(auto) div(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(left)>) 
            {
                left /= right;
                return std::forward<LType>(left);
            } 
            else 
            {
                auto r = left;
                r /= right;
                return r;
            }
        }
    };

    // scalar op matrix
    template <detail::scalar ScalarType, arithmetic T1>
    struct matrix_opt_impl<ScalarType, matrix<T1>> 
    {
        template <typename LType, typename RType>
        static decltype(auto) add(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(right)>) 
            {
                right += left;
                return std::forward<RType>(right);
            } 
            else 
            {
                auto r = right;
                r += left;
                return r;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) mul(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(right)>) 
            {
                right *= left;
                return std::forward<RType>(right);
            } 
            else 
            {
                auto r = right;
                r *= left;
                return r;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) sub(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(right)>) 
            {
                for (auto& e : right.m_arr) e = left - e;
                return std::forward<RType>(right);
            } 
            else 
            {
                auto r = right;
                for (auto& e : r.m_arr) e = left - e;
                return r;
            }
        }

        template <typename LType, typename RType>
        static decltype(auto) div(LType&& left, RType&& right) 
        {
            if constexpr (std::is_rvalue_reference_v<decltype(right)>) 
            {
                for (auto& e : right.m_arr) e = left / e;
                return std::forward<RType>(right);
            } 
            else 
            {
                auto r = right;
                for (auto& e : r.m_arr) e = left / e;
                return r;
            }
        }
    };


};




template <detail::scalar_matrix LType, detail::scalar_matrix RType>
decltype(auto) operator+(LType&& left, RType&& right) 
{
    using left_t = std::decay_t<LType>;
    using right_t = std::decay_t<RType>;
    if constexpr (detail::matrix<LType>) 
    {
        // if left is matrix
        return left_t::template matrix_opt_impl<
            left_t, right_t>::template add(std::forward<LType>(left),
                                           std::forward<RType>(right));
    } 
    else 
    {
        // if right is matrix
        return right_t::template matrix_opt_impl<
            left_t, right_t>::template add(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
}

template <detail::scalar_matrix LType, detail::scalar_matrix RType>
decltype(auto) operator-(LType&& left, RType&& right) 
{
    using left_t = std::decay_t<LType>;
    using right_t = std::decay_t<RType>;
    if constexpr (detail::matrix<LType>) 
    {
        return left_t::template matrix_opt_impl<
            left_t, right_t>::template sub(std::forward<LType>(left),
                                           std::forward<RType>(right));
    } 
    else 
    {
        return right_t::template matrix_opt_impl<
            left_t, right_t>::template sub(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
}

template <detail::scalar_matrix LType, detail::scalar_matrix RType>
decltype(auto) operator*(LType&& left, RType&& right) 
{
    using left_t = std::remove_cvref_t<LType>;
    using right_t = std::remove_cvref_t<RType>;
    if constexpr (detail::matrix<LType>) 
    {
        return left_t::template matrix_opt_impl<
            left_t, right_t>::template mul(std::forward<LType>(left),
                                           std::forward<RType>(right));
    } 
    else 
    {
        return right_t::template matrix_opt_impl<
            left_t, right_t>::template mul(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
}

template <detail::scalar_matrix LType, detail::scalar_matrix RType>
decltype(auto) operator/(LType&& left, RType&& right) 
{
    using left_t = std::remove_cvref_t<LType>;
    using right_t = std::remove_cvref_t<RType>;
    if constexpr (detail::matrix<LType>) 
    {
        return left_t::template matrix_opt_impl<
            left_t, right_t>::template div(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
    else 
    {
        return right_t::template matrix_opt_impl<
            left_t, right_t>::template div(std::forward<LType>(left),
                                           std::forward<RType>(right));
    }
}

} // end of namespace leviathan::numeric

/*
void test3()
{
    leviathan::matrix<double> m = 
    {
        {1, 2, 3},
        {4, 5, 6}
    };

    leviathan::matrix<double> m0 = 
    {
        {1, 2},
        {3, 4}
    };
    std::cout << m0 * m0 << std::endl;
    auto m1 = 1 + (m0 * m0) + 2;
    std::cout << m1 << std::endl;
    std::cout << (m1 - 1) << std::endl;
    std::cout << (1 - m1) << std::endl;
    std::cout << m1 * 2 << std::endl;
    std::cout << 2 * m1 << std::endl;
    std::cout << 2 / m1 << std::endl;
    std::cout << m1 / 2 << std::endl;
}

void test1()
{
    using T1 = leviathan::matrix<double>;
    using T2 = leviathan::matrix<float>;
    // using T3 = leviathan::matrix<4, 6, std::string>;
    constexpr bool a1 = leviathan::detail::matrix<T1>;
    constexpr bool a2 = leviathan::detail::matrix<const T2>;
    constexpr bool a3 = leviathan::detail::matrix<int>;
    static_assert(a1 == true);
    static_assert(a2 == true);
    static_assert(a3 == false);
    using T0 = leviathan::type::max_type<std::tuple<int, char, double>>::type;
}

void test2()
{
    try
    {
        std::cout << "===========================\n";
        // constexpr auto a = std::numeric_limits<double>::max_digits10;
        std::vector arr{1, 2, 3}, buf{4, 5, 6};
        leviathan::matrix<double> m1{arr, buf};
        leviathan::matrix<double> m2
        {
            {1, 2, 3},
            {3, 4, 5}
        };
        std::cout << m1 << std::endl;
        std::cout << m2 << std::endl;
        auto m3 = m1;
        std::cout << m3 << std::endl;
        m3 = buf;
        std::cout << m3 << std::endl;
        std::cout << m3.transpose() << std::endl;
        m2 -= m2;
        std::cout << m2 << std::endl;
        m2 += 1;
        m2 *= 2;
        std::cout << m2 << std::endl;


        leviathan::matrix<double> m0 =
        {
            {1, 2},
            {3, 4}
        };

        m0 *= m0;
        std::cout << m0 << std::endl;

        std::cout << m0 + m0 << std::endl;

    } 
    catch (std::string x)
    {
        std::cout << x;
    }
}


*/



#endif