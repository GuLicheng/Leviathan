#ifndef __PIPE_HPP__
#define __PIPE_HPP__

#include <concepts>
#include <type_traits>

namespace _Pipe
{
    using namespace std;
    // clang-format off
        template <class _Left, class _Right>
        concept _Can_pipe = requires(_Left&& __l, _Right&& __r) {
            static_cast<_Right&&>(__r)(static_cast<_Left&&>(__l));
        };

        template <class _Left, class _Right>
        concept _Can_compose = constructible_from<remove_cvref_t<_Left>, _Left>
            && constructible_from<remove_cvref_t<_Right>, _Right>;
    // clang-format on

    template <class, class>
    struct _Pipeline;

    template <class _Derived>
    struct _Base
    {
        // clang-format off
            template <class _Other>
                requires _Can_compose<_Derived, _Other>
            constexpr auto operator|(_Base<_Other>&& __r) && noexcept(
                noexcept(_Pipeline{static_cast<_Derived&&>(*this), static_cast<_Other&&>(__r)})) {
            // clang-format on
            _STL_INTERNAL_STATIC_ASSERT(derived_from<_Derived, _Base<_Derived>>);
            _STL_INTERNAL_STATIC_ASSERT(derived_from<_Other, _Base<_Other>>);
            return _Pipeline{static_cast<_Derived &&>(*this), static_cast<_Other &&>(__r)};
        }

        // clang-format off
            template <class _Other>
                requires _Can_compose<_Derived, const _Other&>
            constexpr auto operator|(const _Base<_Other>& __r) && noexcept(noexcept(
                _Pipeline{static_cast<_Derived&&>(*this), static_cast<const _Other&>(__r)})) {
            // clang-format on
            _STL_INTERNAL_STATIC_ASSERT(derived_from<_Derived, _Base<_Derived>>);
            _STL_INTERNAL_STATIC_ASSERT(derived_from<_Other, _Base<_Other>>);
            return _Pipeline{static_cast<_Derived &&>(*this), static_cast<const _Other &>(__r)};
        }

        // clang-format off
            template <class _Other>
                requires _Can_compose<const _Derived&, _Other>
            constexpr auto operator|(_Base<_Other>&& __r) const& noexcept(
                noexcept(_Pipeline{static_cast<const _Derived&>(*this), static_cast<_Other&&>(__r)})) {
            // clang-format on
            _STL_INTERNAL_STATIC_ASSERT(derived_from<_Derived, _Base<_Derived>>);
            _STL_INTERNAL_STATIC_ASSERT(derived_from<_Other, _Base<_Other>>);
            return _Pipeline{static_cast<const _Derived &>(*this), static_cast<_Other &&>(__r)};
        }

        // clang-format off
            template <class _Other>
                requires _Can_compose<const _Derived&, const _Other&>
            constexpr auto operator|(const _Base<_Other>& __r) const& noexcept(noexcept(
                _Pipeline{static_cast<const _Derived&>(*this), static_cast<const _Other&>(__r)})) {
            // clang-format on
            _STL_INTERNAL_STATIC_ASSERT(derived_from<_Derived, _Base<_Derived>>);
            _STL_INTERNAL_STATIC_ASSERT(derived_from<_Other, _Base<_Other>>);
            return _Pipeline{static_cast<const _Derived &>(*this), static_cast<const _Other &>(__r)};
        }

        template <_Can_pipe<const _Derived &> _Left>
        friend constexpr auto operator|(_Left &&__l, const _Base &__r) noexcept(
            noexcept(static_cast<const _Derived &>(__r)(_STD forward<_Left>(__l))))
        {
            return static_cast<const _Derived &>(__r)(_STD forward<_Left>(__l));
        }

        template <_Can_pipe<_Derived> _Left>
        friend constexpr auto operator|(_Left &&__l, _Base &&__r) noexcept(
            noexcept(static_cast<_Derived &&>(__r)(_STD forward<_Left>(__l))))
        {
            return static_cast<_Derived &&>(__r)(_STD forward<_Left>(__l));
        }
    };

    template <class _Left, class _Right>
    struct _Pipeline : _Base<_Pipeline<_Left, _Right>>
    {
        /* [[no_unique_address]] */ _Left __l;
        /* [[no_unique_address]] */ _Right __r;

        template <class _Ty1, class _Ty2>
        constexpr explicit _Pipeline(_Ty1 &&_Val1, _Ty2 &&_Val2) noexcept(
            is_nothrow_convertible_v<_Ty1, _Left> &&is_nothrow_convertible_v<_Ty2, _Right>)
            : __l(_STD forward<_Ty1>(_Val1)), __r(_STD forward<_Ty2>(_Val2)) {}

        template <class _Ty>
        _NODISCARD constexpr auto operator()(_Ty &&_Val) noexcept(
            noexcept(__r(__l(_STD forward<_Ty>(_Val))))) requires requires
        {
            __r(__l(static_cast<_Ty &&>(_Val)));
        }
        {
            return __r(__l(_STD forward<_Ty>(_Val)));
        }

        template <class _Ty>
        _NODISCARD constexpr auto operator()(_Ty &&_Val) const
            noexcept(noexcept(__r(__l(_STD forward<_Ty>(_Val))))) requires requires
        {
            __r(__l(static_cast<_Ty &&>(_Val)));
        }
        {
            return __r(__l(_STD forward<_Ty>(_Val)));
        }
    };

    template <class _Ty1, class _Ty2>
    _Pipeline(_Ty1, _Ty2) -> _Pipeline<_Ty1, _Ty2>;
} // namespace _Pipe

#endif
