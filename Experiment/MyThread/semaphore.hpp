#pragma once

#include <semaphore.h>
#include <atomic>
#include "mutex.hpp"

struct Semaphore
{
    using clock_type = std::chrono::system_clock;
    static constexpr std::ptrdiff_t max_sem = SEM_VALUE_MAX;

    NOCOPYABLE(Semaphore)

    explicit Semaphore(std::ptrdiff_t count) noexcept
    {
        ::sem_init(&m_sem, 0, count);
    }

    ~Semaphore() { ::sem_destroy(&m_sem); }

    void acquire() noexcept
    {
        while (1) 
        {
            auto err = ::sem_wait(&m_sem);
            if (err && (errno == EINTR)) 
                continue;
            else if (err)
                std::terminate();
            else 
                break;
        }
    }

    bool try_acquire() noexcept
    {
        while (1) 
        {
            auto err = ::sem_wait(&m_sem);
            if (err && (errno == EINTR)) 
                return false;
            else if (err)
                std::terminate();
            else 
                break;
        }
        return true;
    }

    void release(std::ptrdiff_t update) noexcept
    {
        for (; update; --update)
        {
            auto err = ::sem_post(&m_sem);
            if (err)
                std::terminate();
        }
    }

    bool try_acquire_until_impl(const std::chrono::time_point<clock_type>& atime) noexcept
    {
        auto ts = ChronoToCTime(atime);
        while (1)
        {
            if (auto err = ::sem_timedwait(&m_sem, &ts))
            {
                if (errno == EINTR)
                    continue;
                else if (errno == ETIMEDOUT || errno == EINVAL)
                    return false;
                else
                    std::terminate();
            }
            else
                break;
        }
        return true;
    }

    template <typename ClockType, typename DurationType>
    bool try_acquire_until(const std::chrono::time_point<ClockType, DurationType>& atime) noexcept
    {
        if constexpr (std::is_same_v<clock_type, ClockType>)
        {
            return try_acquire_until_impl(atime);
        }
        else 
        {
            const auto c_entry = ClockType::now();
            const auto s_entry = clock_type::now();
            const auto delta = atime - c_entry;
            const auto s_atime = s_entry + delta;
            if (try_acquire_until_impl(s_atime))
                return true;
            // We got a timeout when measured against ClockType but
            // we need to check against the caller-supplied clock
            // to tell whether we should return a timeout.
            return ClockType::now() < atime;
        }
    }

    template <typename Rep, typename Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& rtime) noexcept
    { return try_acquire_until(clock_type::now() + rtime); }

    ::sem_t m_sem;
};


template <std::ptrdiff_t Count = INT_MAX>
class CountingSemaphore
{
    static_assert(Count >= 0);
public:

    NOCOPYABLE(CountingSemaphore)

    explicit CountingSemaphore(std::ptrdiff_t desired) noexcept
        : m_semaphore{ desired }
    {
    }

    ~CountingSemaphore() = default;

    static constexpr auto max() noexcept
    { return Count; }

    void release(std::ptrdiff_t update = 1) noexcept(noexcept(m_semaphore.release(update)))
    { m_semaphore.release(update); }

    void acquire() noexcept(noexcept(m_semaphore.acquire()))
    { m_semaphore.acquire(); }

    bool try_acquire() noexcept(noexcept(m_semaphore.try_acquire()))
    { return m_semaphore.try_acquire(); }

    template <typename Rep, typename Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& rtime) noexcept
    { return m_semaphore.try_acquire_for(rtime); }

    template <typename ClockType, typename DurationType>
    bool try_acquire_until(const std::chrono::time_point<ClockType, DurationType>& atime) noexcept
    { return m_semaphore.try_acquire_until(atime); }

private:
    Semaphore m_semaphore;
};

using BinarySemaphore = CountingSemaphore<1>;