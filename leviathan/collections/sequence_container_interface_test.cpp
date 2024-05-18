#include "sequence_container_interface.hpp"
#include <catch2/catch_all.hpp>


// #include "pch.h"
// #include "CppUnitTest.h"

// using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace leviathan::collections;

struct Assert
{
    template <typename T>
    static void AreEqual(const T& expected, const T& actual)
    {
        REQUIRE(expected == actual);
    }

    static void IsTrue(bool value)
    {
        REQUIRE(value == true);
    }
};


namespace TestSequenceContainerInterface
{ 
struct IntArray : reversible_container_interface
{
    using value_type = int;
    using iterator = std::array<int, 10>::iterator;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    IntArray() = default;

    void init()
    {
        for (size_t i = 0; i < data.max_size(); ++i)
        {
            data[i] = (int)i;
        }
    }

    iterator begin() { return data.begin(); }
    const_iterator begin() const
    {
        return std::make_const_iterator(const_cast<IntArray&>(*this).data.begin());
    }

    iterator end() { return data.end(); }
    const_iterator end() const
    {
        return std::make_const_iterator(const_cast<IntArray&>(*this).data.end());
    }

    std::array<int, 10> data;
};

// TEST_CLASS(TestSequenceContainerInterface)
// {
// public:

    // TEST_METHOD(TestSequenceContainerBeginAndEndType)
    TEST_CASE("TestSequenceContainerBeginAndEndType")
    {
        IntArray a;

        using I = IntArray::iterator;
        using CI = IntArray::const_iterator;

        // begin
        Assert::IsTrue(std::is_same_v<I, decltype(a.begin())>);
        Assert::IsTrue(std::is_same_v<I, decltype(std::move(a).begin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::as_const(a).begin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(std::as_const(a)).begin())>);

        // end
        Assert::IsTrue(std::is_same_v<I, decltype(a.end())>);
        Assert::IsTrue(std::is_same_v<I, decltype(std::move(a).end())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::as_const(a).end())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(std::as_const(a)).end())>);

        // cbegin
        Assert::IsTrue(std::is_same_v<CI, decltype(a.cbegin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(a).cbegin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::as_const(a).cbegin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(std::as_const(a)).cbegin())>);

        // cend
        Assert::IsTrue(std::is_same_v<CI, decltype(a.cend())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(a).cend())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::as_const(a).cend())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(std::as_const(a)).cend())>);
    }

    // TEST_METHOD(TestReversibleContainerBeginAndEndType)
    TEST_CASE("TestReversibleContainerBeginAndEndType")
    {
        IntArray a;

        using I = IntArray::reverse_iterator;
        using CI = IntArray::const_reverse_iterator;

        // rbegin
        Assert::IsTrue(std::is_same_v<I, decltype(a.rbegin())>);
        Assert::IsTrue(std::is_same_v<I, decltype(std::move(a).rbegin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::as_const(a).rbegin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(std::as_const(a)).rbegin())>);

        // rend
        Assert::IsTrue(std::is_same_v<I, decltype(a.rend())>);
        Assert::IsTrue(std::is_same_v<I, decltype(std::move(a).rend())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::as_const(a).rend())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(std::as_const(a)).rend())>);

        // rcbegin
        Assert::IsTrue(std::is_same_v<CI, decltype(a.rcbegin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(a).rcbegin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::as_const(a).rcbegin())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(std::as_const(a)).rcbegin())>);

        // rcend
        Assert::IsTrue(std::is_same_v<CI, decltype(a.rcend())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(a).rcend())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::as_const(a).rcend())>);
        Assert::IsTrue(std::is_same_v<CI, decltype(std::move(std::as_const(a)).rcend())>);
    }

    // TEST_METHOD(TestSequenceContainerValue)
    TEST_CASE("TestSequenceContainerValue")
    {

        IntArray v;
        v.init();

        Assert::AreEqual(*v.begin(), 0);
        Assert::AreEqual(*v.begin(), 0);
        Assert::AreEqual(*(v.end() - 1), 9);

        Assert::AreEqual(*std::as_const(v).begin(), 0);
        Assert::AreEqual(*(std::as_const(v).end() - 1), 9);

        Assert::AreEqual(*v.cbegin(), 0);
        Assert::AreEqual(*(v.cend() - 1), 9);

        Assert::AreEqual(*std::as_const(v).cbegin(), 0);
        Assert::AreEqual(*(std::as_const(v).cend() - 1), 9);

        // Reverse version
        Assert::AreEqual(*v.rbegin(), 9);
        Assert::AreEqual(*(v.rend() - 1), 0);

        Assert::AreEqual(*std::as_const(v).rbegin(), 9);
        Assert::AreEqual(*(std::as_const(v).rend() - 1), 0);

        Assert::AreEqual(*v.rcbegin(), 9);
        Assert::AreEqual(*(v.rcend() - 1), 0);

        Assert::AreEqual(*std::as_const(v).rcbegin(), 9);
        Assert::AreEqual(*(std::as_const(v).rcend() - 1), 0);
    }

// };

}

namespace TestIteratorFacade
{
struct InDecrement : postfix_increment_and_decrement_operation
{
    int x;

    InDecrement& operator++() { x++; return *this; }
    InDecrement& operator--() { x--; return *this; }

    using postfix_increment_and_decrement_operation::operator++;
    using postfix_increment_and_decrement_operation::operator--;
};

struct Dereference : arrow_operation
{
    inline static int Value = 1;

    int* x = &Value;

    int& operator*() const { return *x; }
};


// TEST_CLASS(TestIteratorFacade)
// {
    // TEST_METHOD(TestIncrementDecrement)
    TEST_CASE("TestIncrementDecrement")
    {
        InDecrement i;
        i.x = 0;
        i++;
        Assert::AreEqual(i.x, 1);
        ++i;
        Assert::AreEqual(i.x, 2);
        i--;
        Assert::AreEqual(i.x, 1);
        --i;
        Assert::AreEqual(i.x, 0);
    }

    // TEST_METHOD(TestDereference)
    TEST_CASE("TestDereference")
    {
        Dereference de;

        Assert::AreEqual(de.operator->(), &Dereference::Value);
    }
// };

}


