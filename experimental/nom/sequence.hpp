

// Gets an object from the first parser, then gets another object from the second parser.
inline constexpr struct 
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1&& f1, F2&& f2)
    {
        auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
        {
            using R1 = typename std::invoke_result_t<std::decay_t<F1>, ParseContext&>::value_type;
            using R2 = typename std::invoke_result_t<std::decay_t<F2>, ParseContext&>::value_type;
            using R = IResult<std::pair<R1, R2>>;

            auto [first, second] = (FunctionTuple&&)fns;

            auto result1 = first(ctx);

            if (!result1)
            {
                return R(std::unexpect, std::move(result1.error()));
            }

            auto result2 = second(ctx);

            if (!result2)
            {
                return R(std::unexpect, std::move(result2.error()));
            }

            return R(std::in_place, std::move(*result1), std::move(*result2));
        };

        return make_parser_binder(fn, (F1&&)f1, (F2&&)f2);
    }
} pair;

// Matches an object from the first parser and discards it, 
// then gets an object from the second parser.
inline constexpr struct 
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1&& f1, F2&& f2)
    {
        // return Preceded<std::decay_t<F1>, std::decay_t<F2>>((F1&&)f1, (F2&&)f2);
        auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
        {
            using R2 = std::invoke_result_t<std::decay_t<F2>, ParseContext&>;

            auto [first, second] = (FunctionTuple&&)fns;

            if (auto result1 = first(ctx); !result1)
            {
                return R2(std::unexpect, std::move(result1.error()));
            }

            auto result2 = second(ctx);

            if (!result2)
            {
                return R2(std::unexpect, std::move(result2.error()));
            }

            return R2(std::in_place, std::move(*result2));
        };

        return make_parser_binder(fn, (F1&&)f1, (F2&&)f2);
    }
} preceded;

// Gets an object from the first parser, then matches an object 
// from the sep_parser and discards it, then gets another object 
// from the second parser.
inline constexpr struct 
{
    template <typename First, typename Sep, typename Second>
    static constexpr auto operator()(First&& first, Sep&& sep, Second&& second)
    {
        return pair(
            (First&&)first, 
            preceded((Sep&&)sep, (Second&&)second)
        );
    }
} separated_pair;

// Gets an object from the first parser, then matches 
// an object from the second parser and discards it.
inline constexpr struct 
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1&& f1, F2&& f2)
    {
        auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
        {
            using First = std::decay_t<F1>;
            using R1 = std::invoke_result_t<First, ParseContext&>;

            auto [first, second] = (FunctionTuple&&)fns;

            auto result1 = first(ctx);

            if (!result1)
            {
                return R1(std::unexpect, std::move(result1.error()));
            }

            if (auto result2 = second(ctx); !result2)
            {
                return R1(std::unexpect, std::move(result2.error()));
            }

            return R1(std::in_place, std::move(*result1));
        };

        return make_parser_binder(fn, (F1&&)f1, (F2&&)f2);
    }
} terminated;

// Matches an object from the first parser and discards it, 
// then gets an object from the second parser, and finally matches 
// an object from the third parser and discards it.
inline constexpr struct 
{
    template <typename First, typename Second, typename Third>
    static constexpr auto operator()(First&& first, Second&& second, Third&& third)
    {
        return preceded((First&&)first, terminated((Second&&)second, (Third&&)third));
    }
} delimited;

} // namespace nom

