// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0323r11.html

#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <exception>
#include <stdexcept>
#include <memory>

namespace std
{
    // �.�.3, class template unexpected
    template <class E>
    class unexpected;

    // �.�.4, class bad_expected_access
    template <class E>
    class bad_expected_access;

    // �.�.5, Specialization for void
    template <>
    class bad_expected_access<void>;

    // in-place construction of unexpected values
    struct unexpect_t
    {
        explicit unexpect_t() = default;
    };
    inline constexpr unexpect_t unexpect{};

    // �.�.7, class template expected
    template <class T, class E>
    class expected;

    // �.�.8, class template expected<cv void, E>
    template <class T, class E>
    requires is_void_v<T>
    class expected<T, E>;

    template <class E>
    class unexpected
    {
    public:
        constexpr unexpected(const unexpected &) = default;
        constexpr unexpected(unexpected &&) = default;

        template <class... Args>
        requires(is_constructible_v<E, Args...>) constexpr explicit unexpected(in_place_t, Args &&...args)
            : val{(Args &&) args...}
        {
        }

        // template <class U, class... Args>
        // requires(is_constructible_v<E, initializer_list<U> &, Args...>) constexpr explicit unexpected(in_place_t, initializer_list<U>, Args &&...);

        template <class Err = E>
        requires(!is_same_v<remove_cvref_t<Err>, unexpected> && 
            !is_same_v<remove_cvref_t<Err>, in_place_t> && 
            is_constructible_v<E, Err>) 
        constexpr explicit unexpected(Err &&err)
            : val{(Err &&) err}
        {
        }

        constexpr unexpected &operator=(const unexpected &) = default;
        constexpr unexpected &operator=(unexpected &&) = default;

        constexpr const E &value() const &noexcept { return val; }
        constexpr E &value() &noexcept { return val; }
        constexpr const E &&value() const &&noexcept { return std::move(val); }
        constexpr E &&value() &&noexcept { return std::move(val); }

        constexpr void
        swap(unexpected &other) noexcept(is_nothrow_swappable_v<E>) requires(is_swappable_v<E>)
        {
            using std::swap;
            swap(val, other.val);
        }

        template <class E2>
        friend constexpr bool operator==(const unexpected &x, const unexpected<E2> &y)
        {
            return x.val == y.val;
        }

    private:
        E val; // exposition only
    };

    template <class E>
    unexpected(E) -> unexpected<E>;

    template <class E>
    unexpected<typename std::decay<E>::type> make_unexpected(E &&e)
    {
        return unexpected<typename std::decay<E>::type>((E &&)e);
    }


    template <typename E>
    constexpr void swap(unexpected<E> &x, unexpected<E> &y) noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

    template <>
    class bad_expected_access<void> : public exception
    {
    protected:
        bad_expected_access() noexcept = default;

        bad_expected_access(const bad_expected_access &) = default;
        bad_expected_access(bad_expected_access &&) = default;
        bad_expected_access &operator=(const bad_expected_access &) = default;
        bad_expected_access &operator=(bad_expected_access &&) = default;

        virtual ~bad_expected_access() noexcept = default;

    public:
        const char *what() const noexcept override { return "bad_expected_access"; }
    };

    template <class E>
    class bad_expected_access : public bad_expected_access<void>
    {
    public:
        explicit bad_expected_access(E e) : val(std::move(e)) {}
        const char *what() const noexcept override { return "bad_expected_access"; }
        const E &value() const &noexcept { return val; }
        E &value() &noexcept { return val; }
        const E &&value() const &&noexcept { return std::move(val); }
        E &&value() &&noexcept { return std::move(val); }

        ~bad_expected_access() noexcept = default;

    private:
        E val; // exposition only
    };


    namespace detail
    {

        template <typename T>
        struct is_unexpected : false_type
        {
        };

        template <typename E>
        struct is_unexpected<unexpected<E>> : true_type
        {
        };

        template <class T, class E>
        struct expected_traits
        {
            constexpr static bool copyable = is_copy_constructible_v<T> && is_copy_constructible_v<E>;
            constexpr static bool trivial_copyable = is_trivially_copy_constructible_v<T> && is_trivially_copy_constructible_v<E>;

            constexpr static bool moveable = is_move_constructible_v<T> && is_move_constructible_v<E>;
            constexpr static bool trivial_moveable = is_trivially_move_constructible_v<T> && is_trivially_move_constructible_v<E>;

            constexpr static bool nothrow_move_ctor = is_nothrow_move_constructible_v<T> && is_nothrow_move_constructible_v<E>;

            constexpr static bool trivial_dctor =
                is_trivially_destructible_v<T> && is_trivially_destructible_v<E>;

            constexpr static bool copy_assign = copyable && is_copy_assignable_v<T> && is_copy_assignable_v<E> && (is_nothrow_constructible_v<E> || is_nothrow_constructible_v<E>);

            constexpr static bool move_assign = moveable && is_move_assignable_v<T> && is_move_assignable_v<E> && ((is_nothrow_constructible_v<E> || is_nothrow_constructible_v<E>));

            constexpr static bool nothrow_move_assign = nothrow_move_ctor && is_nothrow_move_assignable_v<T> && is_nothrow_move_assignable_v<E>;
        };

        template <typename T, typename E, typename U, typename G, typename UF, typename GF>
        struct enable_construct_from_other
        {
            constexpr static bool require =
                is_constructible_v<T, UF> && is_constructible_v<E, GF> && !is_constructible_v<T, expected<U, G> &> && !is_constructible_v<T, expected<U, G>> && !is_constructible_v<T, const expected<U, G> &> && !is_constructible_v<T, const expected<U, G>> && !is_convertible_v<expected<U, G> &, T> && !is_convertible_v<expected<U, G> &&, T> && !is_convertible_v<const expected<U, G> &, T> && !is_convertible_v<const expected<U, G> &&, T> && !is_constructible_v<unexpected<E>, expected<U, G> &> && !is_constructible_v<unexpected<E>, expected<U, G>> && !is_constructible_v<unexpected<E>, const expected<U, G> &> && !is_constructible_v<unexpected<E>, const expected<U, G>>;

            constexpr static bool explicitable = !is_convertible_v<UF, T> || !is_convertible_v<GF, E>;
        };

        template <typename T, typename E, typename GF>
        struct enable_assign_from_other_unexpected
        {
            constexpr static bool require = is_constructible_v<E, GF> && is_assignable_v<E &, GF> && (is_nothrow_constructible_v<E, GF> || is_nothrow_move_constructible_v<T>);
        };

        template <class T, class U, class... Args>
        constexpr void reinit_expected(T &newval, U &oldval, Args &&...args)
        {
            if constexpr (is_nothrow_constructible_v<T, Args...>)
            {
                destroy_at(addressof(oldval));
                construct_at(addressof(newval), std::forward<Args>(args)...);
            }
            else if constexpr (is_nothrow_move_constructible_v<T>)
            {
                T tmp(std::forward<Args>(args)...);
                destroy_at(addressof(oldval));
                construct_at(addressof(newval), std::move(tmp));
            }
            else
            {
                U tmp(std::move(oldval));
                destroy_at(addressof(oldval));
                try
                {
                    construct_at(addressof(newval), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    construct_at(addressof(oldval), std::move(tmp));
                    throw;
                }
            }
        }
    } // namespace detail

    template <class T, class E>
    class expected
    {

        struct Empty_Class
        {
        };
        using Traits = detail::expected_traits<T, E>;

    public:
        // using std::unexpected;

        using value_type = T;
        using error_type = E;
        using unexpected_type = unexpected<E>;

        template <class U>
        using rebind = expected<U, error_type>;

        // �.�.7.1, constructors
        constexpr expected() requires(is_default_constructible_v<T>)
            : val{}, has_val{true}
        {
        }
        constexpr expected(const expected &) requires(Traits::trivial_copyable) = default;
        constexpr expected(expected &&) noexcept requires(Traits::trivial_moveable) = default;

        constexpr expected(const expected &rhs) requires(!Traits::trivial_copyable && Traits::copyable)
            : _{}, has_val{rhs.has_val}
        {
            if (rhs.has_val)
                std::construct_at(std::addressof(this->val), *rhs);
            else
                std::construct_at(std::addressof(this->unex), rhs.error());
        }

        constexpr expected(expected &&rhs) noexcept(Traits::nothrow_move_ctor) requires(!Traits::trivial_moveable && Traits::moveable)
            : _{}, has_val{rhs.has_val}
        {
            if (rhs.has_val)
                std::construct_at(std::addressof(this->val), std::move(*rhs));
            else
                std::construct_at(std::addressof(this->unex), std::move(rhs).error());
        }

        template <class U, class G, typename TR = detail::enable_construct_from_other<T, E, U, G, const G &, const U &>>
        requires(TR::require) constexpr explicit(TR::explicitable) expected(const expected<U, G> &rhs)
            : _{}, has_val{rhs.has_value()}
        {
            if (rhs.has_value())
                std::construct_at(std::addressof(this->val), *rhs);
            else
                std::construct_at(std::addressof(this->unex), rhs.error());
        }

        template <class U, class G, typename TR = detail::enable_construct_from_other<T, E, U, G, G, U>>
        requires(TR::require) constexpr explicit(TR::explicitable) expected(expected<U, G> &&rhs)
            : _{}, has_val{rhs.has_value()}
        {
            if (rhs.has_value())
                std::construct_at(std::addressof(this->val), std::move(*rhs));
            else
                std::construct_at(std::addressof(this->unex), std::move(rhs).error());
        }

        template <class U = T>
        requires(!is_same_v<remove_cvref_t<U>, in_place_t> && !is_same_v<expected<T, E>, remove_cvref_t<U>> && is_constructible_v<T, U> && !detail::is_unexpected<std::remove_cvref_t<U>>::value) constexpr explicit(!is_convertible_v<U, T>) expected(U &&v)
            : val{std::forward<U>(v)}, has_val{true}
        {
        }

        template <class G>
        requires(is_constructible_v<E, const G &>) constexpr explicit(!is_convertible_v<const G &, E>) 
        expected(const unexpected<G> &e)
            : unex{ e.value()}, has_val{false}
        {
        }

        template <class G>
        requires(is_constructible_v<E, const G &>) constexpr explicit(!is_convertible_v<const G &, E>) expected(unexpected<G> &&e)
            : unex{std::move(e).value()}, has_val{false}
        {
        }

        template <class... Args>
        requires(is_constructible_v<T, Args...>) constexpr explicit expected(in_place_t, Args &&...args)
            : val{(Args &&) args...}, has_val{true}
        {
        }

        // template<class U, class... Args> requires (is_constructible_v<T, initializer_list<U>&, Args...>)
        //     constexpr explicit expected(in_place_t, initializer_list<U> il, Args&&... args);
        //  has-value = true, Direct-non-list-initializes val with il, std::forward<Args>(args)...

        template <class... Args>
        requires(is_constructible_v<E, Args...>) constexpr explicit expected(unexpect_t, Args &&...args)
            : unex{(Args &&) args...}, has_val{false}
        {
        }

        // template<class U, class... Args> requires (is_constructible_v<E, initializer_list<U>&, Args...>)
        //     constexpr explicit expected(unexpect_t, initializer_list<U> il, Args&&... args)
        //         : unex{ il, (Args&&)args... }, has_val{false}
        // {
        // }

        // �.�.7.2, destructor
        constexpr ~expected() requires(Traits::trivial_dctor) = default;
        constexpr ~expected()
        {
            if (has_val)
                destroy_at(addressof(val));
            else
                destroy_at(addressof(unex));
        }

        // �.�.7.3, assignment
        constexpr expected &operator=(const expected &rhs) requires(Traits::trivial_copyable) = default;
        constexpr expected &operator=(expected &&rhs) noexcept requires(Traits::trivial_moveable) = default;

        constexpr expected &operator=(const expected &rhs) requires(Traits::copy_assign && !Traits::trivial_copyable)
        {
            if (this->has_value() && rhs.has_value())
                val = *rhs;
            else if (this->has_value())
                detail::reinit_expected(unex, val, rhs.error());
            else if (rhs.has_value())
                detail::reinit_expected(val, unex, *rhs);
            else
                unex = rhs.error();
            has_val = rhs.has_val;
            return *this;
        }

        constexpr expected &operator=(expected &&rhs) noexcept(Traits::nothrow_move_assign) 
        requires(Traits::move_assign && !Traits::trivial_moveable)
        {
            if (this->has_value() && rhs.has_value())
                val = std::move(*rhs);
            else if (this->has_value())
                detail::reinit_expected(unex, val, std::move(rhs.error()));
            else if (rhs.has_value())
                detail::reinit_expected(val, unex, std::move(*rhs));
            else
                unex = std::move(rhs).error();
            has_val = rhs.has_val;
            return *this;
        }

        template <class U = T>
        requires(!is_same_v<expected<T, E>, remove_cvref_t<U>> && 
        is_constructible_v<T, U> && 
        is_assignable_v<T &, U> && 
        !detail::is_unexpected<std::remove_cvref_t<U>>::value && 
        (is_nothrow_constructible_v<T, U> || is_nothrow_move_constructible_v<E>)) 
        constexpr expected &operator=(U &&v)
        {
            if (has_value())
                val = (U &&) v;
            else
                detail::reinit_expected(val, unex, (U &&) v);
            has_val = true;
            return *this;
        }

        template <class G = E>
        requires(detail::enable_assign_from_other_unexpected<T, E, const G &>::require) constexpr expected &operator=(const unexpected<G> &e)
        {
            if (has_value())
                detail::reinit_expected(unex, val, e.value());
            else
                unex = e.value();
            has_val = false;
            return *this;
        }
        template <class G = E>
        requires(detail::enable_assign_from_other_unexpected<T, E, G>::require) constexpr expected &operator=(unexpected<G> &&e)
        {
            if (has_value())
                detail::reinit_expected(unex, val, std::move(e).value());
            else
                unex = std::move(e).value();
            has_val = false;
            return *this;
        }
        // �.�.7.4, modifiers

        template <class... Args>
        requires(is_nothrow_constructible_v<T, Args...>) constexpr T &emplace(Args &&... args) noexcept
        {
            if (has_value())
                destroy_at(addressof(val));
            else
            {
                destroy_at(addressof(unex));
                has_val = true;
            }
            return *construct_at(addressof(val), (Args &&) args...);
        }

        // template<class U, class... Args>
        //     constexpr T& emplace(initializer_list<U>, Args&&...) noexcept;

        // �.�.7.5, swap
        constexpr void swap(expected &rhs) noexcept(is_nothrow_move_constructible_v<T> &&
                                                        is_nothrow_swappable_v<T> &&
                                                            is_nothrow_move_constructible_v<E> &&
                                                                is_nothrow_swappable_v<E>) requires(is_swappable_v<T>
                                                                                                        &&is_swappable_v<E> &&
                                                                                                    (is_move_constructible_v<T> && is_move_constructible_v<E>)&&(is_nothrow_move_constructible_v<T> || is_nothrow_move_constructible_v<E>))
        {
            if (has_value() && rhs.has_value())
            {
                using std::swap;
                std::swap(val, rhs.val);
            }
            else if (has_value())
            {
                if constexpr (is_nothrow_move_constructible_v<E>) 
                {
                    E tmp(std::move(rhs.unex));
                    destroy_at(addressof(rhs.unex));
                    try 
                    {
                        construct_at(addressof(rhs.val), std::move(val));
                        destroy_at(addressof(val));
                        construct_at(addressof(unex), std::move(tmp));
                    } 
                    catch(...) 
                    {
                        construct_at(addressof(rhs.unex), std::move(tmp));
                        throw;
                    }
                }
                else
                {
                    T tmp(std::move(val));
                    destroy_at(addressof(val));
                    try
                    {
                        construct_at(addressof(unex), std::move(rhs.unex));
                        destroy_at(addressof(rhs.unex));
                        construct_at(addressof(rhs.val), std::move(tmp));
                    }
                    catch (...)
                    {
                        construct_at(addressof(val), std::move(tmp));
                        throw;
                    }
                }
            }
            else if (rhs.has_value())
            {
                rhs.swap(*this);
            }
            else
            {
                using std::swap;
                swap(unex, rhs.unex);
            }
        }

        // �.�.7.6, observers
        constexpr const T *operator->() const noexcept { return std::addressof(val); }
        constexpr T *operator->() noexcept { return std::addressof(val); }

        constexpr const T &operator*() const &noexcept { return val; }
        constexpr T &operator*() &noexcept { return val; }
        constexpr const T &&operator*() const &&noexcept { return std::move(val); }
        constexpr T &&operator*() &&noexcept { return std::move(val); }

        constexpr explicit operator bool() const noexcept { return has_val; }
        constexpr bool has_value() const noexcept { return has_val; }

        constexpr const T &value() const & { return val; }
        constexpr T &value() & { return val; }
        constexpr const T &&value() const && { return std::move(val); }
        constexpr T &&value() && { return std::move(val); }

        constexpr const E &error() const & { return unex; }
        constexpr E &error() & { return unex; }
        constexpr const E &&error() const && { return std::move(unex); }
        constexpr E &&error() && { return std::move(unex); }

        template <class U>
        constexpr T value_or(U &&v) const &
        {
            return has_value() ? **this : static_cast<T>(std::forward<U>(v));
        }
        template <class U>
        constexpr T value_or(U &&v) &&
        {
            return has_value() ? std::move(**this) : static_cast<T>(std::forward<U>(v));
        }

        // �.�.7.7, Expected equality operators
        template <class T2, class E2>
        requires(!is_void_v<T2>) friend constexpr bool operator==(const expected &x, const expected<T2, E2> &y)
        {
            if (x.has_value() != y.has_value())
                return false;
            return x.has_value() ? *x == *y : x.error() == y.error();
        }
        template <class T2>
        friend constexpr bool operator==(const expected &x, const T2 &v)
        {
            return x.has_value() && static_cast<bool>(*x == v);
        }
        template <class E2>
        friend constexpr bool operator==(const expected &x, const unexpected<E2> &e)
        {
            return !x.has_value() && static_cast<bool>(x.error() == e.value());
        }

    private:
        union
        {
            Empty_Class _;
            T val;  // exposition only
            E unex; // exposition only
        };
        bool has_val; // exposition only
    };

    // �.�.7.10, Specialized algorithms
    template <typename T, typename E>
    constexpr void swap(expected<T, E> &x, expected<T, E> &y) noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

} // namespace std