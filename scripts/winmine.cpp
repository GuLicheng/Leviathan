#include <iostream>
#include <format>
#include <span>
#include <random>
#include <vector>
#include <algorithm>
#include <ranges>
#include <leviathan/extc++/all.hpp>

enum class State
{
    Blank = 0,
    Digit,  // 1-8
    Mine = 9,
};

struct IndexWriterInterator 
{
    std::vector<State>* ptr;

    IndexWriterInterator(std::vector<State>& v) : ptr(std::addressof(v)) {}

    void operator()(size_t index)
    {
        ptr->at(index) = State::Mine;
    }
};

class Winmine
{
    std::vector<State> field;
    int width;
    int height;

public:

    Winmine(int width, int height, int mines) : width(width), height(height)
    {
        if (width <= 0 || height <= 0 || mines < 0 || mines > width * height)
            throw std::invalid_argument("Invalid dimensions or number of mines");

        const auto size = width * height;
        field.resize(size, State::Blank);
        auto indices = std::views::iota(0, size) | std::ranges::to<std::vector>();
        auto writer = leviathan::function_output_iterator<IndexWriterInterator>(field);
        std::ranges::sample(indices, writer, mines, std::mt19937(std::random_device()()));
        FillNumbers();
        DrawMap();
    }

    inline static int dx[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    inline static int dy[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };

    void FillNumbers()
    {
        auto XAsis = std::views::iota(0, height);
        auto YAsis = std::views::iota(0, width);

        auto CalculateState = [this](auto axis) {

            auto [axis_x, axis_y] = axis;
            auto index = axis_x * width + axis_y;

            if (field[index] == State::Mine)
            {
                return State::Mine;
            }

            auto fn = [=, this](int x, int y)
            {
                auto xx = x + axis_x;
                auto yy = y + axis_y;
                return xx >= 0 && yy >= 0 && xx < height && yy < width && field[xx * width + yy] == State::Mine;
            };

            return static_cast<State>(
                std::ranges::count(std::views::zip_transform(fn, dx, dy), true)
            );
        };

        std::ranges::transform(std::views::cartesian_product(XAsis, YAsis), field.begin(), CalculateState);
    }

    void DrawMap() const
    {
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                const auto index = i * width + j;
                switch (field[index])
                {
                case State::Blank:
                    std::cout << ". ";
                    break;
                case State::Mine:
                    std::cout << "* ";
                    break;
                default:
                    // std::cout << "0 ";
                    std::cout << std::format("{} ", static_cast<int>(field[index]));
                    break;
                }
            }
            std::cout << '\n';
        }
    }

};

int main(int argc, char const *argv[])
{
    Winmine(10, 10, 10);
    return 0;
}

