#include <iostream>
#include <format>
#include <span>
#include <random>
#include <vector>
#include <algorithm>
#include <ranges>
#include <leviathan/utils/iterator.hpp>

enum class State
{
    Blank = 0,
    Digit,  // 1-8
    Mine = 9,
};

struct IndexWriterInterator : leviathan::output_iterator_interface<IndexWriterInterator>
{
    std::vector<State>* ptr;

    IndexWriterInterator(std::vector<State>& v) : ptr(std::addressof(v)) {}

    IndexWriterInterator& operator=(size_t index)
    {
        if (index >= ptr->size())
            throw std::out_of_range("Index out of range");
        (*ptr)[index] = State::Mine;
        return *this;
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
        std::ranges::sample(indices, IndexWriterInterator(field), mines, std::mt19937{ std::random_device()() });
        FillNumbers();
        DrawMap();
    }

    inline static int dx[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    inline static int dy[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };

    void FillNumbers()
    {
        auto XAsis = std::views::iota(0, height);
        auto YAsis = std::views::iota(0, width);

        auto CalculateState = [this](auto xy) {

            auto [x, y] = xy;
            auto index = x * width + y;

            // if (field[index] == State::Mine)
            // {
            //     return State::Mine;
            // }

            auto AsXY = [=](int k) { return std::make_pair(x + dx[k], y + dy[k]); };
            auto IsMine = [this](auto p) 
            { 
                return static_cast<size_t>(p.first) < static_cast<size_t>(height) 
                    && static_cast<size_t>(p.second) < static_cast<size_t>(width)  
                    && field[p.first * width + p.second] == State::Mine; 
            };

            return field[index] == State::Mine 
                 ? State::Mine 
                 : static_cast<State>(std::ranges::size(std::views::iota(0, 8) | std::views::transform(AsXY) | std::views::filter(IsMine)));

            // int count = 0;
            // for (int k = 0; k < 8; ++k)
            // {
            //     const int ni = x + dx[k];
            //     const int nj = y + dy[k];
            //     if (ni >= 0 && ni < height && nj >= 0 && nj < width && field[ni * width + nj] == State::Mine)
            //         ++count;
            // }
            // return static_cast<State>(count);
        };

        
        auto states = std::views::cartesian_product(XAsis, YAsis)
                    | std::views::transform(CalculateState)
                    | std::ranges::to<std::vector>();

        std::ranges::copy(states, field.begin());
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

