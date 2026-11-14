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

struct Disposer
{
    inline static int Counter = 0;

    static void operator()(auto node)
    {
        ++Counter;
        node->unlink();
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

    CHECK(ns.size() == 3);
    CHECK(we.size() == 3);
    CHECK(!ns.empty());
    CHECK(!we.empty());

    CHECK(ns.front().direction == "north");
    CHECK(ns.back().direction == "south");
    CHECK(we.front().direction == "east");
    CHECK(we.back().direction == "west");

    auto r1 = ns | std::views::transform(&Direction::direction);
    auto r2 = we | std::views::transform(&Direction::direction);

    CHECK(std::ranges::equal(r1, std::vector<std::string>{"north", "middle", "south"}));
    CHECK(std::ranges::equal(r2, std::vector<std::string>{"east", "middle", "west"}));

    ns.pop_back();
    ns.pop_front();

    CHECK(ns.size() == 1);
    CHECK(ns.front().direction == "middle");

    we.pop_back_and_dispose(Disposer{});
    we.pop_front_and_dispose(Disposer{});

    CHECK(Disposer::Counter == 2);
    CHECK(we.size() == 1);
    CHECK(we.front().direction == "middle");

    we.clear_and_dispose(Disposer{});
    CHECK(Disposer::Counter == 3);
    CHECK(we.size() == 0);
}

struct DirectionComparator
{
    static bool compare(const Direction& a, const Direction& b)
    {
        return a.direction < b.direction;
    }
};

TEST_CASE("Intrusive list sort operation", "[intrusive_list]") 
{
    auto ns = cpp::collections::intrusive::list<cpp::refl::search_tag(^^Direction, "north-south")>();

    Direction north{"north"};
    Direction south{"south"};
    Direction middle{"middle"};

    ns.push_back(north);
    ns.push_back(middle);
    ns.push_back(south);

    ns.sort(DirectionComparator::compare);

    auto r = ns | std::views::transform(&Direction::direction);
    CHECK(std::ranges::equal(r, std::vector<std::string>{"middle", "north", "south"}));
}
