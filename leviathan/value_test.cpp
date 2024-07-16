#include <leviathan/value.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <catch2/catch_all.hpp>

struct Object;

struct DummyHash
{
    template <typename T>
    static constexpr size_t operator()(const T& x)
    {
        return 0;
    }
};

using None = std::nullptr_t;
using Number = double;
using String = std::string;
using List = std::vector<Object>;
using Dict = std::unordered_map<Object, Object, DummyHash>;

using ObjectBase = leviathan::value<
            leviathan::to_unique_ptr_if_large_than<16>, 
            None, 
            Number, 
            String, 
            List, 
            Dict>;

struct Object : ObjectBase
{
    using ObjectBase::ObjectBase;

    auto& data() const
    {
        return m_data;
    }

    template <typename T>
    static bool equal_impl(const Object& x, const Object& y) 
    {
        if (x.is<T>())
        {
            return x.as<T>() == y.as<T>();
        }
        return false;
    }

    friend bool operator==(const Object& x, const Object& y) 
    {
        if (x.m_data.index() != y.m_data.index())
        {
            return false;
        }
        return equal_impl<None>(x, y)
            || equal_impl<Number>(x, y) 
            || equal_impl<String>(x, y) 
            || equal_impl<List>(x, y) 
            || equal_impl<Dict>(x, y); 
    };
};

TEST_CASE("object")
{
    Object object;

    REQUIRE(object.is<None>());
}

TEST_CASE("object convert")
{
    Object object = String("Hello World");

    REQUIRE(object.is<String>());
    REQUIRE(object.as<String>() == "Hello World");
}

TEST_CASE("object list")
{
    Object obj1 = nullptr, obj2 = Number(3.14), obj3 = String("Alice");
    
    Object list = List();

    REQUIRE(list.is<List>());

    list.as<List>().emplace_back(std::move(obj1));
    list.as<List>().emplace_back(std::move(obj2));
    list.as<List>().emplace_back(std::move(obj3));

    REQUIRE(list.as<List>()[0].is<None>());
    REQUIRE(list.as<List>()[1].is<Number>());
    REQUIRE(list.as<List>()[2].is<String>());

    REQUIRE(list.as<List>()[0].as<None>() == nullptr);
    REQUIRE(list.as<List>()[1].as<Number>() == 3.14);
    REQUIRE(list.as<List>()[2].as<String>() == "Alice");
}

TEST_CASE("object dictionary")
{
    Object name = String("Alice"), age = Number(18);

    Object dict = Dict();

    dict.as<Dict>().try_emplace(std::move(name), std::move(age));

    REQUIRE(dict.as<Dict>().size() == 1);

    Object target = String("Alice");

    REQUIRE(dict.as<Dict>().contains(target));
}



