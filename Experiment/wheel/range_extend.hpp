#pragma once

/*
template <typename F>
class closure : public std::ranges::range_adaptor_closure<closure<F>> {
    F f;
public:
    constexpr closure(F f) : f(f) { }

    template <std::ranges::viewable_range R>
        requires std::invocable<F const&, R>
    constexpr operator()(R&& r) const {
        return f(std::forward<R>(r));
    }
};

template <typename F>
class adaptor {
    F f;
public:
    constexpr adaptor(F f) : f(f) { }

    template <typename... Args>
    constexpr operator()(Args&&... args) const {
        if constexpr (std::invocable<F const&, Args...>) {
            return f(std::forward<Args>(args)...);
        } else {
            return closure(std::bind_back(f, std::forward<Args>(args)...));
        }
    }
};
*/