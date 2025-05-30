#pragma once

#include <random>

namespace cpp::random
{

inline std::random_device global_random_device; // Global random device for seeding

template <typename Distribution, typename Engine>
class radom_generator
{

public:

    using distribution_type = Distribution; // Type alias for the distribution
    using engine_type = Engine; // Type alias for the random number generator engine

    radom_generator(Distribution distribution, Engine engine) 
        : m_distribution(std::move(distribution)), m_engine(std::move(engine)) { } 

    auto operator()() 
    {
        return m_distribution(m_engine); // Generate a random number using the distribution and engine
    }

    void seed(unsigned int s) 
    {
        m_engine.seed(s); // Seed the random number generator engine
    }

private:

    Distribution m_distribution; // Distribution to generate numbers
    Engine m_engine; // Random number generator engine
};

template <std::floating_point T>
class normal_distribution : public radom_generator<std::normal_distribution<T>, std::mt19937>
{
    using base = radom_generator<std::normal_distribution<T>, std::mt19937>;

public:

    using typename base::distribution_type; // Type alias for the distribution
    using typename base::engine_type; // Type alias for the random number generator engine

    normal_distribution(T mean = T(0.0), T stddev = (1.0))
        : base(distribution_type(mean, stddev), engine_type(global_random_device())) { }
};

template <std::integral T>
class uniform_int_distribution : public radom_generator<std::uniform_int_distribution<T>, std::mt19937>
{
    using base = radom_generator<std::uniform_int_distribution<T>, std::mt19937>;

public:

    using typename base::distribution_type; // Type alias for the distribution
    using typename base::engine_type; // Type alias for the random number generator engine

    uniform_int_distribution(
        T min_value = std::numeric_limits<T>::min(), 
        T max_value = std::numeric_limits<T>::max()
    ) : base(distribution_type(min_value, max_value), engine_type(global_random_device())) { }
};

}
