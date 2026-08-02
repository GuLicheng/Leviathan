#include <bits/stdc++.h>

template <size_t Radix>
struct Decoder
{
    using result_type = std::vector<std::byte>;

    // 0-1
    static constexpr result_type operator()(std::string bin) requires (Radix == 2)
    {
        auto decode_trunk = [](auto chunk) static
        { 
            auto res =  std::ranges::fold_left(chunk, 0, [](int acc, char c) 
            {
                return (acc << 1) | (c - '0');
            }); 

            return std::byte(res);
        };

        return bin 
             | std::views::chunk(4)
             | std::views::transform(decode_trunk)
             | std::ranges::to<result_type>();
    }

    // 0-9, A-F, a-f
    static constexpr result_type operator()(std::string hex) requires (Radix == 16)     
    {
        auto decode_trunk = [](auto chunk) static
        { 
            auto res =  std::ranges::fold_left(chunk, 0, [](int acc, char c) 
            {
                return (acc << 4) | (std::isdigit(c) ? (c - '0') : (std::tolower(c) - 'a' + 10));
            }); 

            return std::byte(res);
        };

        return hex 
             | std::views::chunk(2)
             | std::views::transform(decode_trunk)
             | std::ranges::to<result_type>();
    }

};

template <>
struct std::formatter<std::byte> 
{
    auto format(std::byte b, auto& ctx) const
    {
        return std::format_to(ctx.out(), "{:02x}", std::to_integer<int>(b));
    }

    static constexpr auto parse(auto& ctx) { return ctx.begin(); }
};

void print_bytes(const std::vector<std::byte>& bytes)
{
    for (const auto& b : bytes) {
        std::print("{:02x}", std::to_integer<int>(b));
    }
}

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <binary_string> <hex_string>" << std::endl;
        return 1;
    }

    std::string code = argv[1];

    int radix = 0;

    if (argc >= 3) radix = std::stoi(argv[2]);

    if (radix == 2) {
        print_bytes(Decoder<2>::operator()(code));      
    } else if (radix == 16) {
        print_bytes(Decoder<16>::operator()(code));
    } else {
        std::cerr << "Unsupported radix: " << radix << std::endl;
        return 1;
    }
}

