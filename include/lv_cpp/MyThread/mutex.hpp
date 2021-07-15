#pragma once

#include <pthread.h> 
#include <assert.h>

#include <system_error>  // for std::system_error
#include <chrono>  // for time component


// Some macros
#define NOCOPYABLE(classname)  \
    classname(const classname&) = delete; \
    classname & operator=(const classname&) = delete; 

#define DEFAULT_CONSTRACTOR(classname) \
    classname() = default;   \
    ~classname() = default;   \
    NOCOPYABLE(classname)


// some help functions
template <typename DurationType>
::timespec ChronoToCTime(const std::chrono::time_point<std::chrono::system_clock, DurationType>& atime)
{
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(atime);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(atime - seconds);
    return {
        static_cast<::time_t>(seconds.time_since_epoch().count()),
        static_cast<long>(nanoseconds.count())
    };
}

template <typename RepType, typename PeriodType>
auto GetSystemRTime(const std::chrono::duration<RepType, PeriodType>& rtime)
{
    using clock_type = std::chrono::system_clock;
    auto rt = std::chrono::duration_cast<clock_type::duration>(rtime);
    if (std::ratio_greater<clock_type::period, PeriodType>())
        ++rt;
    return rt;
}

class MutexBase
{
public:

    using native_type = pthread_mutex_t;
    using native_handle_type = native_type*;

    native_type* handle() noexcept
    { return std::addressof(this->m_mutex); }

    MutexBase(native_type mode = PTHREAD_MUTEX_INITIALIZER) noexcept
    { this->m_mutex = mode; }

    ~MutexBase() noexcept = default;

    NOCOPYABLE(MutexBase)

    void lock()
    {
        // https://docs.oracle.com/cd/E19253-01/819-7051/sync-238/index.html
        // Return 0 if succeed
        int e = pthread_mutex_lock(handle());
        if (e) 
            throw std::system_error();  
    }

    bool try_lock() noexcept
    { 
        // https://docs.oracle.com/cd/E19253-01/819-7051/sync-240/index.html
        // Return 0 if succeed
        return !pthread_mutex_trylock(handle()); 
    }

    void unlock() 
    { pthread_mutex_unlock(handle()); }

    native_handle_type native_handle() noexcept
    { return this->handle(); }

private:
    native_type m_mutex;
};

class Mutex : public MutexBase { };

struct DeferLock { explicit DeferLock() = default; };
struct TryToLock { explicit TryToLock() = default; };
struct AdoptLock { explicit AdoptLock() = default; };

constexpr DeferLock defer_lock { };
constexpr TryToLock try_to_lock { };
constexpr AdoptLock adopt_lock { };

class RecursiveMutex : public MutexBase 
{
public:

    RecursiveMutex() : MutexBase(PTHREAD_RECURSIVE_MUTEX_INITIALIZER) { }

    ~RecursiveMutex() = default;

    NOCOPYABLE(RecursiveMutex)
};



template <typename Derived>
struct TimeOperation
{
    template <typename RepType, typename PeriodType>
    bool try_lock_for(const std::chrono::duration<RepType, PeriodType>& rtime)
    {
        const auto rt = GetSystemRTime(rtime);
        return this->try_lock_until(std::chrono::system_clock::now() + rt);
    }

    template <typename ClockType, typename DurationType>
    bool try_lock_until(const std::chrono::time_point<ClockType, DurationType>& atime)
    {
        static_assert(std::chrono::is_clock_v<ClockType>);
        if constexpr (std::same_as<ClockType, std::chrono::system_clock>)
            return static_cast<Derived&>(*this).try_lock_until_impl(atime);

        auto now = ClockType::now();
        do
        {
            auto rtime = atime - now;
            if (this->try_lock_for(rtime))
                return true;
            now = ClockType::now();
        } while (atime > now);
        return false;
    }

};

// avoid multiple inheritance
class TimedMutex : public MutexBase, public TimeOperation<TimedMutex>
{
public:
    using MutexBase::MutexBase;

    template <typename DurationType>
    bool try_lock_until_impl(const std::chrono::time_point<std::chrono::system_clock, DurationType>& atime)
    {
        auto ts = ChronoToCTime(atime);
        return !pthread_mutex_timedlock(native_handle(), &ts);
    }

};

class RecursiveTimedMutex : public TimedMutex
{
public:
    RecursiveTimedMutex() : TimedMutex(GENERIC_RECURSIVE_INITIALIZER) { }

    ~RecursiveTimedMutex() = default;

    NOCOPYABLE(RecursiveTimedMutex)

};

class SharedMutex
{
    pthread_rwlock_t m_rwlock;
public:
    using native_handle_type = pthread_rwlock_t*;
    using native_type = pthread_rwlock_t;
    
    SharedMutex(pthread_rwlock_t code = PTHREAD_RWLOCK_INITIALIZER) 
    { this->m_rwlock = code; }

    ~SharedMutex() = default;
    
    NOCOPYABLE(SharedMutex)

    native_handle_type native_handle() noexcept
    { return std::addressof(this->m_rwlock); }

    void lock()
    {
        int ret = pthread_rwlock_wrlock(native_handle());
        if (ret == EDEADLK)
            throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
        // Errors not handled: EINVAL
        assert(ret == 0);
    }

    bool try_lock()
    {
        int ret = pthread_rwlock_trywrlock(native_handle());
        if (ret == EBUSY) return false;
        // Errors not handled: EINVAL
        assert(ret == 0);
        return true;
    }

    void unlock()
    {
        [[maybe_unused]] 
        int ret = pthread_rwlock_unlock(native_handle());
        // Errors not handled: EPERM, EBUSY, EINVAL
        assert(ret == 0);
    }

    void lock_shared()
    {
        int ret;
        do ret = pthread_rwlock_rdlock(native_handle()); while (ret == EAGAIN);
        if (ret == EDEADLK)
            throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
        // Errors not handled: EINVAL
        assert(ret == 0);
    }

    bool try_lock_shared()
    {
        int ret = pthread_rwlock_tryrdlock(native_handle());
        if (ret == EBUSY || ret == EAGAIN) return false;
        // Errors not handled: EINVAL
        assert(ret == 0);
        return true;
    }

    void unlock_shared()
    { unlock(); }

};

template <typename Derived>
struct SharedTimedOperation
{
    template <typename RepType, typename PeriodType>
    bool try_lock_shared_for(const std::chrono::duration<RepType, PeriodType>& rtime)
    {
        const auto rt = GetSystemRTime(rtime);
        return try_lock_shared_until(std::chrono::system_clock::now() + rt);
    }

    template <typename ClockType, typename DurationType>
    bool try_lock_shared_until(const std::chrono::time_point<ClockType, DurationType> &atime)
    {
        static_assert(std::chrono::is_clock_v<ClockType>);
        if constexpr (std::same_as<ClockType, std::chrono::system_clock>)
            return static_cast<Derived&>(*this).try_lock_shared_until_impl(atime);  

        typename ClockType::time_point now = ClockType::now();
        do
        {
            auto rtime = atime - now;
            if (try_lock_shared_for(rtime)) return true;
            now = ClockType::now();
        } while (atime > now);
        return false;
    }
};

class SharedTimedMutex : public SharedMutex, public TimeOperation<SharedTimedMutex>, public SharedTimedOperation<SharedTimedMutex>
{
public:

    DEFAULT_CONSTRACTOR(SharedTimedMutex)

    // Exclusive ownership
    // lock, try_lock, unlock form base class

    template <typename DurationType>
    bool try_lock_until_impl(const std::chrono::time_point<std::chrono::system_clock, DurationType>& atime)
    {
        auto ts = ChronoToCTime(atime);
        int ret = pthread_rwlock_timedwrlock(native_handle(), &ts);
        if (ret == ETIMEDOUT || ret == EDEADLK) return false;
        assert(ret == 0);
        return true;
    }

    template <typename DurationType>
    bool try_lock_shared_until_impl(const std::chrono::time_point<std::chrono::system_clock, DurationType>& atime)
    {
        auto ts = ChronoToCTime(atime);
        int ret;
        do ret = pthread_rwlock_timedrdlock(native_handle(), &ts); while (ret == EAGAIN || ret == EDEADLK);
        if (ret == ETIMEDOUT) return false;
        assert(ret == 0);
        return true;
    }

};


