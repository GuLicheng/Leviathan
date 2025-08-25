#include <leviathan/type_caster.hpp>
#include <print>
#include <functional>
#include <leviathan/meta/type.hpp>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>

#include <experimental/nom/nom.hpp>
#include <experimental/nom/parser.hpp>
#include <leviathan/config_parser/json/value.hpp>
#include <leviathan/type_caster.hpp>


using Value = cpp::json::value;

template <typename Context>
struct Parser
{
    static auto sp(Context& ctx) 
    {
        return nom::character::multispace0(ctx);
    }

    static auto parse_str(Context& ctx)
    {
        return nom::bytes::escaped(
            nom::character::alphanumeric1, '\\', nom::character::one_of("\"n\\")
        )(ctx);
    }

    static nom::IResult<Value> boolean(Context& ctx)
    {
        auto parse_true = nom::combinator::value(Value(true), nom::tag("true"));
        auto parse_false = nom::combinator::value(Value(false), nom::tag("false"));
        return nom::branch::alt(std::move(parse_true), std::move(parse_false))(ctx);
    }

    static nom::IResult<Value> null(Context& ctx)
    {
        return nom::combinator::value(Value(nullptr), nom::tag("null"))(ctx);
    }

    static auto string(Context& ctx)
    {
        return nom::sequence::preceded(
            nom::character::char_('"'),
            nom::sequence::terminated(&Parser::parse_str, nom::character::char_('"'))
        )(ctx);
    }

    static nom::IResult<Value> array(Context& ctx)
    {
        return nom::sequence::preceded(
            nom::character::char_('[') ,
            nom::sequence::terminated(
                nom::multi::separated_list0(
                    nom::sequence::preceded(&Parser::sp, nom::character::char_(',')),
                    &Parser::json_value),
                nom::sequence::preceded(&Parser::sp, nom::character::char_(']'))
            )
        )(ctx);
    }

    static nom::IResult<Value> key_value(Context& ctx)
    {
        return nom::sequence::separated_pair(
            nom::sequence::preceded(&Parser::sp, &Parser::string),
            nom::sequence::preceded(&Parser::sp, nom::character::char_(':')),
            &Parser::json_value
        )(ctx);
    }

    static nom::IResult<Value> hash(Context& ctx)
    {
        return nom::sequence::preceded(
            nom::character::char_('{'),
            nom::sequence::terminated(
                nom::combinator::map(
                    nom::multi::separated_list0(
                        nom::sequence::preceded(&Parser::sp, nom::character::char_(',')),
                        &Parser::key_value
                    ),
                    [](auto&& tuple_vec) { 
                        cpp::json::object obj;
                        
                        // for (auto&& pairlike : tuple_vec)
                        // {
                        //     obj.emplace(
                        //         std::move(std::get<0>(pairlike)), 
                        //         std::move(std::get<1>(pairlike))
                        //     );
                        // }
                        
                        return Value(obj); 
                    }
                ),
                nom::sequence::preceded(&Parser::sp, nom::character::char_('}'))
            )
        )(ctx);
    }

    static nom::IResult<Value> json_value(Context& ctx)
    {
        return nom::sequence::preceded(
            &Parser::sp,
            nom::branch::alt(
                &Parser::null,
                &Parser::boolean,
                nom::combinator::map(&Parser::string, cpp::json::make),
                nom::combinator::map(nom::number::double_, cpp::json::make),                
                &Parser::array,
                &Parser::hash                
            )
        )(ctx);
    }

    static nom::IResult<Value> root(Context& ctx)
    {
        return nom::sequence::delimited(
            &Parser::sp,
            nom::branch::alt(
                &Parser::hash,
                &Parser::array
            ),
            &Parser::sp
        )(ctx);
    }

};

auto JsonParser = Parser<std::string_view>();

int main(int argc, char const *argv[])
{
    std::string_view input = R"(
    {
        "name": "John Doe",
        "age": 30,
        "is_student": false,
        "courses": ["Math", "Science", "History"],
        "address": {
            "street": "123 Main St",
            "city": "Anytown",
            "zip": "12345"
        },
        "null_value": null
    }
    )";

    auto result = JsonParser.root(input);

    std::println("Rest: '{}'", input);

    return 0;
}
