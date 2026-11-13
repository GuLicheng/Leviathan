#include "intrusive_list.hpp"
#include <iostream>
#include <print>
#include <leviathan/extc++/meta.hpp>
#include <catch2/catch_all.hpp>

struct Direction
{
    std::string direction;

    [[=cpp::refl::tag("north-south")]]
    cpp::collections::intrusive::list_member_hook NS;

    [[=cpp::refl::tag("west-east")]]
    cpp::collections::intrusive::list_member_hook WE;

    Direction(const Direction& other) = delete;

    Direction(std::string dir)
        : direction(std::move(dir))
    {
        NS.reset();
        WE.reset();
    }

    void Show() const
    {
        std::cout << "(" << direction << ")" << std::endl;
    }
};

TEST_CASE("Intrusive list basic operations", "[intrusive_list]") 
{
    auto ns = cpp::collections::intrusive::list<cpp::refl::search_tag(^^Direction, "north-south")>();
    auto we = cpp::collections::intrusive::list<cpp::refl::search_tag(^^Direction, "west-east")>();

    Direction north{"north"};
    Direction south{"south"};
    Direction west{"west"};
    Direction east{"east"};
    Direction middle{"middle"};

    ns.push_back(north);
    ns.push_back(middle);
    ns.push_back(south);

    we.push_front(west);
    we.push_front(middle);
    we.push_front(east);

    auto r1 = ns | std::views::transform(&Direction::direction);
    auto r2 = we | std::views::transform(&Direction::direction);

    CHECK(std::ranges::equal(r1, std::vector<std::string>{"north", "middle", "south"}));
    CHECK(std::ranges::equal(r2, std::vector<std::string>{"east", "middle", "west"}));

}