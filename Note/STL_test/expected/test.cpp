#include "expected.hpp"
// TODO: add specialize for expected<void, E>
// std::expected will supported in C++23

#include <vector>
#include <system_error>

namespace P0323R11
{

    using std::string;
    using std::expected;
    using std::unexpected;
    using std::vector;
    using std::unexpect_t;
    using std::unexpect;
    using std::errc;
    using std::in_place;

    void test3_1()
    {
        expected<int, string> ei = 0;
        expected<int, string> ej = 1;
        expected<int, string> ek = unexpected(string());

        ei = 1;
        // ej = unexpected(E());
        ek = 0;

        // ei = unexpected(E());
        ej = 0;
        ek = 1;
    }

    void test3_3()
    {
        string s{"STR"};

        expected<string, errc> es{s}; // requires Copyable<T>
        expected<string, errc> et = s; // requires Copyable<T>
        expected<string, errc> ev = string{"STR"}; // requires Movable<T>

        expected<string, errc> ew; // expected value
        expected<string, errc> ex{}; // expected value
        expected<string, errc> ey = {}; // expected value
        expected<string, errc> ez = expected<string,errc>{}; // expected value

        expected<string, int> ep{unexpected(-1)}; // unexpected value, requires Movable<E>
        expected<string, int> eq = unexpected(-1); // unexpected value, requires Movable<E>

        struct MoveOnly { 
            MoveOnly(...) { }
            MoveOnly(const MoveOnly&) = delete;
            MoveOnly(MoveOnly&&) noexcept = default; 
        };

        expected<MoveOnly, errc> eg; // expected value
        expected<MoveOnly, errc> eh{}; // expected value
        expected<MoveOnly, errc> ei{in_place}; // calls MoveOnly{} in place
        expected<MoveOnly, errc> ej{in_place, "arg"}; // calls MoveOnly{"arg"} in place

        expected<int, string> ei2{unexpect}; // unexpected value, calls string{} in place
        expected<int, string> ej2{unexpect, "arg"}; // unexpected value, calls string{"arg"} in place

    }

    void test3_6()
    {
        expected<int, errc> ei = 1; // works
        expected<int, errc> ei2{in_place, 1};
        // expected<int, errc> ei = success(1); implicit conversion is dangerous
        auto ec = std::errc::address_family_not_supported;
        expected<int, errc> ej = unexpected(ec); // An alternative will be to make it explicit 
    }

    void test3_7()
    {
        // expected<string, errc> exp1 = unexpected(static_cast<errc>(1));
        // expected<string, errc> exp2 = {unexpect, static_cast<errc>(1)};
        // exp1 = static_cast<errc>(1);
        // exp2 = {unexpect, 1};
        // While some situations would work with the {unexpect, ...} syntax, 
        // using unexpected makes the programmer’s intention as clear and less cryptic. Compare these
        
        // auto func1 = [] () -> expected<vector<int>, int> {
        //     return {unexpect, 1}; -> Mark it explicit
        // };
        auto func2 = [] () -> expected<vector<int>, int> {
            return unexpected(1);
        };
        auto func3 = [] () -> expected<vector<int>, int> {
            return expected<vector<int>, int>{unexpect, 1};
        };
        auto func4 = [] () -> expected<vector<int>, int> {
            return unexpected(1);
        };

    }

    void test3_11()
    {
        auto readNextChar = [] { return unexpected(errc::io_error); };
        if (expected<char, errc> ch = readNextChar())
        {
            // ...
        }
    }

    void test3_17()
    {
        auto getIntOrZero = []() -> expected<int, errc> {
            auto r = []() { return expected<int, errc>{unexpect, errc::io_error}; }();
            if (!r && r.error() == errc::io_error) {
                return 0;
            }
            return r;
        };
        getIntOrZero();
    }

    void test3_24()
    {
        using error = string;
        struct Big {
            Big() = default;
            Big(const char*) { }
        };
        expected<Big, error> eb{in_place, "1"}; // calls Big{"1"} in place (no moving)
        expected<Big, error> ec{in_place}; // calls Big{} in place (no moving)
        expected<Big, error> ed{}; // calls Big{} (expected state)
        expected<Big, error> eb2{unexpect, "1"}; // calls error{"1"} in place (no moving)
        expected<Big, error> ec2{unexpect}; // calls error{} in place (no moving)
    }

}

int main()
{
    P0323R11::test3_1();
    P0323R11::test3_3();
    P0323R11::test3_6();
    P0323R11::test3_7();
    P0323R11::test3_11();
    P0323R11::test3_17();
    P0323R11::test3_24();
}
