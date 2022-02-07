#include <variant>
#include <vector>
#include <array>
#include <type_traits>

#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/meta/type_list.hpp>
#include <lv_cpp/io/console.hpp>


template <typename Init, template <typename> typename... Fs>
struct curry;

template <typename Init, template <typename> typename F1, template <typename> typename... Fs>
struct curry<Init, F1, Fs...>
{
private:
    using inner_type = typename F1<Init>::type;
public:
    using type = typename curry<inner_type, Fs...>::type;
};

template <typename Init>
struct curry<Init>
{
    using type = Init;
};


template <int N, template <typename...> typename F, typename List>
struct iterate 
{
    using last_type = typename iterate<N - 1, F, List>::type;
    using type = typename F<last_type>::type;
};

template <template <typename...> typename F, typename List>
struct iterate<0, F, List> 
{
    using type = typename F<List>::type;
};


template <typename List, size_t N>
struct drop
{
    using type = typename iterate<N - 1, leviathan::meta::pop_front, List>::type;
};

template <typename List, size_t N>
struct take
{
private:
    constexpr static auto size = leviathan::meta::size<List>::value;
public:
    using type = typename iterate<size - N - 1, leviathan::meta::pop_back, List>::type;
};

template <typename List, size_t From, size_t To, size_t Stride = 1>
struct slice;

template <typename List, size_t From, size_t To>
struct slice<List, From, To, 1>
{
    // drop + take
public:
    using type = take<typename drop<List, From>::type, To - From>::type;
};

template <typename List, size_t N>
struct take_last
{
private:
    constexpr static auto size = leviathan::meta::size<List>::value;
public:
    using type = typename iterate<size - N - 1, leviathan::meta::pop_front, List>::type;
};

template <typename List, size_t N>
struct drop_last
{
public:
    using type = typename iterate<N - 1, leviathan::meta::pop_back, List>::type;
};


int main()
{
    using Tuple = std::tuple<int, double, bool, const char*>;
    using U = typename drop<Tuple, 2>::type;
    PrintTypeInfo(U);
    using V = typename take<Tuple, 1>::type;
    PrintTypeInfo(V);
    using W = typename take_last<Tuple, 3>::type;
    PrintTypeInfo(W);
    using X = typename drop_last<Tuple, 3>::type;
    PrintTypeInfo(X);
}
