#include <string_view>
#include <format>
#include <print>
#include <vector>
#include <unordered_map>
#include <ranges>
#include <functional>
#include <algorithm>
#include "header.hpp"  
#include <leviathan/extc++/ranges.hpp>


struct NodeOp
{

};



int main()
{
    std::string_view field_name = "Outputuu";    
    auto ab = false;

    int c = 0;
    std::nullptr_t d = nullptr;

    field_name.npos;
    
    int arr[] { 1, 2, 3 };
    
    std::println("Short name: {}", cpp::shortname(field_name));
    std::println("Long name: {}", cpp::longname(field_name));
    std::println("Lowercase: {}", cpp::lowercase(field_name));
    std::println("Uppercase: {}", cpp::uppercase(field_name));
    std::println("ShortLower: {}", cpp::shortname(cpp::lowercase(field_name)));
    std::println("Custom name: {}", cpp::rename("MyOutput")(field_name));
    std::println("Custom empty: {}", cpp::rename("")(field_name));
    std::println("Custom help: {}", cpp::help("This is a help text").value);

}

