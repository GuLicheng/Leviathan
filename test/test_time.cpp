#include "test_time.hpp"

#include <set>
#include <lv_cpp/collections/sorted_list.hpp>
#include <lv_cpp/collections/skip_list.hpp>

#include <list>


int main()
{
    using STL_SET = std::set<int>;
    leviathan::test::all_test<STL_SET>("std::set");

    using SORTED_LIST = leviathan::collections::sorted_list<int>;
    leviathan::test::all_test<SORTED_LIST>("sorted_list");

    using SKIP_LIST = leviathan::collections::skip_list<int>;
    leviathan::test::all_test<SKIP_LIST>("skip_list");

}














