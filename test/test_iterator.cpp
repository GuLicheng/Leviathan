#include <lv_cpp/utils/iterator.hpp>

#include <catch2/catch_all.hpp>

using leviathan::is_move_iterator_v;

TEST_CASE("throw return itself")
{
    struct ThrowMoveCtor
    {
        ThrowMoveCtor() = default;
        ThrowMoveCtor(ThrowMoveCtor&&) { throw 0; }
    };

    ThrowMoveCtor* ptr = nullptr;

    auto it = leviathan::make_move_iterator_if_noexcept(ptr);

    using T = decltype(it);

    REQUIRE(!is_move_iterator_v<T>);
}

TEST_CASE("nothrow return move iterator")
{
    int* ptr = nullptr;

    auto it = leviathan::make_move_iterator_if_noexcept(ptr);

    using T = decltype(it);

    REQUIRE(is_move_iterator_v<T>);
}

TEST_CASE("move iterator return itself")
{
    int* ptr = nullptr;

    auto move_it1 = leviathan::make_move_iterator_if_noexcept(ptr);
    auto move_it2 = leviathan::make_move_iterator_if_noexcept(move_it1);

    using T1 = decltype(move_it1);
    using T2 = decltype(move_it2);

    REQUIRE(is_move_iterator_v<T1>);
    REQUIRE(is_move_iterator_v<T2>);
}

TEST_CASE("const pointer")
{
    const int* cptr = nullptr;

    auto move_it = std::make_move_iterator(cptr);

    using T = std::iter_reference_t<decltype(move_it)>;

    REQUIRE(std::is_same_v<T, const int&&>);
}
