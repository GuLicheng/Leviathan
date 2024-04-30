// #include <leviathan/algorithm/tim_sort_range.hpp>
// #include <catch2/catch_all.hpp>

// #include <vector>
// #include <random>

// struct RandomGenerator
// {
//     static std::vector<int> random_int()
//     {
//         static std::random_device rd;
//         std::vector<int> vec;
//         for (int i = 0; i < 1'000'000; ++i) 
//         {
//             vec.emplace_back(rd());
//         }
//         return vec;
//     }
// };

// TEST_CASE("tim sort")
// {
//     auto vec1 = RandomGenerator::random_int();
//     auto vec2 = vec1;

//     std::ranges::sort(vec1);
//     leviathan::ranges::tim_sort(vec2);

//     REQUIRE(vec1 == vec2);
// }
