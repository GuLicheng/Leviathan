#include "intrusive_list.hpp"
#include <leviathan/extc++/meta.hpp>
#include <catch2/catch_all.hpp>

struct Direction
{
    std::string direction;

    [[=cpp::refl::member_tag("north-south")]]
    cpp::collections::intrusive::list_member_hook NS;

    [[=cpp::refl::member_tag("west-east")]]
    cpp::collections::intrusive::list_member_hook WE;

    Direction(std::string dir)
        : direction(std::move(dir))
    {
        NS.reset();
        WE.reset();
    }
};

TEST_CASE("Intrusive list basic operations", "[intrusive_list]") 
{
    auto ns = cpp::collections::intrusive::list<cpp::refl::search_member_tag(^^Direction, "north-south")>();
    auto we = cpp::collections::intrusive::list<cpp::refl::search_member_tag(^^Direction, "west-east")>();

    Direction north{"north"};
    Direction south{"south"};
    Direction west{"west"};
    Direction east{"east"};
    Direction middle{"middle"};

    ns.push_back(north);
    ns.push_back(middle);
    ns.push_back(south);

    we.push_back(west);
    we.push_back(middle);
    we.push_back(east);

    auto vec1 = ns | std::views::transform(&Direction::direction) | std::ranges::to<std::vector>();
    auto vec2 = we | std::views::transform(&Direction::direction) | std::ranges::to<std::vector>();

    CHECK(vec1 == std::vector<std::string>{"north", "middle", "south"});
    CHECK(vec2 == std::vector<std::string>{"west", "middle", "east"});
}