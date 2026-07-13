#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/array.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <ranges>
#include <iostream>
#include <list>
#include <unordered_map>
#include "../json/single_include/nlohmann/json.hpp"

using JsonValue = nlohmann::basic_json<>;


template <typename T>
struct Deserialize;

template <typename T>
struct Deserialize
{

private:

    static constexpr auto caster_adaptor = []<typename U>(std::optional<U>& opt, const auto& value)
    {
        opt.emplace(Deserialize<U>()(value));
    };

    template <std::meta::info Field>
    static auto init_field(const JsonValue& value)
    {
        using FieldType = typename [:type_of(Field):];
        std::optional<FieldType> result = std::nullopt;
        
        if constexpr (!cpp::refl::has_annotations(Field, cpp::refl::skip_deserialization, cpp::refl::skip))
        {
            if constexpr (cpp::refl::has_annotations(Field, cpp::refl::flatten))
            {
                result.emplace(Deserialize<FieldType>()(value));
            }
            else
            {
                auto name = cpp::refl::handle<Field>::identifier();
                auto it = value.find(name);

                if (it != value.end())
                {
                    constexpr auto info = cpp::refl::select_annotation_with_type(^^caster_adaptor, Field, ^^cpp::refl::serializer_annotation);
                    std::invoke(extract<typename [:type_of(info):]>(info), result, *it);
                }
                else if (cpp::refl::has_annotations(Field, cpp::refl::required))
                {
                    throw std::runtime_error(std::format("Field {} is required but not found in the JSON object", name));
                }
            }
        }

        if (!result)
        {
            result = cpp::refl::handle<Field>::default_value();
        }

        return *result;
    }

public:

    static T operator()(const JsonValue& value)
    {
        if constexpr (cpp::refl::has_annotations(^^T, cpp::derive::from<JsonValue>))
        {
            if constexpr (is_aggregate_type(^^T))
            {
                constexpr auto bases = define_static_array(bases_of(^^T, std::meta::access_context::current())); 
                constexpr auto [...base_indices] = std::make_index_sequence<bases.size()>();

                constexpr static auto members = define_static_array(nonstatic_data_members_of(^^T, std::meta::access_context::current()));
                constexpr static auto [...indices] = std::make_index_sequence<members.size()>();

                auto result = T(
                    Deserialize<typename [:type_of(bases[base_indices]):]>()(value)...,
                    init_field<members[indices]>(value)...
                );

                if (!cpp::refl::check_field(result))
                {
                    throw std::runtime_error(std::format("Field check failed."));
                }

                return result;  // std::move(result);
            }
            else
            {
                constexpr auto ctor = define_static_array(
                    members_of(^^T, std::meta::access_context::current()) 
                    | std::views::filter(std::meta::is_constructor) 
                    | std::views::filter(std::bind_back(cpp::refl::has_annotations, cpp::refl::constructor))
                    | std::ranges::to<std::vector>()
                );

                static_assert(ctor.size() == 1, "Only one constructor is allowed for deserialization");

                constexpr static auto params = define_static_array(parameters_of(ctor.front()));
                
                auto impl = [&]<size_t Idx>() {
                    constexpr auto typeinfo = std::meta::decay(type_of(params[Idx]));
                    constexpr auto ParamName = identifier_of(params[Idx]);
                    return Deserialize<typename [:typeinfo:]>()(value.find(std::string(ParamName)).value());
                };
                
                constexpr auto [...indices] = std::make_index_sequence<params.size()>();
                auto result = T(impl.template operator()<indices>()...);
                
                if (!cpp::refl::check_field(result))
                {
                    throw std::runtime_error(std::format("Field check failed."));
                }

                return result;
            }   
        }
        else 
        {
            return value.get<T>();
        }
    }
};

inline constexpr auto PlusOne = cpp::refl::make_callable<cpp::refl::serializer_annotation>(
    [](std::optional<std::vector<int>>& opt, const JsonValue& v) 
    {
        auto x1 = Deserialize<int>()(v[0]) + 1;
        auto x2 = Deserialize<int>()(v[1]) + 1;
        auto x3 = Deserialize<int>()(v[2]) + 1;
        opt.emplace(std::vector{ x1,  x2,  x3});
    }
);

struct OtherInfo1
{
    std::string information;
};

struct OtherInfo2
{
    std::string information;
};

// Change class serializer by specialize the optional_caster for OtherInfo1
template <>
struct Deserialize<OtherInfo1>
{
    static OtherInfo1 operator()(const JsonValue& v)
    {
        if (!v.is_object())
        {
            throw std::runtime_error(std::format("Expected an object for OtherInfo1 deserialization."));
        }

        OtherInfo1 info;
        auto it = v.find("information");
        if (it != v.end() && it.value().is_string())
        {
            info.information = "Information1: " + Deserialize<std::string>()(it.value());
            return info;
        }
        else
        {
            throw std::runtime_error(std::format("Field check failed for OtherInfo1 deserialization."));
        }
    }
};

inline constexpr auto SerializeAsTuple = cpp::refl::make_callable<cpp::refl::serializer_annotation>(
    []<typename T>(std::optional<T>& opt, const JsonValue& v) 
    {
        constexpr auto ctx = std::meta::access_context::current();
        constexpr static auto members = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));
        constexpr auto [...indices] = std::make_index_sequence<members.size()>{};
        opt.emplace(T(Deserialize<typename [:type_of(members[indices]):]>()(v[indices])...));
    }
);

// Use user-defined annotation 
inline constexpr auto SerializeOtherInfo2 = cpp::refl::make_callable<cpp::refl::serializer_annotation>(
    [](auto& opt, const JsonValue& v) 
    {
        if (!v.is_object())
        {
            return;
        }

        OtherInfo2 info;
        auto it = v.find("information");
        if (it != v.end() && it.value().is_string())
        {
            info.information = "Information2: " + Deserialize<std::string>()(it.value());
            opt.emplace(std::move(info));
        }
    }
);

struct ComplexString : cpp::refl::initializer_annotation
{
    static constexpr std::list<std::string> operator()()
    {
        return { "Hello", "World" };
    }
};

void AssertTrue(bool condition)
{
    if (!condition)
    {
        throw std::runtime_error("Assertion failed");
    }
}

struct [[=cpp::derive::from<JsonValue>]] Student
{
    [[=cpp::refl::uppercase]]
    std::string name;
    
    [[=cpp::refl::rename("_age_")]]
    int age;
    
    // Gender gender;

    [[=PlusOne]]
    std::vector<int> grades;

    std::map<std::string, std::string> address;

    OtherInfo1 otherInfo1;

    [[=SerializeOtherInfo2]]
    OtherInfo2 otherInfo2;

    struct Profile
    {
        int weight;
        double height;
        std::string nickname;
    };

    struct [[=cpp::derive::from<JsonValue>]] CustomerBasicInfo
    {
        std::string idType;
        std::string idNumber;
    };

    [[=cpp::refl::flatten]]
    CustomerBasicInfo customerBasicInfo;

    [[=SerializeAsTuple]]
    Profile profile;

    [[=cpp::refl::skip, =cpp::refl::default_value("Unknown")]]
    std::string unknownAttribute1;

    [[=cpp::refl::skip, =ComplexString{}]]
    std::list<std::string> unknownAttribute2;
};

int main()
{
    JsonValue v = R"(
        {
            "NAME": "Alice",
            "_age_": 30,
            "grades": [85, 90, 78],
            "address": {
                "street": "123 Main St",
                "city": "Wonderland",
                "zip": "12345"
            },
            "otherInfo1": {
                "information": "This is some other information."
            },
            "otherInfo2": {
                "information": "This is some other information."
            },
            "profile": [1, 3.14, "Hello"],
            "idType": "Passport",
            "idNumber": "A12345678",
            "unknownAttribute1": "This attribute is unknown."
        }
    )"_json;

    auto student = Deserialize<Student>()(v);

    AssertTrue(student.name == "Alice");
    AssertTrue(student.age == 30);
    // AssertTrue(student.gender == Gender::Female);
    AssertTrue(student.grades.size() == 3);
    AssertTrue(student.grades[0] == 86);
    AssertTrue(student.grades[1] == 91);
    AssertTrue(student.grades[2] == 79);
    AssertTrue(student.address.size() == 3);
    AssertTrue(student.address["street"] == "123 Main St");
    AssertTrue(student.address["city"] == "Wonderland");
    AssertTrue(student.address["zip"] == "12345");
    AssertTrue(student.otherInfo1.information == "Information1: This is some other information.");
    AssertTrue(student.otherInfo2.information == "Information2: This is some other information.");
    AssertTrue(student.profile.weight == 1);
    AssertTrue(student.profile.height == 3.14);
    AssertTrue(student.profile.nickname == "Hello");
    AssertTrue(student.customerBasicInfo.idType == "Passport");
    AssertTrue(student.customerBasicInfo.idNumber == "A12345678");
    AssertTrue(student.unknownAttribute1 == "Unknown");
    AssertTrue(student.unknownAttribute2.size() == 2);
    AssertTrue(student.unknownAttribute2.front() == "Hello");
    AssertTrue(student.unknownAttribute2.back() == "World");

    std::cout << "All assertions passed!" << std::endl;
}

