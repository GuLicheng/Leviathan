#include <variant>
#include <vector>
#include <array>
#include <type_traits>

#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/meta/type_list.hpp>
#include <lv_cpp/io/console.hpp>

using namespace leviathan::meta;


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
