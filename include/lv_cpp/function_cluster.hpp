#ifndef __FUNCTIONS_CLUSTER_HPP__
#define __FUNCTIONS_CLUSTER_HPP__

#include <iostream> // for debug
#include <lv_cpp/type_list.hpp>
#include <lv_cpp/utils/null.hpp>
#include <any>


namespace leviathan
{

// helper class, maybe exception some day
struct no_match_paras /* : std::exception */
{
    no_match_paras(...) { }
    friend std::ostream& operator<<(std::ostream& os, const no_match_paras& n)
    {
        return os << "no_match_paras";
    }
};

template <template <typename...> typename Function, 
        typename List1, typename List2>
struct helper;

template <template <typename...> typename Function, 
          template <typename...> typename Container1, typename... Ls1, 
          template <typename...> typename Container2, typename... Ls2>
struct helper<Function, Container1<Ls1...>, Container2<Ls2...>>
{
    // for Function F and f1, f2, f3 in List1, a1, a2, a3 in List2
    // we want get F(f1, a1, a2, a3), F(f2, a1, a2, a3), F(f3, a1, a2, a3)
    using type = std::tuple<
        typename Function<Ls1, Ls2...>::type...
    >;
};



template <typename... Lambdas>
class function_cluster
{
    template <typename _Func, typename... _Args>
    struct __get_return
    {
    private:
        using return_type = std::invoke_result_t<_Func, _Args...>;
    public:
        using type = std::conditional_t<
            std::is_same_v<void, return_type>,
            ::leviathan::null,
            return_type>;
    };
public:

    using member_t = 
        typename ::leviathan::meta::repeat<sizeof...(Lambdas), std::any>::type;

    // function types
    using type_pack = std::tuple<Lambdas...>;

    template <typename... Args>
    auto match_paras_call(Args&&... args) const
    {
        // any = std::get<0>(m_data)
        // f = any_cast<tuple_element_t<0>>(any);
        // retrun = f(args...)
        // ...
        constexpr auto size = sizeof...(Lambdas);
        return match_paras_call_impl(
                std::make_index_sequence<size>(), std::forward<Args>(args)...);
    }
private:

    template <typename _Fn, typename... Args>
    decltype(auto) call_unit(const _Fn& f, Args&&... args) const
    noexcept(noexcept(f(args...)))
    {
        // std::cout << "This call unit: " << (noexcept(f(args...))) << std::endl;
        using return_type = std::invoke_result_t<_Fn, Args...>;
        if constexpr (std::is_same_v<void, return_type>)
        {
            f(std::forward<Args>(args)...);
            return null{};
        }
        else
        {
            return f(std::forward<Args>(args)...);
        }       
    }


    template <typename... Args, size_t... Idx>
    typename helper<__get_return, type_pack, std::tuple<Args...>>::type
    match_paras_call_impl(std::index_sequence<Idx...>, Args&&... args) const
    {
        return 
        // not support void return type version:
        // {
        //     std::any_cast<std::tuple_element_t<Idx, type_pack>>(std::get<Idx>(this->functions))
        //                     (std::forward<Args>(args)...) ...
        // };
        {
            this->call_unit(
                *std::any_cast<std::tuple_element_t<Idx, type_pack>>(&std::get<Idx>(this->functions)),
                std::forward<Args>(args)...) ...
        };
    }

public:
    function_cluster(Lambdas... fns) : functions{fns...} { }

    template <size_t N>
    auto& get() noexcept
    {
        return std::get<N>(this->functions);
    }

    template <size_t N>
    const auto& get() const noexcept 
    {
        return std::get<N>(this->functions);
    }


private:
    member_t functions;
};





}  // namespace leviathan


#endif