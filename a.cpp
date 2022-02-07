#include <optional>
#include <string>
#include <any>

int main()
{
    std::optional<std::string> op = { "Hello" };
    static_assert(!std::is_convertible_v<std::string, const char[6]>);
}
