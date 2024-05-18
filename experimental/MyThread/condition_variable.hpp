#pragma once

#include "mutex.hpp"
#include "lock.hpp"

#include <stddef.h>
#include <assert.h>

#include <chrono>
#include <memory>

enum class CVStatus { no_timeout, timeout };

class ConditionVariable
{
    using steady_clock = std::chrono::steady_clock;
    using system_clock = std::chrono::system_clock;
    using clock_type = steady_clock; //  pthread -> steady otherwise system

public:
    using native_type = pthread_cond_t;
    using native_handle_type = native_type*;

    ConditionVariable(native_type code = PTHREAD_COND_INITIALIZER) noexcept
    { this->m_cond = code; }

    ~ConditionVariable() noexcept = default;

    NOCOPYABLE(ConditionVariable)

    native_handle_type native_handle() noexcept
    { return std::addressof(this->m_cond); }

    // https://blog.csdn.net/claroja/article/details/89152507
    // https://en.cppreference.com/w/c/thread
    void notify_one() noexcept
    { pthread_cond_signal(native_handle()); }

    void notify_all() noexcept
    { pthread_cond_broadcast(native_handle()); }

    void wait(UniqueLock<Mutex>& lock) noexcept
    { pthread_cond_wait(native_handle(), lock.mutex()->native_handle()); }

    template <typename Predicate>
    void wait(UniqueLock<Mutex>& lock, Predicate p)
    {
        while (!p())
            wait(lock);
    }

    template <typename DurationType>
    CVStatus wait_until(UniqueLock<Mutex>& lock, const std::chrono::time_point<clock_type, DurationType>& atime)
    { return wait_until_impl(lock, atime); }

    template <typename ClockType, typename DurationType>
    CVStatus wait_until(UniqueLock<Mutex>& lock, const std::chrono::time_point<ClockType, DurationType>& atime) 
    { 
        // transfer ClockType -> clock_type
        static_assert(std::chrono::is_clock_v<ClockType>);
        const typename ClockType::time_point c_entry = ClockType::now();
        const clock_type::time_point s_entry = clock_type::now();
        const auto delta = atime - c_entry;
        const auto s_atime = s_entry + delta;
        if (wait_until_impl(lock, s_atime) == CVStatus::no_timeout)
            return CVStatus::no_timeout;
        // we got a timeout when measured against clock_type 
        // but we need to check against the caller-supplied clock
        // to tell whether we should return a timeout
        return ClockType::now() < atime ? CVStatus::no_timeout : CVStatus::timeout;
    }

    template <typename ClockType, typename DurationType, typename Predicate>
    bool wait_until(UniqueLock<Mutex>& lock, const std::chrono::time_point<ClockType, DurationType>& atime, Predicate p)
    {
        while (!p())
            if (wait_until(lock, atime) == CVStatus::timeout)
                return p();
        return true;
    }

    template <typename RepType, typename PeriodType>
    CVStatus wait_for(UniqueLock<Mutex>& lock, const std::chrono::duration<RepType, PeriodType>& rtime)
    {
        using dur = typename steady_clock::duration;
        auto reltime = std::chrono::duration_cast<dur>(rtime);
        if (reltime < rtime)
            ++reltime;
        return wait_until(lock, steady_clock::now() + reltime);
    }

    template <typename RepType, typename PeriodType, typename Predicate>
    bool wait_for(UniqueLock<Mutex>& lock, const std::chrono::duration<RepType, PeriodType>& rtime, Predicate p)
    {
        using dur = typename steady_clock::duration;
        auto reltime = std::chrono::duration_cast<dur>(rtime);
        if (reltime < rtime)
            ++reltime;
        return wait_until(lock, steady_clock::now() + reltime, std::move(p)); 
    }
private:
    template<typename DurationType>
    CVStatus wait_until_impl(UniqueLock<Mutex>& lock,
            const std::chrono::time_point<clock_type, DurationType>& atime)
    {
        auto ts = ChronoToCTime(atime);
        pthread_cond_timedwait(native_handle(), lock.mutex()->native_handle(), &ts);
        return clock_type::now() < atime ? CVStatus::no_timeout : CVStatus::timeout;
    }
    native_type m_cond;
};

// FIXME
class ConditionVariableAny
{
    using steady_clock = std::chrono::steady_clock;
    using system_clock = std::chrono::system_clock;
    using clock_type = steady_clock; //  pthread -> steady otherwise system
    using native_type = pthread_cond_t;  
    ConditionVariable m_cond;
    std::shared_ptr<Mutex> m_mutex;

    template <typename LockType>
    struct UnLockGuard
    {
        explicit UnLockGuard(LockType& lk)
            : m_lock(lk)
        { m_lock.unlock(); }

        ~UnLockGuard() noexcept
        {
            // relock mutex or terminate()
            // condition_variable_any wait functions are required to terminate if
            // the mutex cannot be relocked;
            // we slam into noexcept here for easier user debugging.
            m_lock.lock();
        }

        NOCOPYABLE(UnLockGuard)

        LockType& m_lock;
    };

public:

    ConditionVariableAny() : m_mutex(std::make_shared<Mutex>()) { }
    ~ConditionVariableAny() = default;

    NOCOPYABLE(ConditionVariableAny)

    void notify_one() noexcept
    {
        LockGuard<Mutex> lock(*m_mutex);
        m_cond.notify_one();
    }

    void notify_all() noexcept
    {
        LockGuard<Mutex> lock(*m_mutex);
        m_cond.notify_all();
    }

    template <typename LockType>
    void wait(LockType& lock) 
    { 
        std::shared_ptr<Mutex> ptr = this->m_mutex;
        UniqueLock<Mutex> guard(*ptr);
        UnLockGuard<LockType> unlocker(lock);
        UniqueLock<Mutex> lock2(std::move(guard));
        this->m_cond.wait(lock2);
    }

    template <typename LockType, typename Predicate>
    void wait(LockType& lock, Predicate p) 
    {
        while (!p())
        {
            wait(lock);
        }
    }

    template <typename LockType, typename ClockType, typename DurationType>
    CVStatus wait_until(LockType& lock, const std::chrono::time_point<ClockType, DurationType>& atime)
    { 
        std::shared_ptr<Mutex> ptr = this->m_mutex;
        UniqueLock<Mutex> guard(*ptr);
        UnLockGuard<LockType> unlocker(lock);
        UniqueLock<Mutex> lock2(std::move(guard));
        return this->m_cond.wait_until(lock2, atime);
    }

    template <typename LockType, typename ClockType, typename DurationType, typename Predicate>
    bool wait_until(LockType& lock, const std::chrono::time_point<ClockType, DurationType>& atime, Predicate p)
    {
        while (!p())
            if (wait_until(lock, atime) == CVStatus::timeout)
                return p();
        return true;
    }

    template <typename LockType, typename RepType, typename PeriodType>
    CVStatus wait_for(LockType& lock, const std::chrono::duration<RepType, PeriodType>& rtime)
    { return wait_until(lock, clock_type::now() + rtime); }

    template <typename LockType, typename RepType, typename PeriodType, typename Predicate>
    bool wait_for(LockType& lock, const std::chrono::duration<RepType, PeriodType>& rtime, Predicate p)
    { return wait_until(lock, clock_type::now() + rtime, std::move(p)); }

private:

    // TODO: x 4
    // template <typename LockType, typename Predicate>
    // bool wait(LockType& lock, std::stop_token, Predicate p) { ... }

};




