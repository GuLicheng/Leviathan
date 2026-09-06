#include <winnow/error.hpp>
#include <winnow/utils.hpp>
#include <winnow/result.hpp>
#include <print>

template <typename O, typename E>
using ModalResult = winnow::modal_result<O, winnow::err_mode<E>>;

int main()
{
    ModalResult<int, std::string> result(std::in_place, 42);

    auto result2 = result.transform([](int value) { return value * 2.5555; });

    std::println("value = {}", result2.value());

}
