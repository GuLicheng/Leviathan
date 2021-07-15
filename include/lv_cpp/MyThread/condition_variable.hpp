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
        this->m_cond.wait_until(lock2, atime);
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


#ifdef __cpp_lib_atomic_wait
#include <atomic>
#include <chrono>
#ifndef USE_SEM_CriticalSection_SpinCount
#define USE_SEM_CriticalSection_SpinCount 100
#endif

template <::ptrdiff_t MaxCount = USE_SEM_CriticalSection_SpinCount>
class CountingSemaphore
{
public:

    using native_type = sem_t;
    using native_handle_type = native_handle_type*;

    static constexpr ptrdiff_t max() noexcept
    {
        return MaxCount;
    }

    constexpr explicit CountingSemaphore(ptrdiff_t desired) = default;

    ~CountingSemaphore() = default;

    //NOCOPYABLE(CountingSemaphore)

    void release(ptrdiff_t update = 1)
    {

    }
    // TODO: ...
    void acquire();
    bool try_acquire() noexcept;
    template <class Rep, class Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time);
    template <class Clock, class Duration>
    bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time);

private:

    void check_range(ptrdiff_t i)
    {
        assert(i > 0 && i <= MaxCount);
    }

    std::atomic<::ptrdiff_t> m_counter;
};
#endif

#include <semaphore.h>
struct __platform_semaphore
{
    using __clock_t = std::chrono::system_clock;
#ifdef SEM_VALUE_MAX
    static constexpr ptrdiff_t _S_max = SEM_VALUE_MAX;
#else
    static constexpr ptrdiff_t _S_max = _POSIX_SEM_VALUE_MAX;
#endif

    explicit __platform_semaphore(ptrdiff_t __count) noexcept
    {
        sem_init(&_M_semaphore, 0, __count);
    }

    __platform_semaphore(const __platform_semaphore &) = delete;
    __platform_semaphore &operator=(const __platform_semaphore &) = delete;

    ~__platform_semaphore()
    {
        sem_destroy(&_M_semaphore);
    }

    _GLIBCXX_ALWAYS_INLINE void
    _M_acquire() noexcept
    {
        for (;;)
        {
            auto __err = sem_wait(&_M_semaphore);
            if (__err && (errno == EINTR))
                continue;
            else if (__err)
                std::terminate();
            else
                break;
        }
    }

    _GLIBCXX_ALWAYS_INLINE bool
    _M_try_acquire() noexcept
    {
        for (;;)
        {
            auto __err = sem_trywait(&_M_semaphore);
            if (__err && (errno == EINTR))
                continue;
            else if (__err && (errno == EAGAIN))
                return false;
            else if (__err)
                std::terminate();
            else
                break;
        }
        return true;
    }

    _GLIBCXX_ALWAYS_INLINE void
    _M_release(std::ptrdiff_t __update) noexcept
    {
        for (; __update != 0; --__update)
        {
            auto __err = sem_post(&_M_semaphore);
            if (__err)
                std::terminate();
        }
    }

    bool
    _M_try_acquire_until_impl(const std::chrono::time_point<__clock_t> &__atime) noexcept
    {

        auto __ts = ChronoToCTime(__atime);

        for (;;)
        {
            if (auto __err = sem_timedwait(&_M_semaphore, &__ts))
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

    template <typename _Clock, typename _Duration>
    bool
    _M_try_acquire_until(const std::chrono::time_point<_Clock,
                                                  _Duration> &__atime) noexcept
    {
        if constexpr (std::is_same_v<__clock_t, _Clock>)
        {
            return _M_try_acquire_until_impl(__atime);
        }
        else
        {
            const typename _Clock::time_point __c_entry = _Clock::now();
            const auto __s_entry = __clock_t::now();
            const auto __delta = __atime - __c_entry;
            const auto __s_atime = __s_entry + __delta;
            if (_M_try_acquire_until_impl(__s_atime))
                return true;

            // We got a timeout when measured against __clock_t but
            // we need to check against the caller-supplied clock
            // to tell whether we should return a timeout.
            return (_Clock::now() < __atime);
        }
    }

    template <typename _Rep, typename _Period>
    _GLIBCXX_ALWAYS_INLINE bool
    _M_try_acquire_for(const std::chrono::duration<_Rep, _Period> &__rtime) noexcept
    {
        return _M_try_acquire_until(__clock_t::now() + __rtime);
    }

private:
    sem_t _M_semaphore;
};
