#ifndef __BIGINT_HPP__
#define __BIGINT_HPP__

#include <vector>
#include <string_view>
#include <iostream>

#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>

namespace leviathan
{

    template <typename T>
    constexpr bool commutative_relationship = false;

    template <typename T>
    constexpr bool commutativeable = commutative_relationship<std::remove_cvref_t<T>>;

    template <typename Lhs, typename Rhs, typename = std::enable_if<(commutativeable<Lhs> && commutativeable<Rhs>)>> 
    auto operator+(Lhs&& lhs, Rhs&& rhs)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(lhs)>)
        {
            lhs += rhs;
            return std::move(lhs);
        }
        else if constexpr (std::is_rvalue_reference_v<decltype(rhs)>)
        {
            rhs += lhs;
            return std::move(rhs);
        }
        else
        {
            auto tmp = lhs;
            tmp += rhs;
            return tmp;
        }
    }

    template <typename Lhs, typename Rhs, typename = std::enable_if<(commutativeable<Lhs> && commutativeable<Rhs>)>> 
    auto operator*(Lhs&& lhs, Rhs&& rhs)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(lhs)>)
        {
            lhs *= rhs;
            return std::move(lhs);
        }
        else if constexpr (std::is_rvalue_reference_v<decltype(rhs)>)
        {
            rhs *= lhs;
            return std::move(rhs);
        }
        else
        {
            auto tmp = lhs;
            tmp *= rhs;
            return tmp;
        }
    }

    template <typename Lhs, typename Rhs, typename = std::enable_if<(commutativeable<Lhs> && commutativeable<Rhs>)>> 
    auto operator-(Lhs&& lhs, Rhs&& rhs)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(lhs)>)
        {
            lhs -= rhs;
            return std::move(lhs);
        }
        else if constexpr (std::is_rvalue_reference_v<decltype(rhs)>)
        {
            rhs -= lhs;
            rhs.negative();
            return std::move(rhs);
        }
        else
        {
            auto tmp = lhs;
            tmp -= rhs;
            return tmp;
        }
    }


    enum sign { illegal = 0, pos = 1, neg = -1 };
    
    template <typename CharType>
    struct encoding_traits;

    template <>
    struct encoding_traits<char>
    {
        using value_type = char;
        // sizeof(CharType) <= sizeof(int)
        constexpr static bool isdigit(int c) noexcept
        {
            return ::isdigit(c);
        }

        constexpr static char decode(int c) noexcept
        {
            return static_cast<char>(c + '0');
        } 

        constexpr static char encode(int c) noexcept
        {
            return static_cast<char>(c - '0');
        }

        constexpr static size_t length(const char* s) noexcept
        {
            if (!s)
                return 0;
            return ::strlen(s);
        }

    };

    template <typename EncodingTraits>
    class basic_biginteger
    {
    public:
        using underlying_type = typename EncodingTraits::value_type;

        basic_biginteger(std::basic_string_view<underlying_type> sv)
        {
            basic_biginteger(sv.data());
        }

        basic_biginteger() 
        {
            this->m_sign = sign::pos;
            this->m_item.resize(1); // default is zero
        }

        explicit basic_biginteger(const underlying_type* str)
        {
            const auto len = EncodingTraits::length(str);
            if (len > 0)
            {
                this->m_sign = sign::pos;
                init(str, len);
            }
            else
            {
                basic_biginteger();  // bigint i = "" => i = 0
            }
            normalize();
            if (this->m_sign == illegal)
                std::cout << "Error\n";
        }


        basic_biginteger(const basic_biginteger& rhs) = default;
        basic_biginteger(basic_biginteger&& rhs) noexcept = default;

        basic_biginteger& operator=(const basic_biginteger&) = default;
        basic_biginteger& operator=(basic_biginteger&&) noexcept = default;
        ~basic_biginteger() = default;

        // operator

        basic_biginteger& operator+=(const basic_biginteger& rhs)
        {
            // FIXME: ADD Only Support pos + pos
            if ((int)this->m_sign * (int)rhs.m_sign < 0)
            {
                // ...
                if (rhs.m_sign == sign::neg)
                    this->operator-=(rhs);
                else
                    *this = rhs - std::move(*this);
                return *this;
            }

            underlying_type curry = 0;
            auto min_size = std::min(this->m_item.size(), rhs.m_item.size());
            for (size_t i = 0; i < min_size; ++i)
            {
                this->m_item[i] += rhs.m_item[i] + curry;
                curry = 0;
                if (this->m_item[i] >= 10)
                {
                    curry = 1;
                    this->m_item[i] -= 10;
                }
            }
            // this.length < rhs.length
            this->m_item.reserve(rhs.m_item.size() + 1);
            this->m_item.insert(this->m_item.end(), rhs.m_item.begin() + min_size, rhs.m_item.end());
            // this.length > rhs.length
            auto max_size = std::max(this->m_item.size(), rhs.m_item.size());
            for (size_t i = 0; i < max_size; ++i)
            {
                if (curry == 0)
                    break;
                this->m_item[i] ++;
                curry = 0;
                if (this->m_item[i] >= 10)
                {
                    this->m_item[i] -= 10;
                    curry = 1;
                }
            }
            if (curry)
                this->m_item.emplace_back(1);
            return *this;
        }

        // TODO:
        basic_biginteger& operator-=(const basic_biginteger& rhs) { return *this; }
        basic_biginteger& operator/=(const basic_biginteger& rhs) { return *this; }
        basic_biginteger& operator*=(const basic_biginteger& rhs) { return *this; }
        basic_biginteger& operator%=(const basic_biginteger& rhs) { return *this; }


        // helper
        std::basic_string<underlying_type> string() const 
        {
            std::basic_string<underlying_type> res;
            res.reserve(this->m_item.size() + 1);
            auto iter = res.begin();
            if (this->m_sign == sign::neg)
                res += '-', iter ++;
            for (auto ch : this->m_item)
                res += EncodingTraits::decode(ch);
            std::reverse(iter, res.end());
            return res;
        }

        bool is_legal() const noexcept
        {
            return this->m_sign != sign::illegal;
        }

        basic_biginteger& negative() 
        {
            this->m_sign = static_cast<sign>(this->m_sign * -1);
            return *this;
        }

        bool operator<(const basic_biginteger& rhs) const noexcept
        {
            if ((int)this->m_sign * (int)rhs.m_sign < 0)
            {
                return this->m_sign == sign::neg;
            }
            // compare numeric
            bool res;
            if (this->m_item.size() > rhs.m_item.size())
                res = false;
            else if (this->m_item.size() < rhs.m_item.size())
                res = true;
            
            // compare number
            int cmp = equal_reversed_range(rhs.m_item);
            
            if (cmp == 0)
                return false;
            res = cmp == 1 ? true : false;
            
            return this->m_sign == sign::neg ? !res : res;
        }

        bool operator==(const basic_biginteger& rhs) const noexcept
        {
            return this->m_sign == rhs.m_sign && this->m_item.size() == rhs.m_item.size() 
                && std::equal(this->m_item.begin(), this->m_item.end(), rhs.m_item.begin());
        }

        bool operator!=(const basic_biginteger& rhs) const noexcept
        {
            return !this->operator==(rhs);
        }

        bool operator<=(const basic_biginteger& rhs) const noexcept
        {
            return !this->operator>(rhs);
        }

        bool operator>=(const basic_biginteger& rhs) const noexcept 
        {
            return !this->operator<(rhs);
        }

        bool operator>(const basic_biginteger& rhs) const noexcept
        {
            return rhs.operator<(*this);
        }

    private:

        int equal_reversed_range(const std::vector<underlying_type>& vec) const noexcept
        {
            for (size_t i = this->m_item.size() - 1; i != static_cast<size_t>(-1); --i)
            {
                if (this->m_item[i] < vec[i])
                    return -1;
                else if (this->m_item[i] > vec[i])
                    return 1;
            }
            return 0;
        }

        /**
         *  Remove zero, if value is 0, keep sign positive
         */
        void normalize()
        {
            const auto zero = EncodingTraits::encode('0');
            while (this->m_item.size() && this->m_item.back() == zero) 
                this->m_item.pop_back();
            if (this->m_item.empty())
            {
                this->m_item.emplace_back(0);
                this->m_sign = sign::pos;
            }
        }

        static bool check(const underlying_type* str, size_t len)
        {
            // len >= 1
            auto first = str;
            auto ch = *first;
            if (!EncodingTraits::isdigit(ch))
            {
                if (ch != '-' && ch != '+')
                {
                    return false;
                }
                ++first;    
            }
            return std::all_of(first, str + len, [](auto ch) { return EncodingTraits::isdigit(ch); });
        }

        /**
         *  Convert string to integer when the character is legal
         */
        void init(const underlying_type* str, size_t len) 
        {
            auto is_illegal = check(str, len);
            if (!is_illegal)
            {
                this->m_sign == sign::illegal;
                return;
            }
            // convert
            size_t index = 0;
            if (str[index] == '+')
            {
                this->m_sign = sign::pos;
                index ++;
            }
            else if (str[index] == '-')
            {
                this->m_sign = sign::neg;
                index ++;
            }
            for (; index < len; ++index)
                this->m_item.emplace_back(EncodingTraits::encode(str[index]));
            std::reverse(this->m_item.begin(), this->m_item.end());
        }
public:
        sign m_sign;
        std::vector<underlying_type> m_item;
    };  
    using bigint = basic_biginteger<encoding_traits<char>>;
    template <>
    constexpr bool commutative_relationship<bigint> = true;

} // namespace leviathan



#endif

