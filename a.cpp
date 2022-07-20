#include <iostream>
#include <set>
#include <map>
#include <lv_cpp/collections/internal/avl_tree.hpp>
#include <string>

int main()
{
    ::leviathan::collections::avl_set<std::string> as;

    as.begin()->push_back('a');

}



