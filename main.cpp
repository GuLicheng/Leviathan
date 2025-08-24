#include <leviathan/type_caster.hpp>
#include <print>
#include <functional>
#include <leviathan/meta/type.hpp>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>

class Value;

using Null = std::nullptr_t;
using Boolean = bool;
using Number = double;
using String = std::string;
using Object = std::unordered_map<std::string, Value>;
using Array = std::vector<Value>;

class Value : public std::variant<Null, Boolean, Number, String, Object, Array>
{
};

int main(int argc, char const *argv[])
{
    return 0;
}
